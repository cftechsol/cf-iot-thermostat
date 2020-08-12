/**
 * CF Wi-Fi Thermostat.
 * 
 * IoT - Configurable thermostat using ESP8266 (ESP01 or NodeMCU 1.0 ESP-12E), DHT (11 or 22) and ThingsBoard as server.
 * 
 * @author  Caio Frota <contact@cftechsol.com>
 * @version 1.0.0
 * @since   August 10, 2020
 */

// Libraries.
#include <DHT.h>                                                                // DHT.
#include <WiFiManager.h>                                                        // Wi-Fi Manager.
#include <ArduinoJson.h>                                                        // Arduino Json.
#include <ThingsBoard.h>                                                        // Things Board.

boolean debug = true;

// DHT Sensor Config.
//#define DHT0TYPE         DHT11                                                  // Type of sensor. DHT11.
#define DHT0TYPE         DHT22                                                  // Type of sensor. DHT22.
#define DHT0PIN          2                                                      // DHT Pin using ESP01 Module.
//#define DHT0PIN          D2                                                     // DHT Pin using NodeMCU ESP-12E Module.

// DHT.
DHT dht0(DHT0PIN, DHT0TYPE);                                                    // DHT Object.

// Wi-Fi.
WiFiClient wifiClient;                                                          // Wi-Fi Client.
WiFiServer server(80);                                                          // Wi-Fi Server.

// Wi-Fi Manager.
WiFiManager wifiManager;                                                        // Wi-Fi Manager.

// Wi-Fi Manager custom parameters.
bool saveWiFiCustomParametersFlag = false;                                      // Flag that saves custom Wi-Fi Manager custom parameters.
char deviceName[50] = "";                                                       // Device Name.
char serverURL[50] = "";                                                        // Server URL.
char token[30] = "";                                                            // Token.
WiFiManagerParameter pDeviceName("devicename", "Device Name", serverURL, 50);   // Wi-Fi Manager Server URL object.
WiFiManagerParameter pServerURL("serverurl", "Server URL", serverURL, 50);      // Wi-Fi Manager Server URL object.
WiFiManagerParameter pToken("token", "Token", token, 30);                       // Wi-Fi Manager Token object.

// Things Board.
#define TBTTS           1000                                                    // Things Board minimum frequency to send data. Default: 1000.
#define TBTTR           5000                                                    // Things Board minimum time to wait to retry connecting in case of fail. Default: 5000.
ThingsBoard tb(wifiClient);                                                     // Things Board object.
unsigned long tbLastSend = 0;                                                   // Variable that saves the last time that a data was sent to server.

/**
 * TBData Struct to store data collected from sensors.
 */
struct TBData {
  struct DHTData {
    float temperature;
    float humidity;
  };
  DHTData dht0;
};
TBData data;

/**
 * Initial setup.
 */
void setup() {
  Serial.begin(9600);                                                           // Start Serial Monitor.
  dht0.begin();                                                                 // Start DHT.

  // Wi-Fi Manager - Reset settings
  //SPIFFS.format();                                                              // Format SPIFFS. Uncomment this line to format SPIFFS at boot.
  //wifiManager.resetSettings();                                                  // Reset config. Uncomment this line to reset settings at boot.

  // Wi-Fi Manager custom parameters - Start.
  wifiManager.setDebugOutput(debug);                                            // Set if it's in debug mode.
  loadWiFiCustomParameters();                                                   // Load Wi-Fi custom parameters.
  wifiManager.setSaveConfigCallback(saveWiFiParametersCallback);                // Set callback to save Wi-Fi Manager custom params.
  wifiManager.addParameter(&pDeviceName);                                       // Add Device Name param.
  wifiManager.addParameter(&pServerURL);                                        // Add Server URL param.
  wifiManager.addParameter(&pToken);                                            // Add Token param.

  // Wi-Fi Manager starting.
  wifiManager.autoConnect();                                                    // Start auto connect.

  // Wi-Fi Manager custom parameters - Save.
  saveWiFiCustomParameters();                                                   // Start custom params.

  // Server.
  server.begin();                                                               // Start server.
}

/**
 * Loop to run repeatedly.
 */
void loop() {
  wifiManagerServerListener();
  thingsBoardListener();
  delay(1000);
}

/**
 * Get Humidity and Temperature from DHT Sensor.
 * 
 * @param *data TBData Struct to store data collected form sensor.
 * @return boolean true if no errors.
 * it could get data from sensor.
 */
boolean getDHT0Data(TBData *data) {
  data->dht0.temperature = dht0.readTemperature();
  data->dht0.humidity = dht0.readHumidity();
  if (isnan(data->dht0.humidity) || isnan(data->dht0.temperature)) {
    serialPrintLn("Failed to read from DHT sensor!");
    return false;
  }
  return true;
}

/**
 * Things Board send thermostat data.
 */
void thingsBoardSendData(TBData *data) {
  if (millis() - tbLastSend > 1000) {
    serialPrintLn("Sending data to server...");
    if (getDHT0Data(data)) {
      serialPrintLn("Humidity: " + String(data->dht0.humidity) + "%");
      serialPrintLn("Temperature: " + String(data->dht0.temperature) + "c");
      tb.sendTelemetryFloat("humidity", data->dht0.humidity);
      tb.sendTelemetryFloat("temperature", data->dht0.temperature);
    }
    tbLastSend = millis();
  }
  tb.loop();
}

/**
 * Things Board listener that check connection with server and connect if it's down.
 */
void thingsBoardListener() {
  if (!tb.connected()) {
    serialPrint("Connecting to ThingsBoard node... ");
    if (tb.connect(serverURL, token)) {
      serialPrintLn("Ok.");
    } else {
      serialPrintLn("Fail... ");
      serialPrintLn("Retrying in " + String(TBTTR/1000) + " seconds.");
      delay(TBTTR);
      return;
    }
  }
  thingsBoardSendData(&data);
}

