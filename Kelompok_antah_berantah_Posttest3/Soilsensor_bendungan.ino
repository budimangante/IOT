#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// --- Konfigurasi WiFi & MQTT ---
const char* ssid = "Kemale"; 
const char* password = "987654321";
const char* mqtt_server = "broker.emqx.io";

// --- Definisi Pin ---
#define BUZZER_PIN 16
#define SENSOR_PIN 34
#define SERVO_PIN  17

// --- Inisialisasi Objek ---
WiFiClient espClient;
PubSubClient client(espClient);
Servo myServo;

// --- Variabel Global Sistem ---
String modeSistem = "Otomatis";
int nilaiAir = 0;
String statusAir = "Aman";

// Variabel Kontrol Aktuator
int servoDerajat = 0;
bool buzzerMenyala = false;
bool buzzerBerkedip = false;

// --- Variabel Millis (Timer Tanpa Delay) ---
unsigned long previousMillisBlink = 0;
const long intervalBlink = 500;

unsigned long previousMillisPublish = 0;
const long intervalPublish = 2000;

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

// Fungsi Callback yang Diperbarui
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Pesan masuk [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // 1. Menerima Perintah Mode
  if (String(topic) == "mal/iot/kontrol") {
    if (message == "Otomatis") modeSistem = "Otomatis";
    else if (message == "Manual") modeSistem = "Manual";
  }
  
  // 2. Menerima Perintah Buzzer Manual (Hanya di mode Manual)
  else if (String(topic) == "mal/iot/kontrol_buzzer") {
    // Paksa pindah ke Manual jika tombol ditekan
    modeSistem = "Manual"; 
    
    if (message == "ON") { 
      buzzerBerkedip = false; 
      buzzerMenyala = true; 
    }
    else if (message == "OFF") { 
      buzzerBerkedip = false; 
      buzzerMenyala = false; 
    }
  }

  // 3. Menerima Perintah Servo Manual (Hanya di mode Manual)
  else if (String(topic) == "mal/iot/kontrol_servo") {
    // Paksa pindah ke Manual jika tombol ditekan
    modeSistem = "Manual";

    if (message == "ON") { 
      servoDerajat = 180; // Buka penuh
    }
    else if (message == "OFF") { 
      servoDerajat = 0;   // Tutup
    }
  }
}

// Fungsi Reconnect yang Diperbarui
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP32_Mal_Client")) {
      Serial.println("Terhubung");
      
      // SUBCRIBE KE KETIGA TOPIK KONTROL
      client.subscribe("mal/iot/kontrol");
      client.subscribe("mal/iot/kontrol_buzzer");
      client.subscribe("mal/iot/kontrol_servo"); 
      
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(BUZZER_PIN, OUTPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(servoDerajat); 
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. Baca Sensor Air
  nilaiAir = analogRead(SENSOR_PIN);

  // 2. Penentuan Status
  if (nilaiAir <= 800) {
    statusAir = "Aman";
  } else if (nilaiAir > 800 && nilaiAir <= 1500) {
    statusAir = "Waspada";
  } else if (nilaiAir > 1500) {
    statusAir = "Bahaya";
  }

  // 3. Logika Otomatis
  if (modeSistem == "Otomatis") {
    if (statusAir == "Aman") {
      servoDerajat = 0;
      buzzerBerkedip = false;
      buzzerMenyala = false;
    } 
    else if (statusAir == "Waspada") {
      servoDerajat = 90;
      buzzerBerkedip = false;
      buzzerMenyala = false;
    } 
    else if (statusAir == "Bahaya") {
      servoDerajat = 180;
      buzzerBerkedip = true; 
    }
  }

  // 4. Eksekusi Servo
  myServo.write(servoDerajat);

  // 5. Eksekusi Buzzer
  unsigned long currentMillis = millis();
  if (buzzerBerkedip) {
    if (currentMillis - previousMillisBlink >= intervalBlink) {
      previousMillisBlink = currentMillis;
      buzzerMenyala = !buzzerMenyala;
      digitalWrite(BUZZER_PIN, buzzerMenyala ? HIGH : LOW);
    }
  } else {
    digitalWrite(BUZZER_PIN, buzzerMenyala ? HIGH : LOW);
  }

  // 6. Publish Data
  if (currentMillis - previousMillisPublish >= intervalPublish) {
    previousMillisPublish = currentMillis;

    String teksBuzzer = "Mati";
    if (buzzerBerkedip) teksBuzzer = "Berkedip";
    else if (buzzerMenyala) teksBuzzer = "Menyala";

    client.publish("mal/iot/nilai", String(nilaiAir).c_str());
    client.publish("mal/iot/air", statusAir.c_str());
    client.publish("mal/iot/mode", modeSistem.c_str());
    client.publish("mal/iot/buzzer", teksBuzzer.c_str());
    client.publish("mal/iot/servo", String(servoDerajat).c_str());
    client.publish("mal/iot/koneksi", "Connected");
  }
}