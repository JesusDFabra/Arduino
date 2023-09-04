/*Electronic Drum MIDI
 * para 1 Pad.
 * 
 * por: Jesus David Fabra Tapias
 * Medellín-Colombia
 * Sep-04-2023
*/
#include <MIDI.h>
#include <EEPROM.h>
#include <LiquidCrystal.h> 

MIDI_CREATE_DEFAULT_INSTANCE();

#define PAD1 0     //A0
#define pinTest 7 //A7 Pruebas se activa con 5v
#define A 2        //CLK
#define B 3        //DT
#define botonE 4   //boton encoder
#define boton1 12  //Up
#define boton2 11  //Down

//LCD
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//CARACTERES ESPECIALES
//LOGO
byte logo1[8] = { B00000, B00000, B00000, B00001, B00010, B00000, B00000, B00000 };
byte logo2[8] = { B01010, B01010, B11011, B01010, B01010, B01010, B01010, B01010 };
byte logo3[8] = { B00000, B00000, B00000, B10000, B01000, B00000, B00000, B00010 };
byte logo4[8] = { B01000, B01000, B01000, B00100, B00100, B00010, B00001, B00000 };
byte logo5[8] = { B01011, B01010, B01010, B01010, B01010, B01010, B01010, B11011 };
byte logo6[8] = { B00010, B00010, B00010, B00100, B00100, B01000, B10000, B00000 };

byte barra[8] = { B00100, B00100, B00100, B00100, B00100, B00100, B00100, B00100 };  //Separador
byte flecha[8] = { B00000, B00100, B00110, B00111, B00110, B00100, B00000, B00000 };   //Flechita

//Memoria EEPROM
const int memoryKEY = 1012;
const int memoryThrMin = 1013;
const int memoryThrMax = 1014;

//VARIABLES
float thrMin = EEPROM.read(memoryThrMin);
float thrMax = EEPROM.read(memoryThrMax);
int KEY = EEPROM.read(memoryKEY);  // 0 - 255, 0 = C-2 (Do -2) //volatile, que se usa en el atach
bool testMode;

int tMin;
int tMax;


String Notas[12] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
String NOTA = "";
int OCTAVA;  //DE -2 A 8
int nPAD = 1;

//velocidad del pad (voltaje de entrada)
unsigned int vPad1, vPad1Ant = 0;
bool botPulsado = false;
bool calculando = false;
int vel = 0;
bool opc, opcThr = 0;

//Temporizador de escritura EEPROM
static unsigned long escrituraAnte = 0;
unsigned long escrituraAct = 0;
const long tiempoEntraEscrit = 3500;  //milis
bool guardando = false;

//Temporizadoor de intervalo golpes
static unsigned long golpeAnterior1 = 0;
unsigned long golpeActual1 = 0;
const long intervGolpes1 = 100;  //milis
//temporitador Encoder
static unsigned long ultimaInterrupcion = 0;
unsigned long tiempoInterrupcion = 0;
int intervaloInterrupcion = 200;  //milisegundos

//Var en funciones
float velocidad;
unsigned long numerador, denominador;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  Serial.begin(115200);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);
  pinMode(botonE, INPUT_PULLUP);  // boton Encoder
 
  //MIDI.begin(); //iniciamos transmicion Midi.
  attachInterrupt(digitalPinToInterrupt(A), encoderA, CHANGE);  // interrupcion sobre pin A con Falling
  //attachInterrupt(digitalPinToInterrupt(B), encoderB, RISING);

  //Para la primera ejecución en un nuevo arduino.
  if (thrMin < 0 || thrMin >= thrMax) { thrMin = 5; }
  if (thrMax <= thrMin || thrMax > 100) { thrMin = 25; }
  if (KEY < 0 || KEY > 127) { KEY = 0; }

  opc = 0;
  NOTA = Notas[KEY % 12];
  OCTAVA = (KEY / 12) - 2;
  tMin = thrMin;
  tMax = thrMax;

  Logo();
  PantallaMain();
  LCD_refresh();
}

