#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "api.h"
#include "prometheus.h"
#include "reader.h"
#include "router.h"
#include "settings.h"

#define BLE_MTU 20
#define SERVICE_UUID "0000FFE0-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_UUID "0000FFE1-0000-1000-8000-00805F9B34FB"

#define BLE_STATUS_IDLE 0
#define BLE_STATUS_ADVERTISING 1
#define BLE_STATUS_PAIRING 2
#define BLE_STATUS_CONNECTED 3
#define BLE_STATUS_RXBLE 4
#define BLE_STATUS_TXROUTER 5
#define BLE_STATUS_RXROUTER 6
#define BLE_STATUS_TXBLE 7
#define BLE_STATUS_BLENOTIFY 8

namespace ArduMower
{
  namespace Modem
  {
    class BleAdapter : public BLEServerCallbacks, public BLECharacteristicCallbacks, public ArduMower::Modem::Api::Bluetooth
    {
    private:
      Settings::Settings &settings;
      Router &router;
      unsigned int count;
      ArduMower::Modem::Prometheus::CallbackMeasurement *countMetric;

      BLECharacteristic *chr;
      int status;
      int expectNotify;

      Reader bleReader;
      String receivedFromBle;
      String sendToBle;
      String sendToMower;

      void reset();
      void loopBleRx();
      void loopBleTx();
      void loopRouterTx();
      void startAdvertising();

      unsigned int writeCount(char *buffer, unsigned int size);

      bool initBluetooth();
      char *bda2str(const uint8_t* bda, char *str, size_t size);
      
    public:
      BleAdapter(Settings::Settings &s, Router &r);

      void begin();
      void loop();
      virtual void clearPairings() override;

      // BLEServerCallbacks
      virtual void onConnect(BLEServer *s, ble_gap_conn_desc *p) override;
      virtual void onDisconnect(BLEServer *s) override;
      virtual uint32_t onPassKeyRequest() override;
      virtual bool onConfirmPIN(uint32_t pin) override;
      virtual void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) override;
      virtual void onAuthenticationComplete(ble_gap_conn_desc* desc) override;

      // BLECharacteristicCallbacks
      virtual void onNotify(NimBLECharacteristic* pCharacteristic) override;
      virtual void onWrite(BLECharacteristic *c) override;
    };
  }

}
