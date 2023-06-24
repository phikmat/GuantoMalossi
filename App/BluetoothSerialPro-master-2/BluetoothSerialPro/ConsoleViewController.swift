//
//  SerialViewController.swift
//  BluetoothSerialPro
//
//  Created by Alex on 27/11/2016.
//  Copyright © 2016 Hangar42. All rights reserved.
//
//  In order to hide the gradient background of the tableView, we add two white uitableviewcells
//  both with a height equal to that of the entire screen. So we also have to adjust the top/bottom
//  content insets (minus view.frame.bounds)
//
//  Sound tings: 1054 1313
//
//  For some unknown reason the textView only scrolls to the bottom if the last character is a
//  newLine character. Hence the workaround where the newline is added in addLineHeader and removed
//  in appendAttributedText. For this we also need previousLineSent to color the linebreak correctly.
//  Possibly related: https://stackoverflow.com/questions/46457660
//
//  textView contentinset in IB is set to 'never'; here done programatically for backwards-compatibility
//  with iOS 9/10
//
//  In displayOptionsChanged() we add scrollToBottom to the textCopy queue with a 0.5s delay as a workaround
//  for the unexplained erratic scrolling behaviour.
//
import UIKit
import CoreBluetooth
import AVFoundation
import Dispatch

class ConsoleViewController: UIViewController, BluetoothSerialDelegate, UITextFieldDelegate, UITableViewDelegate, UITableViewDataSource, URLSessionTaskDelegate {
    
    // MARK: - Outlets
    
    @IBOutlet weak var textView: UITextView!
    @IBOutlet weak var inputField: UITextField!
    @IBOutlet weak var bottomView: UIView!
    @IBOutlet weak var bottomConstraint: NSLayoutConstraint!
    @IBOutlet weak var sideLineConstraint: NSLayoutConstraint!
    @IBOutlet weak var sideLine: UIView!
    
    
    // MARK: - Variables
    
    var tableView: UITableView!
    var isChatEnabled = false
    
    var messages = [Message]()
    var bytesOnLine = 0
    var lines = 0
    var previousLineSent = false
    
    //var alerts = [DataAlert]()
    //var triggeredAlerts = [DataAlert]()
    var isShowingAlert = false
    
    var displayStyle = DisplayStyle.chatBox
    var displayFormat = String.Format.utf8
    var displaySent = true
    var displayNewlineRule = NewlineAfter.message
    var displayTimeStamp = true
    var displayMicroseconds = false
    var displayLineNumbers = false
    
    var autoScroll = true
    
    var inputFormat = String.Format(rawValue: Settings.inputFormat.value)!
    var inputPostfix = Settings.appendToInput.value
    
    var playSounds = true
    
    let dateFormatter = DateFormatter()
    var defaultAttributes = [NSAttributedStringKey: Any]()
    var sentAttributes = [NSAttributedStringKey: Any]()
    
    let bottomViewHeight: CGFloat = 49
    var topWhiteCellHeight: CGFloat { return UIScreen.main.bounds.height }
    var bottomWhiteCellHeight: CGFloat { return UIScreen.main.bounds.height*2 }
    
    var localIp: String?
    
    
    // MARK: - Multi Threading
    
    let queues = (textCopy:   DispatchQueue(label: "nl.hangar42.bluetoothserialpro.attrtextqueue"),
                  textUpdate: DispatchQueue(label: "nl.hangar42.bluetoothserialpro.textupdatequeue"),
                  isWaiting:  DispatchQueue(label: "nl.hangar42.bluetoothserialpro.iswaitingqueue"))
    
    private var _attributedTextCopy = NSMutableAttributedString()
    private var _isWaitingForUpdate = false

    var isWaitingForUpdate: Bool {
        set {
            queues.isWaiting.async {
                self._isWaitingForUpdate = newValue
            }
        }
        get {
            var val = true
            queues.isWaiting.sync {
                val = self._isWaitingForUpdate
            }
            return val
        }
    }
    
