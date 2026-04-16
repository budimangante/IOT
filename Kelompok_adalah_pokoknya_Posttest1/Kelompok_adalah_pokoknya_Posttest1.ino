//blynk
#define BLYNK_TEMPLATE_ID "TMPL6BGNoVzLY"
#define BLYNK_TEMPLATE_NAME "Sensor mq Gas"
#define BLYNK_AUTH_TOKEN "BZdEfKM5rEW0opxjSHHkXC_yORSTq-yj"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

const char* ssid = "Mountain";
const char* password = "apayaaaa";

#define LED1 27
#define LED2 26
#define LED3 25
#define MQ_AOUT 33
#define MQ_DOUT 32

BlynkTimer timer; 

void kirimDataSensor() {
  int levelGas = analogRead(MQ_AOUT);
  Blynk.virtualWrite(V1, levelGas);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(MQ_DOUT, INPUT); 
  Serial.println("Menghubungkan ke WiFi dan Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("Berhasil terhubung!");
  timer.setInterval(1000L, kirimDataSensor);
}

void blinkLED(int pin, int durasiDetik) {
  unsigned long waktuMulai = millis();
  unsigned long durasiMs = durasiDetik * 1000;
  
  while (millis() - waktuMulai < durasiMs) {
    digitalWrite(pin, !digitalRead(pin)); 
    delay(250); 
    Blynk.run();
    timer.run();
  }
  digitalWrite(pin, LOW); 
}

void loop() {
  Blynk.run();
  timer.run();

  int statusGas = digitalRead(MQ_DOUT); 
  int levelGas = analogRead(MQ_AOUT);

  if (statusGas == LOW) { 

    Serial.print("AWAS! Gas terdeteksi. Level: ");
    Serial.println(levelGas);

    if (levelGas >= 0 && levelGas <= 200) {
      digitalWrite(LED1, 1);
    } 
    else if (levelGas > 200 && levelGas <= 500) {
      digitalWrite(LED2, 1);
    } 
    else if (levelGas > 500) {
      digitalWrite(LED3, 1);
    }
  } 
  else { 
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 0);
    digitalWrite(LED3, 0);
  }
}