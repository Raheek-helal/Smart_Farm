#include <DHT.h>
#include <Servo.h>

// Sensor Pins
#define DHTPIN 2
#define DHTTYPE DHT11
const int soilMoisturePin = A0;  // POT-HG in Proteus
const int ldrPin = A1;           // LDR in Proteus

// Actuator Pins
const int relayPin = 7;     // Irrigation relay
const int led1 = 8;         // Motor status (LED-RED)
const int led2 = 9;         // Temp alert (LED-YELLOW)
const int led3 = 10;        // Light alert (LED-GREEN)
const int fanPin = 11;      // DC Fan (MOTOR-DC)
const int growLightPin = 13; // Grow light (LED-RGB)

DHT dht(DHTPIN, DHTTYPE);
Servo ventServo;

void setup() {
  Serial.begin(9600);
  dht.begin();
  ventServo.attach(6);  // SERVO-MOTOR in Proteus

  // Initialize all pins
  pinMode(relayPin, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(growLightPin, OUTPUT);

  // Default states
  digitalWrite(relayPin, LOW);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(fanPin, LOW);
  digitalWrite(growLightPin, LOW);
  ventServo.write(0); // Vent closed

  Serial.println("Smart Farm System Ready");
}

void loop() {
  // Read sensors
  int soilMoisture = analogRead(soilMoisturePin);
  int ldrValue = analogRead(ldrPin);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Print sensor data
  Serial.println("\n=== Sensor Data ===");
  Serial.print("Soil: "); Serial.print(soilMoisture);
  Serial.print(" | Light: "); Serial.print(ldrValue);
  if (!isnan(temperature) && !isnan(humidity)) {
    Serial.print(" | Temp: "); Serial.print(temperature); Serial.print("°C");
    Serial.print(" | Humidity: "); Serial.print(humidity); Serial.println("%");
  }

  // Control actuators
  controlIrrigation(soilMoisture);
  controlClimate(temperature, humidity);
  controlLighting(ldrValue);  // Now using single-parameter version

  delay(2000);
}

void controlIrrigation(int moisture) {
  if (moisture < 500) {  // Dry soil threshold
    digitalWrite(relayPin, HIGH);
    digitalWrite(led1, HIGH);
    Serial.println("ACTION: Water pump ON");
  } else {
    digitalWrite(relayPin, LOW);
    digitalWrite(led1, LOW);
  }
}

void controlClimate(float temp, float hum) {
  // EMERGENCY FIX - Force fan OFF when temp drops
  static unsigned long lastOnTime = 0;
  
  if (temp > 30.0) {
    digitalWrite(fanPin, HIGH);
    digitalWrite(led2, HIGH);
    ventServo.write(90);
    lastOnTime = millis();
    Serial.println("FAN: ON (Overheat)");
  } 
  else if (temp < 28.0 || (millis() - lastOnTime > 10000)) { 
    // Force OFF if temp <28°C OR fan was on >10sec
    digitalWrite(fanPin, LOW);
    digitalWrite(led2, LOW);
    ventServo.write(0);
    Serial.println("FAN: FORCED OFF");
  }


  // Additional humidity control (optional)
  if (hum > 75) {  // High humidity
    ventServo.write(45);  // Partially open vent
    Serial.println("CLIMATE: Vent Partially Open (High Humidity)");
  }
}

// Simplified single-parameter version
void controlLighting(int light) {
  if (light < 200) {  // Low light
    digitalWrite(growLightPin, HIGH);
    digitalWrite(led3, HIGH);
    Serial.println("ACTION: Grow light ON");
  } else {
    digitalWrite(growLightPin, LOW);
    digitalWrite(led3, LOW);
  }
}