#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266WebServer.h>   // Include Webserver 
#include <PubSubClient.h>       // Include Lib Client MQTT
#include "DHT.h"                // Include Lib Sensor DHT

//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2321

const char* ssid     = "Mama adelia";  // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "uu311009";     // The password of the Wi-Fi network

// MQTT Broker Setting
const char *mqtt_broker = "mqtt.telkomiot.id";
const char *topic_pubs = "v2.0/pubs/APP6356c38bde5a779890/DEV63d2c0fabbbd249223";
const char *topic_subs = "v2.0/subs/APP6356c38bde5a779890/DEV63d2c0fabbbd249223";
const char *mqtt_username = "b7cbbccae0abefc7";
const char *mqtt_password = "cfe6c1aebfec3fdf";
const int mqtt_port = 1883;
int device_read  = 1;

WiFiClient espClient;
PubSubClient client(espClient);

// Run web server on port 80
ESP8266WebServer server(80);

// DHT Sensor listening PIN on board ESP8266
uint8_t DHTPin = D5; 
               
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);                

float Temperature;
float Humidity;

void setup() {
  Serial.begin(115200);                           // Start the Serial communication to send messages to the computer
  delay(500);                                     // Set delay 0.5 Detik untuk cek suhu pada sendor    
  
  pinMode(DHTPin, INPUT);                         // Set PIN Sensor pada board ESP8266
  
  dht.begin();                                    // Start device sensor

  WiFi.begin(ssid, password);                     // Connect to the network wifi
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {          // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());                  // Send the IP address of the ESP8266 to the computer

  server.on("/", handle_OnConnect);                // call function handle_onConnect jika webserver ESP8266 on
  server.onNotFound(handle_NotFound);              // call function handle_notFound webserver ESP8266 off                   

  server.begin();                                  // webserver ESP8266 on start 
  Serial.println("HTTP server started");

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt  broker connected");
          Serial.print("MQTT Broker: ");
          Serial.printf(mqtt_broker);
          Serial.println("");
          Serial.print("MQTT Topic Pubs: ");
          Serial.printf(topic_pubs);
          Serial.println("");
          Serial.print("MQTT Topic Subs: ");
          Serial.printf(topic_subs);
          Serial.println("");         
      } else {
          Serial.println("failed with connect to mqtt ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic_pubs, "hello emqx");
  client.subscribe(topic_subs);
}

void loop() {
  Serial.print("Device Sensor Run : ");
  Serial.print("Device Sensor Suhu Read Ke : ");
  Serial.println(device_read);
  device_read++;
  delay(5000);                                      //Set atur pengulangan setiap 5 detik   
   
  //MQTT (MQTT Sub Running tetapi Pub not running)
  //client.loop();
  //client.publish(topic_pubs, "hello emqx");
  //client.loop();

  //Start Web Server
  server.handleClient(); 
}


void handle_OnConnect() {
  Temperature = dht.readTemperature();                              // Gets the values of the temperature
  Humidity = dht.readHumidity();                                    // Gets the values of the humidity 
  server.send(200, "text/html", SendHTML(Temperature,Humidity));    // Temp & Humanditu send to View HTML
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat,float Humiditystat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta http-equiv=\"refresh\" content=\"5\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Weather Report</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Dendie <br /> ESP8266 NodeMCU Weather Report</h1>\n";
  
  ptr +="<p>Temperature: ";
  ptr +=(int)Temperaturestat;
  ptr +="°C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)Humiditystat;
  ptr +="%</p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in subs topic: ");
    Serial.println(topic_subs);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}