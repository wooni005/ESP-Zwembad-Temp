#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>

const char* sensorTempTopicWater	= "huis/ESP/Temp-Zwembad-Water/temp";
const char* sensorTempTopicRuimte = "huis/ESP/Temp-Zwembad-Ruimte/temp";
const char* sensorTempTopicKast	 = "huis/ESP/Temp-Zwembad-Kast/temp";
const char* strClientID = "ESP-Temp-Zwembad";
long lastMsgTime = 1000; //[ms] Offset in time that not all sensors are reporting at same time

#define DEBUG		1	// set to 1 to display each loop() run and PIR trigger

// Update these with values suitable for your network.
const char* SSID        = "WifiSSID";
const char* PASSWORD    = "WifiPassword";
const char* MQTT_SERVER = "192.168.5.248";

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2 // GPIO2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char msg[50];
int value = 0;
float tempWater = 0;
float tempRuimte = 0;
float tempKast = 0;
char data[80];

void setup_wifi() {

	delay(10);
	// We start by connecting to a WiFi network
#if DEBUG
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);
#endif

	WiFi.begin(SSID, PASSWORD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
#if DEBUG
		Serial.print(".");
#endif
	}

#if DEBUG
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
#endif
}

void publish_temp() {
		//sensors.setResolution(12);
		sensors.requestTemperatures(); // Send the command to get temperatures

		tempWater = sensors.getTempCByIndex(1);
#if DEBUG
		Serial.print("tempWater is: ");
		Serial.println(tempWater);
#endif
		if((tempWater >= -55) && (tempWater <= 125)){
			String payload = "{\"Temperature\":";
			payload += String(tempWater).c_str();
			payload += "}";
			payload.toCharArray(data, (payload.length() + 1));
			//mqttClient.publish("message", data);
			mqttClient.publish(sensorTempTopicWater, data, true);
			//mqttClient.publish(sensorTempTopic, String(temp).c_str(), true);
		}

		tempRuimte = sensors.getTempCByIndex(0);
#if DEBUG
		Serial.print("tempRuimte is: ");
		Serial.println(tempRuimte);
#endif
		if((tempRuimte >= -55) && (tempRuimte <= 125)){
			String payload = "{\"Temperature\":";
			payload += String(tempRuimte).c_str();
			payload += "}";
			payload.toCharArray(data, (payload.length() + 1));
			//mqttClient.publish("message", data);
			mqttClient.publish(sensorTempTopicRuimte, data, true);
			//mqttClient.publish(sensorTempTopic, String(temp).c_str(), true);
		}

		tempKast = sensors.getTempCByIndex(2);
#if DEBUG
		Serial.print("tempKast is: ");
		Serial.println(tempKast);
#endif
		if((tempKast >= -55) && (tempKast <= 125)){
			String payload = "{\"Temperature\":";
			payload += String(tempKast).c_str();
			payload += "}";
			payload.toCharArray(data, (payload.length() + 1));
			//mqttClient.publish("message", data);
			mqttClient.publish(sensorTempTopicKast, data, true);
			//mqttClient.publish(sensorTempTopic, String(temp).c_str(), true);
		}
}

void setup() {
#if DEBUG
	Serial.begin(115200);
	Serial.print("[");
	Serial.print(strClientID);
	Serial.println("]");
#endif

	setup_wifi();
	mqttClient.setServer(MQTT_SERVER, 1883);

	// Start up the library
	sensors.begin();
}

void reconnect() {
	// Loop until we're reconnected
	while (!mqttClient.connected()) {
#if DEBUG
		Serial.print("Attempting MQTT connection...");
#endif
		// Attempt to connect
		if (mqttClient.connect(strClientID)) {
#if DEBUG
			Serial.println("connected");
#endif
			// Once connected, publish temp
			publish_temp();
		} else {
#if DEBUG
			Serial.print("failed, rc=");
			Serial.print(mqttClient.state());
			Serial.println(" try again in 5 seconds");
#endif
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void loop() {
	delay(100);
	if (!mqttClient.connected()) {
		reconnect();
	}
	mqttClient.loop();

	long now = millis();
	if (now - lastMsgTime > 180000) {
		lastMsgTime = now;
		publish_temp();
	}
}