/**
 * Wi-Fi Manager server listener that prints a page with device information and reset button.
 */
void wifiManagerServerListener() {
  String header;                                                                // Variable to store the HTTP request.
  WiFiClient client = server.available();                                       // Listen for incoming clients.
  if (client) {                                                                 // True if a new client connects.
    serialPrintLn("New Client.");
    String currentLine = "";                                                    // Make a String to hold incoming data from the client.
    while (client.connected()) {                                                // Loop while the client is connected.
      if (client.available()) {                                                 // If there's bytes to read from the client,
        char c = client.read();                                                 // read a byte, then
        Serial.write(c);                                                        // print it out the serial monitor.
        header += c;
        if (c == '\n') {                                                        // If the byte is a newline character.
          if (currentLine.length() == 0) {
            // If the current line is blank, you got two newline characters in a row.
            // That's the end of the client HTTP request, so send a response.
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Actions.
            if (header.indexOf("GET /reset") >= 0) {
              serialPrintLn("Reseting device.");
              wifiManager.resetSettings();                                      // Reset Wi-Fi Manager settings.
              ESP.restart();                                                    // Restart device.
            }

            // Display the HHTML web page.
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Device " + String(ESP.getChipId()) + "</title>");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Style.
            client.println("<style>html { font-family: Verdana; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("button { border:0; border-radius:0.3rem; background-color:#1fa3ec; color:#fff; line-height:2.4rem; font-size:1.2rem; width: 100%; }");
            client.println("</style></head>");
            // Web Page Heading.
            client.println("<body><div style='text-align:left;display:inline-block;min-width:260px;'>");
            client.println("<h1>Device " + String(ESP.getChipId()) + "</h1>");
            // Display Wi-Fi Manager custom parameters.
            client.println("<p><b>Device Name:</b> " + String(deviceName) + "</p>");
            client.println("<p><b>Server URL:</b> " + String(serverURL) + "</p>");
            client.println("<p><b>Token:</b> " + String(token) + "</p>");
            // Display reset button.
            client.println("<p><a href=\"/reset\" onclick=\"return confirm('Are you sure you want to reset this device?')\"><button>RESET DEVICE</button></a></p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line.
            client.println();

            // Break out of the while loop.
            break;
          } else {                                                              // If you got a newline, then clear currentLine.
            currentLine = "";
          }
        } else if (c != '\r') {                                                 // If you got anything else but a carriage return character,
          currentLine += c;                                                     // add it to the end of the currentLine.
        }
      }
    }
    // Clear the header variable.
    header = "";
    // Close the connection.
    client.stop();
    serialPrintLn("Client disconnected.");
  }
}

/**
 * Load Wi-Fi Manager custom parameters.
 */
void loadWiFiCustomParameters() {
  serialPrint("Mount FS... ");
  if (SPIFFS.begin()) {
    serialPrintLn("Ok.");
    serialPrintLn("Loading Wi-Fi custom parameters.");

    // Open file for reading.
    String filePath = "/config.json";
    if (SPIFFS.exists(filePath)) {
      serialPrint("Open file for reading... ");
      File file = SPIFFS.open(filePath, "r");
      if (file) {
        serialPrintLn("Ok.");
        
        // Create Json objects.
        DynamicJsonDocument doc(1024);

        // Deserialize file into Json object.
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
          serialPrint("Fail to deserialize file. Code: ");
          serialPrintLn(error.c_str());
        }

        // Set custom parameters.
        strlcpy(deviceName, doc["devicename"], sizeof(deviceName));
        strlcpy(serverURL, doc["serverurl"], sizeof(serverURL));
        strlcpy(token, doc["token"], sizeof(token));

        // Close file.
        file.close();
      } else {
        serialPrintLn("Fail.");
      }
    }
  } else {
    serialPrintLn("Fail.");
  }
}

/**
 * Save Wi-Fi Manager custom parameters.
 */
void saveWiFiCustomParameters() {
  if (saveWiFiCustomParametersFlag) {
    // Load values from Wi-Fi Manager form.
    strlcpy(deviceName, pDeviceName.getValue(), sizeof(serverURL));
    strlcpy(serverURL, pServerURL.getValue(), sizeof(serverURL));
    strlcpy(token, pToken.getValue(), sizeof(token));
    
    serialPrintLn("Saving Wi-Fi custom parameters.");

    // Open file for writing.
    String filePath = "/config.json";
    serialPrint("Open file for writing... ");
    File file = SPIFFS.open(filePath, "w");
    if (file) {
      serialPrintLn("Ok.");

      // Create Json objects.
      DynamicJsonDocument doc(1024);

      // Set custom pameters.
      doc["devicename"] = deviceName;
      doc["serverurl"] = serverURL;
      doc["token"] = token;

      if (serializeJson(doc, file) == 0) {
        serialPrintLn("Fail to serialize file.");
      }
      
      // Close file.
      file.close();
    } else {
      serialPrintLn("Fail.");
    }
  }
}

/** 
 * Callback notifies you that it needs to save Wi-Fi Manager custom parameters. 
 */
void saveWiFiParametersCallback() {
  serialPrintLn("Should save params.");
  saveWiFiCustomParametersFlag = true;
}

/**
 * Calls Serial.println if it's in debug mode.
 * 
 * @param text Text.
 */
template <typename Generic>
void serialPrintLn(Generic text) {
  if (debug) {
    Serial.println(text);
  }
}

/**
 * Calls Serial.print if it's in debug mode.
 * 
 * @param text Text.
 */
template <typename Generic>
void serialPrint(Generic text) {
  if (debug) {
    Serial.print(text);
  }
}
