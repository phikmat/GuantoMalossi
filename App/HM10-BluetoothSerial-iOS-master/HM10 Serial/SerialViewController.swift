//
//  SerialViewController.swift
//  HM10 Serial
//
//  Created by Alex on 10-08-15.
//  Copyright (c) 2015 Balancing Rock. All rights reserved.
//

import UIKit
import CoreBluetooth
import QuartzCore
import MessageKit
import InputBarAccessoryView

/// The option to add a \n or \r or \r\n to the end of the send message
enum MessageOption: Int {
    case noLineEnding,
         newline,
         carriageReturn,
         carriageReturnAndNewline
}

/// The option to add a \n to the end of the received message (to make it more readable)
enum ReceivedMessageOption: Int {
    case none,
         newline
}

final class SerialViewController: MessagesViewController, UITextFieldDelegate, BluetoothSerialDelegate {

//MARK: IBOutlets
    
    @IBOutlet weak var mainTextView: UITextView!
    @IBOutlet weak var messageField: UITextField!
    @IBOutlet weak var bottomView: UIView!
    @IBOutlet weak var bottomConstraint: NSLayoutConstraint! // used to move the textField up when the keyboard is present
    @IBOutlet weak var barButton: UIBarButtonItem!
    @IBOutlet weak var navItem: UINavigationItem!


//MARK: Functions
    override func viewDidLoad() {
        super.viewDidLoad()
        
        
        //init messageKit
        messageInputBar.delegate = self
        messagesCollectionView.messagesDataSource = self
        messagesCollectionView.messagesLayoutDelegate = self
        messagesCollectionView.messagesDisplayDelegate = self
        
                
        // init serial
        serial = BluetoothSerial(delegate: self)
        
        // UI
        mainTextView.text = ""
        reloadView()
        
        NotificationCenter.default.addObserver(self, selector: #selector(SerialViewController.reloadView), name: NSNotification.Name(rawValue: "reloadStartViewController"), object: nil)
        
        // we want to be notified when the keyboard is shown (so we can move the textField up)
        NotificationCenter.default.addObserver(self, selector: #selector(SerialViewController.keyboardWillShow(_:)), name: UIResponder.keyboardWillShowNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(SerialViewController.keyboardWillHide(_:)), name: UIResponder.keyboardWillHideNotification, object: nil)
        
        // to dismiss the keyboard if the user taps outside the textField while editing
        let tap = UITapGestureRecognizer(target: self, action: #selector(SerialViewController.dismissKeyboard))
        tap.cancelsTouchesInView = false
        view.addGestureRecognizer(tap)
        
        // style the bottom UIView
        bottomView.layer.masksToBounds = false
        bottomView.layer.shadowOffset = CGSize(width: 0, height: -1)
        bottomView.layer.shadowRadius = 0
        bottomView.layer.shadowOpacity = 0.5
        bottomView.layer.shadowColor = UIColor.gray.cgColor
        
        let navBarAppearance = UINavigationBarAppearance()
        navBarAppearance.configureWithOpaqueBackground()
        navBarAppearance.titleTextAttributes = [.foregroundColor: UIColor.black]
        navBarAppearance.largeTitleTextAttributes = [.foregroundColor: UIColor.black]
        navBarAppearance.backgroundColor = UIColor.white
        self.navigationController?.navigationBar.standardAppearance = navBarAppearance
        self.navigationController?.navigationBar.scrollEdgeAppearance = navBarAppearance
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
    }
    
    @objc func keyboardWillShow(_ notification: Notification) {
        // animate the text field to stay above the keyboard
        var info = (notification as NSNotification).userInfo!
        let value = info[UIResponder.keyboardFrameEndUserInfoKey] as! NSValue
        let keyboardFrame = value.cgRectValue
        
        //TODO: Not animating properly
        UIView.animate(withDuration: 1, delay: 0, options: UIView.AnimationOptions(), animations: { () -> Void in
            self.bottomConstraint.constant = keyboardFrame.size.height
            }, completion: { Bool -> Void in
            self.textViewScrollToBottom()
        })
    }
    
    @objc func keyboardWillHide(_ notification: Notification) {
        // bring the text field back down..
        UIView.animate(withDuration: 1, delay: 0, options: UIView.AnimationOptions(), animations: { () -> Void in
            self.bottomConstraint.constant = 0
        }, completion: nil)

    }
    
    @objc func reloadView() {
        // in case we're the visible view again
        serial.delegate = self
        
        if serial.isReady {
            navItem.title = serial.connectedPeripheral!.name
            barButton.title = "Disconnect"
            barButton.tintColor = UIColor.red
            barButton.isEnabled = true
        } else if serial.centralManager.state == .poweredOn {
            navItem.title = "Bluetooth Serial"
            barButton.title = "Connect"
            barButton.tintColor = view.tintColor
            barButton.isEnabled = true
        } else {
            navItem.title = "Bluetooth Serial"
            barButton.title = "Connect"
            barButton.tintColor = view.tintColor
            barButton.isEnabled = false
        }
    }
    
    func textViewScrollToBottom() {
        let range = NSMakeRange(NSString(string: mainTextView.text).length - 1, 1)
        mainTextView.scrollRangeToVisible(range)
    }
    

//MARK: BluetoothSerialDelegate
    
    private func insertNewMessage(_ message: Message) {
        //add the message to the messages array and reload it
        messages.append(message)
        messagesCollectionView.reloadData()
        DispatchQueue.main.async {
            self.messagesCollectionView.scrollToLastItem(at: .bottom, animated: true)
        }
    }
    
    var bufferedMessage: String = ""
    func serialDidReceiveString(_ message: String) {
        var cleanedMessage = message.replacingOccurrences(of: "\0", with: "")
        guard cleanedMessage != "" else { return }
        
        if !bufferedMessage.isEmpty {
            cleanedMessage = bufferedMessage + cleanedMessage
        }
        
        var regexId: String = ""
        var regexMessage: String = ""
        var regexMessageType: String = ""
        
        //uso regex per prendere contenuto e id e tipologia
        let range = NSRange(location: 0, length: cleanedMessage.utf16.count)
        let regex = try! NSRegularExpression(pattern: ".*id=([^>]+)>([^<]+)<\\/([^>]+)>")
        let matches = regex.matches(in: cleanedMessage, options: [], range: range)
        
        if let match = matches.first {
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
                let savedMessage = messages.first(where: { return $0.messageId == regexId}) as? Message
                savedMessage?.read = true
                
                messagesCollectionView.reloadData()
                DispatchQueue.main.async {
                    self.messagesCollectionView.scrollToLastItem(at: .bottom, animated: true)
                }
                
                return
            }
            
        } else {
            bufferedMessage = cleanedMessage
            return
        }
        
        // add the received text to the chat message array
        let chatMessage = Message(
            id: regexId,
            content: regexMessage,
            created: DateFormatter().string(from: Date()),
            senderID: guantoSender.senderId,
            senderName: guantoSender.displayName,
            read: true
        )
        insertNewMessage(chatMessage)
        
        // add the received text to the textView, optionally with a line break at the end
        mainTextView.text! += cleanedMessage
        let pref = UserDefaults.standard.integer(forKey: ReceivedMessageOptionKey)
        if pref == ReceivedMessageOption.newline.rawValue { mainTextView.text! += "\n" }
        textViewScrollToBottom()
    }
    
    func serialDidDisconnect(_ peripheral: CBPeripheral, error: NSError?) {
        reloadView()
        dismissKeyboard()
        let hud = MBProgressHUD.showAdded(to: view, animated: true)
        hud?.mode = MBProgressHUDMode.text
        hud?.labelText = "Disconnected"
        hud?.hide(true, afterDelay: 1.0)
    }
    
    func serialDidChangeState() {
        reloadView()
        if serial.centralManager.state != .poweredOn {
            dismissKeyboard()
            let hud = MBProgressHUD.showAdded(to: view, animated: true)
            hud?.mode = MBProgressHUDMode.text
            hud?.labelText = "Bluetooth turned off"
            hud?.hide(true, afterDelay: 1.0)
        }
    }
    
    
//MARK: UITextFieldDelegate
    
    private func sendMessageToGuanto(_ message: String) -> Bool {
        if !serial.isReady {
            let alert = UIAlertController(title: "Not connected", message: "What am I supposed to send this to?", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "Dismiss", style: UIAlertAction.Style.default, handler: { action -> Void in self.dismiss(animated: true, completion: nil) }))
            present(alert, animated: true, completion: nil)
            messageField.resignFirstResponder()
            return false
        }
        
        /*
        // send the message to the bluetooth device
        // but fist, add optionally a line break or carriage return (or both) to the message
        let pref = UserDefaults.standard.integer(forKey: MessageOptionKey)
        var msg = message
        switch pref {
        case MessageOption.newline.rawValue:
            msg += "\n"
        case MessageOption.carriageReturn.rawValue:
            msg += "\r"
        case MessageOption.carriageReturnAndNewline.rawValue:
            msg += "\r\n"
        default:
            msg += ""
        }
         */
        
        // send the message and clear the textfield
        serial.sendMessageToDevice(message)
        messageField.text = ""
        
        return true
    }
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        sendMessageToGuanto(textField.text!)
    }
    
