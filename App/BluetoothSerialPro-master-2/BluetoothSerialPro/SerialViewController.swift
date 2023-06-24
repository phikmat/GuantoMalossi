//
//  SerialViewController.swift
//  BluetoothSerialPro
//
//  Created by Alex on 27/11/2016.
//  Copyright © 2016 Hangar42. All rights reserved.
//

import UIKit

class SerialViewController: UIViewController {
    
    // MARK: - Outlets

    @IBOutlet weak var segments: UISegmentedControl!
    
    
    // MARK: - Variables
    
    var mode = Mode.chatBox
    var consoleViewController: ConsoleViewController!
    var disconnectWasUserTriggered = false
    
    // MARK: - ViewController
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        NotificationCenter.default.addObserver(self, selector: #selector(disconnected), name: .disconnected)
        
        segments.selectedSegmentIndex = Settings.mode.value
        
        let sWidth = UIScreen.main.bounds.width
        let sHeight = UIScreen.main.bounds.height
        if sqrt(pow(sWidth, 2)+pow(sHeight, 2)) < 660 { // 4 inch = 652pt diagonal
            segments.setTitle(nil, forSegmentAt: 0) // used in DisplayOptions too
            segments.setTitle(nil, forSegmentAt: 1)
            segments.setImage(#imageLiteral(resourceName: "Console"), forSegmentAt: 0)
            segments.setImage(#imageLiteral(resourceName: "Actions"), forSegmentAt: 1)
        }

        let storyboard = UIStoryboard(name: "Main", bundle: Bundle.main)

        consoleViewController = storyboard.instantiateViewController(withIdentifier: "ConsoleViewController") as? ConsoleViewController
        consoleViewController.view.translatesAutoresizingMaskIntoConstraints = false
        addChildViewController(consoleViewController)
        view.addSubview(consoleViewController.view)
        
        let v = consoleViewController.view!
        let hc = NSLayoutConstraint(item: v, attribute: .height, relatedBy: .equal, toItem: view, attribute: .height, multiplier: 1.0, constant: 0)
        let wc = NSLayoutConstraint(item: v, attribute: .width, relatedBy: .equal, toItem: view, attribute: .width, multiplier: 1.0, constant: 0)
        let xc = NSLayoutConstraint(item: v, attribute: .centerX, relatedBy: .equal, toItem: view, attribute: .centerX, multiplier: 1.0, constant: 0)
        let yc = NSLayoutConstraint(item: v, attribute: .centerY, relatedBy: .equal, toItem: view, attribute: .centerY, multiplier: 1.0, constant: 0)
        view.addConstraints([hc, wc, xc, yc])
        consoleViewController.didMove(toParentViewController: self)

        reload()
        
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

    
    @objc func disconnected() {
        performSegue(withIdentifier: "unwindToScan", sender: self)
    }
    
    func reload() {
        mode = Mode(rawValue: Settings.mode.value)!
        if mode == .console {
            Settings.displayStyle.value = 0
            NotificationCenter.default.post(name: .displayOptionsChanged)
        } else if mode == .chatBox {
            Settings.displayStyle.value = 1
            NotificationCenter.default.post(name: .displayOptionsChanged)
        }
    }
    

    // MARK: - Actions
    
    @IBAction func changeMode(_ sender: Any) {
        Settings.mode.value = segments.selectedSegmentIndex
        reload()
    }

    @IBAction func clearScreen(_ sender: UIBarButtonItem) {
        let sheet = UIAlertController(title: nil, message: nil, preferredStyle: .actionSheet)
        sheet.addAction(UIAlertAction(title: "Clear Screen", style: .destructive) { _ in
                NotificationCenter.default.post(name: .clearScreen)
            })
        if UIDevice.isPhone {
            sheet.addAction(UIAlertAction(title: "Cancel", style: .cancel) { _ in
                self.dismiss(animated: true, completion: nil)
            })
        }
        sheet.popoverPresentationController?.barButtonItem = sender
        sheet.popoverPresentationController?.permittedArrowDirections = .up
        present(sheet, animated: true)
    }
    
    @IBAction func disconnect(_ sender: Any) {
        disconnectWasUserTriggered = true
        serial.disconnect()
    }
}
