/*  semplice controller per il terrazzo:
 *   
 *   domande per pisto?
 *   - perchè si blocca del tutto se la seconda sonda trova acqua, devo creare un buleano?
 *   - per fare pause più lunghe usare un'altra variabile al posto di int? long?
 *   clock e 2 sensori umidità, uno per il terreno e uno per sapere 
 *   sto allagando tutto.
 *   Attivano o Disattivano il relè che fa andare l'elettrovalvola
 *   
 *   elenco dei pin per arduinoUno:
 * CLOCK: 
 * 3V !!!
 * SCL
 * SDA
 * 
 * RELÈ: digitale 5, 5V
 * 
 * LED per sapere se è vivo: digitale 9, 5V
 * 
 * SENSORI UMIDITÀ: 5v
 * TERRENO : A1
 * acqua perterra : A2
 */





#include <Wire.h>
#include "RTClib.h"

#define PIN_POMPA 5
#define PIN_LEDVIVO 9
unsigned long t1, dt1; // per fare il timer non bloccante
unsigned long tV, dtV; // per gestire il tempo LED vivo 
unsigned long t4, dt4; // per gestire il tempo della pompa 
int tledV = 600; // tempo in cui lampeggia il led vivo 
int stV = LOW; // stato del led vivo
int terra; // per misurare l'umidità 
int pav; // se c'è acqua sul pavimento
int th_terra = 850; // soglia per attivare il task di attivazione  se il sensore segna valori sopra gli 840 parte l'irrigazione ( se fuori dalla terra va a 900) 
int th_pav = 500; // soglia per attivare il task di attivazione 
bool POMPA; // da il via al task pompa 
int t_on_pompa = 1000; // gestione del task 
int statopompa = LOW;   // per capire quando accendo o spengo la pompa 
int mm, hh;
bool MATTINO;

RTC_DS3231 rtc;

char *res = malloc(5);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_POMPA, OUTPUT);
  digitalWrite(PIN_POMPA, LOW); 
  pinMode(PIN_LEDVIVO, OUTPUT);
  pinMode(2, INPUT);
  

  if (!rtc.begin()) {
    Serial.println("Controlla le connessioni");
    while(true);
  }

  if (rtc.lostPower()) {  // per inizzializzare di nuovo la scheda
    Serial.println("Imposto data/ora");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));    
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  leggiTerra(); // funzione per leggere l'umidità del terreno 
  leggiOra(); // per sapere quando è giorno
  leggiVivo();
}

String pad(int n) {
  sprintf(res, "%02d", n);
  return String(res);  
}

void loop() {
   /**
   * TIMER
   */
  dt1 = millis() - t1;
  if (dt1 >= 2000) {
    t1 = millis();
    leggiTerra();
    leggiOra();
    leggiVivo();
    
  }
  

  DateTime now = rtc.now();
  //int v = analogRead(A5);
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(pad(now.month()));
  Serial.print('/');
  Serial.print(pad(now.day()));
  Serial.print(" - ");
  Serial.print(pad(now.hour()));
  Serial.print(':');
  Serial.print(pad(now.minute()));
  Serial.print(':'); 
  Serial.print(pad(now.second()));
  Serial.print("    ");
  Serial.print("terra:   ");
  Serial.print(terra);
  Serial.print("      pavimento:  ");
  Serial.println( pav );

   if (POMPA) {
    dt4 = millis() - t4;
    if (dt4 > t_on_pompa) {
      t4 = millis();
      statopompa = !statopompa;
      if (statopompa) {   // se è vero 
        digitalWrite(PIN_POMPA, HIGH); // accendo la pompa 
        t_on_pompa = 30000;  // tempo dello spruzzo ( valore massimo int 32767)
      } else {
        digitalWrite(PIN_POMPA, LOW);
        t_on_pompa = 3000;
      }
    }  
  } 

}

void leggiOra(){ // per sapere quando è giorno
  DateTime now = rtc.now();
  hh = now.hour();
  mm = now.minute();  
  
  if ((hh >= 4) && (hh < 12)) { // ho cambiato questo!!!
    MATTINO = true;
  } else {
    MATTINO = false;  
  }
  
}

void leggiVivo() {
  dtV = millis() - tV;
  if (dtV > tledV) {
    tV = millis();
    stV = !stV;
    digitalWrite(PIN_LEDVIVO, stV);
    Serial.println("OK");
  }
  
}

void leggiTerra(){
  terra = 0;
  pav = 0;
  for (int i = 0; i < 10; i++) {  // leggo una decina di letture 
    terra += analogRead(A1);  
  } 
  terra = terra / 10;  // faccio la media delle letture 
  
  for (int j = 0; j < 10; j++) {  // leggo una decina di letture 
    pav += analogRead(A2);  
  } 
  pav = pav / 10;  // faccio la media delle letture 


// decido quando attivare l'irrigazione 
  if (MATTINO) {
    if (terra < th_terra && pav > th_pav ) {
      POMPA = LOW;
      digitalWrite(PIN_POMPA, LOW);  // per essere sicuro che la pompa si spenga
      Serial.println ("   umidità terreno raggiunta");;
    } else if (terra < th_terra && pav < th_pav  ) {
      POMPA = LOW;
      digitalWrite(PIN_POMPA, LOW);  // per essere sicuro che la pompa si spenga
      Serial.println ("   Acqua per terra!!!!!!!!!!!!!!!!!!!!");          
    } else if (terra > th_terra && pav < th_pav  ) {
      POMPA = LOW;
      digitalWrite(PIN_POMPA, LOW);  // per essere sicuro che la pompa si spenga
      Serial.println ("   11111111111111111111111111111111111111");          
    } else if (terra > th_terra && pav > th_pav  ) {
      POMPA = HIGH;     // accendo l'irrigazione 
      Serial.println ("   PompaON");            
    } else {
      POMPA = LOW;
      digitalWrite(PIN_POMPA, LOW);  // per essere sicuro che la pompa si spenga
    }
  } else {
    POMPA = LOW;
    digitalWrite(PIN_POMPA, LOW);
  }
}
