#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "TimeManager.h"
#include "Config.h"

TimeManager timeManager;

WiFiUDP udp;
NTPClient timeClient(udp, NTP_SERVER2, TIMEZONE_OFFSET, SYNC_INTERVAL);

// Menghitung tahun dari epoch
int calculateYear(unsigned long &epoch) {
    int year = 1970;
    unsigned long secondsInYear;

    while (true) {
        // Hitung detik dalam tahun (365 hari atau 366 hari jika kabisat)
        secondsInYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 31622400 : 31536000;

        // Jika epoch lebih kecil dari detik dalam tahun, keluar dari loop
        if (epoch < secondsInYear) {
            break;
        }
        epoch -= secondsInYear;
        year++;
    }
    return year;
}

// Menghitung bulan dari epoch
int calculateMonth(unsigned long &epoch, int year) {
    // Jumlah hari per bulan untuk tahun biasa
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Periksa apakah tahun kabisat
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29;  // Februari memiliki 29 hari
    }

    int month = 0;
    while (epoch >= daysInMonth[month] * 86400) {  // Konversi hari ke detik
        epoch -= daysInMonth[month] * 86400;
        month++;
    }

    return month + 1;  // Bulan dimulai dari 1
}

// Menghitung hari dari epoch
int calculateDay(unsigned long epoch) {
    // Sisa epoch dikonversi menjadi hari
    return (epoch / 86400) + 1;
}

// Inisialisasi TimeManager
void TimeManager::init() {
    timeClient.begin();
}

// Memperbarui waktu NTP
void TimeManager::update() {
    timeClient.update();
}

// Mendapatkan waktu dalam format JSON
String TimeManager::getTimeJson() {
    // Ambil epoch dari timeClient
    unsigned long epoch = timeClient.getEpochTime();

    // Salin epoch untuk perhitungan
    unsigned long epochCopy = epoch;

    // Hitung tahun
    int year = calculateYear(epochCopy);

    // Hitung bulan
    int month = calculateMonth(epochCopy, year);

    // Hitung hari
    int day = calculateDay(epochCopy);

    // Format waktu: HH:MM:SS
    String formattedTime = timeClient.getFormattedTime();

    // Gabungkan ke dalam format ISO 8601
    String isoTime = String(year) + "-" + String(month) + "-" + String(day) + "T" + formattedTime;

    // Membuat objek JSON menggunakan ArduinoJson
    JsonDocument doc;  // Ukuran buffer JSON
    doc["Time"] = isoTime;        // Menambahkan data waktu dalam format ISO 8601 ke objek JSON

    // Mengonversi objek JSON ke string
    String output;
    serializeJson(doc, output);

    return output;
}

int TimeManager::getHours()
{
    return timeClient.getHours();
}

int TimeManager::getMinutes()
{
    return timeClient.getMinutes();
}

int TimeManager::getSeconds()
{
    return timeClient.getSeconds();
}

int TimeManager::getDay()
{
    return timeClient.getDay();
}

bool TimeManager::isTimeSet()
{
    return timeClient.isTimeSet();
}