void loop() { 
  if (analogRead(pinTest) > 500)testMode = 1;
  else testMode = 0;
  
  if (guardando) {
    escrituraAct = millis();
    if (escrituraAct - escrituraAnte > tiempoEntraEscrit) {
      if (opc == 0) {
        EEPROM.write(memoryKEY, KEY);
      } else {
        EEPROM.write(memoryThrMin, thrMin);  //Guardar en EEPROM
        EEPROM.write(memoryThrMax, thrMax);  //Guardar en EEPROM
      }
    }
    guardando = false;
  }
  if (digitalRead(botonE) == LOW && botPulsado == false) {
    opc = !opc;
    botPulsado = true;
    LCD_refresh();
  }
  //Botones 1 y 2, detección de pulso.
  if (digitalRead(boton1) == HIGH && botPulsado == false) {
    if (opc == 0) {
      if (KEY <= 115) {
        KEY = KEY + 12;
      } else {
        KEY = 127;
      }
      LCD_refresh();
      botPulsado = true;
    } else {
      opcThr = 0;
      LCD_refresh();
      botPulsado = true;
    }
    guardando = true;
  } else if (digitalRead(boton2) == HIGH && botPulsado == false) {
    if (opc == 0) {
      if (KEY >= 12) {
        KEY = KEY - 12;
      } else {
        KEY = 0;
      }
      LCD_refresh();
      botPulsado = true;
    } else {
      opcThr = 1;
      LCD_refresh();
      botPulsado = true;
    }
    guardando = true;
  } else if (digitalRead(boton2) == LOW && digitalRead(boton1) == LOW && digitalRead(botonE) != LOW && botPulsado == true) {
    botPulsado = false;
  }

  vPad1 = analogRead(PAD1);

  if(vPad1 > thrMin*10.23){
    golpeActual1 = millis();
    if(golpeActual1-golpeAnterior1 > intervGolpes1){
      if (vPad1 > vPad1Ant){
        calculando = 1;
        vel = 0;
      }
      
      while (vPad1 > vPad1Ant) {
        //Imprime();
        vPad1Ant = vPad1;
        vPad1 = analogRead(PAD1);
      }
      golpeAnterior1 = golpeActual1; //intervalos de golpes
    }
    //si entró al while
    if (calculando == 1) {
      calculando = 0;
      vel = VelGolpe(vPad1Ant); //Valor máximo alcanzado , numero de pad
      if (vel > 5) MIDI.sendNoteOn(KEY, vel, 1);
      //MIDI.sendNoteOff(KEY, vel , 1);
      //Imprime();
    }
  }
  else{
    vPad1Ant = thrMin*10.23;
  }
}

///// FUNCION PARA CAL. LA VELOCIDAD
int VelGolpe(int valorPad) {
  if (digitalRead(13) == LOW) { //pasar todo a float
    numerador = valorPad - (thrMin * 10.23);
    denominador = (thrMax - thrMin) * 10.23;
    if (numerador > 0) {
      velocidad = (numerador * 127) / denominador;
      if (velocidad > 127) velocidad = 127;
    } else {
      velocidad = 0;
    }
  } else {
    velocidad = 127;
  }
  return velocidad;
}

