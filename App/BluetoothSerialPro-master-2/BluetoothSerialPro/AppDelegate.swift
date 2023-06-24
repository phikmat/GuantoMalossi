//
//  AppDelegate.swift
//  BluetoothSerialPro
//
//  Created by Alex on 27/11/2016.
//  Copyright © 2016 Hangar42. All rights reserved.
//
//  • RappleActivityIndicatorView is aangepast voor touch selector!
//  • iCloud is toegevoegd voor gebruik UIDocumentPickerViewController
//
// TODO's:
// v Bubble shape
// v Sounds
// v Input chars allowed
// v Finals overal ? Cell leading margin
// v Chunking-up
// v Function Selection cell
// v App Info / Feedback
// x Function collectioncell reorder
// x Function with/without response
// x NMEA String
// v Scan reset om de seconde voor als peripherals weg gaan..
// v BLE Device info's
// v Function lighter color
// v Loading animation
// v RSSI indicator
// v Share Sheet
// v Analysis RSSI
// v Auto Write with/without response
// v Scan 'connectable' ad info
// v Button Function UI
// v Dismiss keyboard tap events
// v iPhone orientation changes
// v Segments icons bij iphone
// - Testen segments displayoptions iphone5
//
//
// Before Release:
// - Update info text (every minor release)
//
//
// Version 1.2.0
// > Function append \n\r in function settings
// > Separate UUID settings for read/write (test!)
// > Auto Reconnect
// > Clear Element function
// > Import/Export functions

// TODO: Update info text!!!

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?
    
    func application(_ application: UIApplication, willFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey : Any]? = nil) -> Bool {
        Settings.registerDefaultValues()
        Settings.migrateToLatest()
        return true
    }
    
    private func present(_ viewController: UIViewController, animated: Bool, completion: (() -> Void)? = nil) {
        // create temporary window, will be deallocated automatcially after dismissal
        let tempWindow = UIWindow(frame: UIScreen.main.bounds)
        tempWindow.rootViewController = UIViewController()
        tempWindow.windowLevel = UIWindowLevelAlert + 1
        tempWindow.makeKeyAndVisible() // TODO: Attempting to load the view of a view controller while it is deallocating is not allowed and may result in undefined behavior
        
        // present
        tempWindow.rootViewController?.present(viewController, animated: true, completion: completion)
    }

}
