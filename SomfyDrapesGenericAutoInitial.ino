// Somfy motorized drapery controller, works well with NodeMCU v1.0 board
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#define LEDBLU  2  // Controls blue LED on WiFi module
#define LEDRED  16 // Controls red LED on NodeMCU board
#define LEDGRN  4  // Added to include 3rd function (MY) control and added green LED at D2/GPIO4 of NodeMCU v1.0 ESP8266 board
//schematic is pin D2 to 470 ohm resistor -/\/\/\- to green LED cathode -|<- anode to 3V3 pin; LED is on when D2 false
void prepareIds();
boolean connectWifi();
boolean connectUDP();
void startHttpServer();
void turnOnRelay();
void turnOffRelay();
void sendRelayState();
//void drapeSync();

const char* ssid = "yourSSID";
const char* password = "yourwifiPassword";
String friendlyName = "Drapes"; // Alexa device name; if "drapes," Alexa recognizes Open drapes and close drapes as well as turn on drapes (open), turn off drapes (close)

unsigned int localPort = 1900;      // local port to listen on

WiFiUDP UDP;
boolean udpConnected = false;
IPAddress ipMulti(239, 255, 255, 250);
unsigned int portMulti = 1900;      // local port to listen on

ESP8266WebServer HTTP(80);
 
boolean wifiConnected = false;
boolean relayState = false;
boolean myState = false;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,

String serial;
String persistent_uuid;
boolean cannotConnectToWifi = false;

void setup() {
  Serial.begin(115200);

  // Setup LED/Drape control pins and flash twice
  pinMode(LEDBLU, OUTPUT);
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGRN, OUTPUT);
  digitalWrite(LEDBLU, LOW);
  digitalWrite(LEDRED, LOW);
  digitalWrite(LEDGRN, LOW);
  delay(1000);
  digitalWrite(LEDBLU, HIGH);
  digitalWrite(LEDRED, HIGH);
  digitalWrite(LEDGRN, HIGH);
  delay(1000);
  digitalWrite(LEDBLU, LOW);
  digitalWrite(LEDRED, LOW);
  digitalWrite(LEDGRN, LOW);
  delay(1000);
  digitalWrite(LEDBLU,HIGH);
  digitalWrite(LEDRED, HIGH);
  digitalWrite(LEDGRN, HIGH);
  
  
  prepareIds();
  
  // Initialise wifi connection
  wifiConnected = connectWifi(); //Proceed only if WiFi connected
  if(wifiConnected){
    Serial.println("Ask Alexa to discover devices");
    udpConnected = connectUDP();
    
    if (udpConnected){
      // initialise pins if needed 
      startHttpServer();
      Serial.println("HttpServer started in setup");
    }
  }
  drapeSync(); //synchronize Booleans relayState, myState with physical drapes position
}

void loop() {

  HTTP.handleClient();
  delay(1);
  
  
  // if there's data available, read a packet
  // check if the WiFi and UDP connections were successful
  if(wifiConnected){
    if(udpConnected){    
      // if there’s data available, read a packet
      int packetSize = UDP.parsePacket();
      
      if(packetSize) {
        //Serial.println("");
        //Serial.print("Received packet of size ");
        //Serial.println(packetSize);
        //Serial.print("From ");
        IPAddress remote = UDP.remoteIP();
        
        for (int i =0; i < 4; i++) {
          Serial.print(remote[i], DEC);
          if (i < 3) {
            Serial.print(".");
          }
        }
        
        Serial.print(", port ");
        Serial.println(UDP.remotePort());
        
        int len = UDP.read(packetBuffer, 255);
        
        if (len > 0) {
            packetBuffer[len] = 0;
        }

        String request = packetBuffer;
        //Serial.println("Request:");
        //Serial.println(request);
         
        if(request.indexOf('M-SEARCH') > 0) {
            if(request.indexOf("urn:Belkin:device:**") > 0) {
                Serial.println("Responding to search request ...");
                respondToSearch();
            }
        }
      }
        
      delay(10);
    }
  } else {
      // Turn on/off to indicate cannot connect ..      
  }
}

