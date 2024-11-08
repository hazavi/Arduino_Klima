// Libraries: DHT11 by Dhruba Saha, DHT sensor library by Adafruit, Adafruit Unified Sensor

// Grøn: Lyser, når den aflæste temperatur er tæt på den forvalgte temperatur /  Lyser, når currentTemp er inden for ±1°C (tempTolerance) af targetTemp.
// Rød: Lyser, når den aflæste temperatur er lavere end den forvalgte / Lyser, når currentTemp er under targetTemp minus tolerance.
// Gul: Lyser, når den aflæste temperatur er højere end den forvalgte / Lyser, når currentTemp er over targetTemp plus tolerance.
// Hvid: Lyser, når termostaten er afbrudt med knappen / Lyser, når termostaten er slået fra.

/**
 * @file klima3.ino
 * @author Hazel
 * @brief Klima Thermostat kode
 * @version 0.1
 * @date 2024-11-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "DHT.h"
#include <EEPROM.h>

#define DHT11_PIN 7       ///< Pin for DHT11 sensor
#define RED_LED 9         ///< Pin for Red LED
#define YELLOW_LED 11     ///< Pin for Yellow LED
#define GREEN_LED 10      ///< Pin for Green LED
#define WHITE_LED 12      ///< Pin for White LED (thermostat off)
#define BUTTON_PIN 8      ///< Pin for Button to toggle thermostat
#define POT_PIN A0        ///< Analog pin for Potentiometer

DHT dht11(DHT11_PIN, DHT11);

bool thermostatActive = true; ///< Thermostat state
float targetTemp = 25.0;      ///< Target temperature
float currentTemp = 0.0;      ///< Current measured temperature
const float tempTolerance = 1.0; ///< Tolerance for "close enough" temperature

/**
 * @brief Sets up the initial state of the system, including initializing sensors and LEDs.
 */
void setup() {
  Serial.begin(9600);
  dht11.begin();
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  EEPROM.get(0, targetTemp);

  // Set default temperature if EEPROM data is out of range
  if (targetTemp < 15 || targetTemp > 30) {
    targetTemp = 25.0;
    EEPROM.put(0, targetTemp);
  }

  Serial.print("Indlæst forvalgt temperatur: ");
  Serial.println(targetTemp);
}

/**
 * @brief Main loop that reads sensor values, updates LED states, and checks button status.
 */
void loop() {
  // Toggle thermostat status on button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    thermostatActive = !thermostatActive;
    Serial.println("Knap blev trykket, termostat er afbrudt!");
    delay(300);  // Debounce delay
  }

  // Adjust target temperature using potentiometer
  int potValue = analogRead(POT_PIN);
  targetTemp = map(potValue, 0, 1023, 15, 30);
  Serial.print("Potentiometer justeret: ");
  Serial.println(targetTemp);

  EEPROM.put(0, targetTemp);

  // Read current temperature from DHT11 sensor
  currentTemp = dht11.readTemperature();

  // Check if sensor reading failed
  if (isnan(currentTemp)) {
    Serial.println("Fejl ved læsning af DHT11 sensor!");
    currentTemp = targetTemp;  // Set current temperature to target as fallback
  }

  // Adjust LED states based on temperature
  if (thermostatActive) {
    if (currentTemp < targetTemp - tempTolerance) {
      digitalWrite(RED_LED, HIGH);   
      digitalWrite(GREEN_LED, LOW);  
      digitalWrite(YELLOW_LED, LOW); 
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("RØD lyser [for lav temperatur].");
    } else if (currentTemp > targetTemp + tempTolerance) {
      digitalWrite(RED_LED, LOW);    
      digitalWrite(GREEN_LED, LOW);  
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("GUL lyser [for høj temperatur].");
    } else {
      digitalWrite(RED_LED, LOW);    
      digitalWrite(GREEN_LED, HIGH); 
      digitalWrite(YELLOW_LED, LOW); 
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("GRØN lyser [temperaturen er tæt på den forvalgte].");
    }
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);
    Serial.println("Termostat er afbrudt, HVID lyser");
  }

  Serial.print("Forvalgt temp: ");
  Serial.print(targetTemp);
  Serial.print(" C, Målt temp: ");
  Serial.print(currentTemp);
  Serial.println(" C");

  delay(2000);
}
