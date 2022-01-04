// Import required libraries
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>S
#endif
#include <OneWire.h>
#include <DallasTemperature.h>

// network credentials
char ssid[] = "Upperchurch";
char pass[] = "XXXXXXXXX";

const int temperaturePin = 2;
const int relayPin = 15;

// set desired temperature in degrees celcius
const int desiredTemperature = 20;

OneWire oneWire(temperaturePin);

DallasTemperature sensors(&oneWire);

// Variable to store temperature values
String temperatureC = "";
String power = "";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readTempC() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC);
  }
  return String(tempC);
}

bool powerUp() {
  if (temperatureC.toFloat() < 20) {
    power = "On";
  }
  else {
    power = "Off";
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP DS18B20 Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Power</span>
    <span id="power">%RELAY%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 10000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("relay").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/power", true);
  xhttp.send();
}, 10000) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATUREC") {
    return temperatureC;
  }
  else if (var == "RELAY") {
    return power;
  }
  return String();
}

void setup() {
  // start serial monitor
  Serial.begin(115200);

  // initialize digital pin for relay
  pinMode(relayPin, OUTPUT);
  // start temperature sensor
  sensors.begin();
  temperatureC = readTempC();

  WiFi.begin(ssid, pass);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/power", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", power.c_str());
  });
  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    temperatureC = readTempC();
    lastTime = millis();
  }
}
