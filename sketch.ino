#include <DHT.h>

// Pin configuration
#define DHTPIN 15          // DHT22 data pin
#define DHTTYPE DHT22
#define HEATER_PIN 2       // Simulated heater (LED)
#define BUZZER_PIN 4       // Buzzer for overheat alert

#define TARGET_TEMP 30.0
#define HEATING_MARGIN 2.0
#define OVERHEAT_TEMP 40.0

DHT dht(DHTPIN, DHTTYPE);

enum SystemState {
  IDLE,
  HEATING,
  STABILIZING,
  TARGET_REACHED,
  OVERHEAT
};

SystemState currentState = IDLE;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Heater Control System Started");
}

void loop() {
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println("Failed to read temperature!");
    delay(2000);
    return;
  }

  updateState(temp);
  handleOutputs();
  logStatus(temp);

  delay(2000);
}

void updateState(float temp) {
  switch (currentState) {
    case IDLE:
      if (temp < TARGET_TEMP - HEATING_MARGIN) {
        currentState = HEATING;
      }
      break;

    case HEATING:
      if (temp >= TARGET_TEMP - HEATING_MARGIN && temp < TARGET_TEMP + HEATING_MARGIN) {
        currentState = STABILIZING;
      } else if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;

    case STABILIZING:
      if (temp >= TARGET_TEMP && temp < OVERHEAT_TEMP) {
        currentState = TARGET_REACHED;
      } else if (temp < TARGET_TEMP - HEATING_MARGIN) {
        currentState = HEATING;
      } else if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;

    case TARGET_REACHED:
      if (temp < TARGET_TEMP - HEATING_MARGIN) {
        currentState = HEATING;
      } else if (temp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;

    case OVERHEAT:
      if (temp < TARGET_TEMP) {
        currentState = IDLE;
      }
      break;
  }
}

void handleOutputs() {
  switch (currentState) {
    case IDLE:
    case TARGET_REACHED:
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      break;
    case HEATING:
    case STABILIZING:
      digitalWrite(HEATER_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
      break;
    case OVERHEAT:
      digitalWrite(HEATER_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
      break;
  }
}

void logStatus(float temp) {
  String stateStr;
  switch (currentState) {
    case IDLE: stateStr = "IDLE"; break;
    case HEATING: stateStr = "HEATING"; break;
    case STABILIZING: stateStr = "STABILIZING"; break;
    case TARGET_REACHED: stateStr = "TARGET_REACHED"; break;
    case OVERHEAT: stateStr = "OVERHEAT"; break;
  }
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C, State: ");
  Serial.println(stateStr);
}