void prepareIds() {
  uint32_t chipId = ESP.getChipId();
  char uuid[64];
  sprintf_P(uuid, PSTR("38323636-4558-4dda-9188-cda0e6%02x%02x%02x"),
        (uint16_t) ((chipId >> 16) & 0xff),
        (uint16_t) ((chipId >>  8) & 0xff),
        (uint16_t)   chipId        & 0xff);

  serial = String(uuid);
  persistent_uuid = "Socket-1_0-" + serial;
}

void respondToSearch() {
    Serial.println("");
    Serial.print("Sending response to ");
    Serial.println(UDP.remoteIP());
    Serial.print("Port : ");
    Serial.println(UDP.remotePort());

    IPAddress localIP = WiFi.localIP();
    char s[16];
    sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

    String response = 
         "HTTP/1.1 200 OK\r\n"
         "CACHE-CONTROL: max-age=86400\r\n"
         "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n"
         "EXT:\r\n"
         "LOCATION: http://" + String(s) + ":80/setup.xml\r\n"
         "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
         "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
         "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
         "ST: urn:Belkin:device:**\r\n"
         "USN: uuid:" + persistent_uuid + "::urn:Belkin:device:**\r\n"
         "X-User-Agent: redsonic\r\n\r\n";

    UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
    UDP.write(response.c_str());
    UDP.endPacket();                    

     Serial.println("Response sent !");
}

void drapeSync(){  // By opening drapes, then closing them 20S later, drapeSync assures synchronization of program Booleans relayState and myState with the physical drapes,
    turnOnRelay();
    Serial.println("Drapes opening");
    delay(3000);
    turnOffRelay(); // regardless of the initial state of the physical drapes. After drapeSync runs (power up or reboot), drapes will be closed.
    Serial.println("Drapes closing and initialzed");
  }

