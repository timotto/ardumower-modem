#include "ble_adapter.h"
#include "log.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

using namespace ArduMower::Modem;

BleAdapter::BleAdapter(Settings::Settings &s, Router &r)
    : settings(s), router(r),
      count(0), countMetric(
                    new Prometheus::CallbackMeasurement(
                        "ardumower_modem_ble_request_count",
                        std::bind(
                            &BleAdapter::writeCount,
                            this,
                            std::placeholders::_1,
                            std::placeholders::_2))),
      chr(NULL), status(BLE_STATUS_IDLE), expectNotify(0),
      bleReader("\n"), receivedFromBle(""), sendToBle(""),
      sendToMower("")
{
}

unsigned int BleAdapter::writeCount(char *buffer, unsigned int size)
{
  return snprintf(buffer, size, "%u", count);
}

void BleAdapter::begin()
{
  if (!settings.bluetooth.enabled)
    return;

  BLEDevice::init(settings.general.name.c_str());
  if (settings.bluetooth.pin_enabled)
  {
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(settings.bluetooth.pin);
  } else {
    NimBLEDevice::setSecurityAuth(false, false, false);
  }

  auto server = BLEDevice::createServer();
  server->setCallbacks(this);

  auto service = server->createService(SERVICE_UUID);

  uint32_t props;
  if (settings.bluetooth.pin_enabled)
    props = NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN |
            NIMBLE_PROPERTY::WRITE_ENC |
            NIMBLE_PROPERTY::WRITE_AUTHEN |
            NIMBLE_PROPERTY::WRITE_NR;
  else
    props = NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_NR;
  chr = service->createCharacteristic(CHARACTERISTIC_UUID, props);
  chr->addDescriptor(new NimBLE2904());
  chr->setCallbacks(this);

  service->start();
  server->getAdvertising()->start();
}

void BleAdapter::loop()
{
  if (!settings.bluetooth.enabled)
    return;

  static int last_status = -1;
  if (status != last_status)
  {
    Log(DBG, "BleAdapter::loop::transition(%d -> %d)", last_status, status);
    last_status = status;
  }

  switch (status)
  {
  case BLE_STATUS_IDLE:
    startAdvertising();
    break;

  case BLE_STATUS_PAIRING:
    // TODO timeout
    break;

  case BLE_STATUS_CONNECTED:
    if (receivedFromBle.length() > 0)
      status = BLE_STATUS_RXBLE;
    break;

  case BLE_STATUS_RXBLE:
    loopBleRx();
    // TODO timeout
    break;

  case BLE_STATUS_TXROUTER:
    loopRouterTx();
    break;

  case BLE_STATUS_RXROUTER:
    // TODO timeout
    break;

  case BLE_STATUS_TXBLE:
    loopBleTx();
    break;

  case BLE_STATUS_BLENOTIFY:
    // TODO timeout
    break;
  }
}

// void BleAdapter::clearPairings()
// {
//   Log(DBG, "BleAdapter::clearPairings");
//   if (!settings.bluetooth.enabled)
//     return;
//   NimBLEDevice::deleteAllBonds();
// }

bool BleAdapter::initBluetooth()
{
  if(!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }
 
  if(esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }
 
  if(esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  return true;
}

char *BleAdapter::bda2str(const uint8_t* bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}

#define PAIR_MAX_DEVICES 20
#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remov

void BleAdapter::clearPairings() {
  initBluetooth();
  
  //char bda_str[18];
  uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
  // Serial.print("ESP32 bluetooth address: "); 
  // Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
  // Get the numbers of bonded/paired devices in the BT module
  int count = esp_bt_gap_get_bond_device_num();
  if(!count) {
    Serial.println("No bonded device found.");
  } else {
    Serial.print("Bonded device count: "); Serial.println(count);
    if(PAIR_MAX_DEVICES < count) {
      count = PAIR_MAX_DEVICES; 
      Serial.print("Reset bonded device count: "); Serial.println(count);
    }
    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if(ESP_OK == tError) {
      for(int i = 0; i < count; i++) {
        // Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
        // Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));     
        if(REMOVE_BONDED_DEVICES) {
          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if(ESP_OK == tError) {
            //Serial.print("Removed bonded device # "); 
          } else {
            Serial.print("Failed to remove bonded device # ");
          }
          Serial.println(i);
        }
      }        
    }
  }
}

void BleAdapter::loopBleRx()
{
  while (receivedFromBle.length() > 0)
  {
    char c = receivedFromBle[0];
    receivedFromBle = receivedFromBle.substring(1);

    String line = bleReader.update(c);
    if (line != "")
    {
      count++;
      sendToMower = line;
      status = BLE_STATUS_TXROUTER;
      return;
    }
  }
}

void BleAdapter::loopBleTx()
{
  while (true)
  {
    const size_t n = sendToBle.length();
    if (n == 0)
    {
      status = BLE_STATUS_CONNECTED;
      return;
    }

    const size_t len = n > BLE_MTU ? BLE_MTU : n;
    char *data = strdup(sendToBle.substring(0, len).c_str());
    sendToBle = sendToBle.substring(len);

    chr->setValue((uint8_t *)data, len);
    expectNotify++;
    chr->notify();
    free(data);

    if (expectNotify > 0)
    {
      status = BLE_STATUS_BLENOTIFY;
      Log(DBG, "BleAdapter::loopBleTx::expect_notify(data=%s,length=%u)", data, len);
      return;
    }
  }
}

void BleAdapter::loopRouterTx()
{
  if (!router.send(sendToMower, [&](String res, int err)
                   {
                     sendToBle = res + "\r\n";
                     status = BLE_STATUS_TXBLE;
                   }))
    return;

  status = BLE_STATUS_RXROUTER;
}

void BleAdapter::startAdvertising()
{
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);

  // iPhone
  advertising->setMinPreferred(0x06);
  advertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  if (status != BLE_STATUS_IDLE)
    return;

  status = BLE_STATUS_ADVERTISING;
}

