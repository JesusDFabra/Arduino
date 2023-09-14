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
#include <Keypad.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define PAD1 0     //A0
#define pinTest 7  //A7 Pruebas se activa con 5v
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

byte barra[8] = { B00100, B00100, B00100, B00100, B00100, B00100, B00100, B00100 };   //Separador
byte flecha[8] = { B00000, B00100, B00110, B00111, B00110, B00100, B00000, B00000 };  //Flechita

//Memoria EEPROM
int memoryKEY[4] = { 1006, 1007, 1008, 1009 };
const int memoryThrMin = 1010;
const int memoryThrMax = 1011;

//VARIABLES
float thrMin = EEPROM.read(memoryThrMin);
float thrMax = EEPROM.read(memoryThrMax);
int KEYS[4] = { EEPROM.read(memoryKEY[0]), EEPROM.read(memoryKEY[1]), EEPROM.read(memoryKEY[2]), EEPROM.read(memoryKEY[3]) };
bool testMode;

int tMin;
int tMax;

String Notas[12] = { "C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B " };
String NOTA = "";
int OCTAVA;  //DE -2 A 8
int nPAD = 1;
char tecla = '1';

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
const long intervGolpes1 = 30;  //milis

//temporitador Encoder o pulsadores
static unsigned long ultimaInterrupcion = 0;
unsigned long tiempoInterrupcion = 0;
int intervaloInterrupcion = 45;  //milisegundos

//temporitador Rebote
static unsigned long ultimoRebote = 0;
unsigned long tiempoRebote = 0;
int intervaloRebotes = 25;  //entre rebotes

//Var en funciones
float velocidad;
long numerador, denominador;

int Max = 0;  //golpe Max y a las cuantas crestas calcula
int MaxAnterior = 0;

/////teclado matricial 4x1
const byte ROWS = 1;  //four rows
const byte COLS = 4;  //three columns
char keys[ROWS][COLS] = { { '1', '2', '3', '4' } };
bool teclaPresionada = false;  // para saber si se presiono del teclado matrix
bool flagSensib, flagAnt = false;
bool LED = digitalRead(13);

byte rowPins[ROWS] = { A5 };              //connect to the row pinouts of the keypad
byte colPins[COLS] = { A3, A4, A1, A2 };  //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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
  for (int i = 0; i < 4; i++) {
    if (KEYS[i] < 0 || KEYS[i] > 127) { KEYS[i] = 0; }
  }
  opc = 0;

  nPAD = tecla - 48;  //'1' char = 47 int
  NOTA = Notas[KEYS[nPAD - 1] % 12];
  OCTAVA = (KEYS[nPAD - 1] / 12) - 2;
  tMin = thrMin;
  tMax = thrMax;

  Logo();
  PantallaMain();
  LCD_refresh();
  tecla = '1';
}

void loop() {
  tecla = keypad.getKey();
  if (tecla) {
    nPAD = tecla - 48;
    teclaPresionada = true;
    LCD_refresh();
    teclaPresionada = false;
  }
  flagAnt = LED;
  LED = digitalRead(13);
  if (flagAnt != LED) {
    if (LED == false) {
      thrMin = EEPROM.read(memoryThrMin);
      thrMax = EEPROM.read(memoryThrMax);
    } else {
      thrMin = EEPROM.read(memoryThrMin) * 2;
      thrMax = EEPROM.read(memoryThrMax) * 2;
      if(thrMax > 100) thrMax = 100;
    }
    flagSensib = true;
    LCD_refresh();
    flagSensib = false;
  }

  if (guardando) Guardar();
  if (digitalRead(botonE) == LOW && botPulsado == false) {
    opc = !opc;
    botPulsado = true;
    LCD_refresh();
  }
  //Botones 1 y 2, detección de pulso.
  if (digitalRead(boton1) == HIGH && botPulsado == false) {
    if (opc == 0) {
      if (KEYS[nPAD - 1] <= 115) KEYS[nPAD - 1] = KEYS[nPAD - 1] + 12;
      else KEYS[nPAD - 1] = 127;
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
      if (KEYS[nPAD - 1] >= 12) KEYS[nPAD - 1] = KEYS[nPAD - 1] - 12;
      else KEYS[nPAD - 1] = 0;
      LCD_refresh();
      botPulsado = true;
    } else {
      opcThr = 1;
      LCD_refresh();
      botPulsado = true;
    }
    guardando = true;
  } else if (digitalRead(boton2) == LOW && digitalRead(boton1) == LOW && digitalRead(botonE) != LOW && botPulsado == true) {
    tiempoInterrupcion = millis();
    if (tiempoInterrupcion - ultimaInterrupcion > intervaloInterrupcion * 10) {
      botPulsado = false;
    }
  }
  vPad1Ant = vPad1;
  vPad1 = analogRead(PAD1);

  if (vPad1 > 100) Imprime();

  if (vPad1 > thrMin * 10.23) {
    CalculoGolpe();
  } else {
    Max = 0;  //listo para otro rebote
    /*if(Max > 0){
      tiempoRebote = millis();
      if (tiempoRebote - ultimoRebote > intervaloRebotes) {
        Max = 0;
      }
    ultimoRebote = tiempoRebote;
    }*/
  }
}

