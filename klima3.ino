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
 * Dette program styrer en simpel termostat med temperaturmåling, LED’er til 
 * visning af temperaturstatus, og en potentiometer til at justere måltemperaturen.
 */

#include "DHT.h"  // Tilføj bibliotek for DHT11-sensor
#include <EEPROM.h>  // Tilføj bibliotek til EEPROM-hukommelse

// Definer pins for sensorer, LED'er og knapper
#define DHT11_PIN 7       ///< Pin til DHT11 sensor
#define RED_LED 9         ///< Pin til Rød LED
#define YELLOW_LED 11     ///< Pin til Gul LED
#define GREEN_LED 10      ///< Pin til Grøn LED
#define WHITE_LED 12      ///< Pin til Hvid LED (slukket termostat)
#define BUTTON_PIN 8      ///< Pin til knap (tænder/slukker termostaten)
#define POT_PIN A0        ///< Analog pin til potentiometer

DHT dht11(DHT11_PIN, DHT11);  // Initialiser DHT11-sensoren

// Definer variabler til termostatstatus og temperaturindstillinger
bool thermostatActive = true; ///< Er termostaten tændt? 
float targetTemp = 25.0;      ///< Ønsket temperatur (standard 25°C)
float currentTemp = 0.0;      ///< Målt temperatur
const float tempTolerance = 1.0; ///< Tolerance for "tætpå" temperatur

/**
 * @brief Setup-funktion, kører én gang når Arduino starter.
 * Initialiserer seriel kommunikation, DHT11-sensoren, LED'er og knappen.
 */
void setup() {
  Serial.begin(9600);  // Start seriel kommunikation for debugging
  dht11.begin();  // Start DHT11-sensor
  
  // Sæt LED'er og knappen til input eller output
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Hent tidligere gemt måltemperatur fra EEPROM
  EEPROM.get(0, targetTemp);

  // Sæt en standard temperatur, hvis den gemte værdi er uden for 15-30°C
  if (targetTemp < 15 || targetTemp > 30) {
    targetTemp = 25.0;
    EEPROM.put(0, targetTemp);  // Gem standard værdi til EEPROM
  }

  // Vis den læste måltemperatur i Seriel Monitor
  Serial.print("Indlæst forvalgt temperatur: ");
  Serial.println(targetTemp);
}

/**
 * @brief Hovedprogrammet, som gentages hele tiden.
 * Læser sensorværdier, opdaterer LED'er, og håndterer knapinput.
 */
void loop() {
  // Skift termostatstatus ved tryk på knappen
  if (digitalRead(BUTTON_PIN) == LOW) {
    thermostatActive = !thermostatActive;  // Skift termostatens tilstand
    Serial.println("Knap blev trykket, termostat Afbrudt!");
    delay(300);  // delay for at undgå gentagne tryk
  }

  // Læs potentiometeret for at justere måltemperaturen
  int potValue = analogRead(POT_PIN);  // Læs potentiometerets værdi
  targetTemp = map(potValue, 0, 1023, 15, 30);  // Skaler til 15-30°C interval
  Serial.print("Potentiometer justeret: ");
  Serial.println(targetTemp);

  // Gem den justerede måltemperatur i EEPROM
  EEPROM.put(0, targetTemp);

  // Læs temperaturen fra DHT11-sensoren
  currentTemp = dht11.readTemperature();

  // Kontroller om sensorens læsning lykkedes
  if (isnan(currentTemp)) {
    Serial.println("Fejl ved læsning af DHT11 sensor!");
    currentTemp = targetTemp;  // Brug måltemperatur som backup
  }

  // Opdater LED-status baseret på mål- og faktisk temperatur
  if (thermostatActive) {  // Kun når termostaten er tændt
    if (currentTemp < targetTemp - tempTolerance) {
      digitalWrite(RED_LED, HIGH);   // Rød LED: for lav temperatur
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(WHITE_LED, LOW);
      Serial.println("RØD lyser [for lav temperatur].");  
    } else if (currentTemp > targetTemp + tempTolerance) {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);  // Gul LED: for høj temperatur
      digitalWrite(WHITE_LED, LOW);
      Serial.println("GUL lyser [for høj temperatur].");
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);  // Grøn LED: temperaturen er tæt på mål
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(WHITE_LED, LOW);
      Serial.println("GRØN lyser [temperaturen er tæt på den forvalgte].");
    }
  } else {  // Hvis termostaten er slukket
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(WHITE_LED, HIGH);  // Hvid LED: termostaten er slukket
    Serial.println("Termostat er afbrudt, HVID lyser");
  }

  // Vis aktuelle temperaturer i Seriel Monitor
  Serial.print("Forvalgt temp: ");
  Serial.print(targetTemp);
  Serial.print(" C, Målt temp: ");
  Serial.print(currentTemp);
  Serial.println(" C");

  delay(2000);  // Vent 2 sekunder før næste opdatering
}
