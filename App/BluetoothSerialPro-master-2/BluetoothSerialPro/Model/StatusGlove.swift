//
//  StatusGlove.swift
//  BluetoothSerialPro
//
//  Created by Mattia Picariello on 23/06/23.
//  Copyright © 2023 Hangar42. All rights reserved.
//

import Foundation

struct StatusGlove: Codable {
    var deviceName: String
    var deviceType: String
    var batteryStatus: String
    var sosStatus: String
    
    init(deviceName: String = "DSD TECH", batteryLevel: Int, sosStatus: SosStatusType = .inactive) {
        self.deviceName = deviceName
        self.deviceType = "Mobile"
        self.batteryStatus = "\(batteryLevel) %"
        self.sosStatus = sosStatus.rawValue
    }
    
    enum SosStatusType: String {
        case active = "active"
        case inactive = "inactive"
    }
    
    enum CodingKeys: String, CodingKey {
        case deviceName = "MyDevice"
        case deviceType = "DeviceType"
        case batteryStatus = "BatteryStatus"
        case sosStatus = "SOSStatus"
    }
    
}
