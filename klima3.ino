// Libraries: DHT11 by Dhruba Saha, DHT sensor library by Adafruit, Adafruit Unified Sensor

// Grøn LED: Tændes, når temperaturen er tæt på måltemperaturen (±1°C).
// Rød LED: Tændes, når temperaturen er under måltemperaturen.
// Gul LED: Tændes, når temperaturen er over måltemperaturen.
// Hvid LED: Tændes, når termostaten er slukket.

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

#define DHT11_PIN 7       ///< Pin til DHT11 sensor
#define RED_LED 9         ///< Pin til Rød LED
#define YELLOW_LED 11     ///< Pin til Gul LED
#define GREEN_LED 10      ///< Pin til Grøn LED
#define WHITE_LED 12      ///< Pin til Hvid LED (slukket termostat)
#define BUTTON_PIN 8      ///< Pin til knap (tænder/slukker termostaten)
#define POT_PIN A0        ///< Analog pin til potentiometer

DHT dht11(DHT11_PIN, DHT11);

bool thermostatActive = true; ///< Er termostaten tændt? 
float targetTemp = 25.0;      ///< Ønsket temperatur
float currentTemp = 0.0;      ///< Målt temperatur
const float tempTolerance = 1.0; ///< Tolerance for "close enough" temperature

/**
 * @brief Start indstillinger, sætter sensorer og LED'er op.
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

  // Standard temperatur, hvis værdien i EEPROM er uden for 15-30°C
  if (targetTemp < 15 || targetTemp > 30) {
    targetTemp = 25.0;
    EEPROM.put(0, targetTemp);
  }

  Serial.print("Indlæst forvalgt temperatur: ");
  Serial.println(targetTemp);
}

/**
 * @brief Hoveddelen af koden, læser sensordata, opdaterer LED'er og tjekker knappen.
 */
void loop() {
  // Skift termostatens status, hvis knappen trykkes
  if (digitalRead(BUTTON_PIN) == LOW) {
    thermostatActive = !thermostatActive;
    Serial.println("Knap blev trykket, termostat er afbrudt!");
    delay(300);  // Ventetid for at undgå gentagelser
  }

  // Juster ønsket temperatur med potentiometer
  int potValue = analogRead(POT_PIN);
  targetTemp = map(potValue, 0, 1023, 15, 30);
  Serial.print("Potentiometer justeret: ");
  Serial.println(targetTemp);

  EEPROM.put(0, targetTemp);

  // Læs current temperatur fra DHT11 sensor
  currentTemp = dht11.readTemperature();

  // Check if sensor reading failed
  if (isnan(currentTemp)) {
    Serial.println("Fejl ved læsning af DHT11 sensor!");
    currentTemp = targetTemp;  // Set current temperature to target as fallback
  }

  // Opdater LED'er baseret på temperatur
  if (thermostatActive) {
    if (currentTemp < targetTemp - tempTolerance) {
      digitalWrite(RED_LED, HIGH);   // Tænd Rød LED (for lav temperatur)
      digitalWrite(GREEN_LED, LOW);  
      digitalWrite(YELLOW_LED, LOW); 
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("RØD lyser [for lav temperatur].");
    } else if (currentTemp > targetTemp + tempTolerance) {
      digitalWrite(RED_LED, LOW);    
      digitalWrite(GREEN_LED, LOW);  
      digitalWrite(YELLOW_LED, HIGH);  // Tænd Gul LED (for høj temperatur)
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("GUL lyser [for høj temperatur].");
    } else {
      digitalWrite(RED_LED, LOW);    
      digitalWrite(GREEN_LED, HIGH);  // Tænd Grøn LED (temperatur er passende)
      digitalWrite(YELLOW_LED, LOW); 
      digitalWrite(WHITE_LED, LOW);  
      Serial.println("GRØN lyser [temperaturen er tæt på den forvalgte].");
    }
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);   // Tænd Hvid LED (termostaten er slukket)
    Serial.println("Termostat er afbrudt, HVID lyser");
  }

  Serial.print("Forvalgt temp: ");
  Serial.print(targetTemp);
  Serial.print(" C, Målt temp: ");
  Serial.print(currentTemp);
  Serial.println(" C");

  delay(2000); // Vent 2 sekunder før næste læsning
}
