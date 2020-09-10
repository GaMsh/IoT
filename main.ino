void loop() 
{
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    digitalWrite(BUILTIN_LED, HIGH);
    char incomingPacket[1024];
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    int len = udp.read(incomingPacket, 1024);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    parseCommand(incomingPacket);
    digitalWrite(BUILTIN_LED, LOW);
  }
  
  unsigned long currentMillis = millis();

  previousMillis = currentMillis;
  taskConfig(currentMillis);
  taskPing(currentMillis);
  taskRestart(currentMillis);
  
//  mainProcess();

//  if (currentMillis - previousMillisPing >= PING_INTERVAL) {
//    previousMillisPing = currentMillis;
//
//   
//    timeval tv;
//    struct tm* ptm;
//    char time_string[40];
//    
//    gettimeofday(&tv, nullptr);
//    ptm = localtime (&tv.tv_sec);
//    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
//
//    Serial.println(time_string);
//
//    
//    pingServer();
//  }

  if (currentMillis - previousMillisPing >= 5 * 60 * 1000) {
      WiFi.scanNetworks(true);
  }

  if (currentMillis - previousMillisReport >= REPORT_INTERVAL) {
    previousMillisReport = currentMillis;
    STATUS_REPORT_SEND = false;
  }

  if (!STATUS_REPORT_SEND) {
    //sending status report
    
    STATUS_REPORT_SEND = true;
  }

  int n = WiFi.scanComplete();
  if(n >= 0)
  {
    String wifiList = "";
    Serial.printf("%d network(s) found\n", n);
    for (int i = 0; i < n; i++)
    {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i+1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
      wifiList += String(WiFi.SSID(i).c_str()) + ":" + String(WiFi.channel(i)) + ":" + String(WiFi.RSSI(i)) + ":" + String(WiFi.encryptionType(i)) + ";";
    }
    callServer("W", "", wifiList);
    WiFi.scanDelete();
  }

//  tinyUPnP.updatePortMappings(600000);
}
