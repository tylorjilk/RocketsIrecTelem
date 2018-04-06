#include <Stepper.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>

// The number of steps per revolution
const int stepsPerRev = 200;
const int stepsNorm = 425;
const int stepsInc = 20;

const int NUMBER_OF_STEPS = 2;
const int led = 13;

const char WiFiAPPSK[] = "staging";
int stepstatus = 0;
int holdstatus = 0;

WiFiServer server(80);

// Pins 13,14,15,16 connect from left to right on the stepper board
Stepper myStepper(stepsPerRev, 13,12,14,16);

void setup() {
  pinMode(led, OUTPUT);
  // Set the speed in rpms
  myStepper.setSpeed(120);
  Serial.begin(115200);
  openStepper();

  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Staging-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                  String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "Staging " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
      AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);

  server.begin();
}

void loop() {

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<head><style>p{text-align:center;font-size:24px;font-family:helvetica;padding:30px;border:1px solid black;background-color:powderblue}</style></head><body>";

  // Match the request
  int val = -1; // We'll use 'val' to keep track of both the
                // request type (read/set) and value if set.
  if (req.indexOf("/open") != -1){
    openStepper();
    val = 0;
    stepstatus = 1;
  } else if (req.indexOf("/close") != -1){
    closeStepper();
    val = 1;
    stepstatus = 0;
  } else if (req.indexOf("/status") != -1){
    val = 2;
  } else if (req.indexOf("/hold") != -1){
    holdStepper();
    holdstatus = 1;
    val = 3;
  } else if (req.indexOf("/release") != -1){
    releaseStepper();
    holdstatus = 0;
    val = 4;
  } else if (req.indexOf("/tighten") != -1){
    tightenStepper();
    val = 5;
  } else if (req.indexOf("/loosen") != -1){
    loosenStepper();
    val = 6;
  }

  client.flush();

  if (val == 0) {
    s += "<p>Stepper is now <b>opened!</b></p>";
    
  } else if (val == 1) {
    s += "<p>Stepper is now <b>closed</b></p>";
    
  } else if (val == 2) {
    
    s += "<p>Status of stepper: ";
    if(stepstatus == 1) {
      s += "<b>opened and ";
    } else {
      s += "<b>closed and ";
    }

    if(holdstatus == 1){
      s += "holding.</b></p>";
    } else {
      s += "not holding.</b></p>";
    }
    
  } else if (val==3){
    s += "<p>Stepper is now <b>holding!</b></p>";
    
  } else if (val == 4){
    s += "<p>Stepper is now <b>not holding.</b></p>";

  } else if (val==5){
    s += "<p>Just <b>tightened</b> stepper</p>";

  } else if (val==6){
    s+= "<p>Just <b>loosened</b> stepper</p>";
    
  } else {
    s += "<p>Invalid Request.<br> Try /open, /close, /hold, /release, /tighten, /loosen, or /status.</p>";
  }

  s += "</body></html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

void openStepper(){
  myStepper.step(stepsNorm);
}

void closeStepper(){
  myStepper.step(-stepsNorm);
}

void holdStepper(){
  digitalWrite(13, HIGH);
}

void releaseStepper(){
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
  digitalWrite(15, LOW);
  digitalWrite(16, LOW);
}

void tightenStepper(){
  myStepper.step(-stepsInc);
}

void loosenStepper(){
  myStepper.step(stepsInc);
}