void BleAdapter::reset()
{
  receivedFromBle = "";
  sendToBle = "";
  sendToMower = "";
  bleReader.reset();
  expectNotify = 0;
}

#define BLE_MIN_INTERVAL 2 // connection parameters (tuned for high speed/high power consumption - see: https://support.ambiq.com/hc/en-us/articles/115002907792-Managing-BLE-Connection-Parameters)
#define BLE_MAX_INTERVAL 10
#define BLE_LATENCY 0
#define BLE_TIMEOUT 30
void BleAdapter::onConnect(BLEServer *server, ble_gap_conn_desc *param)
{
  reset();

  server->updateConnParams(param->conn_handle, BLE_MIN_INTERVAL, BLE_MAX_INTERVAL, BLE_LATENCY, BLE_TIMEOUT); // 1, 10, 0, 20
  if (!settings.bluetooth.pin_enabled)
  {
    status = BLE_STATUS_CONNECTED;
    return;
  }

  NimBLEDevice::startSecurity(param->conn_handle);
  status = BLE_STATUS_PAIRING;
}

void BleAdapter::onDisconnect(BLEServer *server)
{
  reset();
  status = BLE_STATUS_IDLE;
}

uint32_t BleAdapter::onPassKeyRequest()
{
  Log(DBG, "BleAdapter::onPassKeyRequest");
  if (!settings.bluetooth.pin_enabled)
    return 0;


  return settings.bluetooth.pin;
}

bool BleAdapter::onConfirmPIN(uint32_t pin)
{
  Log(DBG, "BleAdapter::onConfirmPIN");
  if (!settings.bluetooth.pin_enabled)
    return true;

  return settings.bluetooth.pin == pin;
}

void BleAdapter::onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc)
{
  Log(DBG, "BleAdapter::onMTUChange(%u)", MTU);

}

void BleAdapter::onAuthenticationComplete(ble_gap_conn_desc* desc)
{
  Log(
    DBG, 
    "BleAdapter::onAuthenticationComplete(encrypted=%d,authenticated=%d,bonded=%d)", 
    desc->sec_state.encrypted,
    desc->sec_state.authenticated,
    desc->sec_state.bonded
  );

  if (!settings.bluetooth.pin_enabled)
    status = BLE_STATUS_CONNECTED;
  else if (desc->sec_state.encrypted && desc->sec_state.authenticated && desc->sec_state.bonded)
    status = BLE_STATUS_CONNECTED;
  else
    Log(DBG, "BleAdapter::onAuthenticationComplete - rejected");
}


void BleAdapter::onNotify(NimBLECharacteristic *pCharacteristic)
{
  if (expectNotify > 0)
    expectNotify--;
  else
    Log(ERR, "BleAdapter::onNotify::expectNotify(value=%d)", expectNotify);

  // when client confirms before the ->notify() call returns
  if (status != BLE_STATUS_BLENOTIFY)
    return;

  status = BLE_STATUS_TXBLE;
  Log(DBG, "BleAdapter::onNotify::success");
}

void BleAdapter::onWrite(BLECharacteristic *c)
{
  String val = c->getValue().c_str();

  receivedFromBle += val;

  if (status != BLE_STATUS_CONNECTED)
    return;

  status = BLE_STATUS_RXBLE;
}
