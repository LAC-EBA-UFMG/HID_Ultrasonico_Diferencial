/************************************************************************************
 * Controle HID Ultrasônico Diferencial                                             *
 *                                                                                  *
 * Este é um controle HID(Human Interface Device) que movimenta o  ponteiro         *
 * do Mouse no eixo X (Facilmente modificável para o eixo Y), utilizando um         *
 * Arduino Leonardo ou Pro Micro e dois sensores ultrassônicos de distância em modo *
 * diferencial. Assim, tanto a direção quanto a velocidade do ponteiro são          *
 * calculadas pela diferença da distância entre dois obstáculos, por exemplo        *
 * duas mãos. Ou seja, se as duas mãos estiverem à mesma distância dos sensores,    *
 * não há movimento. Caso haja uma diferença, o cursor se move para o lado da       *
 * mão mais próxima. Quanto maior a diferença maior será a velocidade do ponteiro.  *
 *                                                                                  *
 * Autor: Sandro Benigno                                                            *
 * Licença de uso: GNU GPL 2.0                                                      *
 *                                                                                  *
 * The GNU GPL is the most widely used free software license and has a strong       *
 * copyleft requirement. When distributing derived works, the source code of        *
 * the work must be made available under the same license. There are multiple       *
 * variants of the GNU GPL, each with different requirements.                       *
 *                                                                                  *
 **********************************************************************************/
 
#include <Arduino.h>
#include <Ultrasonic.h> //Biblioteca disponível em https://github.com/ErickSimoes/Ultrasonic
#include <Mouse.h>

#define DEBUG //Comente essa linha para desativar as informações de debug na porta serial
#define MAX_RANGE 50 //Valor máximo útil, dado em centímetros, para a detecção dos obstáculos pelos sonares
#define MAX_ACCEL 20 //Valor máximo de diferença entre as mãos, dado em centímetros (para o cálculo de aceleração do ponteiro)
#define MAX_STICK 25 //Valor máximo de deslocamento em pixels (para o cálculo de aceleração do ponteiro)
#define DEAD_ZONE 4 //Zona morta: pequena diferença em centímetros, que visa não produzir movimento
#define CLICK_ZONE 5 //distância abaixo da qual é gerado um click, caso ambas as mãos estejam próximas e sem gerar diferença
boolean on_click = 0; //variável de controle de clique

Ultrasonic L(4,5); //Sensor ultrassônico Esquerto
Ultrasonic R(2,3); //Sensor ultrassônico Direito

void setup() {
  Mouse.begin(); //Inicializando o Arduino como um dispositivo HID do tipo Mouse
  Serial.begin(115200); //Inicializando a porta serial

  //Configurando o tempo máximo de espera pelo eco ultrassônico
  L.setTimeout(40000UL);
  R.setTimeout(40000UL);
}

/*=====================================================================
* Função de validação a leitura, pela distância máxima entre as mãos. *
* Isso garante ao sistema ignorar pacotes perdidos, eliminando ruídos *
* e também verifica se ambas as mãos estão presentes para medição.    *
=====================================================================*/
bool validPack(uint8_t distl, uint8_t distr){
  if(max(distl,distr) > MAX_RANGE){
    return false;
  }
  else return true;
}

/*=====================================================
 * Função que calcula e gera o deslocamento no eixo X *
=====================================================*/
int8_t geraX(int8_t dir, uint8_t acc){
  int8_t x = dir * (float(float(acc) / float(MAX_ACCEL)) * MAX_STICK);
  return x;
}

/*=======================================================
 * Função que verifica distância mímima para um click,  *
 * dentro da zona de detecção, porém numa diferenciação * 
 * abaixo do limiar, compeendendo a  zona morta         *
=======================================================*/
boolean geraClick(uint8_t distl, uint8_t distr){
  uint8_t dist = min(distl, distr);
  if( dist <= CLICK_ZONE && !on_click){
    on_click = true;
    Mouse.press(); //Clique único no mouse
    delay(500);
    Mouse.release();
  }
}

void loop() {
  uint8_t Dist_L, Dist_R, Accel = 0; //Variáveis de Distancia e Aceleração
  int8_t JoyDir = 0; //Variável de direção

  //Leitura dos sensores
  Dist_L = L.read();
  delay(25); //um intervalo de 25ms previne ruídos por interferência entre os sensores
  Dist_R = R.read();
  delay(25);

  //Testa pacote válido, distâncias dentro do range máximo eatribui os valores
  if(validPack(Dist_L, Dist_R)){
    Accel = constrain(abs(Dist_L - Dist_R),0,MAX_ACCEL);
    if( Accel > DEAD_ZONE){
      if(Dist_L < Dist_R){
        JoyDir = -1;
      }
      else JoyDir = 1;
    }
    else{
      JoyDir = 0;
      Accel = 0;
      geraClick(Dist_L, Dist_R); //Experimental
    }
  }
  else
  {
    JoyDir = 0;
    Accel = 0;
    on_click = false;
  }

  int8_t eixo_x = geraX(JoyDir, Accel); //calcula valor de X do ponteiro

//Prints de debug caso esteja habilitado
#ifdef DEBUG
  Serial.print("L: ");
  Serial.print(Dist_L);
  Serial.print(" R: ");
  Serial.println(Dist_R);

  Serial.print("DIR:");
  Serial.print(JoyDir);

  Serial.print(" ACC:");
  Serial.print(Accel);

  Serial.print(" EIXO X:");
  Serial.println(eixo_x);
#endif
 
 Mouse.move(eixo_x, 0, 0); //Movimenta o ponteiro do mouse
}
