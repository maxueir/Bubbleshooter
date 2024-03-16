

#include <DFRobot_RGBMatrix.h> // Hardware-specific library

#define OE   	9
#define LAT 	10
#define CLK 	11
#define A   	A0
#define B   	A1
#define C   	A2
#define D   	A3
#define E   	A4
#define WIDTH 64
#define _HIGH	64

//taille d'un bubble shooter : 17 de large et x de hauteur
volatile int pos=2;//position de la fleche d'envoi en x
volatile int incl=0;//inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)
volatile bool deplacement=false;//booleen pour indiquer si la boule est en deplacement

DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);

void setup() {
  Serial.begin(9600);
  matrix.begin();
}

void est

void deplacer(int dirdem,int incldem){//deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if(estPossible(dirdem,incldem)){
    int hauteur=0;
    for(int i=0;i<2;i++){

    if(incl==1){
      matrix.drawRect(pos,matrix.height()-7-hauteur,2,2, matrix.Color888(255*i, 0, 255*i));//boule a envoyer
      matrix.drawRect(pos,matrix.height()-4-hauteur,2,4, matrix.Color888(255*i, 0, 0));//base de la fleche
      }
    else if(incl==0){
      matrix.drawLine(pos,matrix.height()-1-hauteur, pos-3, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));//base de la fleche
      matrix.drawLine(pos+1,matrix.height()-1-hauteur, pos-2, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));

      matrix.drawLine(pos-5,matrix.height()-6-hauteur, pos-4, matrix.height()-6-hauteur, matrix.Color888(255*i, 0, 255*i));//boule a envoyer en lignes horizontales
      matrix.drawLine(pos-6,matrix.height()-7-hauteur, pos-5, matrix.height()-7-hauteur, matrix.Color888(255*i, 0, 255*i));
    }
    else if(incl==2){
      matrix.drawLine(pos,matrix.height()-1-hauteur, pos+3, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));//base de la fleche
      matrix.drawLine(pos+1,matrix.height()-1-hauteur, pos+4, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));

      matrix.drawLine(pos+6,matrix.height()-6-hauteur, pos+5, matrix.height()-6-hauteur, matrix.Color888(255*i, 0, 255*i));//boule a envoyer en lignes horizontales
      matrix.drawLine(pos+7,matrix.height()-7-hauteur, pos+6, matrix.height()-7-hauteur, matrix.Color888(255*i, 0, 255*i));
    }

    incl=incl+incldem;
    pos=pos+dirdem;
    }
  }
}

void loop(){
 deplacer(true,);
 delay(1000);
 matrix.fillScreen(matrix.Color888(0,0,0));
}
