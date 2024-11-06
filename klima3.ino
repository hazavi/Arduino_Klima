// Libraries: DHT11 by Dhruba Saha, DHT sensor library by Adafruit, Adafruit Unified Sensor


// Grøn: Lyser, når den aflæste temperatur er tæt på den forvalgte temperatur /  Lyser, når currentTemp er inden for ±1°C (tempTolerance) af targetTemp.
// Rød: Lyser, når den aflæste temperatur er lavere end den forvalgte / Lyser, når currentTemp er under targetTemp minus tolerance.
// Gul: Lyser, når den aflæste temperatur er højere end den forvalgte / Lyser, når currentTemp er over targetTemp plus tolerance.
// Hvid: Lyser, når termostaten er afbrudt med knappen / Lyser, når termostaten er slået fra.


#include "DHT.h"
#include <EEPROM.h>


#define DHT11_PIN 7
#define RED_LED 9
#define YELLOW_LED 11
#define GREEN_LED 10
#define WHITE_LED 12  // Pin for hvid LED
#define BUTTON_PIN 8
#define POT_PIN A0


DHT dht11(DHT11_PIN, DHT11);


bool thermostatActive = true;
float targetTemp = 25.0;  // Standard værdi for den forvalgte temperatur
float currentTemp = 0.0;
const float tempTolerance = 1.0;  // Tolerance for at betragte "tæt nok" til den forvalgte temperatur


void setup() {
  Serial.begin(9600);
  dht11.begin();
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);  // Pin for hvid LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  // Indlæs forvalgt temperatur fra EEPROM
  EEPROM.get(0, targetTemp);


  // Hvis EEPROM er tom, sæt en standard temperatur
  if (targetTemp < 15 || targetTemp > 30) {  // Hvis den læste temperatur er udenfor det ønskede interval
    targetTemp = 25.0;  // Standard værdi
    EEPROM.put(0, targetTemp);  // Gem standard temperatur
  }


  // Udskriv den forvalgte temperatur
  Serial.print("Indlæst forvalgt temperatur: ");
  Serial.println(targetTemp);
}


void loop() {
  // Læs knap og toggl termostat status
  if (digitalRead(BUTTON_PIN) == LOW) {
    thermostatActive = !thermostatActive;
    Serial.println("Knap blev trykket, termostat status ændret!");
    delay(300);  // Debounce forsinkelse for knappen
  }


  // Juster forvalgt temperatur med potentiometeret
  int potValue = analogRead(POT_PIN);
  targetTemp = map(potValue, 0, 1023, 15, 30);  // Temp. interval: 15-30°C
  Serial.print("Potentiometer justeret, ny forvalgt temperatur: ");
  Serial.println(targetTemp);


  // Gem ny forvalgt temperatur i EEPROM
  EEPROM.put(0, targetTemp);


  // Læs temperatur fra DHT11
  currentTemp = dht11.readTemperature();


  // Tjek om sensoren har fejlet (NaN værdi)
  if (isnan(currentTemp)) {
    Serial.println("Fejl ved læsning af DHT11 sensor!");
    currentTemp = targetTemp;  // Hvis der er fejl, vis den forvalgte temperatur
  }


  // Juster LED-status baseret på temperatur og termostat status
  if (thermostatActive) {
    if (currentTemp < targetTemp - tempTolerance) {  // Hvis temperaturen er for lav
      digitalWrite(RED_LED, HIGH);   // Tænd rød LED
      digitalWrite(GREEN_LED, LOW);  // Sluk grøn LED
      digitalWrite(YELLOW_LED, LOW); // Sluk gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("RØD lyser [for lav temperatur].");
    } 
    else if (currentTemp > targetTemp + tempTolerance) {  // Hvis temperaturen er for høj
      digitalWrite(RED_LED, LOW);    // Sluk rød LED
      digitalWrite(GREEN_LED, LOW);  // Sluk grøn LED
      digitalWrite(YELLOW_LED, HIGH); // Tænd gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("GUL [lyser for høj temperatur].");
    } 
    else {  // Hvis temperaturen er tæt på den forvalgte temperatur
      digitalWrite(RED_LED, LOW);    // Sluk rød LED
      digitalWrite(GREEN_LED, HIGH); // Tænd grøn LED
      digitalWrite(YELLOW_LED, LOW); // Sluk gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("GRØN lyser [temperaturen er tæt på den forvalgte].");
    }
  } else {
    // Når termostat er afbrudt, tænd hvid LED
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(WHITE_LED, HIGH); // Tænd hvid LED
    Serial.println("Termostat er afbrudt, HVID lyser");
  }


  // Udskriv værdier til Serial Monitor
  Serial.print("Forvalgt temp: ");
  Serial.print(targetTemp);
  Serial.print(" C, Målt temp: ");
  Serial.print(currentTemp);
  Serial.println(" C");


  delay(2000);  // Vent et øjeblik før næste måling
}

