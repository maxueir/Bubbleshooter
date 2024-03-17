

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
int hauteur=0;//hauteur du jeu

volatile int pos=20;//position de la fleche d'envoi en x
volatile int incl=-1;//inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)

volatile int pos_cube_x=0;//position de la boule en bas a gauche en x
volatile int pos_cube_y=0;//position de la boule en bas a gauche en y
volatile int incl_cube=-1;//inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)

volatile bool deplacement=false;//booleen pour indiquer si la boule est en deplacement

DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);

void setup() {
  Serial.begin(9600);
  matrix.begin();
}

bool estPossible(int a, int b){//a=dir b=incl => verifier si le deplacement est possible(ne pas sortir de la zone de jeu)
  int marge=2;
  if(incl+b==0){
    return ((pos+a)<(63-marge) & (pos+a)>=marge);
  }
  else if(incl+b==1){
    return ((pos+a+6)<(63-marge) & (pos+a)>=marge);
  }
  else if(incl+b==-1){
    return ((pos+a)<(63-marge) & (pos+a-6)>=marge);
  }
  else{
    return false;
  }
}

void deplacer_cube(){
  if(deplacement){
    for(int i=0;i<2;i++){
      if(incl_cube==0){
        matrix.drawRect(pos_cube_x,pos_cube_y-1,2,2, matrix.Color888(255*i, 0, 255*i));//boule a envoyer
        if(i==0){
          pos_cube_y=pos_cube_y-1;
        }
      }

      
    }
  }
}

void deplacer(int dirdem,int incldem){//deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if(estPossible(dirdem,incldem)){
    
    for(int i=0;i<2;i++){

    if(incl==0){
      matrix.drawRect(pos,matrix.height()-7-hauteur,2,2, matrix.Color888(255*i, 0, 255*i));//boule a envoyer
      matrix.drawRect(pos,matrix.height()-4-hauteur,2,4, matrix.Color888(255*i, 0, 0));//base de la fleche
      }
    else if(incl==-1){
      matrix.drawLine(pos,matrix.height()-1-hauteur, pos-3, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));//base de la fleche
      matrix.drawLine(pos+1,matrix.height()-1-hauteur, pos-2, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));

      matrix.drawLine(pos-5,matrix.height()-6-hauteur, pos-4, matrix.height()-6-hauteur, matrix.Color888(255*i, 0, 255*i));//boule a envoyer en lignes horizontales
      matrix.drawLine(pos-6,matrix.height()-7-hauteur, pos-5, matrix.height()-7-hauteur, matrix.Color888(255*i, 0, 255*i));
    }
    else if(incl==1){
      matrix.drawLine(pos,matrix.height()-1-hauteur, pos+3, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));//base de la fleche
      matrix.drawLine(pos+1,matrix.height()-1-hauteur, pos+4, matrix.height()-4-hauteur, matrix.Color888(255*i, 0, 0));

      matrix.drawLine(pos+6,matrix.height()-6-hauteur, pos+5, matrix.height()-6-hauteur, matrix.Color888(255*i, 0, 255*i));//boule a envoyer en lignes horizontales
      matrix.drawLine(pos+7,matrix.height()-7-hauteur, pos+6, matrix.height()-7-hauteur, matrix.Color888(255*i, 0, 255*i));
    }
    if(i==0){
    incl=incl+incldem;
    pos=pos+dirdem;
    }
    }
  }
}

void loop(){
  if (Serial.available() > 0) {
    char command = Serial.read();
    Serial.println(command);
    if(command=='a'){
      deplacer(0,-1);
    }
    else if(command=='e'){
      deplacer(0,1);
    }
    else if(command=='q'){
      deplacer(-1,0);
    }
    else if(command=='d'){
      deplacer(1,0);
    }
    else if(command=='z'){
      if(!deplacement){
        deplacement=true;
        incl_cube=incl;

        if(incl==0){
          pos_cube_x=pos;
          pos_cube_y=63-hauteur-5;
        }
        if(incl==-1){
          pos_cube_x=pos-5;
          pos_cube_y=63-hauteur-5;
        }
        if(incl==1){
          pos_cube_x=pos+5;
          pos_cube_y=63-hauteur-5;
        }
      }
    }
    
    }
}
