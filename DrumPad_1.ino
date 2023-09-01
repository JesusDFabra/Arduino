#include <LiquidCrystal.h>		// importa libreria
#include <MIDI.h>
//LCD
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//ENDODER
#define A 2 //CLK
#define B 3 //DT
#define GolpeMax 13 //velocidad a 127
#define boton1 12 //Up
#define boton2 11 //Down
#define PAD1   0  //A0
int KEY_ANT = 0;	//Guarda el key anterior
//VARIABLES
volatile int KEY = 0;// 0 - 255, 0 = C-2 (Do -2) //volatile, que se usa en el atach
String Notas[12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
String NOTA =  ""; 
int OCTAVA = -2; //DE -2 A 8
int voltIn;
int thrMin = 10;
int thrMax = 70;

//Auxiliares
bool botPulsado = false;
//////////  SETUP   //////////////////
void setup() {
  //INICIALIZACIÓN
  lcd.begin(16, 2);
  Serial.begin(115200);
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(boton1, INPUT);
  pinMode(boton2, INPUT); 
  pinMode(GolpeMax, INPUT);

  attachInterrupt(digitalPinToInterrupt(A), encoder, LOW);// interrupcion sobre pin A con low
  //START
  lcd.clear();
  delay(200);
  NOTA =  Notas[KEY];
  Pantalla();
}
////////////  LOOP  //////////////
void loop() {
  voltIn = analogRead(PAD1); 
  if(voltIn > 5){
    Serial.println(voltIn);
  } 
  //Botones 1 y 2, detección de pulso.
  if(digitalRead(boton1) == HIGH  && botPulsado == false){
    if (KEY <= 115){KEY = KEY + 12;}
    else{KEY = 127;}
    LCD_refresh();
    botPulsado = true;
  }
  else if(digitalRead(boton2) == HIGH && botPulsado == false){
    if (KEY >= 12){KEY = KEY - 12;}
    else{KEY = 0;} 
    LCD_refresh();
    botPulsado = true;
  }
  else if (digitalRead(boton2) == LOW && digitalRead(boton1) == LOW && botPulsado == true){
    botPulsado = false;
  }
}


////////////  FUNCIONES ///////////////
void encoder()  {
  static unsigned long ultimaInterrupcion = 0;
  unsigned long tiempoInterrupcion = millis();
  if (tiempoInterrupcion - ultimaInterrupcion > 5) {	// rutina antirebote desestima
    if (digitalRead(B) != digitalRead(A))			// si B es HIGH, sentido horario
    {
      KEY++;
    }
    else {
      KEY--;
    }
    KEY = min(127, max(0, KEY));	// 0-127 
    ultimaInterrupcion = tiempoInterrupcion;
    LCD_refresh();

  }
}
///// FUNCION PARA CAL. LA VELOCIDAD
int VelGolpe(int valorPad, int Pad) {

  //valorPad = valorPad * 0.0977517106549365 ; //acondicionamos la señal

  if (interrup == HIGH) {
    numerador = valorPad - (thrMin[Pad]*10.23);
    denominador = (thrMax[Pad] - thrMin[Pad]) * 10.23;
    if (numerador > 0) {velocidad = (numerador * 127) / denominador;}
    else {velocidad = 0;}
  }
  else {velocidad = 127;}
  
  if (velocidad > 127) velocidad = 127;

  return velocidad;
}
void Pantalla(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" N ");
  lcd.setCursor(3, 0);
  lcd.print(NOTA);
  lcd.setCursor(5, 0);
  lcd.print(OCTAVA);

  lcd.setCursor(0, 1);
  lcd.print(" K ");
  lcd.setCursor(3, 1);
  lcd.print(KEY);
}

void LCD_refresh(){
    NOTA = Notas[KEY%12];
    OCTAVA = (KEY / 12) - 2;
    
    lcd.setCursor(3, 0);
    lcd.print(NOTA);
    lcd.setCursor(5, 0);
    lcd.print("  ");
    lcd.setCursor(5, 0);
    lcd.print(OCTAVA);
    lcd.setCursor(3, 1);
    lcd.print(KEY);

    if (KEY != KEY_ANT) {	
      if(KEY_ANT >= 10 && KEY < 10){
        lcd.setCursor(4, 1);
        lcd.print(" ");
      }
      else if (KEY_ANT >= 100 && KEY < 100){
        lcd.setCursor(5, 1);
        lcd.print(" ");      
      }
    KEY_ANT = KEY ;	// asigna a ANTERIOR el valor actualizado de POSICION
  }

}
