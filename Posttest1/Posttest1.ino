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
  int levelAlkohol = analogRead(MQ_AOUT);
  Blynk.virtualWrite(V1, levelAlkohol);
  Serial.print("-> Mengirim data ke Blynk: ");
  Serial.println(levelAlkohol);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(MQ_DOUT, INPUT); 

  Serial.println("=================================");
  Serial.println("Menghubungkan ke WiFi dan Blynk...");
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  
  Serial.println("Berhasil terhubung ke Server Blynk!");
  Serial.println("=================================");

  timer.setInterval(1000L, kirimDataSensor);
}

void loop() {
  Blynk.run();
  timer.run(); 

  int statusSensor = digitalRead(MQ_DOUT); 
  int levelAlkohol = analogRead(MQ_AOUT);

  if (statusSensor == LOW) { 
    Serial.print("AWAS! Alkohol Terdeteksi! Level Analog: ");
    Serial.println(levelAlkohol);

    if (levelAlkohol >= 0 && levelAlkohol <= 200) {
      digitalWrite(LED1, 1);
      digitalWrite(LED2, 0);
      digitalWrite(LED3, 0);
    } 
    else if (levelAlkohol > 200 && levelAlkohol <= 500) {
      digitalWrite(LED1, 0);
      digitalWrite(LED2, 1);
      digitalWrite(LED3, 0);
    } 
    else if (levelAlkohol > 500) {
      digitalWrite(LED1, 0);
      digitalWrite(LED2, 0);
      digitalWrite(LED3, 1);
    }
  } 
  else { 
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 0);
    digitalWrite(LED3, 0);
  }
}