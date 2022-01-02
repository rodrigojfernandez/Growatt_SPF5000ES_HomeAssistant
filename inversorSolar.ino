// Based on Growatt Solar Inverter to MQTT
// Repo: https://github.com/nygma2004/growatt2mqtt
// author: Csongor Varga, csongor.varga@gmail.com
// 1 Phase, 2 string inverter version such as MIN 3000 TL-XE, MIC 1500 TL-X

// Libraries:
// - ModbusMaster by Doc Walker
// - SoftwareSerial
// Hardware:
// - Wemos D1 mini
// - RS485 to TTL converter: https://www.aliexpress.com/item/1005001621798947.html
// - Power from inverter usb port


// author: Rodrigo Fernandez, rjfernandez@outlook.com
// original code adapted to integrate device with home assistant
// 1 Phase, 1 string inverter, Growatt SPF5000ES

#include <SoftwareSerial.h>           // Leave the main serial line (USB) for debugging and flashing
#include <ModbusMaster.h>             // Modbus master library for ESP8266
#include <ESP8266WiFi.h>              // Wifi connection
#include <ESP8266WebServer.h>         // Web server for general HTTP response
#include <ESP8266HTTPUpdateServer.h>  // OTA library
#include <PubSubClient.h>             // MQTT support
#include <WiFiUdp.h>

#include "globals.h"
#include "settings.h"

os_timer_t myTimer;
WiFiClient espClient;
PubSubClient mqtt(espClient);
ESP8266WebServer server(2683);
ESP8266HTTPUpdateServer httpUpdater;
SoftwareSerial modbus(MAX485_RX, MAX485_TX, false); //RX, TX
ModbusMaster growatt;


void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void sendModbusError(uint8_t result) {
  String message = "";
  if (result==growatt.ku8MBIllegalFunction) {
    message = "Illegal function";
  }
  if (result==growatt.ku8MBIllegalDataAddress) {
    message = "Illegal data address";
  }
  if (result==growatt.ku8MBIllegalDataValue) {
    message = "Illegal data value";
  }
  if (result==growatt.ku8MBSlaveDeviceFailure) {
    message = "Slave device failure";
  }
  if (result==growatt.ku8MBInvalidSlaveID) {
    message = "Invalid slave ID";
  }
  if (result==growatt.ku8MBInvalidFunction) {
    message = "Invalid function";
  }
  if (result==growatt.ku8MBResponseTimedOut) {
    message = "Response timed out";
  }
  if (result==growatt.ku8MBInvalidCRC) {
    message = "Invalid CRC";
  }
  if (message=="") {
    message = result;
  }
  Serial.println(message);    
  char topic[80];
  char value[30];
  sprintf(topic,"%s/error",topicRoot);
  mqtt.publish(topic, message.c_str());
  delay(5);
}

