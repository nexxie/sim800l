#include <SoftwareSerial.h>

#include "SIM800L.h"

#define SIM800_RX_PIN 11
#define SIM800_TX_PIN 10
#define SIM800_RST_PIN 6

const char APN[] = "Internet.be";
const char URL[] = "http://api.codecustoms.com:3956/send";
const char CONTENT_TYPE[] = "application/json";
const char PAYLOAD[] = "{\"phone\": \"89117436445\", \"status\": \"ok\"}";

SIM800L* sim800l;

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  while(!Serial);

  // Initialize a SoftwareSerial
  SoftwareSerial* serial = new SoftwareSerial(SIM800_RX_PIN, SIM800_TX_PIN);
  serial->begin(9600);
  delay(1000);
   
  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  sim800l = new SIM800L((Stream *)serial, SIM800_RST_PIN, 200, 512);


  // Setup module for GPRS communication
  setupModule();
}
 
void loop() {
  while(1<2){
  // Establish GPRS connectivity (5 trials)
  bool connected = false;
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }


  Serial.println(F("Start HTTP POST..."));

  // Do HTTP POST communication with 10s for the timeout (read and write)
  uint16_t rc = sim800l->doPost(URL, CONTENT_TYPE, PAYLOAD, 10000, 10000);
   if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP POST successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    Serial.println(sim800l->getDataReceived());
  } else {
    // Failed...
    Serial.print(F("HTTP POST error "));
    Serial.println(rc);
    sim800l->reset();
    setupModule();
  }

  delay(1000);
}}


void setupModule() {
    // Wait until the module is ready to accept AT commands
  while(!sim800l->isReady()) {
    Serial.println(F("Problem to initialize AT command, retry in 1 sec"));
    delay(100);
  }
  
  Serial.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  while(signal <= 0) {
    delay(100);
    signal = sim800l->getSignal();
  }
  Serial.print(F("Signal OK (strenght: "));
  Serial.print(signal);
  Serial.println(F(")"));
  delay(1000);

  // Wait for operator network registration (national or roaming network)
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while(network != REGISTERED_HOME && network != REGISTERED_ROAMING) {
    delay(1000);
    network = sim800l->getRegistrationStatus();
  }
  Serial.println(F("Network registration OK"));
  delay(1000);

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(APN);
  while(!success) {
    success = sim800l->setupGPRS(APN);
    delay(5000);
  }
  Serial.println(F("GPRS config OK"));
}
