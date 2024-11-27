#include <SoftwareSerial.h>
#include <Stepper.h>

SoftwareSerial bluetooth(2,3);

#define sensorEstacoes 5
const int PASSOS_POR_VOLTA = 2048;
Stepper mp_translacao(PASSOS_POR_VOLTA, 6, 7, 8, 9);
#define a1a 10
#define a1b 11
#define sol 12
#define sensorFasesLua 13

int modo = 0; // 0 - standby, 1 - fases lua, 2 - estações com rotação terra
/*
1: s13 + motor rotação
2: s5 + motor rotação + motor translação
*/
bool solState = false;
String msg = "";
int pwm=200;
unsigned long partidaMotorIni;
bool partidaMotor = false;
int tempoPartida = 300; // ms

bool girarTerra = true;

bool iniciouSensorLua = false;
bool primeiraFaseLua = false;
bool leituraSensorLua = false;
bool sensorLuaAnterior = true;
unsigned long tempoIniLua;
unsigned long tempoSensorLua = 0;

bool iniciouSensorTranslacao = false;
bool primeiraEstacao = false;
bool leituraSensorTranslacao = false;
bool sensorTranslacaoAnterior = true;
unsigned long tempoIniTranslacao;


int modoRotacao = 0;
int modoTranslacao = 0;

void setup()
{  
  Serial.begin(9600);
  pinMode(a1a, OUTPUT);
  pinMode(a1b, OUTPUT);

  int rpmTranslacao = 5;
  mp_translacao.setSpeed(rpmTranslacao); // leva 336 segundos volta completa, 1:24 por quarto de volta    
  bluetooth.begin(9600);
  pinMode(sol,OUTPUT);
  pinMode(sensorFasesLua,INPUT);
  pinMode(sensorEstacoes,INPUT);
}

void pararMotores(){
  analogWrite(a1a,0);
  analogWrite(a1b,0);
  mp_translacao.step(0);
}

void LeituraSensorEstacoes(){
  leituraSensorTranslacao = digitalRead(sensorEstacoes);
  if (millis() - tempoIniTranslacao >= 1000 && iniciouSensorTranslacao){
    if (primeiraEstacao){
      if (!leituraSensorTranslacao) {        
        iniciouSensorTranslacao = false;
        pararMotores();
      }      
    } else if (millis() - tempoIniTranslacao >= 2000) {
      primeiraEstacao = true;
    }    
  }

  if (leituraSensorTranslacao && sensorTranslacaoAnterior){ // apagou e estava aceso
    iniciouSensorTranslacao = true;
    tempoIniTranslacao = millis();
  } else if (!leituraSensorTranslacao && !sensorTranslacaoAnterior) { // acendeu e estava apagado
    iniciouSensorTranslacao = false;
  }  
  sensorTranslacaoAnterior = !leituraSensorTranslacao;
}

void loop()
{   
  if (modo == 0) {
    pararMotores();
    modoRotacao = 0;
    modoTranslacao = 0;
  } else {
    LeituraSensorEstacoes();
    if (modoRotacao == 1) {
      analogWrite(a1a,0);
      analogWrite(a1b,pwm);
    } else if (modoRotacao == 2) {
      analogWrite(a1a,0);
      analogWrite(a1b,255);
    } else {
      analogWrite(a1a,0);
      analogWrite(a1b,0);
    }
    if (modoTranslacao == 1) {
      mp_translacao.step(PASSOS_POR_VOLTA/360);
    } else if (modoTranslacao == 2) {
      mp_translacao.step(PASSOS_POR_VOLTA/180);
    } else {
      mp_translacao.step(0);
    }
  }
  
  if (bluetooth.available()  > 0){
    msg = bluetooth.readString();
        
    if (msg == "S"){ // sol
      solState = !solState;
      digitalWrite(sol,solState);
    }
    if (msg == "P"){ // parado
      modo = 0;
      pararMotores();
    }
    if (msg == "L"){ // lua
      modo = 1;
      if (modoRotacao != 1){
        modoRotacao = 1;
      } else {
        modoRotacao = 0;
      }
    }
    if (msg == "E"){ // estações
      modo = 2;
      if (modoTranslacao != 1){
        modoTranslacao = 1;
      } else {
        modoTranslacao = 0;
      }
    }
    if (msg == "F"){ // lua rapido
      modo = 1;
      if (modoRotacao != 2){
        modoRotacao = 2;
      } else {
        modoRotacao = 0;
      }
    }
    if (msg == "A"){ // estações rapido
      modo = 2;
      if (modoTranslacao != 2){
        modoTranslacao = 2;
      } else {
        modoTranslacao = 0;
      }
    }
  }  
}