void ReadRegisters() {
  char json[900];
  char topic[80];
  char value[10]; 

  
  uint8_t result;

  

  //ESP.wdtDisable();
  ESP.wdtFeed();
  result = growatt.readInputRegisters(setcounter*64,64);
  //ESP.wdtEnable(1);
  if (result == growatt.ku8MBSuccess)   {
    modbusdata.status = growatt.getResponseBuffer(0);
    modbusdata.realoppercent = growatt.getResponseBuffer(27);
    modbusdata.batsoc = growatt.getResponseBuffer(18);
    modbusdata.inputpower = ((growatt.getResponseBuffer(36) << 16) | growatt.getResponseBuffer(37))*0.1;
    modbusdata.ConstantPowerOKFlag = growatt.getResponseBuffer(47);
    modbusdata.gridvoltage = growatt.getResponseBuffer(20)*0.1;  
    modbusdata.gridfrequency = growatt.getResponseBuffer(21)*0.01;
    modbusdata.outputpower = ((growatt.getResponseBuffer(9) << 16) | growatt.getResponseBuffer(10))*0.1;
    modbusdata.invertercurrent = growatt.getResponseBuffer(35)*0.01;
    modbusdata.outputcurrent = growatt.getResponseBuffer(34)*0.1;
    modbusdata.opdcvolt = growatt.getResponseBuffer(24)*0.001;
    modbusdata.pv1voltage = growatt.getResponseBuffer(1)*0.1;
    modbusdata.pv1power = ((growatt.getResponseBuffer(3) << 16) | growatt.getResponseBuffer(4))*0.1;
    modbusdata.batchargepower = ((growatt.getResponseBuffer(69-64) << 16) | growatt.getResponseBuffer(70-64))*0.1;
    modbusdata.batchargecurrent = growatt.getResponseBuffer(68-64)*0.1;
    modbusdata.batdiscargepower = ((growatt.getResponseBuffer(73-64) << 16) | growatt.getResponseBuffer(74-64))*0.1;
    modbusdata.batpower = ((growatt.getResponseBuffer(77-64) << 16) | growatt.getResponseBuffer(78-64))*0.1;
    modbusdata.tempinverter = growatt.getResponseBuffer(25)*0.1;
    modbusdata.tempdcdc = growatt.getResponseBuffer(26)*0.1;
    modbusdata.BatOverCharge = growatt.getResponseBuffer(80-64);
    modbusdata.faultcode = growatt.getResponseBuffer(42);
    modbusdata.warningcode = growatt.getResponseBuffer(43);
    modbusdata.totalworktime = ((growatt.getResponseBuffer(30) << 16) | growatt.getResponseBuffer(31))*0.1;
    modbusdata.pv1current = ((growatt.getResponseBuffer(186-64) << 16) | growatt.getResponseBuffer(187-64))*0.1;
    modbusdata.InvFanSpeed = growatt.getResponseBuffer(82-64);
    modbusdata.MpptFanSpeed = growatt.getResponseBuffer(81-64);

    // Generate the modbus MQTT message
    sprintf(json,"{",json);
    char* temp;
    switch(modbusdata.status){
            case 0:
                temp="Standby";
            break;
            case 2:
                temp="Discharge";
            break;
            case 3:
                temp="Fault";
            break;
            case 4:
                temp="Flash";
            break;
            case 5:
                temp="PV_Charge";
            break;
            case 6:
                temp="AC_Charge";
            break;
            case 7:
                temp="Combine_Charge";
            break;
            case 8:
                temp="Combine_Charge_and_Bypass";
            break;
            case 9:
                temp="PV_Charge_and_Bypass";
            break;
            case 10:
                temp="AC_Charge_and_Bypass";
            break;
            case 11:
                temp="Bypass";
            break;
            case 12:
                temp="PV_Charge_and_Discarge";
            break;
        }
    char topic[80];
    sprintf(topic,"%s/estado",topicRoot);
    mqtt.publish(topic, temp);
    sprintf(json,"%s\"realopper\":%d,",json,modbusdata.realoppercent);
    sprintf(json,"%s\"batsoc\":%.1f,",json,modbusdata.batsoc);
    sprintf(json,"%s\"inputpow\":%.1f,",json,modbusdata.inputpower);
    switch(modbusdata.ConstantPowerOKFlag){
            case 0:
                temp="Not_OK;";
            break;
            default:
                temp="OK"; 
            break;
        }
    sprintf(json,"%s\"constpow\":\"%s\",",json,temp);
    sprintf(json,"%s\"gridvolt\":%.1f,",json,modbusdata.gridvoltage);
    sprintf(json,"%s\"gridfreq\":%.2f,",json,modbusdata.gridfrequency);
    sprintf(json,"%s\"outputpow\":%.1f,",json,modbusdata.outputpower);
    sprintf(json,"%s\"invertercurr\":%.1f,",json,modbusdata.invertercurrent);
    sprintf(json,"%s\"opcurr\":%.1f,",json,modbusdata.outputcurrent);
    sprintf(json,"%s\"opdcvolt\":%.1f,",json,modbusdata.opdcvolt);
    sprintf(json,"%s\"pv1volt\":%.1f,",json,modbusdata.pv1voltage);
    sprintf(json,"%s\"pv1pow\":%.1f,",json,modbusdata.pv1power);
    sprintf(json,"%s\"pv1curr\":%.1f,",json,modbusdata.pv1current);
    sprintf(json,"%s\"batchrgpow\":%.1f,",json,modbusdata.batchargepower);
    sprintf(json,"%s\"batchrgcurr\":%.1f,",json,modbusdata.batchargecurrent);
    sprintf(json,"%s\"batdischrgpow\":%.1f,",json,modbusdata.batdiscargepower);
    sprintf(json,"%s\"tempinverter\":%.1f,",json,modbusdata.tempinverter);
    sprintf(json,"%s\"tempdc\":%.1f,",json,modbusdata.tempdcdc);
    switch(modbusdata.BatOverCharge){
            case 1:
                temp="Over_Load";
            break;
            default:
                temp="ok"; 
            break;
        }
    sprintf(json,"%s\"BatOverCharge\":\"%s\",",json,temp);
    switch(modbusdata.faultcode){
            case 8:
                temp="Bat_Voltage_High";
            break;
            case 9:
                temp="Over_Temperature";
            break;
            case 10:
                temp="Over_Load";
            break;
            case 25:
                temp="MOV_Break";
            break;
            case 26:
                temp="Over_Current";
            break;
            case 27:
                temp="Li‐Bat_Over_Load";
            break;
            default:
                sprintf(temp, "%d", modbusdata.faultcode); 
            break;
        }
    sprintf(json,"%s \"faultcode\":\"%s\",",json,temp);
    switch(modbusdata.warningcode){
            case NULL:
                temp="0";
            break;
            default:
                sprintf(temp, "%d", modbusdata.warningcode); 
            break;
        }
    sprintf(json,"%s\"warncod\":\"%s\",",json,temp);
    sprintf(json,"%s\"InvFan\":%d,",json,modbusdata.InvFanSpeed);
    sprintf(json,"%s\"MpptFan\":%d,",json,modbusdata.MpptFanSpeed);
    
    //ESP.wdtDisable();
    ESP.wdtFeed();
    result = growatt.readHoldingRegisters(setcounter*64,64);
    //ESP.wdtEnable(1);
    if (result == growatt.ku8MBSuccess)   {    
        modbussettings.powerstate = growatt.getResponseBuffer(0) ;
        modbussettings.OutputConfig = growatt.getResponseBuffer(1) ;
        modbussettings.ChargeConfig = growatt.getResponseBuffer(2) ;
        modbussettings.UtiOutStart = growatt.getResponseBuffer(3) ;
        modbussettings.UtiOutEnd = growatt.getResponseBuffer(4) ;
        modbussettings.UtiChargeStart = growatt.getResponseBuffer(5) ;
        modbussettings.UtiChargeEnd = growatt.getResponseBuffer(6) ;
        modbussettings.OutputVoltType = growatt.getResponseBuffer(18) ;
        modbussettings.OutputFreqType = growatt.getResponseBuffer(19) ;
        modbussettings.OverLoadRestart = growatt.getResponseBuffer(20) ;
        modbussettings.OverTempRestart = growatt.getResponseBuffer(21) ;
        modbussettings.BuzzerEN = growatt.getResponseBuffer(22) ;
        modbussettings.MaxChargeCurr = growatt.getResponseBuffer(34) ;
        modbussettings.batteryType = growatt.getResponseBuffer(39) ;
        modbussettings.BulkChargeVolt = growatt.getResponseBuffer(35)*0.1;
        modbussettings.FloatChargeVolt = growatt.getResponseBuffer(36)*0.1;
        modbussettings.BatLowToUtiVolt = growatt.getResponseBuffer(37)*0.1;
        modbussettings.FloatChargeCurr = growatt.getResponseBuffer(38);
        //strncpy(modbussettings.firmware,"      ",6);
        modbussettings.firmware[0] = growatt.getResponseBuffer(9) >> 8;
        modbussettings.firmware[1] = growatt.getResponseBuffer(9) & 0xff;
        modbussettings.firmware[2] = growatt.getResponseBuffer(10) >> 8;
        modbussettings.firmware[3] = growatt.getResponseBuffer(10) & 0xff;
        modbussettings.firmware[4] = growatt.getResponseBuffer(11) >> 8;
        modbussettings.firmware[5] = growatt.getResponseBuffer(11) & 0xff;
        //modbussettings.firmware[strlen(modbussettings.firmware)] = '\0';
        
        //strncpy(modbussettings.controlfirmware,"      ",6);
        modbussettings.controlfirmware[0] = growatt.getResponseBuffer(12) >> 8;
        modbussettings.controlfirmware[1] = growatt.getResponseBuffer(12) & 0xff;
        modbussettings.controlfirmware[2] = growatt.getResponseBuffer(13) >> 8;
        modbussettings.controlfirmware[3] = growatt.getResponseBuffer(13) & 0xff;
        modbussettings.controlfirmware[4] = growatt.getResponseBuffer(14) >> 8;
        modbussettings.controlfirmware[5] = growatt.getResponseBuffer(14) & 0xff;
        //modbussettings.controlfirmware[strlen(modbussettings.controlfirmware)-1] = '\0';*/
        
        char* temp;
        switch(modbussettings.powerstate){
            case 0:
                temp="Standby off, Output enable";
            break;
            case 1:
                temp="Standby on, Output enable";
            break;
            case 4:
                temp="Standby off, Output disable";
            break;
            case 5:
                temp="Standby on, Output disable";
            break;
        }
        sprintf(json,"%s\"powerstate\":\"%s\",",json,temp);
        switch(modbussettings.OutputConfig){
            case 0:
                temp="BAT_First";
            break;
            case 1:
                temp="PV_First";
            break;
            case 2:
                temp="UTI_First";
            break;
        }
        sprintf(json,"%s\"OPConfig\":\"%s\",",json,temp);
        switch(modbussettings.ChargeConfig){
            case 0:
                temp="PV_first";
            break;
            case 1:
                temp="PV&UTI";
            break;
            case 2:
                temp="PV_Only";
            break;
        }
        sprintf(json,"%s\"ChrgConfig\":\"%s\",",json,temp);
        sprintf(json,"%s\"UtiOutStart\":%d,",json,modbussettings.UtiOutStart);
        sprintf(json,"%s\"UtiOutEnd\":%d,",json,modbussettings.UtiOutEnd);
        sprintf(json,"%s\"UtiChrgStart\":%d,",json,modbussettings.UtiChargeStart);
        sprintf(json,"%s\"UtiChrgEnd\":%d,",json,modbussettings.UtiChargeEnd);
        switch(modbussettings.OutputVoltType){
            case 0:
                temp="208VAC";
            break;
            case 1:
                temp="230VAC";
            break;
            case 2:
                temp="240VAC";
            break;
        }
        sprintf(json,"%s\"opVoltType\":\"%s\",",json,temp);
        switch(modbussettings.OutputFreqType){
            case 0:
                temp="50Hz";
            break;
            case 1:
                temp="60Hz";
            break;
        }
        sprintf(json,"%s\"opFreqType\":\"%s\",",json,temp);
        switch(modbussettings.OverLoadRestart){
            case 0:
                temp="Yes";
            break;
            case 1:
                temp="No";
            break;
            case 2:
                temp="Swith_to_UTI";
            break;
        }
        sprintf(json,"%s\"OverLoadRestart\":\"%s\",",json,temp);
        switch(modbussettings.OverTempRestart){
            case 0:
                temp="Yes";
            break;
            case 1:
                temp="No";
            break;
        }
        sprintf(json,"%s\"OverTempRestart\":\"%s\",",json,temp);
        switch(modbussettings.BuzzerEN){
            case 1:
                temp="Enable";
            break;
            case 0:
                temp="Disable";
            break;
        }
        sprintf(json,"%s\"BuzzerEN\":\"%s\",",json,temp);
        sprintf(json,"%s\"MaxChrgCurr\":%d,",json,modbussettings.MaxChargeCurr);
        switch(modbussettings.batteryType){
            case 0:
                temp="Lead_Acid";
            break;
            case 1:
                temp="Lithium";
            break;
            case 2:
                temp="CustomLead_Acid";
            break;
        }
        sprintf(json,"%s\"batType\":\"%s\",",json,temp);
        sprintf(json,"%s\"BulkChrgVolt\":%.1f,",json,modbussettings.BulkChargeVolt);
        sprintf(json,"%s\"FloatChrgVolt\":%.1f,",json,modbussettings.FloatChargeVolt);
        sprintf(json,"%s\"BatLowToUtiVolt\":%.1f,",json,modbussettings.BatLowToUtiVolt);
        sprintf(json,"%s\"FloatChrgCurr\":%.1f,",json,modbussettings.FloatChargeCurr);
        if(strlen(modbussettings.controlfirmware)>6)
          { for(byte i=6;i<strlen(modbussettings.controlfirmware);i++) { modbussettings.controlfirmware[i]='\0' ; }
          }
        sprintf(json,"%s\"mpptfirm\":\"%s\",",json,modbussettings.controlfirmware);
        if(strlen(modbussettings.firmware)>6)
          { for(byte i=6;i<strlen(modbussettings.firmware);i++) { modbussettings.firmware[i]='\0' ; }
          }
        sprintf(json,"%s\"firm\":\"%s\"}",json,modbussettings.firmware);
        
        
    }
    else {
    

    Serial.print(F("Error: "));
    sendModbusError(result);
    }
    sprintf(topic,"%s/data",topicRoot);
    mqtt.publish(topic, json);
    //Serial.println();Serial.print("Topic: "); Serial.print(topic); Serial.println("--------------------");
    //Serial.println();Serial.print("JSON: "); Serial.print(json); Serial.println("--------------------"); 
  } else {
    

    Serial.print(F("Error: "));
    sendModbusError(result);
  }
  



    
}



