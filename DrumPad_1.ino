// include the library code:
#include <LiquidCrystal.h>
#include <MIDI.h>   // Cargarmos libreria "MIDI Library".
#include <EEPROM.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int rs = 11, en = 12, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//CARACTERES ESPECIALES
//LOGO
byte A[8] = { B00000, B00000, B00000, B00001, B00010, B00000, B00000, B00000 };
byte B[8] = { B01010, B01010, B11011, B01010, B01010, B01010, B01010, B01010 };
byte C[8] = { B00000, B00000, B00000, B10000, B01000, B00000, B00000, B00010 };
byte D[8] = { B01000, B01000, B01000, B00100, B00100, B00010, B00001, B00000 };
byte E[8] = { B01011, B01010, B01010, B01010, B01010, B01010, B01010, B11011 };
byte F[8] = { B00010, B00010, B00010, B00100, B00100, B01000, B10000, B00000 };

byte G[8] = { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111 }; //Separador
byte H[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }; //Espacio lleno para pa pantalla


float velocidad;
unsigned long numerador,denominador;

bool bandera = 0; //para saber si el boton sigue undido o no
int key = 0;// Notas inicial C-2 (Do -2)
int n = 0;
bool opc = 0; //selector entre threshol min y max en 
int tran = 0; //para saber qué movimient hacer si de izq a der o vis


//// MIDI PAD (ANALOG)
int interrup = 0; // palanquita

unsigned char vel;

//ENCODER
#define outputA 6 //CLK
#define outputB 7 //DT
#define interruptor 8 //boton para vel max
#define boton1 9
#define boton2 10
#define botonE 13 //boton encoder

int estadoA, estadoB;
int estadoA_ant;

////// Entradas analog de los pads
#define PAD1 0

//EEPROM configuración (dirección, valor)
int octava, nota;
String Notas[12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};

int vPad1 = 0; //velocidad del golpe del cada pad
int bPad1 = 0; //Banderas (para saber si estan presionados los pads

int vPad1Ant = 0; //velocidad del golpe anterior
int porcTmin = 20;
int porcTmax = 75; //sencibilidad

int thrMin = EEPROM.read(25); //El Threshold MIN  (0 - 100)
int thrMax = EEPROM.read(26); //El Threshold MAX  (0 - 100)

//int Keys EEPROM.read(0);
int Keys =0;

//declaraión de función para saber la velocidad del golpe
char rango(long valor);

void pantalla();
void pantalla2();
void setup() {
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  //LOGO
  lcd.createChar(0, A);
  lcd.createChar(1, B);
  lcd.createChar(2, C);
  lcd.createChar(3, D);
  lcd.createChar(4, E);
  lcd.createChar(5, F);

  lcd.createChar(6, G);
  lcd.createChar(7, H);

  //MIDI.begin(); //iniciamos transmicion Midi.
  Serial.begin(115200);

  /////////////////START
  lcd.clear();
  delay(200);
  lcd.setCursor(2, 0);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(2, 1);
  lcd.write(byte(3));
  lcd.setCursor(3, 1);
  lcd.write(byte(4));
  lcd.setCursor(4, 1);
  lcd.write(byte(5));
  delay(800);
  lcd.setCursor(6, 0);
  lcd.print("Jesus D");
  lcd.setCursor(6, 1);
  lcd.print("Fabra");
  delay(1600);
  lcd.clear();
  delay(300);
  /////////////////////
  pinMode(boton1, INPUT_PULLUP); //pin9
  pinMode(boton2, INPUT_PULLUP); //pin10
  pinMode(botonE, INPUT_PULLUP); //pin13 boton Encoder
  /////ENCODER//////
  pinMode (outputA, INPUT);
  pinMode (outputB, INPUT);
  estadoA_ant = digitalRead(outputA);
  pantalla();

  //Inicializaión vel pads
  vPad1Ant = analogRead(PAD1);
}

void loop() {
  /////SWITCH VEL MAX
  interrup = digitalRead(interruptor);
  if (interrup == 1){
    lcd.setCursor(0, 1);
    lcd.print(".");
  }
  else{
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }

  if (estadoA != estadoA_ant) {
    if (menu == 0) { //MENU INICIAL
      if (estadoB != estadoA) {
        if (padSelect < 7)padSelect++;
      }
      else {
        if (padSelect > 0)padSelect--;
      }
      pantalla();
    }
    else if (menu == 1) { //MENU 1
      key = Keys[padSelect];
      nota = key % 12;
      //+1/2 Tono
      if (estadoB != estadoA) {
        if (key < 128) {
          nota++;
          key++;
          Keys[padSelect] = key ;
          notasPads[padSelect] = Notas[nota];
          ///// Comprobación octava
          if (Keys[padSelect] % 12 == 0) {
            //octava = octava + (notasPads[padSelect] / 12) ;
            notasPads[padSelect] = Notas[0];
          }
        }
      }
      //-1/2 Tono
      else {
        if (key > 0) {
          key--;
          nota--;
          Keys[padSelect] = key ;
          notasPads[padSelect] = Notas[nota];
          if (nota == -1) {
            octava -- ;
            notasPads[padSelect] = Notas[11];
          }
        }
      }
      pantalla2();
    }
    else { //MENU THRESHOLD
      if (estadoB != estadoA) {
        if (porct < 100)porct++;
      }
      else {
        if (porct > 0)porct--;
      }
      pantalla();
    }
    estadoA_ant = estadoA;
  }
}

void pantalla() {
  lcd.clear();
  lcd.write(126); //Flechita
  lcd.print("N ");
  lcd.print(Notas[0]);
  lcd.print("");
  lcd.print(digitalRead(botonE));
  lcd.print(digitalRead(boton1));
  lcd.print(digitalRead(boton2));
  
}

