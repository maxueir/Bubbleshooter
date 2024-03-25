

#include <DFRobot_RGBMatrix.h>  // Hardware-specific library
#include <TimerThree.h>

#define OE 9
#define LAT 10
#define CLK 11
#define A A0
#define B A1
#define C A2
#define D A3
#define E A4
#define WIDTH 64
#define _HIGH 64

DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);

//taille d'un bubble shooter : 17 de large et x de hauteur
int hauteur = 5;  //hauteur du jeu
int marge = 0;    //marge du jeu
int en_tete = 0;  //en_tete du jeu

volatile int pos = 20;  //position de la fleche d'envoi en x
volatile int incl = 0;  //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)

volatile int pos_cube_x = 0;    //position de la boule en bas a gauche en x
volatile int pos_cube_y = 0;    //position de la boule en bas a gauche en y
volatile int incl_cube = -1;    //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)
volatile int couleur_cube = 1;  //indice de la couleur utilisee dans le tableau couleurs

int jeu[26][17];//que des 0 partout par defaut, donc aucune bille

color couleurs[7] = { matrix.Color888(0, 0, 0), matrix.Color888(255, 0, 255), matrix.Color888(0, 0, 255), matrix.Color888(255, 0, 0), matrix.Color888(0, 255, 0), matrix.Color888(255, 255, 0), matrix.Color888(0, 255, 255) };

volatile bool deplacement = false;  //booleen pour indiquer si la boule est en deplacement

//DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);

void setup() {
  Serial.begin(9600);
  matrix.begin();
  //Timer3.initialize(75000);//defini l'intervalle
  Timer3.initialize(1000);//deux 0 en plus
  Timer3.attachInterrupt(deplacer_cube);
  initialisation_jeu();
  afficher_jeu();
}

void afficher_jeu(){
  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < 17; j++) {
      jeu[i][j]=random(1,7);
    }
  }
}

void initialisation_jeu(){
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 17; j++) {
      jeu[i][j]=random(1,7);
    }
  }
}

bool estPossible(int a, int b) {  //a=dir b=incl => verifier si le deplacement est possible(ne pas sortir de la zone de jeu)

  if (incl + b == 0) {
    return ((pos + a) < (63 - marge) & (pos + a) >= marge);
  } else if (incl + b == 1) {
    return ((pos + a + 6) < (63 - marge) & (pos + a) >= marge);
  } else if (incl + b == -1) {
    return ((pos + a) < (63 - marge) & (pos + a - 6) >= marge);
  } else {
    return false;
  }
}

bool case_libre() {

  return pos_cube_y > 1 + en_tete;
}

void deplacer_cube() {
  if (deplacement & case_libre()) {
    for (int i = 0; i < 2; i++) {

      if (incl_cube == 0) {
        matrix.drawRect(pos_cube_x, pos_cube_y - 1, 2, 2, couleurs[couleur_cube * i]);  //boule a envoyer
        if (i == 0) {
          pos_cube_y = pos_cube_y - 1;
        }
      } else if (incl_cube == 1) {

        matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 1, pos_cube_y, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 2, pos_cube_y - 1, couleurs[couleur_cube * i]);

        //matrix.drawLine(pos_cube_x+1,pos_cube_y-1, pos_cube_x+2, pos_cube_y-1, matrix.Color888(255, 0, 255));
        //matrix.drawLine(pos_cube_x,pos_cube_y-2, pos_cube_x+1, pos_cube_y-2, matrix.Color888(255, 0, 255));
        if (i == 0) {
          pos_cube_x = pos_cube_x + 1;
          pos_cube_y = pos_cube_y - 1;
          if (pos_cube_x == 62 - marge) {  //gere la collision a droite
            incl_cube = -1;
          }
        }
      } else if (incl_cube == -1) {

        matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 1, pos_cube_y, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x, pos_cube_y - 1, couleurs[couleur_cube * i]);

        //matrix.drawLine(pos_cube_x+1,pos_cube_y-1, pos_cube_x+2, pos_cube_y-1, matrix.Color888(255, 0, 255));
        //matrix.drawLine(pos_cube_x,pos_cube_y-2, pos_cube_x+1, pos_cube_y-2, matrix.Color888(255, 0, 255));
        if (i == 0) {
          pos_cube_x = pos_cube_x - 1;
          pos_cube_y = pos_cube_y - 1;

          if (pos_cube_x == marge) {  //gere la collision a gauche
            incl_cube = 1;
          }
        }
      }
    }
  } else if (deplacement){
    deplacement = false;
    couleur_cube = random(1, 7);
    if (incl == 0) {
      pos_cube_x = pos;
      pos_cube_y = 63 - hauteur - 5;
      matrix.drawRect(pos, matrix.height() - 7 - hauteur, 2, 2, couleurs[couleur_cube]);      //boule a envoyer
    }
    else if (incl == -1) {
      pos_cube_x = pos - 5;
      pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 4, matrix.height() - 6 - hauteur, couleurs[couleur_cube ]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 5, matrix.height() - 7 - hauteur, couleurs[couleur_cube ]);
    }
    else if (incl == 1) {
      pos_cube_x = pos + 5;
      pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos + 6, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube ]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos + 7, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube ]);
    }
  }
}


void deplacer(int dirdem, int incldem) {  //deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if (estPossible(dirdem, incldem)) {

    for (int i = 0; i < 2; i++) {

      if (incl == 0) {
        if(not(deplacement)){
        matrix.drawRect(pos, matrix.height() - 7 - hauteur, 2, 2, couleurs[couleur_cube * i]);      //boule a envoyer
        }
        matrix.drawRect(pos, matrix.height() - 4 - hauteur, 2, 4, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        
      } else if (incl == -1) {
        matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos - 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos - 2, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));

        if(not(deplacement)){
        matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 4, matrix.height() - 6 - hauteur, couleurs[couleur_cube * i]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 5, matrix.height() - 7 - hauteur, couleurs[couleur_cube * i]);
        }
      } else if (incl == 1) {
        matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos + 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos + 4, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));
        if(not(deplacement)){
        matrix.drawLine(pos + 6, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube * i]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos + 7, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube * i]);
        }
      }
      if (i == 0) {
        incl = incl + incldem;
        pos = pos + dirdem;
      }
    }
  }
}

void loop() {

  if (Serial.available() > 0) {
    char command = Serial.read();
    Serial.println(command);
    if (command == 'a') {
      Serial.print("a");
      deplacer(0, -1);
    } else if (command == 'e') {
      deplacer(0, 1);
    } else if (command == 'q') {
      deplacer(-1, 0);
    } else if (command == 'd') {
      deplacer(1, 0);
    } else if (command == 'z') {
      if (!deplacement) {
        deplacement = true;
        incl_cube = incl;

        if (incl == 0) {
          pos_cube_x = pos;
          pos_cube_y = 63 - hauteur - 5;
        }
        if (incl == -1) {
          pos_cube_x = pos - 5;
          pos_cube_y = 63 - hauteur - 5;
        }
        if (incl == 1) {
          pos_cube_x = pos + 5;
          pos_cube_y = 63 - hauteur - 5;
        }
      }
    }
  }
}
