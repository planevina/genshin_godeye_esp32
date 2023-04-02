#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "boardconf.h"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

char BLEbuf[32] = {0};
String ble_rcv_data = "";

// 定义收发服务的UUID（唯一标识）用UUID来区分各种设备
#if LIYUE_VER
#define SERVICE_UUID "7F7C610C-7A86-4892-94BA-88B70DC790F5"
#else
#define SERVICE_UUID "B98BC7A2-ED66-4BD1-A75A-54F426D67466"
#endif

// RX串口标识
#define CHARACTERISTIC_UUID_RX "C02E69C2-E503-43F8-A74D-B95C1F5AF088"
// TX串口标识
#define CHARACTERISTIC_UUID_TX "D914E6B6-509C-4803-9FB5-9454782478A6"

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        delay(2);
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        delay(2);
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0)
        {
            Serial.println("[BLE] Received Value");
            ble_rcv_data = rxValue.c_str();
            Serial.println(ble_rcv_data);
        }
    }
};

void init_ble(std::string name="ESP32 Eye of God")
{
    // 初始化蓝牙设备
    Serial.println("[BLE] Init start...");
    BLEDevice::init(name);
    // 为蓝牙设备创建服务器
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // 基于SERVICE_UUID来创建一个服务
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    // 开启服务
    pService->start();
    //pServer->startAdvertising();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    Serial.println("[BLE] Init finished.");
}

void start_ble()
{
    // pServer->getAdvertising()->start();
    pServer->startAdvertising();
    Serial.println("[BLE] Started.");
}