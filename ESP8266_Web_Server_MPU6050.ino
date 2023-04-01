// Include library yang dibutuhkan

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include <LittleFS.h>
#include <Wire.h>

// Ganti dengan nama wifi dan password wifi yang sesuai
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

// Web server akan berjalan pada port 80
AsyncWebServer server(80);
// Mendaftarkan event source dengan url /events
AsyncEventSource events("/events");

// Membuat variable untuk mengatur delay saat membaca data dari 
// sensor
JSONVar readings;
unsigned long previous_time = 0;  
unsigned long previous_time_temp = 0;
unsigned long previous_time_acceleration = 0;
unsigned long gyro_delay  = 20;
unsigned long temperature_delay  = 1000;
unsigned long accelerometer_delay  = 200;

// Membuat variable untuk library MPU6050
// dan membuat object memory sensors_event_t untuk menampung 
// data yang dibaca dari sensor
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

// Membuat variabel yang akan digunakan untuk mengakses dan 
// menyimpan posisi sudut pada ketiga sumbu
float rotationX, rotationY, rotationZ;
float accelerationX, accelerationY, accelerationZ;
float temperature;

// Mendifinisikan error value
float rotationX_error = 0.05;
float rotationY_error = 0.02;
float rotationZ_error = 0.01;

// Fungsi untuk mengembalikan string JSON yang terdiri dari 
// posisi sudut sensor pada sumbu x, y, dan z.
String getGyroscopeReadings(){
  mpu.getEvent(&a, &g, &temp);

  float rotationX_temporary = g.gyro.x;
  if(abs(rotationX_temporary) > rotationX_error)  {
    rotationX += rotationX_temporary*0.01;
  }
  
  float rotationY_temporary = g.gyro.y;
  if(abs(rotationY_temporary) > rotationY_error) {
    rotationY += rotationY_temporary*0.01;
  }

  float rotationZ_temporary = g.gyro.z;
  if(abs(rotationZ_temporary) > rotationZ_error) {
    rotationZ += rotationZ_temporary*0.01;
  }

  readings["rotationX"] = String(rotationX);
  readings["rotationY"] = String(rotationY);
  readings["rotationZ"] = String(rotationZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Fungsi untuk mengembalikan string JSON yang terdiri dari 
// akselerasi sensor pada sumbu x, y, dan z.
String getAccelerationReadings() {
  mpu.getEvent(&a, &g, &temp);

  accelerationX = a.acceleration.x;
  accelerationY = a.acceleration.y;
  accelerationZ = a.acceleration.z;
  readings["accelerationX"] = String(accelerationX);
  readings["accelerationY"] = String(accelerationY);
  readings["accelerationZ"] = String(accelerationZ);
  String accString = JSON.stringify (readings);
  return accString;
}

// Fungsi untuk mengembalikan string suhu dari pembacaan sensor
String getTemperatureReadings(){
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}

void setup() {
  
  // Menggunakan baud rate 115200
  Serial.begin(115200);

  // Mendaftarkan PIN MPU6050
  Wire.begin(D7,D6);

  // Menghubungkan ke WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());

  // Inisiasi LittleFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Inisiasi MPU
  if (!mpu.begin()) {
    Serial.println("MPU6050 is not properly connected. Check circuit!");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found");

  // Menggunakan metode on() pada objek server untuk permintaan 
  // HTTP yang masuk dan menjalankan fungsi yang sesuai 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/",LittleFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationX=0;
    rotationY=0;
    rotationZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationX=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationY=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationZ=0;
    request->send(200, "text/plain", "OK");
  });

  // Mendaftarkan Event Source
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
}

// Mengirim semua data sensor yang diperbarui ke browser web
// sesuai dengan penundaan yang telah kami tetapkan.
void loop() {
  if ((millis() - previous_time) > gyro_delay ) {
    events.send(getGyroscopeReadings().c_str(),"gyro_readings",millis());
    previous_time = millis();
  }
  if ((millis() - previous_time_acceleration) > accelerometer_delay ) {
    events.send(getAccelerationReadings().c_str(),"accelerometer_readings",millis());
    previous_time_acceleration = millis();
  }
  if ((millis() - previous_time_temp) > temperature_delay ) {
    events.send(getTemperatureReadings().c_str(),"temperature_reading",millis());
    previous_time_temp = millis();
  }
}
