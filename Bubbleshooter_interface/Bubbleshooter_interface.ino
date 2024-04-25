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

uint8_t nb_couleur = 7;       //affiche nb_couleur - 1 dans le jeu
uint8_t nb_tirs = 4;          //nb de tirs avant que ca descende

volatile long score = 77777;  // score
volatile int level = 1;

void setup() {
  matrix.begin();
  Serial.begin(9600);
  Serial2.begin(9600); // Initialise la communication série
  delay(1100);
  //init_anim(); // animation de lancement du jeu
  init_interface(); // initialise l'interface de jeu
  Timer3.initialize(25000);
  //Timer3.attachInterrupt(deplacer_cube);
}

void init_anim() {
  static int x_letter = (matrix.width() / 2) - 3;
  static int y_letter = (matrix.height() / 2) - 3;

  static int x;
  static int y;
  static int dx;
  static int dy;

  for (int i = 0; i < 2; i++) {    // répète la boucle d'animation, 2 fois
    for (int k = 0; k < 3; k++) {  // les leds de la matrice s'allument toutes, 3 fois
      matrix.fillRect(0, 0, matrix.width() - 1, matrix.height() - 1, matrix.Color333(255, 0, 0));
      delay(50);
      matrix.fillScreen(matrix.Color333(0, 0, 0));  // Efface l'écran
    }

    for (int j = 0; j < 2; j++) {  // allume les leds en faisant le contour de la matrice, 2 fois
      x = 1;
      y = 0;
      dx = 1;
      dy = 0;
      matrix.drawChar(x_letter - (7 * 3), (matrix.height() / 2) - 8, 'B', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - (7 * 2), (matrix.height() / 2) - 8, 'u', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - 7, (matrix.height() / 2) - 8, 'b', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter, (matrix.height() / 2) - 8, 'b', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + 7, (matrix.height() / 2) - 8, 'l', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 2), (matrix.height() / 2) - 8, 'e', matrix.Color333(255, 0, 0), 0, 1);

      matrix.drawChar(x_letter - (7 * 3), (matrix.height() / 2), 'S', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - (7 * 2), (matrix.height() / 2), 'h', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - 7, (matrix.height() / 2), 'o', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter, (matrix.height() / 2), 'o', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + 7, (matrix.height() / 2), 't', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 2), (matrix.height() / 2), 'e', matrix.Color333(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 3), (matrix.height() / 2), 'r', matrix.Color333(255, 0, 0), 0, 1);

      while (x != 0 || y != 0) {                           // animation pixel tournant
        matrix.drawPixel(x, y, matrix.Color333(7, 7, 7));  // Allumer les pixels en blanc
        x += dx;                                           // Mettre à jour les positions
        y += dy;
        if (x == 0 && y == 0) {  // Vérifier si la ligne a atteint un bord et changer de direction si nécessaire
          dx = 1;
          dy = 0;
        } else if (x == matrix.width() - 1 && y == 0) {
          dx = 0;
          dy = 1;
        } else if (x == matrix.width() - 1 && y == matrix.height() - 1) {
          dx = -1;
          dy = 0;
        } else if (x == 0 && y == matrix.height() - 1) {
          dx = 0;
          dy = -1;
        }
        delay(3);  // Contrôle de la vitesse de l'animation
      }
      matrix.fillScreen(matrix.Color333(0, 0, 0));
    }
  }
}

