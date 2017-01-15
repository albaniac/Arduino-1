/*
 * This sketch connects to a local wifi AP and listens on Port 80.
 * 
 * When a client connects is sends a series of text strings in HTML format
 * to create a web page with a button in a form.
 * 
 * When the button on the web page is clicked a GET request containing 
 * the word "/change/" is sent back to the ESP8266 server.
 * 
 * This takes GPIO2 LOW for 100ms before returning it HIGH
 * 
 * An Arduino can monitor this pin via a level shifter (3V3 to 5V)
 * and take some action.
 * 
 * It was intended to control a Christmas star.
 * 
 * The contents of the web page can easily be modified in this sketch.
 */

#include <ESP8266WiFi.h>

const char* ssid = "Kercem2";
const char* password = "E0E3106433F4";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  if (req.indexOf("change") != -1) {
                            digitalWrite(2, LOW);
                            delay(100);
                            digitalWrite(2, HIGH);
  }
  
  client.flush();


  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<br><br>");
  client.println("<h1 style = \"color:red \"><center>Michelle, Grace and Iain's Star</center></h1>");
  client.println("<br><br><center>"); 
  client.println("<form action=\"/change/\" method=\"get\">");
  client.println("<button type=\"submit\" style = \"width:500px; height:100px; font-weight: bold; color: #FFF; background-color:red;border:10px double #00FF00; font-size: 200%;\">");
  client.println("Pattern Change</button>");
  client.println("</form></center></html>");
 
  Serial.println("Client disconnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

