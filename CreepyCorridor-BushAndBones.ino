



// ***************************************
// ********** Global Variables ***********
// ***************************************

/*
 * NOTE: Even though this is named BushAndBones, The Bones portion has been changed
 *       to control the lighting effects of WLED in the Harvest Room. Lol.
 */

//Globals for Wifi Setup and OTA
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//WiFi Credentials
#ifndef STASSID
#define STASSID "your_ssid"
#endif
#ifndef STAPSK
#define STAPSK  "your_password"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;

//MQTT
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#ifndef AIO_SERVER
#define AIO_SERVER      "your_MQTT_server_address"
#endif
#ifndef AIO_SERVERPORT
#define AIO_SERVERPORT  0000 //Your MQTT port
#endif
#ifndef AIO_USERNAME
#define AIO_USERNAME    "your_MQTT_username"
#endif
#ifndef AIO_KEY
#define AIO_KEY         "your_MQTT_key"
#endif
#define MQTT_KEEP_ALIVE 150
unsigned long previousTime;

//Initialize and Subscribe to MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe creepyRustle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/creepy-corridor.bushrustle");

// Input/Output
#define bushRelay D3 //For some fucking reason, 5V relays use opposite HIGH/HIGH logic, just go with it
#define harvestModeRelay D5

//Variables
float mqttConnectFlag = 0.0;




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Relay Setup
  pinMode(bushRelay, OUTPUT);
  pinMode(harvestModeRelay, OUTPUT);
  digitalWrite(bushRelay, HIGH);
  digitalWrite(harvestModeRelay, HIGH);

  //Initialize Serial, WiFi, & OTA
  wifiSetup();

  //Initialize MQTT
  mqtt.subscribe(&creepyRustle);
  MQTT_connect();

  //Bush Test
  digitalWrite(bushRelay, LOW);
  delay(2300);
  digitalWrite(bushRelay, HIGH);

  Serial.println("Setup Complete!");

}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  //OTA & MQTT
  ArduinoOTA.handle();
  MQTT_connect();

  //Recieve MQTT
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(0.01))) {
    uint16_t value = atoi((char *)creepyRustle.lastread);

    if(value == 1){
       Serial.println("[Begin BushAndBones Performance]");

        //** CHANGE HARVEST TO ACTIVE MODE 
       delay(300);
       digitalWrite(harvestModeRelay, LOW);
       delay(250);    // Short Press
       digitalWrite(harvestModeRelay, HIGH);
       
       //** START BUSH RUSTLE
       delay(5600);
       digitalWrite(bushRelay, LOW);       

       //** CHANGE HARVEST TO PASSIVE MODE
       delay(10);
       digitalWrite(harvestModeRelay, LOW);
       delay(1000);   // Long Hold
       digitalWrite(harvestModeRelay, HIGH);
       
       //** STOP BUSH RUSTLE
       delay(4000);
       digitalWrite(bushRelay, HIGH);    

       Serial.println("[End Harvest Performance]");
    }
    delay(1);
  }
  delay(1);
}



  
// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void wifiSetup() {

  //Serial
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println();
  Serial.println("****************************************");
  Serial.println("Booting");

  //WiFi and OTA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("CreepyCorridor-BushAndBones");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void MQTT_connect() {

  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    if (mqttConnectFlag == 0) {
      //Serial.println("Connected");
      mqttConnectFlag++;
    }
    return;
  }
  Serial.println("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      //while (1);
      Serial.println("Wait 5 secomds to reconnect");
      delay(5000);
    }
  }
}
