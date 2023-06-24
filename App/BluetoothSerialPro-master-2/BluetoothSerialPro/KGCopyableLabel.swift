//
//  KGCopyableLabel.swift
//  BluetoothSerialPro
//
//  Created by Mattia Picariello on 24/06/23.
//  Copyright © 2023 Hangar42. All rights reserved.
//

import UIKit

class KGCopyableLabel: UILabel {
    
    override public var canBecomeFirstResponder: Bool {
        get {
            return true
        }
    }

    override init(frame: CGRect) {
        super.init(frame: frame)
        setup()
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        setup()
    }

    func setup() {
        isUserInteractionEnabled = true
        addGestureRecognizer(UILongPressGestureRecognizer(
            target: self,
            action: #selector(showCopyMenu(sender:))
        ))
    }

    override func copy(_ sender: Any?) {
        UIPasteboard.general.string = text
        UIMenuController.shared.hideMenu()
    }

    @objc func showCopyMenu(sender: Any?) {
        becomeFirstResponder()
        let menu = UIMenuController.shared
        if !menu.isMenuVisible {
            menu.showMenu(from: self, rect: bounds)
        }
    }

    override func canPerformAction(_ action: Selector, withSender sender: Any?) -> Bool {
        return (action == #selector(copy(_:)))
    }
}
