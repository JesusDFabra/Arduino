//Test para probar velocidad de golpe
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define PAD1 0
int vPad1, vPad1Ant = 0;
bool calculando = false;
int vel=0;
//Temporizadoor de intervaloz
unsigned long golpeAnterior1 = 0;
unsigned long golpeActual1   = 0;
const long intervGolpes1 = 40;

int thrMin = 8;
int thrMax = 25;


float velocidad;
unsigned long numerador,denominador;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //MIDI.begin(); //iniciamos transmicion Midi.

}

void loop() {
  vPad1 = analogRead(PAD1);

  if (vPad1 <= (thrMin*10.23)) {
    vPad1Ant = thrMin * 10.23;
  }

  while (vPad1 > vPad1Ant) {
    calculando = true;
    
    vPad1Ant = vPad1;
    vPad1 = analogRead(PAD1);
    /*
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
      MIDI.sendNoteOn(40, 127 , 1);
      delay(5);
      //MIDI.sendNoteOff(40, 127 , 1);
      golpeAnterior1 = golpeActual1;
    }
  }
  //delay(2);
}
///// FUNCION PARA CAL. LA VELOCIDAD
int VelGolpe(int valorPad) {
  
  //valorPad = valorPad * 0.0977517106549365 ; //acondicionamos la señal

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