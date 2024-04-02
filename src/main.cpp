#include <bluefruit.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// ========> Macros define <=======
#define BUS_PIN A0

//========> Class define <=====
OneWire wire=OneWire(BUS_PIN);
DallasTemperature sensors= DallasTemperature(&wire);
DeviceAddress insideThermometer;
// BLEUart bleuart; // uart over ble
BLEDis bledis;
void startAdv(float temp);
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void BLEinit();
void printTemperature(DeviceAddress insideThermometer);
void printAddress(DeviceAddress insideThermometer);
#include <bluefruit.h>

struct ADV_PACKET
{
  uint8_t id1=0x11;
  uint8_t id2=0x36;
  uint8_t singht;
  uint32_t* tempIce;
  uint8_t id=0x03;
};
struct SCNR_PACKET
{
  uint8_t id1=0x11;
  uint8_t id2=0x36;

};

void startAdv(float temp1)
{  
  int buffInt;
  Bluefruit.Advertising.clearData();  // refresh advertising data for each cycle
  Bluefruit.ScanResponse.clearData();  // refresh scan response data for each cycle
  SCNR_PACKET scnr_packet;
  ADV_PACKET advr_packet;
    if (temp1<0)
    {
    temp1*=-1;
    buffInt=temp1*100;
    advr_packet.singht=0x01;
    advr_packet.tempIce=reinterpret_cast<uint32_t*>(buffInt);
    }
    else
    {
    buffInt=temp1*100;
    advr_packet.singht=0x02;
    advr_packet.tempIce=reinterpret_cast<uint32_t*>(buffInt);
    }
    
    
    Serial.println();
    
  // Set Flag for discovery mode, optional 
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &advr_packet, sizeof(advr_packet));
  Bluefruit.ScanResponse.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &scnr_packet, sizeof(scnr_packet));
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);  // for XIAO BLE
//  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED);   // for mobile tool
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setIntervalMS(100, 100);    // in unit of ms
  Bluefruit.Advertising.setFastTimeout(1);          // number of seconds in fast mode
  Bluefruit.Advertising.start(1);                   // stop advertising after 1 seconds
}

// arrays to hold device address
BLEService es_svc=BLEService("23535ce8-f98a-4e38-9314-330b30f875f6");
BLECharacteristic es_chr=BLECharacteristic("939d9a1c-2a08-490c-a166-aedc851e5cd3");
void setup()
{
    Bluefruit.begin();
  Bluefruit.autoConnLed(true); // turn off LED to save power
  Bluefruit.setTxPower(6);    // Check bluefruit.h for supported values

  Serial.begin(9600);
  Serial.print("BLE activated");
  // ds.begin();                                             // инициализация

  // BLEinit();
  // startAdv();
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 12);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void loop()
{

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); 
  Serial.println("DONE");

  float tempC =   sensors.getTempCByIndex(0);        

  startAdv(tempC);
  __WFE();
  __WFI();
  delay(10000);  // advertising period = 10s

}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
void BLEinit(){
  Bluefruit.begin();
  Bluefruit.autoConnLed(true);
  // Bluefruit.setName("Temperature scan");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  Bluefruit.setTxPower(8);
  
  // bledis.setManufacturer("VLADISLAV ENGEERING");
  // bledis.setModel("NRF52840");
  // bledis.begin();
//   es_svc.begin();
//   es_chr.setProperties(CHR_PROPS_NOTIFY);
// es_chr.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
//   es_chr.begin();

}
