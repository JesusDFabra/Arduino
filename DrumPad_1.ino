//Test para probar velocidad de golpe
#include <MIDI.h>
#include <LiquidCrystal.h>		// importa libreria
MIDI_CREATE_DEFAULT_INSTANCE();

#define PAD1 0
#define A 3 //CLK
#define B 4 //DT
#define botonE 2 //boton encoder
#define boton1 12 //Up
#define boton2 11 //Down

//LCD
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Caracteres especiales (LOGO y Barra separadora)
//CARACTERES ESPECIALES
//LOGO
byte logo1[8] = { B00000, B00000, B00000, B00001, B00010, B00000, B00000, B00000 };
byte logo2[8] = { B01010, B01010, B11011, B01010, B01010, B01010, B01010, B01010 };
byte logo3[8] = { B00000, B00000, B00000, B10000, B01000, B00000, B00000, B00010 };
byte logo4[8] = { B01000, B01000, B01000, B00100, B00100, B00010, B00001, B00000 };
byte logo5[8] = { B01011, B01010, B01010, B01010, B01010, B01010, B01010, B11011 };
byte logo6[8] = { B00010, B00010, B00010, B00100, B00100, B01000, B10000, B00000 };

byte barra[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B10000, B10000 }; //Separador
byte H[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }; //Espacio lleno



//Temporizadoor de intervaloz
unsigned long golpeAnterior1 = 0;
unsigned long golpeActual1   = 0;
const long intervGolpes1     = 30; //milis

//VARIABLES
volatile int KEY = 0;// 0 - 255, 0 = C-2 (Do -2) //volatile, que se usa en el atach
int          KEY_ANT = 0;	//Guarda el key anterior
String Notas[12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
String NOTA =  ""; 
int OCTAVA = -2; //DE -2 A 8

//velocidad del pad (voltaje de entrada)
int vPad1, vPad1Ant = 0;
bool botPulsado = false;
bool calculando = false;
int vel=0;
bool entradaA,entradaB;
bool opc, opcThr = 0;

//Thresholds
int thrMin = 4;
int thrMax = 15;


//Var en funciones
float velocidad;
unsigned long numerador,denominador;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  Serial.begin(115200);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(boton1, INPUT_PULLUP); 
  pinMode(boton2, INPUT_PULLUP); 
  pinMode(botonE, INPUT_PULLUP); // boton Encoder

  //LOGO
  lcd.createChar(0, logo1);
  lcd.createChar(1, logo2);
  lcd.createChar(2, logo3);
  lcd.createChar(3, logo4);
  lcd.createChar(4, logo5);
  lcd.createChar(5, logo6);
  //Barra sep
  lcd.createChar(6, barra);
  lcd.createChar(7, H);


  //MIDI.begin(); //iniciamos transmicion Midi.
  attachInterrupt(digitalPinToInterrupt(A), encoderA, LOW);// interrupcion sobre pin A con low
  attachInterrupt(digitalPinToInterrupt(botonE), BotonEncoder, LOW);// interrupcion sobre pin A con low


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

  opc = 0;
  NOTA = Notas[KEY%12];
  OCTAVA = (KEY / 12) - 2;
  PantallaMain();

}

void loop() {
  //Botones 1 y 2, detección de pulso.
  if(digitalRead(boton1) == HIGH  && botPulsado == false){
    if(opc==0){
      if (KEY <= 115){KEY = KEY + 12;}
      else{KEY = 127;}
      LCD_refresh();
      botPulsado = true;
    }
    else{
      opcThr = 0;
      LCD_refresh();
      botPulsado = true;
    }
  }
  else if(digitalRead(boton2) == HIGH && botPulsado == false){
    if(opc==0){
      if (KEY >= 12){KEY = KEY - 12;}
      else{KEY = 0;} 
      LCD_refresh();
      botPulsado = true;
    }
    else{
      opcThr = 1;
      LCD_refresh();
      botPulsado = true;
    }
  }
  else if (digitalRead(boton2) == LOW && digitalRead(boton1) == LOW && digitalRead(botonE) != LOW && botPulsado == true){
    botPulsado = false;
  }

  vPad1 = analogRead(PAD1);

  if (vPad1 <= (thrMin*10.23)) {
    vPad1Ant = thrMin * 10.23;
  }

  while (vPad1 > vPad1Ant) {
    calculando = true;
    
    vPad1Ant = vPad1;
    vPad1 = analogRead(PAD1);
    /*
    //Ver señales por Plotter
    Serial.print("Lectura Pad1:");
    Serial.print(vPad1);
    Serial.print("\t");
    Serial.print("Pad1 anterior:");
    Serial.print(vPad1Ant);
    Serial.print("\t");
    Serial.print("% Vel final:");
    Serial.println(vel);*/
    vel = 0;
  }
  //Salió del While ?
  if (calculando == true) {
    calculando = false;
    golpeActual1 = millis();

    if(golpeActual1 >= golpeAnterior1 + intervGolpes1){
      vel = VelGolpe(vPad1Ant); //Valor máximo alcanzado , numero de pad
      //MIDI.sendControlChange(40 , vel, 1);
      MIDI.sendNoteOn(KEY, vel , 1);
      //MIDI.sendNoteOff(40, 127 , 1);
      golpeAnterior1 = golpeActual1;
    }
  }
}