///// FUNCION PARA CAL. LA VELOCIDAD
void CalculoGolpe() {
  int contBajas = 0;
  int contCalculos = 0;
  golpeActual1 = millis();

  if (Max == 0 && (golpeActual1 - golpeAnterior1 > intervGolpes1) && (vPad1 - vPad1Ant) > 50) {
    if (vPad1 >= vPad1Ant) {
      calculando = 1;
      vel = 0;
    }
    while (vPad1 >= vPad1Ant && contBajas < 3 && contCalculos <= 2) {
      vPad1Ant = vPad1;
      vPad1 = analogRead(PAD1);
      if (vPad1 < vPad1Ant) {
        if (Max <= vPad1Ant) {
          //MaxAnterior = Max;
          Max = vPad1Ant;
        }
        if (vPad1 > 0) {
          vPad1Ant = vPad1 - 1;
        } else {
          vPad1Ant = 0;
        }
        contBajas++;
      } else {
        if (Max < vPad1) Max = vPad1;
        contBajas = 0;
        contCalculos++;
      }
      Imprime();
    }
    //si entró al while
    if (calculando == 1) {
      calculando = 0;
      if(Max * 2 >= MaxAnterior){
        vel = VelGolpe(Max);  //Valor máximo alcanzado , numero de pad
        if (vel > 10 ) MIDI.sendNoteOn(KEYS[nPAD - 1], vel, 1);
      }
      //MIDI.sendNoteOff(KEY, vel , 1);
      Imprime();
      MaxAnterior = Max;
      Max = 0;
      vel = 0;
      Imprime();
    }
    golpeAnterior1 = golpeActual1;  //intervalos de golpes
  }
}
int VelGolpe(int valorPad) {
  if (LED == LOW) {
    numerador = valorPad - (thrMin * 10.23);
    denominador = (thrMax - thrMin) * 10.23;
    if (numerador > 0) {
      velocidad = (numerador * 127) / denominador;
      //offset
      //velocidad += 20;
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
        KEYS[nPAD - 1]++;
      } else if (opcThr == 0 && thrMin < 100 && thrMin < thrMax - 1) {
        thrMin++;
      } else if (opcThr == 1 && thrMax < 100) {
        thrMax++;
      }
    } else {
      if (opc == 0) {
        KEYS[nPAD - 1]--;
      } else if (opcThr == 0 && thrMin > 0) {
        thrMin--;
      } else if (opcThr == 1 && thrMax > 0 && thrMax > thrMin + 1) {
        thrMax--;
      }
    }
    KEYS[nPAD - 1] = min(127, max(0, KEYS[nPAD - 1]));  // 0-127
    ultimaInterrupcion = tiempoInterrupcion;
    LCD_refresh();
  }
  guardando = true;
}
void LCD_refresh() {
  NOTA = Notas[KEYS[nPAD - 1] % 12];
  OCTAVA = (KEYS[nPAD - 1] / 12) - 2;

  //float a int
  tMin = thrMin;
  tMax = thrMax;

  lcd.setCursor(0, 1);
  lcd.print(nPAD);

  if (opc == 0 || teclaPresionada) {
    opcThr = 0;
    //Flechas
    if (opc == 0) {
      lcd.setCursor(1, 0);
      lcd.write(byte(7));  //arrow
      lcd.setCursor(9, 1);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(" ");
    }
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
    lcd.print(KEYS[nPAD - 1]);
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
  }
  if (opc == 1 || flagSensib == true) {
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
void Logo() {
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
void Guardar() {
  escrituraAct = millis();
  if (escrituraAct - escrituraAnte > tiempoEntraEscrit) {
    if (opc == 0) {
      EEPROM.write(memoryKEY[nPAD - 1], KEYS[nPAD - 1]);
    } else {
      if (LED) {
        EEPROM.write(memoryThrMin, thrMin / 2);  //Guardar en EEPROM
        EEPROM.write(memoryThrMax, thrMax / 2);  //Guardar en EEPROM
      } else {
        EEPROM.write(memoryThrMin, thrMin);  //Guardar en EEPROM
        EEPROM.write(memoryThrMax, thrMax);  //Guardar en EEPROM
      }
    }
  }
  guardando = false;
}
void Imprime() {
  if (0) {
    Serial.print("Lectura Pad1:");
    Serial.print(vPad1);
    Serial.print("\t");
    Serial.print("Pad1 anterior:");
    Serial.print(vPad1Ant);
    Serial.print("\t");
    Serial.print("Max:");
    Serial.print(Max);
    Serial.print("\t");
    Serial.print("Tiempo:");
    Serial.print(golpeActual1 - golpeAnterior1);
    Serial.print("\t");
    Serial.print("thrMin:");
    Serial.print(thrMin * 10.23);
    Serial.print("\t");
    Serial.print("Velovicad:");
    Serial.println(vel);
  }
}