    func appendToAttributedText(_ attr: NSAttributedString, _ preserveLinebreak: Bool) {
        queues.textCopy.async {
            if !preserveLinebreak && self._attributedTextCopy.length > 0 {
                let last = NSRange(location: self._attributedTextCopy.length-1, length: 1)
                self._attributedTextCopy.deleteCharacters(in: last)
            }
            self._attributedTextCopy.append(attr)
            
            guard !self.isWaitingForUpdate else { return }
            self.isWaitingForUpdate = true
            DispatchQueue.main.async {
                self.isWaitingForUpdate = false
                self.textView.attributedText = self.getAttributedText()
                self.scrollToBottom()
            }
        }
    }
    
    func getAttributedText() -> NSAttributedString {
        var attr: NSAttributedString!
        queues.textCopy.sync {
            attr = self._attributedTextCopy.copy() as! NSAttributedString
        }
        return attr
    }
    
    func clearAttributedText() {
        queues.textCopy.async {
            self._attributedTextCopy = NSMutableAttributedString()
        }
    }
    
    
    // MARK: - ViewController
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        serial.delegate = self
        clear()
        
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillShow), name: NSNotification.Name.UIKeyboardWillShow)
        NotificationCenter.default.addObserver(self, selector: #selector(keyboardWillHide), name: NSNotification.Name.UIKeyboardWillHide)
        NotificationCenter.default.addObserver(self, selector: #selector(inputOptionsChanged), name: .inputOptionsChanged)
        NotificationCenter.default.addObserver(self, selector: #selector(displayOptionsChanged), name: .displayOptionsChanged)
        NotificationCenter.default.addObserver(self, selector: #selector(clear), name: .clearScreen)
        NotificationCenter.default.addObserver(self, selector: #selector(serialDidSendData(_:)), name: .didSendData)

        let tap = UITapGestureRecognizer(target: self, action: #selector(dismissKeyboard))
        tap.cancelsTouchesInView = false
        textView.addGestureRecognizer(tap)
        
        textView.textContainerInset.top = (navigationController?.navigationBar.frame.maxY ?? 64) + 5
        textView.textContainerInset.bottom = bottomViewHeight
        textView.scrollsToTop = false // auto scroll to top has buggy behaviour (screws up bottom inset)
        textView.text = ""
        
        let font = UIFont(name: "Menlo-Regular", size: 15)
        let paragraphStyle = NSMutableParagraphStyle()
        defaultAttributes[NSAttributedStringKey.font] = font
        defaultAttributes[NSAttributedStringKey.paragraphStyle] = paragraphStyle
        sentAttributes = defaultAttributes
        sentAttributes[NSAttributedStringKey.backgroundColor] = view.tintColor.withAlphaComponent(0.07)
        
        inputOptionsChanged()
        displayOptionsChanged()
        
        let data = Settings.sendOnConnect.defaultValue
        if !data.isEmpty {
            
            let taggedMessage = "<i id=0000>" + data.string(withFormat: .utf8).removeNewline() + "</i>"
            let dataTagged = taggedMessage.data(withFormat: .utf8)
            
            //serial.sendDataToDevice(dataTagged)
            messages.append(Message(data: data, sent: false, id: "0000", taggedMessage: taggedMessage))
            addText(forMessage: messages.last!)
            scrollToBottom()
            playSentSound()
        }
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
        super.viewWillTransition(to: size, with: coordinator)
        textView.isScrollEnabled = false // cuz bug in iOS
        textView.isScrollEnabled = true
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        textView.textContainerInset.top = (navigationController?.navigationBar.frame.maxY ?? 64) + 5
        
        if isChatEnabled {
            tableView.contentInset.top = (navigationController?.navigationBar.frame.maxY ?? 64) + 5 - topWhiteCellHeight
            tableView.contentInset.bottom = bottomViewHeight + 8 - bottomWhiteCellHeight
        }
    }
    
    @objc func inputOptionsChanged() {
        inputField.text = ""
        serial.maxChunkSize = Settings.maxChunkSize.value
        inputFormat = String.Format(rawValue: Settings.inputFormat.value)!
        inputPostfix = Settings.appendToInput.value
    }
    
    @objc func displayOptionsChanged() {
        
        //
        // settings
        
        displayStyle = DisplayStyle(rawValue: Settings.displayStyle.value)!
        displayFormat = String.Format(rawValue: Settings.displayFormat.value)!
        displaySent = Settings.displaySentMessages.value
        displayNewlineRule = NewlineAfter(id: Settings.messageSeparation.value)
        displayTimeStamp = Settings.displayTimeStamps.value
        displayMicroseconds = Settings.displayMicroseconds.value
        displayLineNumbers = Settings.displayLineNumbers.value
        autoScroll = Settings.autoScroll.value
        playSounds = Settings.shouldPlaySounds.value
        
        
        //
        // date formatter
        
        if displayMicroseconds {
            dateFormatter.dateFormat = "HH:mm:ss.SSS"
        } else {
            dateFormatter.dateFormat = "HH:mm:ss"
        }
        
        
        //
        // calculate indent
        
        var indent: CGFloat = 0
        if displayTimeStamp { indent += 81 }
        if displayMicroseconds { indent += 37 }
        if displayLineNumbers { indent += 45 }
        
        
        //
        // paragraph style
        
        let paragraphStyle = defaultAttributes[NSAttributedStringKey.paragraphStyle] as! NSMutableParagraphStyle
        paragraphStyle.lineBreakMode = displayFormat == .utf8 ? .byCharWrapping : .byWordWrapping
        paragraphStyle.headIndent = indent
        
        
        //
        // side line
        
        sideLineConstraint.constant = indent
        
        if !isChatEnabled && (displayTimeStamp || displayLineNumbers) {
            sideLine.isHidden = false
        } else {
            sideLine.isHidden = true
        }
        
        
        //
        // display style
        
        if !isChatEnabled && displayStyle == .chatBox {
            createChatTableView()
        } else if isChatEnabled && displayStyle == .console {
            removeChatTableView()
        } else if isChatEnabled {
            tableView.reloadData()
        }
        
        
        //
        // reset and reload
        
        clearAttributedText()
        textView.text = ""
        bytesOnLine = 0
        lines = 0
        for m in messages {
            addText(forMessage: m, updateTableView: false)
        }

        queues.textCopy.async {
            delay(seconds: 0.5) {
                DispatchQueue.main.async {
                    self.scrollToBottom()
                }
            }
        }
    }
    
    func createChatTableView() {
        textView.isHidden = true
        sideLine.isHidden = true
        
        tableView = UITableView(frame: textView.frame)
        tableView.delegate = self
        tableView.dataSource = self
        tableView.showsVerticalScrollIndicator = false
        tableView.contentInset.top = (navigationController?.navigationBar.frame.maxY ?? 64) + 5 - topWhiteCellHeight
        tableView.contentInset.bottom = bottomViewHeight + 8 - bottomWhiteCellHeight
        tableView.register(ChatTableViewCell.self, forCellReuseIdentifier: "cell")
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "whiteCell")
        tableView.translatesAutoresizingMaskIntoConstraints = false
        tableView.separatorStyle = .none
        
        if #available(iOS 11.0, *) {
            tableView.contentInsetAdjustmentBehavior = .never
        }
        
        view.insertSubview(tableView, aboveSubview: textView)
        
        let tap = UITapGestureRecognizer(target: self, action: #selector(dismissKeyboard))
        tap.cancelsTouchesInView = false
        tableView.addGestureRecognizer(tap)
        
        let heightConstraint = NSLayoutConstraint(item: tableView, attribute: .height, relatedBy: .equal, toItem: textView, attribute: .height, multiplier: 1.0, constant: 0.0)
        let topConstraint = NSLayoutConstraint(item: tableView, attribute: .top, relatedBy: .equal, toItem: textView, attribute: .top, multiplier: 1.0, constant: 0.0)
        let leftConstraint = NSLayoutConstraint(item: tableView, attribute: .left, relatedBy: .equal, toItem: textView, attribute: .left, multiplier: 1.0, constant: 8)
        let rightConstraint = NSLayoutConstraint(item: tableView, attribute: .right, relatedBy: .equal, toItem: textView, attribute: .right, multiplier: 1.0, constant: -8)
        
        view.addConstraints([heightConstraint, topConstraint, leftConstraint, rightConstraint])
        
        let gradientView = GradientView()
        gradientView.topColor = #colorLiteral(red: 0.3593252877, green: 0.8365120347, blue: 1, alpha: 1)
        gradientView.bottomColor = #colorLiteral(red: 0.06494087851, green: 0.516777352, blue: 0.9410062065, alpha: 1)
        tableView.backgroundView = gradientView
        
        tableView.reloadData()
        isChatEnabled = true
    }
    
    func removeChatTableView() {
        tableView.removeFromSuperview()
        tableView = nil
        isChatEnabled = false
        textView.isHidden = false
        sideLine.isHidden = false
    }
    
    
    // MARK: - Keyboard
    
    @objc func keyboardWillShow(_ notification: Notification) {
        var info = (notification as NSNotification).userInfo!
        let value = info[UIKeyboardFrameEndUserInfoKey] as! NSValue
        let keyboardFrame = value.cgRectValue
        
        //TODO: Not animating properly
        UIView.animate(withDuration: 1, delay: 0, options: .curveLinear, animations: {
            self.bottomConstraint.constant = keyboardFrame.size.height
        }, completion: { Bool -> Void in
            self.textView.textContainerInset.bottom = keyboardFrame.size.height + self.bottomViewHeight + 8
            if self.isChatEnabled {
                self.tableView.contentInset.bottom = keyboardFrame.size.height + self.bottomViewHeight + 8  - self.bottomWhiteCellHeight
            }
            self.scrollToBottom()
        })
    }
    
    @objc func keyboardWillHide(_ notification: Notification) {
        UIView.animate(withDuration: 1, delay: 0, options: .curveLinear, animations: {
            self.bottomConstraint.constant = 30
        }, completion: { Bool -> Void in
            self.textView.textContainerInset.bottom = self.bottomViewHeight + 8
            if self.isChatEnabled {
                self.tableView.contentInset.bottom = self.bottomViewHeight + 8 - self.bottomWhiteCellHeight
            }
        })
        
    }
    
    @objc func dismissKeyboard() {
        if inputField.isEditing {
            inputField.resignFirstResponder()
        }
    }
    
    @IBAction func sendMessage(_ sender: UIButton?) {
        _ = textFieldShouldReturn(inputField)
    }
    
    
    // MARK: - TextField
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        var data = textField.text!.data(withFormat: inputFormat)
        if !data.isEmpty || !inputPostfix.isEmpty {
            if !inputPostfix.isEmpty {
                data.append(inputPostfix.data(withFormat: .utf8))
            }
            
            let messageId = String(UUID().uuidString.prefix(4))
            let message = data.string(withFormat: .utf8).removeNewline()
            
            //setip 123.123.123.123:5000
            if message.contains("setip") {
                let ip = message.suffix(20)
                print("Ip ottenuto: \(ip)")
                localIp = String(ip)
                let extraConsoleMessage = "Ip setted: "+String(ip)
                
                messages.append(Message(data: message.data(withFormat: .utf8), sent: true, id: messageId, onlyForConsole: true, taggedMessage: message))
                addText(forMessage: messages.last!)
                
                messages.append(Message(data: extraConsoleMessage.data(withFormat: .utf8), sent: false, id: messageId+"i", onlyForConsole: true, taggedMessage: extraConsoleMessage))
                addText(forMessage: messages.last!)
                
                scrollToBottom()
                playSentSound()
                
                textField.text = ""
                return false
            }
            
            let taggedMessage = "<w id=\(messageId)>\(message)</w>"
            let dataTagged = taggedMessage.data(withFormat: .utf8)
            
            serial.sendDataToDevice(dataTagged)
            messages.append(Message(data: message.data(withFormat: .utf8), sent: true, id: messageId, taggedMessage: taggedMessage))
            addText(forMessage: messages.last!)
            scrollToBottom()
            playSentSound()
            
            sendDataToServer(message)
        } else {
            //print("No data sent")
        }
        
        textField.text = ""
        return false
    }
    
    func textField(_ textField: UITextField, shouldChangeCharactersIn range: NSRange, replacementString string: String) -> Bool {
        if string.isEmpty {
            return true
        }
        
        if inputFormat == .utf8 { return true }
        
        var allowed = " ,.-01"
        if inputFormat == .hex { allowed += "23456789abcdef" }
        if inputFormat == .dec { allowed += "23456789" }
        if inputFormat == .oct { allowed += "234567" }
        
        for char in string {
            if !allowed.contains(char) {
                return false
            }
        }
        
        return true
    }
    
    
    // MARK: - TextView
    
    @objc func clear() {
        messages = []
        bytesOnLine = 0
        lines = 0
        
        if isChatEnabled {
            tableView.reloadData()
        }
        
        clearAttributedText()
        textView.text = ""
    }
    
    func scrollToBottom() {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        guard autoScroll else { return }
        if isChatEnabled {
            guard !messages.isEmpty else { return }
            let indexPath = IndexPath(row: messages.count+1, section: 0) // +1 cuz of extra white cell
            tableView.scrollToRow(at: indexPath, at: .bottom, animated: true)
        } else {
            UIView.setAnimationsEnabled(false) // for speed
            let range = NSMakeRange(NSString(string: textView.text).length - 1, 1)
            textView.scrollRangeToVisible(range)
            UIView.setAnimationsEnabled(true)
        }
    }
    
    func addText(forMessage msg: Message, updateTableView: Bool = true) {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        if !displaySent && msg.sent {
            return
        }
        
        if isChatEnabled && updateTableView && !msg.onlyForConsole {
            tableView.insertRows(at: [IndexPath(row: messages.count, section: 0)], with: .none) // not min one cuz of extra white celll
            //return - Still add text to textview for sharesheet function!
        } else if isChatEnabled && updateTableView,
                  let index = messages.firstIndex(where: {msg.id == $0.id+"r"}) {
            let indexPath = IndexPath.init(row: index+1, section: 0)
            tableView.reloadRows(at: [indexPath], with: .automatic)
        }
        
        queues.textUpdate.async {
            self._addText(forMessage: msg)
        }
    }
    
    private func _addText(forMessage msg: Message) {
        var string = ""
        var preserveLinebreak = false
        
        func addLineHeader() {
            lines += 1
            
            if string.isEmpty {
                preserveLinebreak = true
            } else {
                string += "\n"
            }
            
            if displayLineNumbers {
                string += "\(lines)".leftPad("0", minCount: 4) + " "
            }
            
            if displayTimeStamp {
                string += dateFormatter.string(from: msg.timeStamp) + " "
            }
        }
        
        func append(string: String, attributes: [NSAttributedStringKey: Any]) {
            appendToAttributedText(NSAttributedString(string: string, attributes: attributes), preserveLinebreak)
        }
        
        if msg.sent {
            if bytesOnLine > 0 {
                append(string: "\n", attributes: defaultAttributes)
                bytesOnLine = 0
            }
            addLineHeader()
            string += msg.taggedMessage.removeNewline() + "\n"
            append(string: string, attributes: sentAttributes)
            return
        }
        
        switch displayNewlineRule {
        case .message:
            addLineHeader()
            string += msg.taggedMessage.removeNewline() /*+ "\n"*/
            
        case .count(let max):
            for b in msg.data {
                if bytesOnLine == 0 {
                    addLineHeader()
                }
                string += b.string(withFormat: displayFormat).removeNewline()
                if displayFormat != .utf8 {
                    string += " "
                }
                bytesOnLine += 1
                if bytesOnLine == max {
                    //string += "\n"
                    bytesOnLine = 0
                }
            }
            
        case .byte(let byte):
            for b in msg.data {
                if bytesOnLine == 0 {
                    addLineHeader()
                }
                string += b.string(withFormat: displayFormat).removeNewline()
                if displayFormat != .utf8 {
                    string += " "
                }
                bytesOnLine += 1
                if b == byte {
                    //string += "\n"
                    bytesOnLine = 0
                }
            }
            
        case .newLine:
            for b in msg.data {
                if bytesOnLine == 0 {
                    addLineHeader()
                }
                string += b.string(withFormat: displayFormat).removeNewline()
                if displayFormat != .utf8 {
                    string += " "
                }
                bytesOnLine += 1
                if b == 10 {
                    //string += "\n"
                    bytesOnLine = 0
                }
            }
        }

        string += "\n"
        append(string: string, attributes: defaultAttributes)
    }
    
    func playReceivedSound() {
        guard playSounds else { return }
        AudioServicesPlaySystemSound(1003)
    }
    
    func playSentSound() {
        guard playSounds else { return }
        AudioServicesPlaySystemSound(1004)
    }
    
    
    // MARK: - TableView
    
    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        return messages.count + 2
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        if indexPath.row == 0 || indexPath.row == messages.count+1 {
            let cell = tableView.dequeueReusableCell(withIdentifier: "whiteCell", for: indexPath)
            cell.selectionStyle = .none
            return cell
        } else {
            let cell = tableView.dequeueReusableCell(withIdentifier: "cell", for: indexPath) as! ChatTableViewCell
            cell.messageText = messages[indexPath.row-1].data.string(withFormat: displayFormat).removeNewline()
            cell.isSent = messages[indexPath.row-1].sent
            cell.isRead = messages[indexPath.row-1].read
            return cell
        }
    }
    
    func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        if indexPath.row == 0 || indexPath.row == messages.count+1 {
            return indexPath.row == 0 ? topWhiteCellHeight : bottomWhiteCellHeight
        } else {
            return ChatTableViewCell.calculateHeight(forText: messages[indexPath.row-1].data.string(withFormat: displayFormat).removeNewline(), isSent: messages[indexPath.row-1].sent)
        }
    }
    
    func tableView(_ tableView: UITableView, estimatedHeightForRowAt indexPath: IndexPath) -> CGFloat {
        let messages = self.messages.compactMap({return $0.onlyForConsole ? nil : $0})
        if indexPath.row == 0 || indexPath.row == messages.count+1 {
            return indexPath.row == 0 ? topWhiteCellHeight : bottomWhiteCellHeight
        } else {
            return ChatTableViewCell.calculateHeight(forText: messages[indexPath.row-1].data.string(withFormat: displayFormat).removeNewline(), isSent: messages[indexPath.row-1].sent)
        }
    }
    
    
    // MARK: - BluetoothSerial
    
    var bufferedMessage: String = ""
    func serialDidReceiveData(_ data: Data) {
        playReceivedSound()
        
        var cleanedMessage = data.string(withFormat: .utf8).removeNewline()
        //TODO: regex on message
        
        if !bufferedMessage.isEmpty {
            cleanedMessage = bufferedMessage + cleanedMessage
        }
        
        var regexFullMessage: String = ""
        var regexId: String = ""
        var regexMessage: String = ""
        var regexMessageType: String = ""
        
        //uso regex per prendere contenuto e id e tipologia
        let range = NSRange(location: 0, length: cleanedMessage.utf16.count)
        let regex = try! NSRegularExpression(pattern: ".*id=([^>]+)>([^<]+)?<\\/([^>]+)>")
        let matches = regex.matches(in: cleanedMessage, options: [], range: range)
        
        
        
        if let match = matches.first {
            let rangeFullMessage = match.range(at:0)
            if let swiftRangeFullMessage = Range(rangeFullMessage, in: cleanedMessage) {
                regexFullMessage = String(cleanedMessage[swiftRangeFullMessage])
            }
            
            let rangeId = match.range(at:1)
            if let swiftRangeId = Range(rangeId, in: cleanedMessage) {
                regexId = String(cleanedMessage[swiftRangeId])
            }
            
            let rangeMessage = match.range(at:2)
            if let swiftRangeMessage = Range(rangeMessage, in: cleanedMessage) {
                regexMessage = String(cleanedMessage[swiftRangeMessage])
            }
            
            let rangeMessageType = match.range(at:3)
            if let swiftRangeMessageType = Range(rangeMessageType, in: cleanedMessage) {
                regexMessageType = String(cleanedMessage[swiftRangeMessageType])
            }
            bufferedMessage = ""
            
            if (regexMessageType == "r") {
                let savedMessage = messages.first(where: { return $0.id == regexId})
                savedMessage?.read = true
                
                let readNotificationMessage = Message(data: regexMessage.data(withFormat: .utf8), read: true, id: regexId+"r", onlyForConsole: true, taggedMessage: regexFullMessage)
                messages.append(readNotificationMessage)
                addText(forMessage: readNotificationMessage)
                
                return
            }
            
        } else {
            bufferedMessage = cleanedMessage
            return
        }
                        
        //----------------
        let new = Message(data: regexMessage.data(withFormat: .utf8), read: true, id: regexId, taggedMessage: regexFullMessage)
        messages.append(new)
        addText(forMessage: new)
        
        // done in async queue
        //scrollToBottom()
        
        sendDataToServer(regexMessage)
        
    }
    
    func sendDataToServer(_ regexMessage: String) {
        //Verifico se ho l'indirizzo IP locale per mandare i dati al server
        guard let localIp else { return }
        
        //Se è un SOS -> Inviare al server lo stato SOS = active
        //Mando stato della batteria al server  (Per ora simulato con un rand(50-80))
        let status: StatusGlove.SosStatusType = (regexMessage.lowercased() == "sos") ? .active : .inactive
        let batteryLevel = Int.random(in: 50...80)
        let statusGloveData = StatusGlove.init(batteryLevel: batteryLevel, sosStatus: status)
        
        let urlString = endpoint.replacingOccurrences(of: "{ip}", with: localIp)
        print("url: \(urlString)")
        if let url = URL(string: urlString) {
            var request = URLRequest(url: url)
            request.setValue("application/json", forHTTPHeaderField: "Content-Type")
            request.httpMethod = "POST"
            let encoder = JSONEncoder()
            if let data = try? encoder.encode(statusGloveData) {
                request.httpBody = data
                
                URLSession.shared.dataTask(with: request) { data, response, error in
                    if let error {
                        print("Errore invio dati al server: \(error.localizedDescription)")
                        return
                    }
                    
                    print("Invio dati al server avvenuto con successo")
                    print("Response: \(String(describing: response))")
                }.resume()
            }
            
        }
    }
    
    func serialDidChangeState() {
        if serial.centralManager.state != .poweredOn {
            NotificationCenter.default.post(name: .disconnected)
        }
    }
    
    func serialDidDisconnect(_ peripheral: CBPeripheral, error: Error?) {
        NotificationCenter.default.post(name: .disconnected)
    }
    
    @objc func serialDidSendData(_ notification: Notification) {
        let data = notification.userInfo!["data"] as! Data
        
        let messageId = String(UUID().uuidString.prefix(4))
        let message = data.string(withFormat: .utf8).removeNewline()
        
        //setip 123.123.123.123:5000
        if message.contains("setip") {
            let ip = message.suffix(20)
            print("Ip ottenuto: \(ip)")
            localIp = String(ip)
            let extraConsoleMessage = "Ip setted: "+String(ip)
            
            messages.append(Message(data: message.data(withFormat: .utf8), sent: true, id: messageId, onlyForConsole: true, taggedMessage: message))
            messages.append(Message(data: extraConsoleMessage.data(withFormat: .utf8), sent: false, id: messageId+"i", onlyForConsole: true, taggedMessage: extraConsoleMessage))
            addText(forMessage: messages.last!)
            scrollToBottom()
            playSentSound()
            
            return
        }
        
        let taggedMessage = "<w id=\(messageId)>\(message)</w>"
        
        messages.append(Message(data: message.data(withFormat: .utf8), sent: true, id: messageId, taggedMessage: taggedMessage))
        addText(forMessage: messages.last!)
        
        let sound = notification.userInfo!["playSound"] as! Bool
        if sound {
            playSentSound()
        }
        
        sendDataToServer(message)
    }
    
    
    // MARK: - Actions
    
    @IBAction func disconnect(_ sender: Any) {
        serial.disconnect()
    }
}


@IBDesignable class GradientView: UIView {
    @IBInspectable var topColor: UIColor = UIColor.white
    @IBInspectable var bottomColor: UIColor = UIColor.black
    
    override class var layerClass: AnyClass {
        return CAGradientLayer.self
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        (layer as! CAGradientLayer).colors = [topColor.cgColor, bottomColor.cgColor]
    }
}

// Little Hack to make sure \n's get displayed with the correct head indent
extension String {
    fileprivate func removeNewline() -> String {
        return self.replacingOccurrences(of: "\n", with: "").replacingOccurrences(of: "\r", with: "")
    }
}
