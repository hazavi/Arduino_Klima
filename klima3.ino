// Libraries: DHT11 by Dhruba Saha, DHT sensor library by Adafruit, Adafruit Unified Sensor


// Gr�n: Lyser, n�r den afl�ste temperatur er t�t p� den forvalgte temperatur /  Lyser, n�r currentTemp er inden for �1�C (tempTolerance) af targetTemp.
// R�d: Lyser, n�r den afl�ste temperatur er lavere end den forvalgte / Lyser, n�r currentTemp er under targetTemp minus tolerance.
// Gul: Lyser, n�r den afl�ste temperatur er h�jere end den forvalgte / Lyser, n�r currentTemp er over targetTemp plus tolerance.
// Hvid: Lyser, n�r termostaten er afbrudt med knappen / Lyser, n�r termostaten er sl�et fra.


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
float targetTemp = 25.0;  // Standard v�rdi for den forvalgte temperatur
float currentTemp = 0.0;
const float tempTolerance = 1.0;  // Tolerance for at betragte "t�t nok" til den forvalgte temperatur


void setup() {
  Serial.begin(9600);
  dht11.begin();
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(WHITE_LED, OUTPUT);  // Pin for hvid LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  // Indl�s forvalgt temperatur fra EEPROM
  EEPROM.get(0, targetTemp);


  // Hvis EEPROM er tom, s�t en standard temperatur
  if (targetTemp < 15 || targetTemp > 30) {  // Hvis den l�ste temperatur er udenfor det �nskede interval
    targetTemp = 25.0;  // Standard v�rdi
    EEPROM.put(0, targetTemp);  // Gem standard temperatur
  }


  // Udskriv den forvalgte temperatur
  Serial.print("Indl�st forvalgt temperatur: ");
  Serial.println(targetTemp);
}


void loop() {
  // L�s knap og toggl termostat status
  if (digitalRead(BUTTON_PIN) == LOW) {
    thermostatActive = !thermostatActive;
    Serial.println("Knap blev trykket, termostat status �ndret!");
    delay(300);  // Debounce forsinkelse for knappen
  }


  // Juster forvalgt temperatur med potentiometeret
  int potValue = analogRead(POT_PIN);
  targetTemp = map(potValue, 0, 1023, 15, 30);  // Temp. interval: 15-30�C
  Serial.print("Potentiometer justeret, ny forvalgt temperatur: ");
  Serial.println(targetTemp);


  // Gem ny forvalgt temperatur i EEPROM
  EEPROM.put(0, targetTemp);


  // L�s temperatur fra DHT11
  currentTemp = dht11.readTemperature();


  // Tjek om sensoren har fejlet (NaN v�rdi)
  if (isnan(currentTemp)) {
    Serial.println("Fejl ved l�sning af DHT11 sensor!");
    currentTemp = targetTemp;  // Hvis der er fejl, vis den forvalgte temperatur
  }


  // Juster LED-status baseret p� temperatur og termostat status
  if (thermostatActive) {
    if (currentTemp < targetTemp - tempTolerance) {  // Hvis temperaturen er for lav
      digitalWrite(RED_LED, HIGH);   // T�nd r�d LED
      digitalWrite(GREEN_LED, LOW);  // Sluk gr�n LED
      digitalWrite(YELLOW_LED, LOW); // Sluk gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("R�D lyser [for lav temperatur].");
    } 
    else if (currentTemp > targetTemp + tempTolerance) {  // Hvis temperaturen er for h�j
      digitalWrite(RED_LED, LOW);    // Sluk r�d LED
      digitalWrite(GREEN_LED, LOW);  // Sluk gr�n LED
      digitalWrite(YELLOW_LED, HIGH); // T�nd gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("GUL [lyser for h�j temperatur].");
    } 
    else {  // Hvis temperaturen er t�t p� den forvalgte temperatur
      digitalWrite(RED_LED, LOW);    // Sluk r�d LED
      digitalWrite(GREEN_LED, HIGH); // T�nd gr�n LED
      digitalWrite(YELLOW_LED, LOW); // Sluk gul LED
      digitalWrite(WHITE_LED, LOW);  // Sluk hvid LED
      Serial.println("GR�N lyser [temperaturen er t�t p� den forvalgte].");
    }
  } else {
    // N�r termostat er afbrudt, t�nd hvid LED
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(WHITE_LED, HIGH); // T�nd hvid LED
    Serial.println("Termostat er afbrudt, HVID lyser");
  }


  // Udskriv v�rdier til Serial Monitor
  Serial.print("Forvalgt temp: ");
  Serial.print(targetTemp);
  Serial.print(" C, M�lt temp: ");
  Serial.print(currentTemp);
  Serial.println(" C");


  delay(2000);  // Vent et �jeblik f�r n�ste m�ling
}

