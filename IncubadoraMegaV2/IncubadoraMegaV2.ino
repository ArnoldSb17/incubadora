#include <DHT.h>                     //librerias del sensor
#include <DHT_U.h>                   //librerias del sensor
#include <LiquidCrystal.h>           //libreria de la LCD
#include <Wire.h>                    //libreria para la comunicacion I2C (integrados, datos, reloj)
#include "Sodaq_DS3231.h"            //libreria del reloj

//Pines de la lcd 16X2
   int rs = 13;
   int en = 12;
   int d4 = 11;
   int d5 = 10;
   int d6 = 9; 
   int d7 = 8;
   LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//reloj
   char DiaSemana[][4] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
   DateTime dt(2021, 3, 1, 17, 42, 0, 1);  //esta linea solo se ejecuta una vez

//Variables donde se contrala los timpos de cada proceso 
   unsigned long TiempoEstados = 0;           //tiempo de o-18 y de 18-21
   unsigned long Tiempo = 0;                  //tiempo base
   unsigned long TiempoGiroH = 0;             //tiempo giro de los huevos      
   unsigned long TiempoCambioD = 0;           //tiempo de los dias (este cariable de tiempo es provicional, es implementado solo para ver mejor la simulacion)   
   int Vandera0 = 0;                          //Variable para tener un mejor control del giro del motor
   int estado = 0;

//Pines de salida
   int lr = 7;       //Pin 7-Led rojo
   int lv = 6;       //Pin 6-Led verde
   int la = 5;       //Pin 5-Led amarillo
   int bz = 4;       //Pin 4-Buzzer
   int vtbm = 3;     //Pin 3-VentiladoBombillo
   int GMotor = 22;  //Pin 22-Motor
   int Star = 55;    //Pin 55-Inicio del programa
   int sen1 = 25;    //Pin 25-Sensor 1
   int sen2 = 26;    //Pin 26-Sensor 2

//Variables y declaracions para los sensores
   float temsen1;            //Variable de Temperatura sensor 1
   float humsen1;            //Variable de Humedad sensor 1
   DHT dht1 (sen1, DHT11);   //Establecemos que tipo de sensor es (DTH11)

   float temsen2;            //Variable de Temperatura sensor 1
   float humsen2;            //Variable de Humedad sensor 1
   DHT dht2 (sen2, DHT11);   //Establecemos que tipo de sensor es (DTH11)

   int TEMPERATURA;
   int HUMEDAD;

   const float tem39 = 39;  //la temperatura max de 0-18dias
   const float tem38 = 38;  //la temperatura min de 0-18dias
   const float tem35 = 35;  //la temperatura max de 18-21dias
   const float tem34 = 34;  //la temperatura min de 18-21dias


void setup(){

   dht1.begin();           //Iniciamos el sensor 1
   dht2.begin();           //Iniciamos el sensor 2
   rtc.begin();            //Iniciamos el controlador de timpo
   Wire.begin();           //Iniciamos la comunicacion I2C
   lcd.begin(16, 2);       //Declaramos que tipo de LCD es
   Serial.begin(9600);     //Iniciamos el puerto serial para ver errores
   rtc.setDateTime(dt);    //Iniciamos el reloj

//Empeazoamso a correr los timepos
   TiempoEstados = millis();
   TiempoGiroH = millis();
   TiempoCambioD = millis();

//Establecemos el sentido de los pines
   pinMode(lr,OUTPUT);
   pinMode(lv,OUTPUT);
   pinMode(la,OUTPUT);
   pinMode(bz,OUTPUT);
   pinMode(vtbm,OUTPUT);
   pinMode(GMotor,OUTPUT);

   pinMode(Star,INPUT);
   pinMode(sen1,INPUT);
   pinMode(sen2,INPUT);

//Mensaje de bienvenida
   mensaje1();
   //incubadora programada con arduino, proyecto de grado 
   //Luis carlos, Critian torres

}

