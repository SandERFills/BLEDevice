#include <bluefruit.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WARE_D 2
OneWire oneWire(ONE_WARE_D);              // вход датчиков 18b20, аналоговый А1, он же 15 цифровой 
DallasTemperature ds(&oneWire);    
DeviceAddress insideThermometer;
// BLEUart bleuart; // uart over ble
BLEDis bledis;
BLEService filterService("479d422d-69a0-4e39-b4a4-130f8cbb280c");
void startAdv(int temp);
void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void BLEinit();
void printTemperature(DeviceAddress deviceAddress);
void printAddress(DeviceAddress deviceAddress);
void addTempData(float =0.0 ,float =0.0);
#include <bluefruit.h>

struct ATTR_PACKET
{
  uint8_t   mfid=0xFF;
  uint32_t tempIce;
};
void startAdv(int temp1,uint8_t id)
{  
  ATTR_PACKET packet;
    packet.tempIce=temp1;
    Serial.println();
    
  Bluefruit.Advertising.addService(filterService);
  // Set Flag for discovery mode, optional 
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
  // Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &adv_data, sizeof(adv_data));
  Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &packet, sizeof(packet));
  // Bluefruit.ScanResponse.addData(0x22, &packet, sizeof(packet));
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
    if (ds.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  // Assign address manually. The addresses below will need to be changed
  // to valid device addresses on your bus. Device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  // Note that you will need to use your specific address here
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  if (!ds.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // show the addresses we found on the bus
  // Serial.print("Device 0 Address: ");
  // printAddress(insideThermometer);
  // Serial.println();

  // // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  // ds.setResolution(insideThermometer, 12);

  // Serial.print("Device 0 Resolution: ");
  // Serial.print(ds.getResolution(insideThermometer), DEC);
  // Serial.println();
}
// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  // method 1 - slower
  //Serial.print("Temp C: ");
  //Serial.print(sensors.getTempC(deviceAddress));
  //Serial.print(" Temp F: ");
  //Serial.print(sensors.getTempF(deviceAddress)); // Makes a second call to getTempC and then converts to Fahrenheit

  // method 2 - faster
  float tempC = ds.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
// void startAdv()
// {
//   Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
//   Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);

//   Bluefruit.Advertising.addTxPower();
//   Bluefruit.ScanResponse.addName();

//   Bluefruit.Advertising.setInterval(64, 244);    // in unit of 0.625 ms
//   Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
// // }
// void addTempData(float tempIceIn,float tempAirIn)
// {
  
//     Bluefruit.Advertising.clearData();
//     // BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA is 0xFF
//     Bluefruit.Advertising.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &temp_packet, sizeof(temp_packet));
//     Bluefruit.ScanResponse.clearData();
//     Bluefruit.ScanResponse.addData(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, &temp_packet, sizeof(temp_packet));
//   Bluefruit.Advertising.start(0);
// }

void loop()
{
  int x=65222;
  startAdv(x,1);
  __WFE();
  __WFI();
  delay(10000);  // advertising period = 10s
  Bluefruit.Advertising.clearData();  // refresh advertising data for each cycle
  Bluefruit.ScanResponse.clearData();  // refresh scan response data for each cycle

//   ds.requestTemperatures();                               // считываем температуру с датчиков, на это требуется 750мс
  
//   Serial.print("Sensor 0: ");
//  float tempIce =   ds.getTempCByIndex(0);        
//     addTempData(tempIce+1);                                          // отправляем температуру
// char tempChr[6];
//   snprintf(tempChr,6,"%f",tempIce);
//  printTemperature(insideThermometer); // Use a simple function to print out the data
//   if (Bluefruit.connected())
//   {
//     Serial.println("Connected,sending notification");
//     es_chr.notify(&tempChr,sizeof(tempChr));
//   }
//   else
//   {
//     Serial.println("Not connected");
//   }
    
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
