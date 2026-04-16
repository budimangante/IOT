#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

const char* ssid = "Universitas Mulawarman";
const char* password = "";

#define BOT_TOKEN "8638563351:AAF49tzTc9PGC07unlHJVu8xQJt0AQIDZEU"

const String GROUP_CHAT_ID = "-5001260222";
const String ID_ANGGOTA_A = "5877859290";
const String ID_ANGGOTA_B = "6179589660";
const String ID_ANGGOTA_C = "7585338767";

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledUtamaPin = 27;
const int ledAPin = 25;
const int ledBPin = 32;
const int ledCPin = 26;

#define DHTPIN 16
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int mq2Pin = 17;
const int batasGas = 2000;
unsigned long waktuPeringatanTerakhir = 0;
const unsigned long jedaPeringatan = 10000;

void setup() {
  Serial.begin(115200);
  
  pinMode(ledUtamaPin, OUTPUT);
  pinMode(ledAPin, OUTPUT);
  pinMode(ledBPin, OUTPUT);
  pinMode(ledCPin, OUTPUT);
  
  digitalWrite(ledUtamaPin, LOW);
  digitalWrite(ledAPin, LOW);
  digitalWrite(ledBPin, LOW);
  digitalWrite(ledCPin, LOW);

  dht.begin();

  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String from_id = bot.messages[i].from_id;
    String from_name = bot.messages[i].from_name;
    String text = bot.messages[i].text;

    if (text == "/start" || text == "/help") {
      String welcome = "[SISTEM AKTIF] Selamat datang di Pusat Kontrol Smart House.\n\n";
      welcome += "Daftar Akses Perintah:\n";
      welcome += "/led_utama_on | /led_utama_off - Kontrol Lampu Ruang Tamu\n";
      welcome += "/led_a_on | /led_a_off - Kontrol LED Anggota A\n";
      welcome += "/led_b_on | /led_b_off - Kontrol LED Anggota B\n";
      welcome += "/led_c_on | /led_c_off - Kontrol LED Anggota C\n";
      welcome += "/cek_kondisi - Laporan Sensor Suhu & Kelembapan\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    else if (text == "/led_utama_on") {
      digitalWrite(ledUtamaPin, HIGH);
      bot.sendMessage(chat_id, "[AKSES PUBLIK DITERIMA] Lampu Ruang Tamu dinyalakan oleh " + from_name, "");
    }
    else if (text == "/led_utama_off") {
      digitalWrite(ledUtamaPin, LOW);
      bot.sendMessage(chat_id, "[AKSES PUBLIK DITERIMA] Lampu Ruang Tamu dimatikan oleh " + from_name, "");
    }
    else if (text == "/led_a_on") {
      if (from_id == ID_ANGGOTA_A) {
        digitalWrite(ledAPin, HIGH);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Halo " + from_name + ", lampu area pribadimu menyala.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Peringatan! " + from_name + " mencoba menyalakan LED milik Anggota A.", "");
      }
    }
    else if (text == "/led_a_off") {
      if (from_id == ID_ANGGOTA_A) {
        digitalWrite(ledAPin, LOW);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Lampu area pribadimu dimatikan.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Tindakan dibatalkan. Harap hormati privasi.", "");
      }
    }
    else if (text == "/led_b_on") {
      if (from_id == ID_ANGGOTA_B) {
        digitalWrite(ledBPin, HIGH);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Halo " + from_name + ", lampu area pribadimu menyala.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Peringatan! " + from_name + " mencoba menyalakan LED milik Anggota B.", "");
      }
    }
    else if (text == "/led_b_off") {
      if (from_id == ID_ANGGOTA_B) {
        digitalWrite(ledBPin, LOW);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Lampu area pribadimu dimatikan.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Tindakan dibatalkan. Harap hormati privasi.", "");
      }
    }
    else if (text == "/led_c_on") {
      if (from_id == ID_ANGGOTA_C) {
        digitalWrite(ledCPin, HIGH);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Halo " + from_name + ", lampu area pribadimu menyala.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Peringatan! " + from_name + " mencoba menyalakan LED milik Anggota C.", "");
      }
    }
    else if (text == "/led_c_off") {
      if (from_id == ID_ANGGOTA_C) {
        digitalWrite(ledCPin, LOW);
        bot.sendMessage(chat_id, "[OTORISASI BERHASIL] Lampu area pribadimu dimatikan.", "");
      } else {
        bot.sendMessage(chat_id, "[AKSES DITOLAK] Tindakan dibatalkan. Harap hormati privasi.", "");
      }
    }
    else if (text == "/cek_kondisi") {
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float g = analogRead(mq2Pin);

      if (isnan(h) || isnan(t || isnan(g))) {
        bot.sendMessage(chat_id, "[ERROR] Gagal membaca data dari sensor DHT11.", "");
      } else {
        String report = "[PEMINDAIAN SELESAI] Data Lingkungan:\n";
        report += "- Suhu: " + String(t) + " C\n";
        report += "- Kelembapan: " + String(h) + " %\n";
        report += "- Gas: " + String(g) + "%\n";
        bot.sendMessage(chat_id, report, "");
      }
    }
  }
}

void cekSensorGas() {
  int nilaiGas = analogRead(mq2Pin);
  
  if (nilaiGas > batasGas) {
    unsigned long waktuSekarang = millis();
    if (waktuSekarang - waktuPeringatanTerakhir > jedaPeringatan) {
      String pesanBahaya = "[PERINGATAN DARURAT - KODE MERAH]\n\n";
      pesanBahaya += "Sistem mendeteksi kebocoran gas atau asap tebal!\n";
      pesanBahaya += "Segera evakuasi dan buka ventilasi udara!";
      
      bot.sendMessage(GROUP_CHAT_ID, pesanBahaya, "");
      waktuPeringatanTerakhir = waktuSekarang;
    }
  }
}

void loop() {
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (numNewMessages > 0) {
      handleNewMessages(numNewMessages);
    }
    lastTimeBotRan = millis();
  }

  cekSensorGas();
  
  delay(10); 
}