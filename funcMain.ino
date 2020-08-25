void taskRestart(int currentMillis, int previousMillisReboot) {
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

int taskConfig(int currentMillis, int previousMillisConfig) {
  if (currentMillis - previousMillisConfig > RECONFIG_INTERVAL) {
    getDeviceConfiguration(UPnP);
    return currentMillis;
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
  callServer("P", String(pingCount), String(WiFi.RSSI()));
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
