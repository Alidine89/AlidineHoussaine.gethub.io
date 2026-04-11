


 
// include the library code:
//#include <LiquidCrystal.h> //library for LCD
#include <SPI.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h> 
#include <IRremote.h>

//const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 =9, d7 = 8;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

 // Code de la commande pour ouvrir le portail

// Déclaration du pin pour le récepteur IR
const int RECV_PIN = A3;
 
#define bouton1 0xBA45FF00;
#define bouton2 0xB946FF00;
//int Telecommande();
//int scanner_qrcode();

//Configuration des boroches pour le moteur
#define STEPS 64
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7
Stepper myStepper(STEPS, IN4, IN2, IN3, IN1);

 LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//Configuration des boroches pour le scanner 
#define RST_PIN 9
#define SS_PIN 10
#define Max_Acces 4
MFRC522 rfid(SS_PIN, RST_PIN);
byte Count_acces=0; 
byte CodeVerif=0; 
byte Code_Acces[4] = {0xE3, 0x97, 0xFC, 0xA7}; 
// Init array that will store new NUID 
byte nuidPICC[4];
int eta_scanner = 0;



// Configuration des broches du capteur de presence 

const int trigPin = 2; 
const int echoPin = 3; // defines variables 
long duration; 
int distance; 
int moteur_pas_a_pas_gauche();
int moteur_pas_a_pas_droite();

// Configuration des broches pour les boutons poussoirs et le relais
const int PIN_BOUTON_1 = A0;
const int PIN_BOUTON_2 = A1;


// État des boutons poussoirs
int etat_bouton_1 = 0;
int etat_bouton_2 = 0;

//************* Voyant de signalisation *************//

// LED intégrée à l'Arduino (pin 5)
const int GreenLED =8;
const int Buzzer =  A2;

// Création de l'objet decode_results pour stocker les résultats de décodage
decode_results;


void setup()
{

   Serial.begin(9600);
   IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  // Declaration et Initialisation du scanner 
  lcd.begin(16, 2);
   lcd.backlight();
  SPI.begin();
  //module_rfid.PCD_Init();
  
  // Init MFRC522 
  rfid.PCD_Init(); 
  lcd.print("détecteur");
  lcd.setCursor(0, 1);
  lcd.print("de carte");
  lcd.clear();

  // Initialisation du capteur de presence
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output 
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  digitalWrite(trigPin, LOW); 
  pinMode(GreenLED, OUTPUT);
  digitalWrite(GreenLED, LOW);
  pinMode(Buzzer, OUTPUT);
  Serial.println(" Scannez votre carte SVP! ");

  lcd.begin(16,2); // configure le nombre de colonnes et de lignes de l'écran LCD:
  lcd.setCursor(0, 0);
  lcd.print("  LA LUMIERE BRILLANTE ");
  lcd.setCursor(0, 1);
  lcd.print("PORTAIL AUTOMATIQUE");
  lcd.setCursor(0, 2);
  lcd.print(" CARTE:   ");
  lcd.setCursor(0, 3);
  lcd.print("MENTIONNE:   ");

  pinMode(PIN_BOUTON_1, INPUT);
  pinMode(PIN_BOUTON_2, INPUT);

  
  
}

void loop()
{
     
 portail_automatique();
 Telecommande();
}

void portail_automatique()
{

  float detected_seuil = 15;
  int eta_sca = scanner_qrcode();
  Serial.print(" eta_sca ");
  Serial.println(eta_sca);
   
 
}
  
int Fin_Ouverture()
{
  
   float etat_bouton_1 = analogRead(PIN_BOUTON_1);
   Serial.print(" etat_bouton_1 =  ");
   Serial.println(etat_bouton_1);
   if (etat_bouton_1 > 800) 
   {
     Serial.println("porte ouvert");
    return 1;
    }
    else {
       Serial.println("porte fermée");
      return 0;  
    }
}

int Fin_Fermeture()
{
  float etat_bouton_2 = analogRead(PIN_BOUTON_2);
  //Serial.print("eta fer = ");
  //Serial.println(etat_bouton_2);
   if (etat_bouton_2 > 800) 
   {
    Serial.println("porte fermée");
    return 1;
    }
    else {
      Serial.println("porte ouverte");
      return 0;
    }
    
}

float dectect_presence()

