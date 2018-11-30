#include <SPI.h>
#include <Ethernet.h>


byte mac[] = { 0x50, 0xC7, 0xBF, 0xBC, 0xEF, 0x21 };
byte ip[] = { 192, 168, 31, 101 };

EthernetServer server = EthernetServer(101);

int pirPin = 7;
int relayPin = 8;
int calibratingTime = 10;
int btnPin = 9;
int current;
int sts = 0;

long millis_held;


long secs_held;
long prev_secs_held;
byte previous = HIGH;
unsigned long firstTime;

long unsigned int lowIn;
long unsigned int pause = 5000;
unsigned long currentMillis = 0;
unsigned long prevMillis = 0;
int interval = 1000;

bool lockLow = true;
bool takeLowTime;
bool btnSts = false;

String readString;

void setup() {
  // initialize the ethernet device
  Serial.begin(9600);

  pinMode(pirPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(pirPin, LOW);
  digitalWrite(relayPin, LOW);

  Serial.println("callibrating sensor");
  for(int i = 0; i < calibratingTime; i++) {
    delay(1000);
  }
  Serial.println("sensor active");
  Ethernet.begin(mac, ip);
  server.begin();
  EthernetClient client = server.available();
}

void sendData(String data, EthernetClient client) {
//         client.write(0x81);
//         client.write(0x80 | data.length());
         for (int i = 0; i < data.length(); i++) {
           client.write(data[i]);
         }
         // give the client time to receive the data
         delay(10);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
     while (client.connected()) {
       if(digitalRead(pirPin) == HIGH) {
        if(lockLow) {
          lockLow = false;
          int pirVal = (digitalRead(pirPin));
           String dataPir = String(pirVal);
           Serial.println(pirVal);
          sendData(dataPir, client);
          delay(50);
        }
        takeLowTime = true;
       }

      if(digitalRead(pirPin) == LOW) {
          if(takeLowTime) {
            lowIn = millis();
            takeLowTime = false;
          }
          if(!lockLow && millis() - lowIn > pause) {
            lockLow = true;
            int pirVal = (digitalRead(pirPin));
            String dataPir = String(pirVal);
            Serial.println(pirVal);
            sendData(dataPir, client);
            delay(50);
          }
       }
current = digitalRead(btnPin);
  currentMillis = millis();

  if(current == LOW && previous == HIGH && (millis() - firstTime) > 200) {
    firstTime = millis();
  }
  millis_held = (millis()- firstTime);
  secs_held = millis_held / 1000;

  if(millis_held >= 400 && millis_held <= 1100){
          delay(1000);
          prevMillis = currentMillis;
          sts++;
          Serial.println(sts);
   }
   if (currentMillis - prevMillis >= 30000) {
          sts = 0;
    }
   if(sts == 5) {
             Serial.println("sending data Kesehatan ..");
             String btnVal = String("3");
             sendData(btnVal, client);
             ledBlink(60,2000,relayPin);
             delay(10000);
             Serial.println("reset");
             sts = 0;
    }
  if(millis_held > 50){
    if(current == HIGH && previous == LOW){
      if(secs_held >= 6 && secs_held <= 10){
        Serial.println("sending data Darurat ..");
        String btnVal = String("2");
        sendData(btnVal, client);
        ledBlink(60,2000,relayPin);
        sts = 0;
      }

      if(secs_held >= 10) {
        Serial.print("it works !!! second : ");
        Serial.print(secs_held);
        Serial.print("millisecond held:");
        Serial.print(millis_held);
      }
    }
  }
  previous = current;
  prev_secs_held = secs_held;
      if (client.available()) {
      char c = client.read();
      if(c == '0'){
        Serial.println("mulai");
      }else if(c == '1'){
        Serial.println("aktif");
        digitalWrite(relayPin, HIGH);
      }else if(c == '2') {
        Serial.println("nonaktif");
        digitalWrite(relayPin, LOW);
      }

      }
    }
    // close the connection:
    Serial.println("Closing connection");
    client.stop();
  }
 }
 void ledBlink(int times, int lengthms, int pinnum){
    for(int x = 0; x < times; x++){
      digitalWrite(relayPin, HIGH);
      delay(lengthms);
      digitalWrite(relayPin, LOW);
      delay(lengthms);
    }
 }