void encoderA() {
  tiempoInterrupcion = millis();
  if (tiempoInterrupcion - ultimaInterrupcion > intervaloInterrupcion) {  // rutina antirebote desestima
    if (digitalRead(B) != digitalRead(A))                                 // si B es HIGH, sentido horario
    {
      if (opc == 0) {
        KEY++;
      } else if (opcThr == 0 && thrMin < 100 && thrMin < thrMax - 1) {
        thrMin++;
      } else if (opcThr == 1 && thrMax < 100) {
        thrMax++;
      }
    } else {
      if (opc == 0) {
        KEY--;
      } else if (opcThr == 0 && thrMin > 0) {
        thrMin--;
      } else if (opcThr == 1 && thrMax > 0 && thrMax > thrMin + 1) {
        thrMax--;
      }
    }
    KEY = min(127, max(0, KEY));  // 0-127
    ultimaInterrupcion = tiempoInterrupcion;
    LCD_refresh();
  }
  guardando = true;
}
/*
void encoderB()  {
  tiempoInterrupcion = millis();
  if (tiempoInterrupcion - ultimaInterrupcion > intervaloInterrupcion) {	// rutina antirebote desestima
    if (digitalRead(A) == digitalRead(B))			// si B es HIGH, sentido horario
    {
      if(opc == 0){KEY++;}
      else if(opcThr == 0 && thrMin < 100 && thrMin < thrMax-1){thrMin++;}
      else if(opcThr == 1 && thrMax < 100){thrMax++;}
    }
    else {
      if (opc == 0){KEY--;}
      else if(opcThr == 0 && thrMin > 0){thrMin--;}
      else if(opcThr == 1 && thrMax > 0 && thrMax > thrMin + 1){thrMax--;}
    }
    KEY = min(127, max(0, KEY));	// 0-127 
    ultimaInterrupcion = tiempoInterrupcion;
    LCD_refresh();
  }
}
*/
void LCD_refresh() {
  NOTA = Notas[KEY % 12];
  OCTAVA = (KEY / 12) - 2;
  tMin = thrMin;
  tMax = thrMax;
  if (opc == 0) {
    opcThr = 0;
    //Flechas
    lcd.setCursor(1, 0);
    lcd.write(byte(7));  //arrow
    lcd.setCursor(9, 1);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    //Variables
    lcd.setCursor(4, 0);
    lcd.print(NOTA);
    lcd.setCursor(7, 0);
    lcd.print(" ");
    lcd.setCursor(6, 0);
    lcd.print(OCTAVA);
    lcd.setCursor(5, 1);
    lcd.print("  ");
    lcd.setCursor(4, 1);
    lcd.print(KEY);
  } else {
    lcd.setCursor(1, 0);
    lcd.print(" ");
    if (opcThr == 0) {
      lcd.setCursor(9, 1);
      lcd.print(" ");
      lcd.setCursor(9, 0);
    } else {
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(9, 1);
    }
    lcd.write(byte(7));  //arrow
    if (tMin < 10) {
      lcd.setCursor(15, 0);
      lcd.print("  ");
    }
    if (tMax < 10) {
      lcd.setCursor(15, 1);
      lcd.print("  ");
    }
    lcd.setCursor(13, 0);
    lcd.print(tMin);
    lcd.print("%");

    lcd.setCursor(13, 1);
    lcd.print(tMax);
    lcd.print("%");
  }
}
void PantallaMain() {
  lcd.clear();
  //PAD #1
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(0, 1);
  lcd.print("1");

  //Note & Key
  lcd.setCursor(1, 0);
  lcd.write(byte(7));  //arrow
  lcd.setCursor(2, 0);
  lcd.print("N ");
  lcd.setCursor(2, 1);
  lcd.print("K ");
  //Barra separadora
  lcd.setCursor(8, 0);
  lcd.write(byte(6));
  lcd.setCursor(8, 1);
  lcd.write(byte(6));
  //thresholds
  lcd.setCursor(10, 0);
  lcd.print("T- ");
  lcd.print(tMin);
  lcd.print("%");
  lcd.setCursor(10, 1);
  lcd.print("T+ ");
  lcd.print(tMax);
  lcd.print("%");
}
void Logo(){
   //LOGO
  lcd.createChar(0, logo1);
  lcd.createChar(1, logo2);
  lcd.createChar(2, logo3);
  lcd.createChar(3, logo4);
  lcd.createChar(4, logo5);
  lcd.createChar(5, logo6);
  //Barra sep
  lcd.createChar(6, barra);
  lcd.createChar(7, flecha);

  lcd.clear();
  delay(200);
  lcd.setCursor(2, 0);
  lcd.write(byte(0));
  lcd.write(byte(1));
  lcd.write(byte(2));

  lcd.setCursor(2, 1);
  lcd.write(byte(3));
  lcd.write(byte(4));
  lcd.write(byte(5));

  delay(800);
  lcd.setCursor(6, 0);
  lcd.print("Jesus D");
  lcd.setCursor(6, 1);
  lcd.print("Fabra");
  delay(1500);
  lcd.clear();
  delay(250);
}

void Imprime(){
    Serial.print("Lectura Pad1:");
    Serial.print(vPad1);
    Serial.print("\t");
    Serial.print("Pad1 anterior:");
    Serial.print(vPad1Ant);
    Serial.print("\t");
    Serial.print("thrMin:");
    Serial.print(thrMin*10.23);
    Serial.print("\t");
    Serial.print("Velovicad:");
    Serial.println(vel);
}