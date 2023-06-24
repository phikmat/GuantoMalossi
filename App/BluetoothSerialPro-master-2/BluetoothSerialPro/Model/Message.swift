//
//  Message.swift
//  BluetoothSerialPro
//
//  Created by Mattia Picariello on 24/06/23.
//  Copyright © 2023 Hangar42. All rights reserved.
//

import Foundation

class Message {
    var data: Data
    var timeStamp: Date
    var sent: Bool
    var read: Bool
    var id: String
    var onlyForConsole: Bool
    var taggedMessage: String
    
    var date: String {
        return DateFormatter().string(from: timeStamp)
    }
    
    
    
    init(data: Data, sent: Bool = false, read: Bool = false, id: String? = nil, onlyForConsole: Bool = false, taggedMessage: String) {
        self.data = data
        self.timeStamp = Date()
        self.sent = sent
        self.read = read
        self.id = id ?? String(UUID().uuidString.prefix(4))
        self.onlyForConsole = onlyForConsole
        self.taggedMessage = taggedMessage
    }
}