void startHttpServer() {
    HTTP.on("/index.html", HTTP_GET, [](){
      Serial.println("Got Request index.html ...\n");
      HTTP.send(200, "text/plain", "Hello World!");
    });

    HTTP.on("/upnp/control/basicevent1", HTTP_POST, []() {
      Serial.println("########## Responding to  /upnp/control/basicevent1 ... ##########");      

      //for (int x=0; x <= HTTP.args(); x++) {
      //  Serial.println(HTTP.arg(x));
      //}
  
      String request = HTTP.arg(0);      
      Serial.print("request:");
      Serial.println(request);
 
      if(request.indexOf("SetBinaryState") >= 0) {
        if(request.indexOf("<BinaryState>1</BinaryState>") >= 0) {
            Serial.println("Got Turn on request");
            turnOnRelay();
        }
  
        if(request.indexOf("<BinaryState>0</BinaryState>") >= 0) {
            Serial.println("Got Turn off request");
            turnOffRelay();
        }
      }

      if(request.indexOf("GetBinaryState") >= 0) {
        Serial.println("Got binary state request");
        sendRelayState();
      }
      
      
      HTTP.send(200, "text/plain", "");
    });

    HTTP.on("/eventservice.xml", HTTP_GET, [](){
      Serial.println(" ########## Responding to eventservice.xml ... ########\n");
      String eventservice_xml = "<scpd xmlns=\"urn:Belkin:service-1-0\">"
        "<actionList>"
          "<action>"
            "<name>SetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>in</direction>"
                "</argument>"
            "</argumentList>"
          "</action>"
          "<action>"
            "<name>GetBinaryState</name>"
            "<argumentList>"
              "<argument>"
                "<retval/>"
                "<name>BinaryState</name>"
                "<relatedStateVariable>BinaryState</relatedStateVariable>"
                "<direction>out</direction>"
                "</argument>"
            "</argumentList>"
          "</action>"
      "</actionList>"
        "<serviceStateTable>"
          "<stateVariable sendEvents=\"yes\">"
            "<name>BinaryState</name>"
            "<dataType>Boolean</dataType>"
            "<defaultValue>0</defaultValue>"
           "</stateVariable>"
           "<stateVariable sendEvents=\"yes\">"
              "<name>level</name>"
              "<dataType>string</dataType>"
              "<defaultValue>0</defaultValue>"
           "</stateVariable>"
        "</serviceStateTable>"
        "</scpd>\r\n"
        "\r\n";
            
      HTTP.send(200, "text/plain", eventservice_xml.c_str());
    });
    
    HTTP.on("/setup.xml", HTTP_GET, [](){
      Serial.println(" ########## Responding to setup.xml ... ########\n");

      IPAddress localIP = WiFi.localIP();
      char s[16];
      sprintf(s, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
    
      String setup_xml = "<?xml version=\"1.0\"?>"
            "<root>"
             "<device>"
                "<deviceType>urn:Belkin:device:controllee:1</deviceType>"
                "<friendlyName>"+ friendlyName +"</friendlyName>"
                "<manufacturer>Belkin International Inc.</manufacturer>"
                "<modelName>Socket</modelName>"
                "<modelNumber>3.1415</modelNumber>"
                "<modelDescription>Belkin Plugin Socket 1.0</modelDescription>\r\n"
                "<UDN>uuid:"+ persistent_uuid +"</UDN>"
                "<serialNumber>221517K0101769</serialNumber>"
                "<binaryState>0</binaryState>"
                "<serviceList>"
                  "<service>"
                      "<serviceType>urn:Belkin:service:basicevent:1</serviceType>"
                      "<serviceId>urn:Belkin:serviceId:basicevent1</serviceId>"
                      "<controlURL>/upnp/control/basicevent1</controlURL>"
                      "<eventSubURL>/upnp/event/basicevent1</eventSubURL>"
                      "<SCPDURL>/eventservice.xml</SCPDURL>"
                  "</service>"
              "</serviceList>" 
              "</device>"
            "</root>\r\n"
            "\r\n";
            
        HTTP.send(200, "text/xml", setup_xml.c_str());
        
        Serial.print("Sending :");
        Serial.println(setup_xml);
    });
    
    HTTP.begin();  
    Serial.println("HTTP Server started ..");
}


      
// connect to wifi – returns true if successful or false if not
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

boolean connectUDP(){
  boolean state = false;
  
  Serial.println("");
  Serial.println("Connecting to UDP");
  
  if(UDP.beginMulticast(WiFi.localIP(), ipMulti, portMulti)) {
    Serial.println("Connection successful");
    state = true;
  }
  else{
    Serial.println("Connection failed");
  }
  
  return state;
}

void turnOnRelay() {
  if((!relayState) | (myState)){
 digitalWrite(LEDBLU, LOW); // turn on LED on
 delay(500);
 digitalWrite(LEDBLU, HIGH); // then turn it right back off, but set relayState true
 relayState = true;
 myState = false;
  }
  else{
 digitalWrite(LEDGRN, LOW); // turn green LED on (my)
 delay(500);
 digitalWrite(LEDGRN, HIGH); // then turn it right back off, but set relayState false (open)
 myState = true;   
  }
 
 

 Serial.println("$$$$$Exiting turnOnRelay after if statement, relayState should be true, but is " + String(relayState) + " $$$$$");
  String body = 
      "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>\r\n"
      "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">\r\n"
      "<BinaryState>1</BinaryState>\r\n"
      "</u:SetBinaryStateResponse>\r\n"
      "</s:Body> </s:Envelope>";

  HTTP.send(200, "text/xml", body.c_str());
        
  Serial.print("Sending :");
  Serial.println(body);
}

void turnOffRelay() {
  if(relayState | myState){
    digitalWrite(LEDRED, LOW); // turn on relay with voltage HIGH 
    delay(500);
    digitalWrite(LEDRED, HIGH); // then turn it right back off, but leave relay state unchanged
    relayState = false;
    myState = false;
  }
  else{
    digitalWrite(LEDGRN, LOW); // turn green LED on
    delay(500);
    digitalWrite(LEDGRN, HIGH); // then turn it right back off, but keep relayState true
    myState = true;   
  }
  
  
  
  String body = 
      "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>\r\n"
      "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">\r\n"
      "<BinaryState>0</BinaryState>\r\n"
      "</u:SetBinaryStateResponse>\r\n"
      "</s:Body> </s:Envelope>";

  HTTP.send(200, "text/xml", body.c_str());
        
  Serial.print("Sending :");
  Serial.println(body);
}

void sendRelayState() {
  
  String body = 
      "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>\r\n"
      "<u:GetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">\r\n"
      "<BinaryState>";
      
  body += (relayState ? "1" : "0");
  
  body += "</BinaryState>\r\n"
      "</u:GetBinaryStateResponse>\r\n"
      "</s:Body> </s:Envelope>\r\n";
 
   HTTP.send(200, "text/xml", body.c_str());
}