void loop(){ 

   DateTime now = rtc.now();
   temsen1 = dht1.readTemperature();
   humsen1 = dht1.readHumidity();
   temsen2 = dht2.readTemperature();
   humsen2 = dht2.readHumidity();
   TEMPERATURA = ((temsen1+temsen2)/2);
   HUMEDAD = ((humsen1+humsen2)/2);
   Tiempo = millis();

   switch (estado){
   case 0:
   if((digitalRead(Star)==LOW)){
      estado = 0; 
   }
   if((digitalRead(Star)==HIGH)){
      lcd.clear();
      delay(100);
      estado = 1;
   }
      lcd.setCursor(0,0);
      lcd.print("ESPERANDO....");
      TiempoEstados = millis();
      TiempoGiroH = millis();

      break;

//Primero 18 dias
   case 1:
   if((Star==HIGH)&&(Tiempo <= (TiempoEstados + 24000))){  //   1555200seg = 18dias  SE MANTIENE EL CICLO DE 0-18DIAS
      estado = 1;
   }
   if(Tiempo > (TiempoEstados + 24000)){    /// CMBIA AL CICLO DE 18-21DIAS
      estado = 2;
   }


      lcd.setCursor(0,0);
      lcd.print("DIA ");
      lcd.print(now.date() );
      lcd.print('/');
      lcd.print("MES ");
      lcd.print(now.month() );
      lcd.setCursor(0,1);
      lcd.print("Tem: ");
      lcd.print(TEMPERATURA);
      lcd.print("C");
      lcd.print(" ");
      lcd.print("Hum:");
      lcd.print(HUMEDAD);
      lcd.print("%");

   if(TEMPERATURA>tem39){ 
      digitalWrite(lr,HIGH);
      digitalWrite(lv,LOW);
      digitalWrite(la,LOW);
      digitalWrite(bz,HIGH);
      digitalWrite(vtbm,LOW);   
   }
   else if (TEMPERATURA<tem39){
      digitalWrite(lr,LOW);
      digitalWrite(bz,LOW);
   }

   if((TEMPERATURA>=tem35)&&(TEMPERATURA<=tem39)){          //verde
      digitalWrite(lr,LOW);
      digitalWrite(lv,HIGH);
      digitalWrite(la,LOW);
      digitalWrite(bz,LOW);
      digitalWrite(vtbm,HIGH);
   }

   if(TEMPERATURA<tem35){                          //amarillo
      digitalWrite(lr,LOW); 
      digitalWrite(lv,LOW);
      digitalWrite(la,HIGH);
      digitalWrite(bz,HIGH);
      digitalWrite(vtbm,HIGH);
   }
   else if (TEMPERATURA>tem35){
      digitalWrite(la,LOW);
      digitalWrite(bz,LOW);
   }


   if((Vandera0==0)&&(Tiempo <= (TiempoGiroH + 5000))){
      digitalWrite(GMotor,LOW);
   }
   else if((Vandera0==0)&&(Tiempo > (TiempoGiroH + 5000))){
      TiempoGiroH = millis();
      Vandera0=1; 
   }
   if((Vandera0==1)&&(Tiempo <= (TiempoGiroH + 5000))){
      digitalWrite(GMotor,HIGH);
   }
   else if((Vandera0==1)&&(Tiempo > (TiempoGiroH + 5000))){
      TiempoGiroH = millis();
      Vandera0=0;
   } 
      break;

   case 2:
   if((Star==HIGH)&&(Tiempo <= (TiempoEstados + 44000))){     // = 1min
      estado = 2;
   }

      lcd.setCursor(0,0);
      lcd.print(now.date() );
      lcd.print('/');
      lcd.print(now.month() );
      lcd.setCursor(0,1);
      lcd.print("Tem: ");
      lcd.print(TEMPERATURA);
      lcd.print("C"); 
      lcd.print(" ");
      lcd.print("Hum:");
      lcd.print(HUMEDAD);
      lcd.print("%");

   if(TEMPERATURA>tem38){                           //rojo
      digitalWrite(lr,HIGH);
      digitalWrite(lv,LOW);
      digitalWrite(la,LOW);
      digitalWrite(vtbm,LOW);
   }
   else if(TEMPERATURA<tem38){
      digitalWrite(lr,LOW);

        }                                         
   if((TEMPERATURA>=tem34)&&(TEMPERATURA<=tem38)){          //verde
      digitalWrite(lr,LOW);
      digitalWrite(lv,HIGH);
      digitalWrite(la,LOW);
      digitalWrite(vtbm,HIGH);
   }
   if(TEMPERATURA<tem34){                          //amarillo
      digitalWrite(lr,LOW); 
      digitalWrite(lv,LOW);
      digitalWrite(la,HIGH);
      digitalWrite(vtbm,HIGH);
   }
   else if (TEMPERATURA>tem34){
   digitalWrite(la,LOW);
   }

   }

}
void mensaje1(){              
   lcd.setCursor(1,0);
   lcd.print("INCUBADORA");
   delay(50);
   lcd.setCursor(1,1);
   lcd.print("CON ARDUINO");
   delay(500);
   lcd.clear();
   lcd.setCursor(1,0);
   lcd.print("REALIZADO POR");
   lcd.setCursor(0,1);       
   lcd.print("CRISTIAN DANIEL");
   delay(500);
   lcd.clear();
   lcd.setCursor(1,0);
   lcd.print("RELIZADO POR");
   lcd.setCursor(0,1); 
   lcd.print("LUIS CARLOS");
   delay(500);
   lcd.clear();
  }