    @objc func dismissKeyboard() {
        messageField.resignFirstResponder()
    }
    
    
//MARK: IBActions

    @IBAction func barButtonPressed(_ sender: AnyObject) {
        if serial.connectedPeripheral == nil {
            performSegue(withIdentifier: "ShowScanner", sender: self)
        } else {
            serial.disconnect()
            reloadView()
        }
    }
}

public struct Sender: SenderType {
    public let senderId: String

    public let displayName: String
}

class Message {
    let id: String
    let content: String
    let created: String
    let senderID: String
    let senderName: String
    
    var read: Bool
    
    var dictionary: [String: Any] {
        return [
            "id": id,
            "content": content,
            "created": created,
            "senderID": senderID,
            "senderName":senderName]
    }
    
    var writableContent: String {
        return "<w id=\(id)>\(content)</w>"
    }
    
    init(content: String, sender: Sender, read: Bool = false) {
        self.id = "\(UUID().uuidString.prefix(8))"
        self.content = content
        self.created = DateFormatter().string(from: Date())
        self.senderID = sender.senderId
        self.senderName = sender.displayName
        self.read = read
    }
    
    init(id: String, content: String, created:String, senderID: String, senderName: String, read: Bool = false) {
        self.id = id
        self.content = content
        self.created = created
        self.senderID = senderID
        self.senderName = senderName
        self.read = read
    }
    
}