///// FUNCION PARA CAL. LA VELOCIDAD
int VelGolpe(int valorPad) {
  if (1) {
    numerador = valorPad - (thrMin*10.23);
    denominador = (thrMax - thrMin) * 10.23;
    if (numerador > 0) {velocidad = (numerador * 127) / denominador;}
    else {velocidad = 0;}
  }
  else {velocidad = 127;}
  
  if (velocidad > 127) velocidad = 127;

  return velocidad;
}

void encoderA()  {
  static unsigned long ultimaInterrupcion = 0;
  unsigned long tiempoInterrupcion = millis();
  if (tiempoInterrupcion - ultimaInterrupcion > 5) {	// rutina antirebote desestima
    if (digitalRead(B) != LOW)			// si B es HIGH, sentido horario
    {
      if(opc == 0){
        KEY++;
      }
      else if(opcThr == 0 && thrMin < 100 && thrMin < thrMax-1){
        thrMin++;
      }
      else if(opcThr == 1 && thrMax < 100){
        thrMax++;
      }
      
    }
    else {
      if (opc == 0){
        KEY--;
      }
      else if(opcThr == 0 && thrMin > 0){
        thrMin--;
      }
      else if(opcThr == 1 && thrMax > 0 && thrMax > thrMin + 1){
        thrMax--;
      }
    }
    KEY = min(127, max(0, KEY));	// 0-127 
    ultimaInterrupcion = tiempoInterrupcion;
    LCD_refresh();

  }

}

void LCD_refresh(){
  NOTA = Notas[KEY%12];
  OCTAVA = (KEY / 12) - 2;

  if(opc==0){
    opcThr = 0;
    //Flechas
    lcd.setCursor(0, 0);
    lcd.write(126);    //arrow
    lcd.setCursor(9, 1);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print(" ");
    //Variables 
    lcd.setCursor(3, 0);
    lcd.print(NOTA);
    lcd.setCursor(6, 0);
    lcd.print(" ");
    lcd.setCursor(5, 0);
    lcd.print(OCTAVA);
    lcd.setCursor(4, 1);
    lcd.print("  ");
    lcd.setCursor(3, 1);
    lcd.print(KEY);
  }
  else{
    lcd.setCursor(0, 0);
    lcd.print(" ");
    if (opcThr == 0){
      lcd.setCursor(9, 1);
      lcd.print(" ");
      lcd.setCursor(9, 0);
    }
    else{
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(9, 1);
    }  
    lcd.write(126);    //arrow
    if(thrMin < 10){
      lcd.setCursor(15, 0);
      lcd.print("  ");
    }
    if(thrMax < 10){
      lcd.setCursor(15, 1);
      lcd.print("  ");
    }
    lcd.setCursor(13, 0);
    lcd.print(thrMin);
    lcd.print("%");

    lcd.setCursor(13, 1);
    lcd.print(thrMax);
    lcd.print("%");
  }
}
void PantallaMain(){
  lcd.clear();
  //Note & Key
  lcd.setCursor(0, 0);
  lcd.write(126);    //arrow
  lcd.setCursor(1, 0);
  lcd.print("N ");
  lcd.print(NOTA);
  lcd.setCursor(5, 0);
  lcd.print(OCTAVA);

  lcd.setCursor(1, 1);
  lcd.print("K ");
  lcd.print(KEY);

  //Barra separadora
  lcd.setCursor(8,0);
  lcd.write(byte(6));
  lcd.setCursor(8,1);
  lcd.write(byte(6));
  //thresholds
  lcd.setCursor(10, 0);
  lcd.print("T- ");
  lcd.print(thrMin);
  lcd.print("%");
  lcd.setCursor(10, 1);
  lcd.print("T+ ");
  lcd.print(thrMax);
  lcd.print("%");
}

void BotonEncoder(){
  if(botPulsado == false){
    opc = !opc;
    botPulsado = true; 
    LCD_refresh();
  }
}
