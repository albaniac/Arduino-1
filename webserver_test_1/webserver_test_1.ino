/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "Kercem2";
const char* password = "E0E3106433F4";

ESP8266WebServer server(80);

String webPage = "";

int LedPin1 = D1;
int LedPin2 = D2;

void setup(void){
  webPage += "<h1>Robin's Web Server</h1><p>LED 1 <a href=\"LED1On\"><button>ON</button></a>&nbsp;<a href=\"LED1Off\"><button>OFF</button></a></p>";
  webPage += "<p>LED 2 <a href=\"LED2On\"><button>ON</button></a>&nbsp;<a href=\"LED2Off\"><button>OFF</button></a></p>";
  
  // preparing GPIOs
  pinMode(LedPin1, OUTPUT);
  digitalWrite(LedPin1, LOW);
  pinMode(LedPin2, OUTPUT);
  digitalWrite(LedPin2, LOW);
  
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("LED_server", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/LED1On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(LedPin1, HIGH);
    delay(1000);
  });
  server.on("/LED1Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(LedPin1, LOW);
    delay(1000); 
  });
  server.on("/LED2On", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(LedPin2, HIGH);
    delay(1000);
  });
  server.on("/LED2Off", [](){
    server.send(200, "text/html", webPage);
    digitalWrite(LedPin2, LOW);
    delay(1000); 
  });
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void){
  server.handleClient();
} 

