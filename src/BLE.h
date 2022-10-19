#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

char BLEbuf[32] = {0};
String ble_rcv_data = "";

// 定义收发服务的UUID（唯一标识）
#define SERVICE_UUID "6E400081-B5A3-F393-E0A9-E50E24DCCA9E"
// RX串口标识
#define CHARACTERISTIC_UUID_RX "6E400082-B5A3-F393-E0A9-E50E24DCCA9E"
// TX串口标识
#define CHARACTERISTIC_UUID_TX "6E400083-B5A3-F393-E0A9-E50E24DCCA9E"

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

void init_ble()
{
    // 初始化蓝牙设备
    Serial.println("[BLE] Init start...");
    BLEDevice::init("ESP32 Eye of God");
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
    pServer->startAdvertising();
    //pServer->getAdvertising()->start();
    Serial.println("[BLE] Init finished.");
}

void start_ble()
{
    // pServer->getAdvertising()->start();
    pServer->startAdvertising();
    Serial.println("[BLE] Started.");
}