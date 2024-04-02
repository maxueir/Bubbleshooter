
#include <TimerThree.h>
#include <DFRobot_RGBMatrix.h>  // Hardware-specific library
#include <gamma.h>
#include <Adafruit_GFX.h>




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

//taille d'un bubble shooter : 17 de large et 16 de hauteur (17=> peerdu)
int hauteur = 1;  //hauteur du jeu
int marge_g = 1;    //marge du jeu
int marge_d = 1;    //marge du jeu
int en_tete = 1;  //en_tete du jeu

int nb_couleur=7;

volatile int pos = 30;  //position de la fleche d'envoi en x
volatile int incl = 0;  //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)

volatile int score = 0; // score

volatile int pos_cube_x = 0;    //position de la boule en bas a gauche en x
volatile int pos_cube_y = 0;    //position de la boule en bas a gauche en y
volatile int incl_cube = 0;    //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)
volatile int couleur_cube = 1;  //indice de la couleur utilisee dans le tableau couleurs

short jeu[15][15];//que des 0 partout par defaut, donc aucune bille

//DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);
color couleurs[7] = { matrix.Color888(0, 0, 0), matrix.Color888(255, 0, 255), matrix.Color888(0, 0, 255), matrix.Color888(255, 0, 0), matrix.Color888(0, 255, 0), matrix.Color888(255, 255, 0), matrix.Color888(0, 255, 255) };

volatile bool deplacement = false;  //booleen pour indiquer si la boule est en deplacement



void setup() {
  matrix.begin();
  Serial.begin(9600);
  delay(1100); // nécessaire pour que l'animation de lancement du jeu ne se fasse qu'une fois 
  //Serial.println("ici");
  //matrix.fillScreen(0);
  //Timer3.initialize(75000);//defini l'intervalle
  Timer3.initialize(10000);//deux 0 en plus
  Timer3.attachInterrupt(deplacer_cube);
  //init_anim(); // animation de lancement du jeu
  //init_interface(); // initialise l'interface de jeu
  initialisation_jeu();
  //delay(1000);
  //Serial.print("affichage");
  //afficher_jeu();
}

void init_anim() {
    static int x_letter = (matrix.width()/2)-3;
    static int y_letter = (matrix.height()/2)-3;

    static int x;
    static int y;
    static int dx;
    static int dy;

    for (int i = 0; i < 2; i++) { // répète la boucle d'animation, 2 fois
      for (int k = 0; k < 3; k++) { // les leds de la matrice s'allument toutes, 3 fois
        matrix.fillRect(0,0, matrix.width()-1, matrix.height()-1, matrix.Color888(255, 0, 0));
        delay(50);
        matrix.fillScreen(matrix.Color888(0, 0, 0)); // Efface l'écran
      }

      for (int j = 0; j < 2; j++) { // allume les leds en faisant le contour de la matrice, 2 fois
        x = 1;
        y = 0;
        dx = 1;
        dy = 0;     
        matrix.drawChar(x_letter-(7*3), (matrix.height()/2)-8, 'B', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter-(7*2), (matrix.height()/2)-8, 'u', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter-7, (matrix.height()/2)-8, 'b', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter, (matrix.height()/2)-8, 'b', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter+7, (matrix.height()/2)-8, 'l', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter+(7*2), (matrix.height()/2)-8, 'e', matrix.Color888(255,0,0), 0, 1);

        matrix.drawChar(x_letter-(7*3), (matrix.height()/2), 'S', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter-(7*2), (matrix.height()/2), 'h', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter-7, (matrix.height()/2), 'o', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter, (matrix.height()/2), 'o', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter+7, (matrix.height()/2), 't', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter+(7*2), (matrix.height()/2), 'e', matrix.Color888(255,0,0), 0, 1);
        matrix.drawChar(x_letter+(7*3), (matrix.height()/2), 'r', matrix.Color888(255,0,0), 0, 1);

        while (x!=0 || y!=0) {     // animation pixel tournant      
          matrix.drawPixel(x, y, matrix.Color333(7, 7, 7)); // Allumer les pixels en blanc
          x += dx;      // Mettre à jour les positions
          y += dy;
          if (x==0 && y==0) {     // Vérifier si la ligne a atteint un bord et changer de direction si nécessaire
              dx = 1;
              dy = 0;
          }
          else if (x==matrix.width()-1 && y==0) {
              dx = 0;
              dy = 1;
          }
          else if (x==matrix.width()-1 && y==matrix.height()-1) {
              dx = -1;
              dy = 0;
          }
          else if (x==0 && y==matrix.height()-1) {
              dx = 0;
              dy = -1;
          }
          delay(3);     // Contrôle de la vitesse de l'animation
        }
        matrix.fillScreen(matrix.Color888(0, 0, 0));
      }
    }
}

void init_interface() {
  matrix.drawRect(0,0,matrix.width(), matrix.height(), matrix.Color333(7,7,0));
  matrix.drawLine(8,matrix.height()-11,matrix.width()-1,matrix.height()-11, matrix.Color333(7,7,0));
  matrix.drawLine(8,0,8,matrix.height()-11, matrix.Color333(7,7,0));

  // score
  score=0;
  /*std::string t = to_string(score);
  char const *n_char = t.c_str();
  Serial.println(n_char[0]);*/
  matrix.drawChar(matrix.width()-21, matrix.height()-9, '7', matrix.Color333(255,0,0), 0, 1);
  matrix.drawChar(matrix.width()-14, matrix.height()-9, '9', matrix.Color333(255,0,0), 0, 1);
  matrix.drawChar(matrix.width()-7, matrix.height()-9, '1', matrix.Color333(255,0,0), 0, 1); 
}

