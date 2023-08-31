#include <LiquidCrystal.h>		// importa libreria

//LCD
const int rs = 5, en = 6, d4 = 7, d5 = 8, d6 = 9, d7 = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//ENDODER
#define A 2 //CLK
#define B 3 //DT
#define botonUP   12 //CLK
#define botonDown 11 //DT
int KEY_ANT = 0;		
//VARIABLES MIDI
volatile int KEY = 0;// 0 - 255, 0 = C-2 (Do -2)
String Notas[12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};
String NOTA =  ""; 
int OCTAVA = -2; //DE -2 A 8

//////////  SETUP   ////////////
void setup() {
  //INICIALIZACIÓN
  lcd.begin(16, 2);
  
  pinMode(A, INPUT);		// A como entrada
  pinMode(B, INPUT);		// B como entrada
  attachInterrupt(digitalPinToInterrupt(A), encoder, LOW);// interrupcion sobre pin A con low
  //START
  lcd.clear();
  delay(200);
  NOTA =  Notas[KEY];
  pantalla();
}
////////////  LOOP  //////////////
void loop() {
  //Para que cuando pase de 10 a 9 o de 100 a 99 no se quede el 0 pintado
    if (KEY != KEY_ANT) {	
      if(KEY_ANT == 10 && KEY == 9){
        lcd.setCursor(4, 1);
        lcd.print(" ");
      }
      else if (KEY_ANT == 100 && KEY == 99){
        lcd.setCursor(5, 1);
        lcd.print(" ");      
      }
    KEY_ANT = KEY ;	// asigna a ANTERIOR el valor actualizado de POSICION
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

    NOTA = Notas[KEY%12];
    OCTAVA = (KEY / 12) - 2;

    ultimaInterrupcion = tiempoInterrupcion;
    
    lcd.setCursor(3, 0);
    lcd.print(NOTA);
    
    lcd.setCursor(5, 0);
    lcd.print("  ");
    lcd.setCursor(5, 0);
    lcd.print(OCTAVA);
    lcd.setCursor(3, 1);
    lcd.print(KEY);

  }
}

void pantalla(){
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