{
  float distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW); // Reads the echoPin, returns the sound wave travel time in microseconds 
  duration = pulseIn(echoPin, HIGH); // Calculating the distance 
  distance = duration * 0.034 / 2; // Prints the distance on the Serial Monitor 
  Serial.print("Distance: "); 
  Serial.println(distance); 
  Serial.println(" cm");

  // Si la distance est inférieure à 20 cm et le portail est fermé, ouvrir le portail
  if (distance < 20 &&  Fin_Fermeture() == 1) {
    Serial.println("Ouverture du portail...");
    moteur_pas_a_pas_gauche(1);
     digitalWrite(GreenLED,HIGH);
     tone(Buzzer,1200);
     delay(300);
      digitalWrite(GreenLED, LOW); 
    noTone(Buzzer);
   // gateState = 1; // Mettre à jour l'état du portail à ouvert
  }
  // Si la distance est supérieure à 20 cm et le portail est ouvert, fermer le portail
  else if (distance >= 20 && Fin_Ouverture() == 1) {
    Serial.println("Fermeture du portail...");
      moteur_pas_a_pas_droite(1);
     digitalWrite(GreenLED,HIGH);
     delay(300); 
     digitalWrite(GreenLED, LOW);
      digitalWrite(GreenLED, LOW); 
    noTone(Buzzer);
//    gateState = 0; // Mettre à jour l'état du portail à fermé
  }
  return(distance);
}
 

 void moteur_pas_a_pas_droite(int sens)
    {
      int valeur = 1000;
    for(int pos=0; pos < 4500 ; pos++)
    {
        int vitesse=map( valeur,523,1023,5,500);
         myStepper.setSpeed(vitesse);
         myStepper.step( -sens);
        digitalWrite(GreenLED,HIGH);     
    }
    
    } 
        
 void moteur_pas_a_pas_gauche(int sens)
    {
      int valeur = 1000;
   for(int pos=0;pos<4500;pos++)
    {
     int vitesse=map( valeur , 523 , 1023,5,500); 

       
         myStepper.setSpeed(vitesse);
         myStepper.step( sens);
        digitalWrite(GreenLED,HIGH);
       }
       
   }

  void arret_moteur_pas_a_pas()
    {
     
      myStepper.setSpeed(0);
      digitalWrite(GreenLED,LOW);
    }        
           
int scanner_qrcode()
{  
    // Initialisé la boucle si aucun badge n'est présent 
  if ( !rfid.PICC_IsNewCardPresent())
    return;

  // Vérifier la présence d'un nouveau badge 
  if ( !rfid.PICC_ReadCardSerial())
    return;

  // Afffichage 
  Serial.println(F("Un badge est détecté"));

  // Enregistrer l’ID du badge (4 octets) 
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  // Vérification du code 
  CodeVerif= GetAccesState(Code_Acces,nuidPICC); 
  if (CodeVerif!=1)
  {
    Count_acces+=1;
    if(Count_acces==Max_Acces)
    {
     // Dépassement des tentatives (clignotement infinie) 
     
     while(1)
     {
      digitalWrite(GreenLED , HIGH);
      tone(Buzzer, 1000);
      delay(200); 
      digitalWrite(GreenLED , LOW);
      noTone(Buzzer);
      delay(200); 
      // Affichage 
      Serial.println("Alarme!"); 
      }

    }
    else
    {
      // Affichage 
      Serial.println("Code érroné");
    
      // Un seul clignotement: Code erroné
      
      for(int i = 0 ; i < 7 ; i++)
      {
        digitalWrite(GreenLED , HIGH);
        tone(Buzzer,1000);
        delay(200); // Attendre pendant 500 millisecondes
        noTone(Buzzer);
        digitalWrite(GreenLED , LOW);
      }
      
      
      return 0;
    }
  }
  else
  {
    // Affichage 
    Serial.println("Ouverture de la porte");
    
    // Ouverture de la porte & Initialisation 
    tone(Buzzer,1200);
    digitalWrite(GreenLED, HIGH);
    delay(300); 
    noTone(Buzzer);
      moteur_pas_a_pas_gauche(1);
    delay(3000);
   moteur_pas_a_pas_droite(1); 
    digitalWrite(GreenLED, LOW);
    Count_acces=0; 
    return 1 ;
  }

  // Affichage de l'identifiant 
  Serial.println(" L'UID du tag est:");
  for (byte i = 0; i < 4; i++) 
  {
    Serial.print(nuidPICC[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Re-Init RFID
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD
}

byte GetAccesState(byte *CodeAcces,byte *NewCode) 
{
  byte StateAcces=0; 
  if ((CodeAcces[0]==NewCode[0])&&(CodeAcces[1]==NewCode[1])&&
  (CodeAcces[2]==NewCode[2])&& (CodeAcces[3]==NewCode[3]))
    return StateAcces=1; 
  else
    return StateAcces=0; 
}

 void Telecommande()
{
  if (IrReceiver.decode()){
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
   IrReceiver.printIRResultShort(&Serial);
    
    if(IrReceiver.decodedIRData.decodedRawData == 0xBA45FF00){
      digitalWrite(GreenLED, HIGH);
      tone(Buzzer,1200);
      moteur_pas_a_pas_gauche(1);
      delay(300);
      digitalWrite(GreenLED, LOW); 
    noTone(Buzzer);
     
    
    }
    else if(IrReceiver.decodedIRData.decodedRawData == 0xB946FF00){
      digitalWrite(GreenLED, LOW);
     moteur_pas_a_pas_droite(1);
     delay(300);
      digitalWrite(GreenLED, LOW); 
    noTone(Buzzer);
      
    }  
    IrReceiver.resume();
  }
   
  delay(1000); // Wait for 1000 millisecond(s)
}
