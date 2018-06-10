#include <Servo.h> 
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h> 
#define I2C_ADDR    0x27 // I2C Scanner adres
#define BACKLIGHT_PIN     3 
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
Servo myservo;  // creëer een servo object
int pos = 0;// variabele om de hoek van de servomotor op te slaan 
int val; // variabele om de lichtsterktte van de Iralamp op te slaan 255= Max
long FISHFEEDER = 43200000; // 12 uur tussen voederbeurten (in millis)
long endtime; // In een long kan je meer data opslaan als bv. 
//een int daarom gebruik ik een long aangezien we met milliseconden werken.
long now;
long tijdmomenteel=0;
long laatstetijd=0;
long interval=3000; // dit is de interval tussen de laatste tijd en de tijd momenteel 'delay'
int lcd_water , lcd_land;
int n = 1;
int NTC_land = 0;
int NTC_water = 1;
int Vo;
int V1;
int IraPin= 3;
int water= 12;
int uvb= 13;
int minuten_voederen;
float R1 = 10000;
float logR2, R2, T_land,T_water ,Tc_water ,Tc_land ,Tf , R3, logR3;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
long voederen_ms;
int voederen_min;
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
int IraLamp=3;
int ldrPin = A2;
int lichtsterkte;
void setup() 
{
  lcd.begin (16,2); //  Mijn scherm heeft een grootte van 16x2
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); // Zet het LCD aan
  lcd.setBacklight(HIGH); // zet het lcd aan
  lcd.home (); // go home
  myservo.attach(9);  // Hiermee configureer ik pin 9 als pin voor de servomotor 
  myservo.write(0);
  endtime = now + FISHFEEDER; // hiermee bepaal ik om de hoeveel uur ik moet voeden
  delay(15);
}
int Temperatuursmeting_naar_LCD() {
  lcd.clear();
  //T_land berekenen
  Vo = analogRead(NTC_land);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T_land = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc_land = T_land - 273.15;
  Tf = (Tc_land * 9.0)/ 5.0 + 32.0; 
  //T_water berekenen
  V1 = analogRead(NTC_water);
  R3 = R1 * (1023.0 / (float)V1 - 1.0);
  logR3 = log(R3);
  T_water = (1.0 / (c1 + c2*logR3 + c3*logR3*logR3*logR3));
  Tc_water = T_water - 273.15;
  Tf = (Tc_water * 9.0)/ 5.0 + 32.0; 
  //Maak van floats Ints
  lcd_water= (int)Tc_water;
  lcd_land= (int)Tc_land;
  //Print temperatuurswaardes op LCD
  lcd.setCursor (0,0);
  lcd.print("TW=");
  lcd.print(lcd_water);
  lcd.print("C");
  lcd.setCursor (8,0);
  lcd.print("TL=");
  lcd.print(lcd_land);
  lcd.print("C");
  //Omzetten millis naar min
  voederen_ms=FISHFEEDER-millis();
  voederen_min=voederen_ms/60000;
  //Print tijd volgende voederbeurt op LCD
  lcd.setCursor (1,1);
  lcd.print("Next=");
  lcd.print(voederen_min);
  delay(500);
  return(Tc_water,Tc_land);

}
void Voederen(){
  now = millis(); // millis is een commando dat het aantal 
                 //milliseconden sinds het opstarten van de arduino opslaat.
 
  if(now >= endtime) 
  {  
  
  for(pos = 0; pos < 90; pos += 1)  // gaat van 0 tot 90 graden
  {                                  // in stappen van 1 graad 
    myservo.write(pos);              // vertelt de servo om naar de hoek te gaan opgeslagen in variabele 'pos'
    delay(15);                       // wacht 15 milliseconden tussen iedere positieverandering van 1°
  } 
  for(pos = 90; pos>=1; pos-=1)     // gaat van 90 tot 0 graden
  {                                
    myservo.write(pos);              // vertelt de servo om naar de hoek te gaan opgeslagen in variabele 'pos'
    delay(15);                       // wacht 15 milliseconden tussen iedere positieverandering van 1°
  }
  endtime=endtime+FISHFEEDER;
  voederen_min=voederen_min+720;
  } 
}


void licht_aandoen(){
  pinMode(uvb,OUTPUT); // zet pin 13 als output
  lichtsterkte=analogRead(ldrPin); // lees waarde in 
  Serial.println(lichtsterkte); //laad deze op naar de seriële monitor (controle)
  if(lichtsterkte<=920){
  digitalWrite(uvb,LOW); // zet de lamp uit als het donker is buiten
  }else{ 
  digitalWrite(uvb,HIGH); // zet de lamp uit als het licht is buiten
  }
}

void Temperatuursregeling_land(){
 tijdmomenteel=millis(); //sla millis op sinds start programma ( delay )
 if(tijdmomenteel-laatstetijd>=interval){ //ik kan niet met delay werken (houdt programma op)
  //daarom maak ik een kunstmatige delay met bovenstaande lus
  val=constrain(val,0,255); // hierdoor kan 'val' niet hoger worden als waarde 255 (max. waarde)
  if(Tc_land<=30){ // Land kouder dan 30°C? verhoog het vermogen van de lamp.
    analogWrite(IraLamp,val);
    val=val+3;
    laatstetijd=tijdmomenteel;
   }
   if(Tc_land>=32){ // land is wamer? verlaag het vermogen
    analogWrite(IraLamp,val);
    val=val-1;
    laatstetijd=tijdmomenteel;
  }
 }
}

void Temperatuursregeling_water(){
  pinMode(water,OUTPUT); // stel digitale pin 12 in als output
 if (Tc_water <= 25){ // water kouder dan 25°C? verwarmingselement aan
  digitalWrite(water,HIGH); 
 }else{
  digitalWrite(water,LOW);// uit indien warmer dan 25°C
 }
}

void loop(){ // dit is het 'main' programma, alle onderdelen worden hierin continu doorlopen.
Temperatuursmeting_naar_LCD();
Voederen();
Temperatuursregeling_land();
Temperatuursregeling_water();
licht_aandoen();
}
