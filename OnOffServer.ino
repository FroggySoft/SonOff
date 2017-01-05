/*
 *  This sketch switches the relais of the SonOff 
 *  
esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

   NodeMCU 0.9 (ESP-12 Module)
   Connectie met ftdi:
   5V
   RxD
   TxD
   Gnd
   GPIO (not used)

   To enable programmming, startup board with button pressed
 */

#include <ESP8266WiFi.h>

#define ESP8266_LED 13
#define RELAIS      12
#define BUTTON      0

#define RELAIS_OFF 0
#define RELAIS_ON  1
#define LED_OFF    1
#define LED_ON     0

const char* ssid = "Your ssid here";
const char* password = "your password here";
IPAddress ip(192,168,1,120);  //Node static IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

bool mRelaisState = false;
int  mButtonPressedCounter = 0 ;

void setup()
{
  Serial.begin(115200);
  delay(10);

  // prepare GPIO's
  pinMode(RELAIS, OUTPUT);
  digitalWrite(RELAIS, RELAIS_OFF);
  pinMode(ESP8266_LED, OUTPUT);
  digitalWrite(ESP8266_LED, LED_OFF);
  pinMode(BUTTON, INPUT);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  
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

void loop()
{ 
    if (!digitalRead(BUTTON))
    {
      mButtonPressedCounter++;
      if(mButtonPressedCounter==10)
      {
        mRelaisState = !mRelaisState;
        digitalWrite(RELAIS, mRelaisState ? RELAIS_ON : RELAIS_OFF);
        digitalWrite(ESP8266_LED, mRelaisState ? LED_ON : LED_OFF);
      }
    }
    else
    {
      mButtonPressedCounter = 0 ;
    }
    
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)
    {
      return;
    }

    // Wait until the client sends some data
    while(!client.available() && digitalRead(BUTTON))
    {
      delay(10);
    }

      
      // Read the first line of the request
      String req = client.readStringUntil('\r');
      client.flush();
  
      Serial.println(req);
  
      // Match the request
      bool lValidUrl = false;
      String lReply = "HTTP/1.1 200 OK\r\nContent-type: application/json\r\n\r\n{\n";

      req.toLowerCase();
      if (req.indexOf("/on") != -1)
      {
        mRelaisState = true;
        lValidUrl = true;
      }
      else if (req.indexOf("/off") != -1)
      {
        mRelaisState = false;
        lValidUrl = true;
      }
      else if (req.indexOf("/get") != -1)
      {
        lValidUrl = true;
      }
      else if (req.indexOf("/index") != -1)
      {
        lReply = "<html>";
        lReply += "Available commands:<br>";
        lReply += "/On<br>";
        lReply += "/Off<br>";
        lReply += "/Get<br><br>";
        lReply += "Reply is a JSON string with state of the switch<br><br>";
        lReply += "</html>{";      
        lValidUrl = true;
      }
  
      client.flush();
  
      digitalWrite(RELAIS, mRelaisState ? RELAIS_ON : RELAIS_OFF);
      digitalWrite(ESP8266_LED, mRelaisState ? LED_ON : LED_OFF);

      // Prepare the response
      if (lValidUrl)
      {
        lReply += "\"Relais\" : \"";
        lReply += mRelaisState?"On":"Off";
        lReply += "\",\n";
        lReply += "\"status\" : \"OK\"\n}\n";
      }
      else
      {
        lReply += "\"status\" : \"ERR\"\n}\n";
      }   
      // Send the response to the client
      client.print(lReply);
    
    
    delay(20);
  
  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

