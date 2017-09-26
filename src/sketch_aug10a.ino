
// We use DHT library
#include "DHT.h"

// Set the Arduino input/output pins
#define DHT_PIN 4
#define TRIG_PIN 3
#define ECHO_PIN 2

// We trigger count down action just before hitting 0
#define COUNT_DOWN_TRIGGER 1
// The time we slepp after each loop - default 1 sec
#define LOOP_SLEEP_TIME 1000
// Turn on light if a person is less than 80cm away
#define LIGHT_ON_DISTANCE 80
// Distance to switch from automatic to manual mode
#define SWITCH_MODE_DISTANCE 5
// Seconds to sleep after switching modes
#define SWITCH_SLEEP 3
// Distances to turn ventilation on and off
#define SWITCH_MODE_START 10
#define SWITCH_MODE_STOP 25

// Set environments
#define DEV 1
#define TEST 2
#define LIVE 3
#define ENVIRONMENT DEV

// Set DEV environment
#if ENVIRONMENT == DEV
  #define LIGHT_COUNT_DOWN 10
  #define HUMIDITY_TURN_ON 75
  #define HUMIDITY_TURN_OFF 70
#endif

// Set LIVE environment
#if ENVIRONMENT == LIVE
  #define HUMIDITY_TURN_ON 80
  #define HUMIDITY_TURN_OFF 70
  #define LIGHT_COUNT_DOWN 6 * LOOP_SLEEP_TIME
#endif


// Type of sensor can be DHT11 or DHT22
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

boolean personInFront, manualMode;
boolean ventilatorIsOn, lightIsOn;
int duration, distance, humidity, temperature;
int lightOnCounter, displayCounter, i;

// Setup runs once
void setup() {
  // We will use the LED on the board to verify the code is running
  pinMode(LED_BUILTIN, OUTPUT);
  // Ultrasonic DHT pulse
  pinMode(TRIG_PIN, OUTPUT);
  // Ultrasonic DHT listen
  pinMode(ECHO_PIN, INPUT);
  dht.begin();
  Serial.begin(9600);
  // Wait for serial port to initialize
  while (!Serial) {
    ;
  }
  personInFront = false;
  manualMode = false;
  ventilatorIsOn = false;
  lightIsOn = false;
  lightOnCounter = 0;
  displayCounter = 0;
  i = 0;
}

int getDistance() {
  int distance, duration;
  // Make sure the ultrasonic sensor HC-SRO4 is off
  digitalWrite(TRIG_PIN, LOW);
  // Give it 2 ms to turn of
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  // Send impulse for 10ms (40KHz 15 degree angle)
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Listen for the response
  duration = pulseIn(ECHO_PIN, HIGH);
  // This must be calibrated for each sensor
  distance = (duration/2) / 29.1;
  return distance;
}

void processLight(){
  if (lightOnCounter == COUNT_DOWN_TRIGGER) {
    // TO DO: Turn off light here
    lightIsOn = false;
    Serial.println("Turning off lights");
  }
  if (distance < LIGHT_ON_DISTANCE) {
    personInFront = true;
    Serial.println("Person at cooktop");
    lightOnCounter = LIGHT_COUNT_DOWN;
    if (!lightIsOn) {
      lightIsOn = true;
      Serial.println("Turning on light");
      // TO DO: Turn on light here
    }
  } else {
    personInFront = false;
    if (lightOnCounter > 0) {
      lightOnCounter--;
    }
  }
}

void processHumidity() {
    if (manualMode == true) return;
    if (humidity >= HUMIDITY_TURN_ON && !ventilatorIsOn) {
      Serial.println("Turning on hood (auto)");
      // TO DO: turn on
      ventilatorIsOn = true;
    }
    if (humidity <= HUMIDITY_TURN_OFF && ventilatorIsOn) {
      Serial.println("Turning off hood (auto)");
      // TO DO: turn of
      ventilatorIsOn = false;
    }
}

void switchHoodMode() {
  if (distance < SWITCH_MODE_DISTANCE) {
    Serial.println("    Request to switch mode detected");
    if  (distance < SWITCH_MODE_DISTANCE && manualMode) {
      Serial.println("    Switching from manual to auto.");
      manualMode = false;
    } else {
      Serial.println("    Switching from auto to manual");
      manualMode = true;
    }
    Serial.print("    ");
    for (i=0; i<SWITCH_SLEEP; i++) {
      Serial.print("*");
      delay(1000);
    }
    Serial.println(" switched");
  }
}

void outputToSerial(){
  // Output
  displayCounter += 1;
  if (!(displayCounter % 5)) {
    Serial.print("      * Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("      * Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("      * Distance: ");
    Serial.print(distance);
    Serial.println(" cm.");
  }
  if (lightOnCounter > 0) {
    Serial.print("  Turning off lights in ");
    Serial.print(lightOnCounter);
    Serial.println(" sec.");
  }
  if (manualMode) {
    Serial.println("    Hood is manually controlled");
  } else {
    Serial.println("    Hood is auto controlled");
  }
  if (ventilatorIsOn) {
    Serial.println("      Hood is on");
  } else {
    Serial.println("      Hood is off");
  }
}

void processHoodMode() {
    if (manualMode == false) return;
    if (distance >= SWITCH_MODE_START && distance <= SWITCH_MODE_STOP && !ventilatorIsOn) {
      Serial.println("Manually turning on hood");
      // TO DO: turn on
      ventilatorIsOn = true;
      for (i=0; i<SWITCH_SLEEP; i++) {
        Serial.print("*");
        delay(1000);
      }
      Serial.println(" hood switched on");
      return;
    }
    if (distance >= SWITCH_MODE_START && distance <= SWITCH_MODE_STOP && ventilatorIsOn) {
      Serial.println("Manually turning off hood");
      // TO DO: turn of
      ventilatorIsOn = false;
      for (i=0; i<SWITCH_SLEEP; i++) {
        Serial.print("*");
        delay(1000);
      }
      Serial.println(" hood switched of");
      return;
    }
}

int getHumidity() {
  return dht.readHumidity();
}

int getTemperature() {
  return (dht.readTemperature(true)-32)/1.8;
}

// Loop runs forever
void loop() {
  // Signal that we are starting
  digitalWrite(LED_BUILTIN, HIGH);
  // Read distance from sensor
  distance = getDistance();
  // Control the lamps
  processLight();
  // Running in manual or automatic mode?
  switchHoodMode();
  processHoodMode();
  // Read humidity from DHT11
  humidity = getHumidity();
  processHumidity();
  //Read temperature from DHT11
  temperature = getTemperature;  
  // Do outputs
  outputToSerial();
  // Signal loop is done
  digitalWrite(LED_BUILTIN, LOW);
  // DHT11 is very cheap an can only be polled once every second
  delay(1000);
}