void init_interface() {
  static int x_letter = 2;
  static int y_letter = 2;

  // grille de l'interface
  matrix.drawRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(7, 7, 0));
  matrix.drawRect(0, 10, matrix.width(), matrix.height() - 20, matrix.Color333(7, 7, 0));
  matrix.drawLine(10, 10, 10, matrix.height() - 11, matrix.Color333(7, 7, 0));
  // ecriture du mot "level"
  matrix.drawChar(x_letter, y_letter, 'L', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(x_letter + 7, y_letter, 'e', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(x_letter + (7 * 2), y_letter, 'v', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(x_letter + (7 * 3), y_letter, 'e', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(x_letter + (7 * 4), y_letter, 'l', matrix.Color333(255, 0, 0), 0, 1);
  // affichage du niveau de difficulté sélectionné
  choix_difficulte();
  // affichage du score
  maj_score();
  // fleche
  matrix.drawLine(4, matrix.height() - 6, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  matrix.drawLine(28 - 3, matrix.height() - 6 - 3, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  matrix.drawLine(28 - 3, matrix.height() - 6 + 3, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  // affichage des carré à venir
  for (int i = 0; i < 5; i++) {
    matrix.fillRect(35 + (5 * i), matrix.height() - 7, 3, 3, matrix.Color333(255, 255, 0));
  }
}

void maj_score() {
  /* convertir int en string en optimisant la mémoire
  int num = 12;
  char buf[10]; // Buffer pour la chaîne de caractères
  sprintf(buf, "%d", num); // Convertit l'entier en chaîne de caractères
  Serial.println(buf); // Affiche "12"
  */
  String ajout_score = String(0);
  // effacement du rectangle contenant l'affichage du score
  matrix.drawRect(11, 11, matrix.width()-1, matrix.height() - 22, matrix.Color333(0, 0, 0));
  int j = 0;
  for (int i = 0; i < ajout_score.length(); i++) {
    matrix.drawChar(matrix.width() - (7*(i+1)), (matrix.height() / 2) - 6, ajout_score[ajout_score.length()-i-1], matrix.Color333(255, 0, 0), 0, 1);
    j++;
  }
  // signe "plus"
  matrix.drawLine(matrix.width() - (7*(j+1)), (matrix.height() / 2) - 6, matrix.width() - (7*(j+1)), matrix.height() / 2, matrix.Color333(255, 0, 0));
  matrix.drawLine(matrix.width() - (7*(j+1)) - 3, matrix.height() / 2 - 3, matrix.width() - (7*(j+1)) + 3, matrix.height() / 2 - 3, matrix.Color333(255, 0, 0));
  String nouv_score = String(score);
  for (int i = 0; i < nouv_score.length(); i++) {
    matrix.drawChar(matrix.width() - (7*(i+1)), (matrix.height() / 2) + 4, nouv_score[nouv_score.length()-i-1], matrix.Color333(255, 0, 0), 0, 1);
  }
}

void choix_difficulte() {
  matrix.fillRect(2 + (7 * 5), 2, 8 * 3, 7, matrix.Color333(0, 0, 0));
  if (level==0) {
    matrix.fillRect(2 + (7 * 5), 2, 8 * (level + 1), 7, matrix.Color333(0, 7, 0));
  }
  else if (level==1) {
    matrix.fillRect(2 + (7 * 5), 2, 8 * (level + 1), 7, matrix.Color333(7, 7, 0));
  }
  else {
    matrix.fillRect(2 + (7 * 5), 2, 8 * (level + 1), 7, matrix.Color333(7, 0, 0));
  }
}

void loop() {
  if (Serial2.available() > 0) { // vrai si o a reçu un caractère sur la liaison série
    String transmit = Serial2.readString(); // met dans lu le caractère lu
    Serial.print("Interface a reçu: ");
    Serial.println(transmit); // Affiche le message reçu

    bool isInt = true; // Supposons que c'est un entier jusqu'à preuve du contraire
    for (int i = 0; i < transmit.length(); i++) {
      if (!isdigit(transmit[i])) { // Si un caractère n'est pas un chiffre
          isInt = false; // La chaîne n'est pas un entier
          break; // Sortir de la boucle
      }
    }
    if (isInt) {
      score = score + transmit.toInt();
      maj_score();
    }
    else {
      if (transmit=="q" && level>0) {
        level--;
        choix_difficulte();
      }
      else if (transmit=="d" && level<2) {
        level++;
        choix_difficulte();
      }
    }
  }
}

/* 
recu :
- score (mise a jour)
- command (pour le score, choix difficulté)

envoi :
- file de couleur de cube

revoir interface pour afficher :
- file de couleurs de cube suivant 
- score
- ajout de X points
- diffulté
*/