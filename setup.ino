void setup() 
{  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_EXTERNAL, OUTPUT);
  pinMode(RESET_WIFI, INPUT_PULLUP);

  Serial.begin(SERIAL_BAUD);
  while(!Serial) {}
    
  Serial.println("Device '" + deviceName + "' is starting...");
  Serial.println("Voltage: " + String(ESP.getVcc()));

  checkWiFiConfiguration();
  
  WiFi.hostname(deviceName);
  
  ticker1.attach_ms(100, tickInternal);
  ticker2.attach_ms(100, tickExternal, MAIN_MODE_OFFLINE);

  if (!setupWiFiManager()) {
    Serial.println("failed to connect and hit timeout");
    delay(15000);
    ESP.restart();
  } else {
    Serial.println("WiFi network connected (" + String(WiFi.RSSI()) + ")");
    NO_INTERNET = false;

    if (LittleFS.begin()) {
      Serial.println(F("LittleFS was mounted"));
    } else {
      Serial.println(F("Error while mounting LittleFS"));
    }

    ///// Config 
    String customSsl = readCfgFile("ssl");
    if (customSsl) {
      OsMoSSLFingerprint = customSsl;
    }

    int customLedBright = readCfgFile("led_bright").toInt();
    if (customLedBright > 0) {
      LED_BRIGHT = customLedBright;
    }

    int customLocalPort = readCfgFile("local_port").toInt();
    if (customLocalPort > 0) {
      LOCAL_PORT = customLocalPort;
    }
    ///// UDP
    udp.begin(LOCAL_PORT);
    
//    ///// UPnP
//    Serial.println("UPnP start");
//    boolean portMappingAdded = false;
//    tinyUPnP.addPortMappingConfig(WiFi.localIP(), LOCAL_PORT, RULE_PROTOCOL_UDP, 30000, deviceName);
//    while (!portMappingAdded) {
//      portMappingAdded = tinyUPnP.commitPortMappings();
//      if (portMappingAdded) {
//        UPnP = true;
//      }
//    }
//    Serial.println("UPnP done");

    ///// Final
    TOKEN = readCfgFile("token");
    callServer("I", "", "");
   
    ticker2.attach_ms(500, tickExternal, MAIN_MODE_OFFLINE);

//    getTimeFromInternet();

    getDeviceConfiguration(UPnP);

    tickOffAll();

    Wire.begin();
  }

  tickOffAll();

  // Завершаем инициализацию устройства, регулируем яркость светодиода по конфигу

  ticker1.attach_ms(2000, tickInternal);
//  ticker2.attach_ms(6000, tickExternal, MAIN_MODE_NORMAL);
}
