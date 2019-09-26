/* DHT and OLED - Simple Proof of Concept
 *
 * Example testing sketch for various DHT humidity/temperature sensors
 * and display values on a small OLED screen
 * 
 * Written by JSaaS, public domain
 *
 * Depends on Adafruit Arduino libraries:
 * https://github.com/adafruit/DHT-sensor-library
 * https://github.com/adafruit/Adafruit-GFX-Library
 * https://github.com/adafruit/Adafruit_SSD1306
 * 
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Display initiation
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

//Probe initiation
#define DHTPIN D6                               // what pin we're connected to

//Wifi initiation
#define wifi_ssid "YOURWIFISSID"                    //WiFi SSID
#define wifi_password "YOURWIFIPASSWORD"          //WiFi Password

//MQTT initiation
#define mqtt_server "YOURMQTTBROKERIP"             //Address to Mosquitto-server
#define mqtt_user "guest"                       //if exist
#define mqtt_password "guest"                   //idem

//MQTT topics
#define temperature_topic "Your/Temperature/Topic/On/MQTT"  //Topic temperature
#define humidity_topic "Your/Humidity/Topic/On/MQTT"        //Topic humidity


// Uncomment whatever type you're using!
//#define DHTTYPE DHT12   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

//Buffer to decode MQTT messages
char message_buff[100];
 
long lastMsg = 0;   
long lastRecu = 0;
bool debug = true;  //Display log message if True

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

//Initialize WiFi
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Serial.println("DHT22 test!");

  pinMode(2,INPUT);

  setup_wifi();                           //Connect to Wifi network
  client.setServer(mqtt_server, 1883);    // Configure MQTT connection
  client.setCallback(callback);           // callback function to execute when a MQTT message
  
  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  Serial.print("\nInitialized screen!");

  display.display();
 
  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(0);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello, \nworld!");
  display.display();
  delay(2000);
  display.clearDisplay();

}

//Connect to wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
 
  WiFi.begin(wifi_ssid, wifi_password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi OK ");
  Serial.print("=> ESP8266 IP address: ");
  Serial.print(WiFi.localIP());
  
}
 
//Reconnect if connection is lost
void reconnect() {
 
  while (!client.connected()) {
    Serial.print("\nConnecting to MQTT broker ...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, error : ");
      Serial.print(client.state());
      Serial.println(" Wait 5 secondes before to retry");
      delay(5000);
    }
  }
}

void loop() {
  //Check if we have a connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // Wait a few seconds between measurements.
  display.clearDisplay();
  
  display.setCursor(0,0);
  display.println(dht_readings());
  display.println(WiFi.localIP());
  display.display();
  delay(30000);
  
}
String dht_readings(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature()-1.5;
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Cannot find the DHT-sensor!");
    return "Cannot find the DHT-sensor!";
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  client.publish(temperature_topic, String(t).c_str(), true);   // Publish temperature on temperature_topic
  client.publish(humidity_topic, String(h).c_str(), true);      // and humidity

  String rtString = "Humidity: " + String(h) + "\nTemp:\n" + String(t);
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  delay(1000);
  return rtString;
}

// MQTT callback function
// D'après http://m2mio.tumblr.com/post/30048662088/a-simple-example-arduino-mqtt-m2mio
void callback(char* topic, byte* payload, unsigned int length) {
 
  int i = 0;
  if ( debug ) {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length,DEC));
  }
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
  
  if ( msgString == "ON" ) {
    digitalWrite(D2,HIGH);  
  } else {
    digitalWrite(D2,LOW);  
  }
}