void afficher_jeu(){//3, fill, 1de marge en x 0 en y 
  
  for (int i = 0; i < 15; i++) {
    for (int j = 0; j < 15; j++) {

      if(i%2==0){
        matrix.fillRect(marge_g+j*3 +j, en_tete+i*3 , 3, 3, couleurs[jeu[i][j]]);
      }
      else{
        matrix.fillRect(marge_g+j*3 +2+j, en_tete+i*3, 3, 3, couleurs[jeu[i][j]]);
      }
      
      
    }
  }
  

}

void initialisation_jeu(){

  deplacer(1,0);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 15; j++) {
      jeu[i][j]=random(1,nb_couleur);
      
    }
  }
  afficher_jeu();
}

bool estPossible(int a, int b) {  //a=dir b=incl => verifier si le deplacement est possible(ne pas sortir de la zone de jeu)

  if (incl + b == 0) {
    return ((pos + a) < (62 - marge_d) & (pos + a) >= marge_g);
  } else if (incl + b == 1) {
    return ((pos + a + 6) < (61 - marge_d) & (pos + a) >= marge_g);
  } else if (incl + b == -1) {
    return ((pos + a) < (62 - marge_d) & (pos + a - 7) >= marge_g);
  } else {
    return false;
  }
}

bool case_libre() {//indique si le cube peut avancer ou s'il vient de se bloquer
  if(pos_cube_y%3==0){//si la prochaine case peut être un cube ou le bord
    
    int ligne=pos_cube_y/3;
    int colonne=pos_cube_x/3;
    //Serial.println(ligne);
    //Serial.println(jeu[ligne][colonne]);
    return (jeu[ligne][colonne]==0) && (pos_cube_y > 2 + en_tete);
  }
  else{//sinon
    return true;
  }
  
}

void deplacer_cube() {
  if (deplacement & case_libre()) {
    for (int i = 0; i < 2; i++) {

      if (incl_cube == 0) {
        matrix.fillRect(pos_cube_x, pos_cube_y - 2, 3, 3, couleurs[couleur_cube * i]);  //boule a envoyer
        if (i == 0) {
          pos_cube_y = pos_cube_y - 1;
        }
      } else if (incl_cube == 1) {
        if(pos_cube_x==59){
          matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
          matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[couleur_cube * i]);
          matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[couleur_cube * i]);

          if (i == 0) {
          pos_cube_x = pos_cube_x + 1;
          pos_cube_y = pos_cube_y - 1;
          incl_cube=-1;
        }
        }
        else{

        

        matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x + 2, pos_cube_y - 2, pos_cube_x + 4, pos_cube_y - 2, couleurs[couleur_cube * i]);

        if (i == 0) {
          pos_cube_x = pos_cube_x + 1;
          pos_cube_y = pos_cube_y - 1;
          
        }
        }
      } else if (incl_cube == -1) {

        if(pos_cube_x==2){
          matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
          matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[couleur_cube * i]);
          matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[couleur_cube * i]);

          if (i == 0) {
          pos_cube_x = pos_cube_x -1;
          pos_cube_y = pos_cube_y - 1;
          incl_cube=1;
        }
        }
        else{
        matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[couleur_cube * i]);
        matrix.drawLine(pos_cube_x - 2, pos_cube_y - 2, pos_cube_x, pos_cube_y - 2, couleurs[couleur_cube * i]);

        if (i == 0) {
          pos_cube_x = pos_cube_x - 1;
          pos_cube_y = pos_cube_y - 1;

          if (pos_cube_x == marge_g) {  //gere la collision a gauche
            incl_cube = 1;
          }
        }
      }
      }
    }
  } else if (deplacement){
    deplacement = false;
    couleur_cube = random(1, nb_couleur);
    if (incl == 0) {
      //pos_cube_x = pos;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);      //boule a envoyer
    }
    else if (incl == -1) {
      //pos_cube_x = pos - 5;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube ]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube ]);
      matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube ]);
    }
    else if (incl == 1) {
      //pos_cube_x = pos + 5;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube ]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube ]);
      matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube ]);
    }
  }
}


void deplacer(int dirdem, int incldem) {  //deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if (estPossible(dirdem, incldem)) {

    for (int i = 0; i < 2; i++) {

      if (incl == 0) {
        if(not(deplacement)){
        matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube * i]);      //boule a envoyer
        }
        matrix.fillRect(pos, matrix.height() - 4 - hauteur, 3, 4, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        
      } else if (incl == -1) {
        matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos - 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos - 2, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));
        matrix.drawLine(pos + 2, matrix.height() - 1 - hauteur, pos - 1, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));

        if(not(deplacement)){
        matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube * i]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube * i]);
        matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube * i]);
        }
      } else if (incl == 1) {
        matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos + 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
        matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos + 4, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));
        matrix.drawLine(pos + 2, matrix.height() - 1 - hauteur, pos + 5, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));

        if(not(deplacement)){
        matrix.drawLine(pos + 5, matrix.height() - 6 - hauteur, pos + 7, matrix.height() - 6 - hauteur, couleurs[couleur_cube * i]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos + 6, matrix.height() - 7 - hauteur, pos + 8, matrix.height() - 7 - hauteur, couleurs[couleur_cube * i]);
        matrix.drawLine(pos + 7, matrix.height() - 8 - hauteur, pos + 9, matrix.height() - 8 - hauteur, couleurs[couleur_cube * i]);
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