// This is the 1 second timer callback function
void timerCallback(void *pArg) {

  
  seconds++;

  // Query the modbus device 
  if (seconds % UPDATE_MODBUS==0) {
    ReadRegisters();
  }

  // Send RSSI and uptime status
  if (seconds % UPDATE_STATUS==0) {
    // Send MQTT update
    if (mqtt_server!="") {
      char topic[80];
      char value[300];
      sprintf(value,"{\"rssi\": %d, \"uptime\": %d, \"ssid\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"clientid\":\"%s\", \"version\":\"%s\"}",WiFi.RSSI(),uptime,WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],clientID,buildversion);
      sprintf(topic,"%s/%s",topicRoot,"status");
      mqtt.publish(topic, value);
      Serial.println(F("MQTT status sent"));
    }
  }


}

// MQTT reconnect logic
void reconnect() {
  //String mytopic;
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    byte mac[6];                     // the MAC address of your Wifi shield
    WiFi.macAddress(mac);
    Serial.print(F("Client ID: "));
    Serial.println(newclientid);
    // Attempt to connect
    if (mqtt.connect(clientID, mqtt_user, mqtt_password)) {
      Serial.println(F("connected"));
      // ... and resubscribe
      char topic[80];
      sprintf(topic,"%swrite/#",topicRoot);
      mqtt.subscribe(topic);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqtt.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  subscribirMQTT();
}

void subscribirMQTT()
{ mqtt.subscribe("as/inversor/configuracion/OutputConfig");
  mqtt.subscribe("as/inversor/configuracion/ChargeConfig");
  mqtt.subscribe("as/inversor/configuracion/UtiOutStart");
  mqtt.subscribe("as/inversor/configuracion/UtiOutEnd");
  mqtt.subscribe("as/inversor/configuracion/UtiChargeStart");
  mqtt.subscribe("as/inversor/configuracion/UtiChargeEnd");
  mqtt.subscribe("as/inversor/configuracion/OverLoadRestart");
  mqtt.subscribe("as/inversor/configuracion/OverTempRestart");
  mqtt.subscribe("as/inversor/configuracion/BuzzerEN");
  mqtt.subscribe("as/inversor/configuracion/MaxChargeCurr");
  mqtt.subscribe("as/inversor/configuracion/BulkChargeVolt");
  mqtt.subscribe("as/inversor/configuracion/FloatChargeVolt");
  mqtt.subscribe("as/inversor/configuracion/BatLowToUtiVolt");
  mqtt.subscribe("as/inversor/configuracion/FloatChargeCurr");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  String mytopic = (char*)topic;
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;
  if(mytopic=="as/inversor/configuracion/OutputConfig"){
    if(message=="UTI_First"){
      growatt.writeSingleRegister(1, 2);
    }else{
      if(message=="BAT_First"){
        growatt.writeSingleRegister(1, 0);
      }else{
        growatt.writeSingleRegister(1, 1); //PV_First;
      }
    }
  }else{
    if(mytopic=="as/inversor/configuracion/ChargeConfig"){
      if(message=="PV_first"){
        growatt.writeSingleRegister(2, 0);
      }else{
        if(message=="PV&UTI"){
          growatt.writeSingleRegister(2, 1);
        }else{
          growatt.writeSingleRegister(2, 2); //PV_Only;;
        }
      }
    }else{
      if(mytopic=="as/inversor/configuracion/UtiOutStart"){
        growatt.writeSingleRegister(3, message.toInt());
      }else{
        if(mytopic=="as/inversor/configuracion/UtiOutEnd"){
          growatt.writeSingleRegister(4, message.toInt());  
        }else{
          if(mytopic=="as/inversor/configuracion/UtiChargeStart"){
            growatt.writeSingleRegister(5, message.toInt());
          }else{
            if(mytopic=="as/inversor/configuracion/UtiChargeEnd"){
              growatt.writeSingleRegister(6, message.toInt());
            }else{
              if(mytopic=="as/inversor/configuracion/OverLoadRestart"){
                if(message=="Yes"){
                  growatt.writeSingleRegister(20, 0);
                }else{
                  if(message=="No"){
                    growatt.writeSingleRegister(20, 1);
                  }else{
                    growatt.writeSingleRegister(20, 2); //Swith_to_UTI;
                  }
                }
              }else{
                if(mytopic=="as/inversor/configuracion/OverTempRestart"){
                  if(message=="Yes"){
                  growatt.writeSingleRegister(21, 0);
                  }else{
                    growatt.writeSingleRegister(21, 1);
                  }
                }else{
                  if(mytopic=="as/inversor/configuracion/BuzzerEN"){
                    if(message=="Enable"){
                      growatt.writeSingleRegister(22, 1);
                    }else{
                       growatt.writeSingleRegister(22, 0); 
                    }
                  }else{
                    if(mytopic=="as/inversor/configuracion/MaxChargeCurr"){
                      growatt.writeSingleRegister(34, message.toInt());
                    }else{
                      if(mytopic=="as/inversor/configuracion/BulkChargeVolt"){
                        growatt.writeSingleRegister(35, message.toInt());
                      }else{
                        if(mytopic=="as/inversor/configuracion/FloatChargeVolt"){
                          growatt.writeSingleRegister(36, message.toInt());  
                        }else{
                          if(mytopic=="as/inversor/configuracion/BatLowToUtiVolt"){
                            growatt.writeSingleRegister(37, message.toInt());
                          }else{
                            if(mytopic=="as/inversor/configuracion/FloatChargeCurr"){
                              growatt.writeSingleRegister(38, message.toInt());
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void handleNotFound(){
  String message = "Pagina no encontrada\n\n";
  message += "VE\n";
  message += "VO\n";
  message += "VE";
  server.send(404, "text/plain", message);
}

void setup() {

  Serial.begin(SERIAL_RATE);
  Serial.println(F("\nGrowatt Solar Inverter to MQTT Gateway"));
  // Init outputs, RS485 in receive mode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Initialize some variables
  uptime = 0;
  seconds = 0;

  // Connect to Wifi
  Serial.print(F("Connecting to Wifi"));
  WiFi.mode(WIFI_STA);

  #ifdef FIXEDIP
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet,dns)) {
    Serial.println("STA Failed to configure");
  }
  #endif
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    seconds++;
    if (seconds>180) {
      // reboot the ESP if cannot connect to wifi
      ESP.restart();
    }
  }
  seconds = 0;
  Serial.println("");
  Serial.println(F("Connected to wifi network"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Signal [RSSI]: "));
  Serial.println(WiFi.RSSI());

  // Set up the Modbus line
  growatt.begin(SLAVE_ID , modbus);
  // Callbacks allow us to configure the RS485 transceiver correctly
  growatt.preTransmission(preTransmission);
  growatt.postTransmission(postTransmission);
  Serial.println("Modbus connection is set up");

  // Create the 1 second timer interrupt
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, 1000, true);

  // Set up the MQTT server connection
  if (mqtt_server!="") {
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setBufferSize(1024);
    mqtt.setCallback(callback);
  }
  subscribirMQTT();
  httpUpdater.setup(&server, OTAPath, OTAUser, OTAPass);
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
  
  modbus.begin(MODBUS_RATE);
}

void loop() {
  // Handle HTTP server requests
  server.handleClient();
  
  // Handle MQTT connection/reconnection
  if (mqtt_server!="") {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
  }

  // Uptime calculation
  if (millis() - lastTick >= 60000) {            
    lastTick = millis();            
    uptime++;            
  }    

  if (millis() - lastWifiCheck >= WIFICHECK) {
    // reconnect to the wifi network if connection is lost
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to wifi...");
      WiFi.reconnect();
    }
    lastWifiCheck = millis();
  }
}
