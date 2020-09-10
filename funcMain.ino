void taskConfig(int currentMillis) {
  if (currentMillis - previousMillisConfig > RECONFIG_INTERVAL) {
    getDeviceConfiguration(UPnP);
    previousMillisConfig = currentMillis;
  }
}

void taskPing(int currentMillis) {
  if (currentMillis - previousMillisPing >= PING_INTERVAL) {
    pingServer();
    previousMillisPing = currentMillis;
  }
}

void taskRestart(int currentMillis) {
  if (!CHIP_TEST) {
    if (currentMillis - previousMillisReboot > REBOOT_INTERVAL) {
      Serial.println("It`s time to reboot");
      if (!NO_INTERNET && !NO_SERVER) {
        ESP.restart();
      } else {
        Serial.println("But it`s impossible, no internet connection");
      }
    }
  }
}

void mainProcess() {
//  String string = "";
//    
//  if (NO_SERVER) {
//    bufferWrite("test", string);
//  } else {
//    callServer("D", String(TOKEN), string);
//  }
}

void actionDo(String urlString) {
  if (WiFi.status() == WL_CONNECTED) {
    analogWrite(LED_EXTERNAL, 3);
    callToServer(urlString);
    analogWrite(LED_EXTERNAL, LED_BRIGHT);
  } else {
    analogWrite(LED_EXTERNAL, LED_BRIGHT);
    writeLocalBuffer(urlString);
    analogWrite(LED_EXTERNAL, 3);
  }
}

boolean parseCommand(String incomingPacket) {
  String other = "";
  Serial.println(incomingPacket);

  String token = "";
  String command = "";
  String string = "";
  String data = "";

  token = getValue(incomingPacket, '_', 0);
  if (TOKEN != token) {
    return false;
  }
  
  other = getValue(incomingPacket, '_', 2);
  
  command = getValue(other, ':', 0);
  other = getValue(other, ':', 1);
  
  string = getValue(other, '|', 0);
  data = getValue(other, '|', 1);

  //////
  if (command == "RC") {
    Serial.println("Remote control");
    if (string == "IL") {
      Serial.println("Ticker set to " + data);  
      ticker2.attach_ms(data.toInt(), tickExternal, MAIN_MODE_NORMAL);
      callServer("RC", "IL", "1");
    }
  }
  //////
  
  return true;
}

boolean callToServer(String urlString) {
  if (NO_INTERNET) {
    NO_INTERNET = false;
    bufferReadAndSend("udp");
    return bufferReadAndSend("data");
  }
    
  Serial.println(urlString);
  
  HTTPClient http; 
  http.begin(OSMO_HTTP_SERVER_SEND, OsMoSSLFingerprint);
  http.setUserAgent(deviceName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(urlString);
  Serial.println("Sending to server...");
  if (httpCode != HTTP_CODE_OK) {
    NO_SERVER = true;
    bufferWrite("data", urlString);
    return false;
  }
  if (NO_SERVER) {
    NO_SERVER = false;
    bufferReadAndSend("data");
  }
  String payload = http.getString();
  Serial.print(String(httpCode) + ": ");
  Serial.println(payload);
  http.end();

  return true;
}

void pingServer() {
  pingCount++;
  callServer("P", "", "");
//  callServer("P", String(pingCount), String(WiFi.RSSI()));
  Serial.println("Ping");
}

void callServer(String command, String string, String data) {
  udp.beginPacket(OSMO_SERVER_HOST, OSMO_SERVER_PORT);
  String query = TOKEN + "__" + command;
  if (string != "") {
    query += ":" + string;
  }
  if (data != "") {
    query += "|" + data;
  }
  udp.print(query);
  udp.endPacket();
}

boolean writeLocalBuffer(String urlString) {
  if (!NO_INTERNET) {
    NO_INTERNET = true;
    Serial.println("NO INTERNET MODE ACTIVATED");
  }

  return bufferWrite("data", urlString);
}