extension Message {
    convenience init?(dictionary: [String: Any]) {
        guard let id = dictionary["id"] as? String,
              let content = dictionary["content"] as? String,
              let created = dictionary["created"] as? String,
              let senderID = dictionary["senderID"] as? String,
              let senderName = dictionary["senderName"] as? String,
            let read = dictionary["read"] as? Bool
        else {return nil}
        self.init(id: id, content: content, created: created, senderID: senderID, senderName: senderName, read: read)
    }
}

extension Message: MessageType {
    var sender: SenderType {
        return Sender(senderId: senderID, displayName: senderName)
    }
    var messageId: String {
        return id
    }
    var sentDate: Date {
        return DateFormatter().date(from:created)!
    }
    
    var kind: MessageKind {
        return .text(content)
    }
}

// Some global variables for the sake of the example. Using globals is not recommended!
let meSender = Sender(senderId: UUID().uuidString, displayName: "Me")
let guantoSender = Sender(senderId: UUID().uuidString, displayName: "Guanto")
var messages: [MessageType] = []

extension SerialViewController: MessagesDataSource {

    var currentSender: SenderType {
        return meSender
    }

    func numberOfSections(in messagesCollectionView: MessagesCollectionView) -> Int {
        return messages.count
    }

    func messageForItem(at indexPath: IndexPath, in messagesCollectionView: MessagesCollectionView) -> MessageType {
        return messages[indexPath.section]
    }
    
}

extension SerialViewController: MessagesDisplayDelegate {
    
    func textColor(for message: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> UIColor {
        isFromCurrentSender(message: message) ? .white : .darkText
      }
    
    func backgroundColor(for message: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> UIColor {
        isFromCurrentSender(message: message) ? .blue : UIColor(red: 230 / 255, green: 230 / 255, blue: 230 / 255, alpha: 1)
      }
    
    func messageStyle(for message: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> MessageStyle {
        let tail: MessageStyle.TailCorner = isFromCurrentSender(message: message) ? .bottomRight : .bottomLeft
        return .bubbleTail(tail, .curved)
      }
    
    func messageBottomLabelAttributedText(for message: MessageType, at indexPath: IndexPath) -> NSAttributedString? {
        let status = (messages.first(where: {$0.messageId == message.messageId}) as? Message)?.read ?? false
        
        
        let dateString = DateFormatter().string(from: message.sentDate)
        let likeString = status ? "Letto" : "Inviato"
        return NSAttributedString(string: "\(dateString) | \(likeString)", attributes: [NSAttributedString.Key.font: UIFont.preferredFont(forTextStyle: .caption2)])
    }
    
}

extension SerialViewController: MessagesLayoutDelegate {
  func cellTopLabelHeight(for _: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> CGFloat {
    18
  }

  func cellBottomLabelHeight(for _: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> CGFloat {
    17
  }

  func messageTopLabelHeight(for _: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> CGFloat {
    20
  }

  func messageBottomLabelHeight(for _: MessageType, at _: IndexPath, in _: MessagesCollectionView) -> CGFloat {
    16
  }
    
}

extension SerialViewController: InputBarAccessoryViewDelegate {
    func inputBar(_ inputBar: InputBarAccessoryView, didPressSendButtonWith text: String) {
        //When use press send button this method is called.
        let chatMessage = Message(
            content: text,
            sender: meSender
        )
        //calling function to insert and save message
        guard sendMessageToGuanto(chatMessage.writableContent) else { return }
        insertNewMessage(chatMessage)
        //clearing input field
        inputBar.inputTextView.text = ""
        messagesCollectionView.reloadData()
        messagesCollectionView.scrollToLastItem(at: .bottom, animated: true)
    }
}
