
#include <TimerThree.h>
#include <TimerFour.h>
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
uint8_t hauteur = 1;  //hauteur du jeu
uint8_t marge_g = 1;  //marge du jeu
uint8_t marge_d = 2;  //marge du jeu
uint8_t en_tete = 1;  //en_tete du jeu

uint8_t nb_couleur = 7;                //affiche nb_couleur - 1 dans le jeu
uint8_t nb_tirs = 4;                   //nb de tirs avant que ca descende
volatile uint8_t numero_tir = 0;       //indique combien de tir on a tiré (pour descendre en fonction)
volatile uint8_t taille_descente = 1;  //indique de combien on descent par descente

uint8_t difficulte=1; 

volatile uint8_t pos = 30;  //position de la fleche d'envoi en x
volatile int incl = 0;      //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)

volatile uint8_t ligne = 0;  //ligne et colonne du cube pour ajuster sa position
volatile uint8_t colonne = 0;

volatile uint8_t clignote = 0;  //0=> eteint 1=> allume

volatile int score = 0;  // score
volatile uint8_t nb_eclates = 0;



volatile uint8_t pos_cube_x = 0;    //position de la boule en bas a gauche en x
volatile uint8_t pos_cube_y = 0;    //position de la boule en bas a gauche en y
volatile int incl_cube = 0;         //inclinaison de la fleche d'envoi (-1=gauche 0=droit 1=droite)
volatile uint8_t couleur_cube = 1;  //indice de la couleur utilisee dans le tableau couleurs
uint8_t prochaine_couleur = 50;//prochaine couleur
uint8_t prochaine_couleur2 = 50;//prochaine couleur

volatile uint8_t res_x[255];
volatile uint8_t res_y[255];
volatile uint8_t file_x[255];
volatile uint8_t file_y[255];
volatile uint8_t taille = 0;  //taille du paquet de retour

volatile uint8_t taille2 = 0;        //taille du nb de billes a faire clignoter a la fin
volatile uint8_t fin_clignoter[15];  //numero de couleur
volatile uint8_t fin_x[15];          //x de la bille a clignoter

volatile uint8_t jeu[17][15];  //que des 0 partout par defaut, donc aucune bille
//volatile bool visite[17][15];//18 où la 18eme => partie perdue

//DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);
color couleurs[7] = { matrix.Color888(0, 0, 0), matrix.Color888(255, 0, 255), matrix.Color888(0, 0, 255), matrix.Color888(255, 125, 0), matrix.Color888(0, 255, 0), matrix.Color888(255, 255, 0), matrix.Color888(0, 255, 255) };

volatile bool deplacement = false;  //booleen pour indiquer si la boule est en deplacement
volatile bool pret = true;          //booleen pour indiquer si la boule est prete a etre deplacee
volatile bool en_jeu = false;//indique si un jeu est en cours
volatile bool reception=false;//indique si on a recu la prochaine bille
volatile bool fct_debut_bubble=true;//indique si on est dans la fonction debut bubble

struct PaireInt {
  int fst;  //premier du couple, coordonnée en x
  int snd;  //second du couple, coordonnée en y
};

void setup() {
  randomSeed(analogRead(0));
  matrix.begin();
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(1100);  // nécessaire pour que l'animation de lancement du jeu ne se fasse qu'une fois

  //Serial.println("ici");
  //matrix.fillScreen(0);
  //Timer3.initialize(75000);//defini l'intervalle
  Timer3.initialize(25000);  //definit l'intervalle un 0 en -
  Timer3.attachInterrupt(deplacer_cube);
  Timer4.initialize(500000);  //clignote toutes les demi secondes
  Timer4.attachInterrupt(clignoter);
  debut_bubble();
  //initialisation_jeu();
  //delay(1000);
  //Serial.print("affichage");
  //afficher_jeu();
}
void transmettre_score() {  //envoyer le score obtenu a l'autre matrice
  if (nb_eclates != 0) {
    for (int i = 0; i < nb_eclates; i++) {
      if (i + 1 < 4) {
        score = score + 10;
      } else if (i + 1 < 6) {
        score = score + 20;
      } else if (i + 1 < 8) {
        score = score + 30;
      } else {
        score = score + 40;
      }
    }

    //transmission du "score"
    //String aux=String(score);
    Serial.println(score);
    Serial2.print(score);

    nb_eclates = 0;
    score = 0;
  }
}

void clignoter() {  //permet de faire clignoter les billes avant qu'elles n'explosent
                    //Serial.println(clignote);
  if (clignote != 0 && en_jeu) {
    if (clignote < 5) {

      for (int k = 0; k < taille; k++) {
        int j = res_x[k];
        int i = res_y[k];

        if (i % 2 == 0) {
          matrix.fillRect(marge_g + j * 3 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32 * (1 - (clignote % 2))]);
        } else {
          matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32 * (1 - (clignote % 2))]);
        }


        if (clignote == 3) {
          if (jeu[i][j] >= 32) {
            jeu[i][j] = 32;
          } else {
            jeu[i][j] = 0;
          }
        }
      }
      clignote++;

      if (clignote == 4) {
        clignote = 0;
        boules_isolees();
      }
    } else {
      Serial.print("banane");
      pret = false;
      for (int k = 0; k < taille; k++) {
        int i = res_y[k];
        int j = res_x[k];
        if (i % 2 == 0) {
          matrix.fillRect(marge_g + j * 3 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32 * (1 - (clignote % 2))]);
        } else {
          matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32 * (1 - (clignote % 2))]);
        }
        if (clignote == 7) {
          if (jeu[i][j] >= 32) {
            jeu[i][j] = 32;
          } else {
            jeu[i][j] = 0;
          }
        }
      }
      clignote++;
      if (clignote == 8) {
        pret = true;
        Serial.print("ROI");
        rearmer();
        /*//couleur_cube = random(1, nb_couleur);  //re-armer le canon
        if(prochaine_couleur==50){
        couleur_cube=prochaine_couleur;
        prochaine_couleur=50;}
        else{

        }
        //prochaine_couleur=prochaine_couleur2;
        if (incl == 0) {
          //pos_cube_x = pos;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
        } else if (incl == -1) {
          //pos_cube_x = pos - 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        } else if (incl == 1) {
          //pos_cube_x = pos + 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        }*/

        clignote = 0;
        //deplacement = false;
      }
    }
  } else if (!en_jeu) {

    for (int k = 0; k < taille2; k++) {
      //volatile uint8_t fin_clignoter[15];  //numero de couleur
      //volatile uint8_t fin_x[15];
      int j = fin_x[k];
      //file_x se transforme en couleurs[15]
      matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + 17 * 3, 3, 3, couleurs[fin_clignoter[k] * (1 - (clignote % 2))]);
    }
    clignote++;
  }
}

void set(uint8_t x, uint8_t y, bool b) {
  if (jeu[y][x] >= 32 && !b) {
    jeu[y][x] = jeu[y][x] - 32;
  } else if (jeu[y][x] < 32 && b) {
    jeu[y][x] = jeu[y][x] + 32;
  }
}
bool get(uint8_t x, uint8_t y) {
  return jeu[y][x] >= 32;
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
      matrix.fillRect(0, 0, matrix.width() - 1, matrix.height() - 1, matrix.Color888(255, 0, 0));
      delay(50);
      matrix.fillScreen(matrix.Color888(0, 0, 0));  // Efface l'écran
    }

    for (int j = 0; j < 2; j++) {  // allume les leds en faisant le contour de la matrice, 2 fois
      x = 1;
      y = 0;
      dx = 1;
      dy = 0;
      matrix.drawChar(x_letter - (7 * 3), (matrix.height() / 2) - 8, 'B', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - (7 * 2), (matrix.height() / 2) - 8, 'u', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - 7, (matrix.height() / 2) - 8, 'b', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter, (matrix.height() / 2) - 8, 'b', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + 7, (matrix.height() / 2) - 8, 'l', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 2), (matrix.height() / 2) - 8, 'e', matrix.Color888(255, 0, 0), 0, 1);

      matrix.drawChar(x_letter - (7 * 3), (matrix.height() / 2), 'S', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - (7 * 2), (matrix.height() / 2), 'h', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter - 7, (matrix.height() / 2), 'o', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter, (matrix.height() / 2), 'o', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + 7, (matrix.height() / 2), 't', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 2), (matrix.height() / 2), 'e', matrix.Color888(255, 0, 0), 0, 1);
      matrix.drawChar(x_letter + (7 * 3), (matrix.height() / 2), 'r', matrix.Color888(255, 0, 0), 0, 1);

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
      matrix.fillScreen(matrix.Color888(0, 0, 0));
    }
  }
  initialisation_jeu();
}


void boules_isolees() {  //peut etre reduit a un pb de graphe -> chaque bille est reliee a ses 6 voisins au max et il s'agit de voir si il existe un chemin entre chaque bille et la ligne -1 (a chercher dans cours de graphe et voir pour la complexite)


  //bool valides[17][15];//normalement ici; visite doit etre remplacé par valides mais pour manque de place (bug de matrice), on reutilise visite qui est lui aussi un tableau de bool
  //forward propagation; on prends le probleme à l'envers; on regarde toutes les billes accessibles depuis la ligne -1
  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      //visite[i][j] = false; ancien
      set(j, i, false);
    }
  }

  //uint8_t file_x[255];
  //uint8_t file_y[255];

  int debut = 0;  // la ou on defile
  int fin = 0;    // la ou on enfile

  for (int i = 0; i < 15; i++) {
    if (jeu[0][i] % 32 != 0) {
      //visite[0][i] = true;ancien
      set(i, 0, true);
      file_x[fin] = i;
      file_y[fin] = 0;
      fin++;
    }
  }

  while (debut != fin) {
    //Serial.println(fin);
    int x = file_x[debut];
    int y = file_y[debut];
    debut++;

    int depl;
    if (y % 2 == 0) {
      depl = -1;
    } else {
      depl = 1;
    }

    int voisins_x[6];
    int voisins_y[6];

    voisins_x[0] = x;
    voisins_y[0] = y - 1;

    //voisins[1].fst = x + depl;
    //voisins[1].snd = y - 1;
    voisins_x[1] = x + depl;
    voisins_y[1] = y - 1;

    //voisins[2].fst = x + 1;
    //voisins[2].snd = y;
    voisins_x[2] = x + 1;
    voisins_y[2] = y;

    //voisins[3].fst = x - 1;
    //voisins[3].snd = y;
    voisins_x[3] = x - 1;
    voisins_y[3] = y;

    //voisins[4].fst = x;
    //voisins[4].snd = y + 1;
    voisins_x[4] = x;
    voisins_y[4] = y + 1;

    //voisins[5].fst = x + depl;
    //voisins[5].snd = y + 1;
    voisins_x[5] = x + depl;
    voisins_y[5] = y + 1;

    for (int i = 0; i < 6; i++) {
      //PaireInt w = voisins[i];
      //int x = w.fst;
      //int y = w.snd;
      int x = voisins_x[i];
      int y = voisins_y[i];
      //Serial.print("x: ");
      //Serial.println(x);
      //Serial.print("y: ");
      //Serial.println(y);

      if (x <= 14 && x >= 0 && y >= 0 && y <= 16 && !get(x, y) && jeu[y][x] % 32 != 0) {  //&& fin != taille_listes  (ancien !visite[y][x])
        //ajouter w dans file attente
        //visite[y][x] = true;ancien
        set(x, y, true);
        //file[fin].fst=x;
        //file[fin].snd=y;
        file_x[fin] = x;
        file_y[fin] = y;
        fin++;
      }
    }
  }

  taille = 0;
  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      if (!get(j, i) && jeu[i][j] % 32 != 0) {  //ancien !visite[i][j]
        res_x[taille] = j;
        res_y[taille] = i;
        taille++;
      }
    }
  }
  nb_eclates = nb_eclates + taille;
  transmettre_score();



  if (taille != 0) {
    if (!en_jeu) {
      pret = true;
    } else {
      clignote = 5;
    }
  } else {
    pret = true;
    Serial.println("DAME");
    rearmer();
    /*//couleur_cube = random(1, nb_couleur);  //re-armer le canon
    couleur_cube=prochaine_couleur;
    prochaine_couleur=50;
    //prochaine_couleur=prochaine_couleur2;
    if (incl == 0) {
      //pos_cube_x = pos;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
    } else if (incl == -1) {
      //pos_cube_x = pos - 5;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
      matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
    } else if (incl == 1) {
      //pos_cube_x = pos + 5;
      //pos_cube_y = 63 - hauteur - 5;
      matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
      matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
      matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
    }*/
  }





  /*
  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      if (i == 0) {
        if (jeu[i][j] != 0) {
          visite[i][j] = true;
        }
      } else {
        int depl;
        if (i % 2 == 0) {
          depl = -1;
        } else {
          depl = 1;
        }

        int aux=j+depl;
        if(aux>14 || aux<0){
          aux=j;
        }
        if ((visite[i - 1][j] || visite[i - 1][aux]) && jeu[i][j] != 0) {
          visite[i][j] = true;
        } else {
          if (jeu[i][j] != 0) {
            jeu[i][j] = 0;

            if (i % 2 == 0) {
              matrix.fillRect(marge_g + j * 3 + j, en_tete + i * 3, 3, 3, couleurs[0]);
            } else {
              matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[0]);
            }
          }
        }
      }
    }
  }*/
}

void descendre() {    //fonction pour faire descendre le jeu et créer une nvlle ligne
  bool verif = true;  //verifie si une bille fait perdre le joueur

  /*for (int i = 17 - taille_descente; i < 17; i++) {  //a remplacer par un while pour la complexité
    for (int j = 0; j < 15; j++) {
      if (jeu[i][j] != 0) {
        verif = false;
      }
    }
  }*/

  //if (verif) {

  /*for (int i = 0; i <15; i++) {
      if(jeu[17 - taille_descente][i]%32!=0){
            verif=false;//TODO bug lignes
            matrix.fillRect(marge_g + i * 3 + 2 + i, en_tete + 17 * 3, 3, 3, couleurs[jeu[17 - taille_descente][i]%32]);
            }
    }*/
  //taille=0;
  for (int i = 17; i >= 0; i--) {
    //for (int j = 14; j >= 0; j--) {
    for (int j = 0; j < 15; j++) {
      if (i < taille_descente) {
        if (jeu[i][j] >= 32) {
          jeu[i][j] = 32 + random(1, nb_couleur);
        } else {
          jeu[i][j] = random(1, nb_couleur);
        }
      } else {
        if (i == 17) {
          if (jeu[i - taille_descente][j] % 32 != 0) {
            verif = false;
            //file_x[taille]=jeu[i - taille_descente][j] % 32;
            //res_x[taille]=j;
            //res_y[taille]=i;
            //taille++;

            fin_x[taille2] = j;
            fin_clignoter[taille2] = jeu[i - taille_descente][j] % 32;
            taille2++;
            matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i - taille_descente][j] % 32]);
          }
        } else {
          if (jeu[i][j] >= 32) {
            jeu[i][j] = 32 + jeu[i - taille_descente][j] % 32;
          } else {
            jeu[i][j] = jeu[i - taille_descente][j] % 32;
          }
          //Serial.println(i==17 && jeu[i - taille_descente][j]!=0);
        }
      }
    }
  }
  afficher_jeu();
  boules_isolees();


  if (!verif) {
    en_jeu = false;
    perdu();
  }
  //interrupts();


  //} else {
  //  en_jeu=false;
  //  perdu();
  //}
}

void perdu() {  //appellée lorsque une bille est trop basse ( -> partie perdue)
  //Serial.println("LE NUL IL A PERDU");
  //a completer
}

void init_interface() {
  matrix.drawRect(0, 0, matrix.width(), matrix.height(), matrix.Color333(7, 7, 0));
  matrix.drawLine(8, matrix.height() - 11, matrix.width() - 1, matrix.height() - 11, matrix.Color333(7, 7, 0));
  matrix.drawLine(8, 0, 8, matrix.height() - 11, matrix.Color333(7, 7, 0));

  // score
  score = 0;
  /*std::string t = to_string(score);
  char const *n_char = t.c_str();
  Serial.println(n_char[0]);*/
  matrix.drawChar(matrix.width() - 21, matrix.height() - 9, '7', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(matrix.width() - 14, matrix.height() - 9, '9', matrix.Color333(255, 0, 0), 0, 1);
  matrix.drawChar(matrix.width() - 7, matrix.height() - 9, '1', matrix.Color333(255, 0, 0), 0, 1);
}



void afficher_jeu() {  //3, fill, 1de marge en x 0 en y

  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {

      if (i % 2 == 0) {
        matrix.fillRect(marge_g + j * 3 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32]);
      } else {
        matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[jeu[i][j] % 32]);
      }
    }
  }
}

void debut_bubble(){
  matrix.fillRect(0, 0, 63, 63, couleurs[0]);
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  // ecriture de "press space to play"
  int x_letter=15;
  int y_letter=15;
  matrix.drawChar(x_letter, y_letter, 'P', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + 7, y_letter, 'r', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 2), y_letter, 'e', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 3), y_letter, 's', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 4), y_letter, 's', matrix.Color333(255, 255, 255), 0, 1);

  matrix.drawChar(x_letter, y_letter +9, 's', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + 7, y_letter+9, 'p', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 2), y_letter+9, 'a', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 3), y_letter+9, 'c', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + (7 * 4), y_letter+9, 'e', matrix.Color333(255, 255, 255), 0, 1);

  matrix.drawChar(x_letter+10, y_letter+18, 't', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter + 17, y_letter+18, 'o', matrix.Color333(255, 255, 255), 0, 1);

  matrix.drawChar(x_letter+3, y_letter+27, 'p', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter  +10, y_letter+27, 'l', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter +3+ (7 * 2), y_letter+27, 'a', matrix.Color333(255, 255, 255), 0, 1);
  matrix.drawChar(x_letter +3+ (7 * 3), y_letter+27, 'y', matrix.Color333(255, 255, 255), 0, 1);
}

void initialisation_jeu() {
  //randomSeed(analogRead(0));
  matrix.fillRect(0, 0, 63, 63, couleurs[0]);
  if(difficulte==0){
    nb_tirs = 4;      
  }
  else if(difficulte==1){
    nb_tirs = 3;      
  }
  else{
    nb_tirs = 2;      
  }
  taille = 0;
  en_jeu = true;
  taille2=0;
  pret = true;
  clignote=0;
  couleur_cube=random(1,nb_couleur);
  reception=false;
  numero_tir = 0;
  pos = 30;  //position de la fleche d'envoi en x
  incl = 0; 
  ligne=0;
  colonne=0;
  score=0;
  nb_eclates=0;
  prochaine_couleur=50;
  prochaine_couleur2=50;
  matrix.drawPixel(0, 52, matrix.Color888(255, 0, 0));
  matrix.drawLine(63, 52, 62, 52, matrix.Color888(255, 0, 0));
  deplacer(1, 0);                //permet d'afficher le canon
  for (int i = 0; i < 17; i++) {  
    for (int j = 0; j < 15; j++) {
      if(i<9){
      jeu[i][j] = random(1, nb_couleur);}
      else{
         jeu[i][j] = 0;
      }
    }
  }
  //jeu[14][13]=2;


  /*jeu[0][10]=3;
  jeu[0][9]=2;
  jeu[1][10]=1;
  jeu[2][10]=4;
  jeu[2][9]=3;
  jeu[3][9]=3;
  jeu[3][8]=4;
  jeu[2][8]=3;*/

  afficher_jeu();
}

bool estPossible(int a, int b) {  //a=dir b=incl => verifier si le deplacement est possible(ne pas sortir de la zone de jeu)

  if (incl + b == 0) {
    return ((pos + a) < (62 - marge_d) && (pos + a) >= marge_g);
  } else if (incl + b == 1) {
    return ((pos + a + 6) < (61 - marge_d) && (pos + a) >= marge_g);
  } else if (incl + b == -1) {
    return ((pos + a) < (62 - marge_d) && (pos + a - 7) >= marge_g);
  } else {
    return false;
  }
}

bool case_libre() {  //indique si le cube peut avancer ou s'il vient de se bloquer
  if (pos_cube_y > 54) {
    return true;
  } else if (incl_cube == 0 && pos_cube_y % 3 == 0) {  //si la prochaine case peut être un cube ou le bord


    //ligne = pos_cube_y / 3 - 1;
    ligne = (pos_cube_y - 1) / 3;
    if (ligne % 2 == 0) {
      //Serial.println(pos_cube_x);
      colonne = pos_cube_x / 4;

      if (pos_cube_x == 1 || (pos_cube_x - 1) % 4 == 1 || (pos_cube_x - 1) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
      } else if ((pos_cube_x - 1) % 4 == 0) {  //si il est entre deux cubes
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
      } else {  //si il est a droite
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
      }


    } else {

      //colonne = 15 - (61 - pos_cube_x) / 4;
      colonne = (pos_cube_x - 1) / 4;
      //Serial.println(pos_cube_x);
      if (pos_cube_x == 59 || (pos_cube_x + 1) % 4 == 2 || (pos_cube_x + 1) % 4 == 3) {  // si il est en 59 ou a droite ou en face
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
      } else if ((pos_cube_x + 1) % 4 == 0) {  //si il est entre deux cubes
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
      } else {  //si il est a gauche
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
      }

      //return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] == 0);
    }

    //Serial.println(pos_cube_y);
    //Serial.println(ligne);
    //Serial.println(colonne);
    //Serial.println(jeu[ligne-1][colonne+1]);
    //Serial.println(bool((pos_cube_y > 2 + en_tete) && jeu[ligne-1][colonne]==0));


  } else if (incl_cube == 1) {
    if (pos_cube_x == 58) {
      ligne = (pos_cube_y - 1) / 3;

      if (pos_cube_y % 3 == 0 && ligne % 2 == 0) {
        colonne = 14;
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);  //les deux du dessus
      } else if (pos_cube_y % 3 == 0 && ligne % 2 == 1) {
        colonne = 14;

        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);  //celle du dessus

      } else if (ligne % 2 == 0 && pos_cube_y % 3 == 2) {
        colonne = 14;
        ligne--;
        //Serial.print(3);
        //int a=0;
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne][colonne-1] % 32 == 0);
      } else {
        return true;
      }
    } else {

      //ligne = pos_cube_y / 3 - 1;
      if (pos_cube_y % 3 == 0) {
        ligne = (pos_cube_y - 1) / 3;
        if (ligne % 2 == 0) {
          //Serial.println(pos_cube_x);
          colonne = (pos_cube_x + 3) / 4;

          if (pos_cube_x + 3 == 60 || (pos_cube_x + 2) % 4 == 1 || (pos_cube_x + 2) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x + 2) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
          } else {  //si il est a droite
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
          }


        } else {

          //colonne = 15 - (61 - pos_cube_x) / 4;
          colonne = (pos_cube_x + 2) / 4;
          //Serial.println(pos_cube_x);
          if (pos_cube_x + 3 == 59 || pos_cube_x + 3 == 60 || (pos_cube_x + 4) % 4 == 2 || (pos_cube_x + 4) % 4 == 3) {  // si il est en 59 ou a droite ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x + 4) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
          } else {  //si il est a gauche
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
          }

          //return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] == 0);
        }
      } else {  //verifier juste a droite du cube

        //Serial.println("test");
        ligne = (pos_cube_y - 1) / 3;
        //pos_cube_y % 3 == 0
        if (pos_cube_y > 3 && ligne % 2 == 0 && (pos_cube_x) % 4 == 2 && pos_cube_x < 53) {  //si il peut y avoir un cube a droite sur une ligne paire

          ligne--;
          colonne = (pos_cube_x) / 4;

          //colonne ou va se poser le cube
          return jeu[ligne][colonne + 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision

        } else if (pos_cube_y > 3 && ligne % 2 == 1 && (pos_cube_x) % 4 == 0 && pos_cube_x < 55) {  //si il peut y avoir un cube a droite sur une ligne impaire
          //colonne--;//on descend le cube
          ligne--;
          colonne = (pos_cube_x) / 4;  //colonne ou va se poser le cube

          return jeu[ligne][colonne + 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision
        } else {
          return true;
        }
      }
    }
  } else if (incl_cube == -1) {


    if (pos_cube_x == 2) {
      ligne = (pos_cube_y - 1) / 3;

      if (pos_cube_y % 3 == 0 && ligne % 2 == 1) {
        colonne = 0;

        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);  //les deux du dessus
      } else if (pos_cube_y % 3 == 0 && ligne % 2 == 0) {
        colonne = 0;

        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);  //celle du dessus

      } else if (ligne % 2 == 1 && pos_cube_y % 3 == 2) {
        colonne = 0;
        ligne--;
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne][colonne+1] % 32 == 0);
      } else {
        return true;
      }
    } else {

      //ligne = pos_cube_y / 3 - 1;
      if (pos_cube_y % 3 == 0) {
        ligne = (pos_cube_y - 1) / 3;
        if (ligne % 2 == 0) {
          //Serial.println(pos_cube_x);
          colonne = (pos_cube_x - 3) / 4;

          if (pos_cube_x - 3 == 0 || (pos_cube_x - 4) % 4 == 1 || (pos_cube_x - 4) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x - 4) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
          } else {  //si il est a droite
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
          }


        } else {

          //colonne = 15 - (61 - pos_cube_x) / 4;
          colonne = (pos_cube_x - 4) / 4;
          //Serial.println(pos_cube_x);
          if (pos_cube_x - 3 == 0 || pos_cube_x - 3 == 1 || (pos_cube_x - 2) % 4 == 2 || (pos_cube_x - 2) % 4 == 3) {  // si il est en 59 ou a droite ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x - 2) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
          } else {  //si il est a gauche
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
          }

          //return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] == 0);
        }
      } else {  //verifier juste a droite du cube

        //Serial.println("test");
        ligne = (pos_cube_y - 1) / 3;
        //pos_cube_y % 3 == 0
        if (pos_cube_y > 3 && ligne % 2 == 0 && (pos_cube_x) % 4 == 0 && pos_cube_x > 7) {  //si il peut y avoir un cube a droite sur une ligne paire

          ligne--;
          colonne = (pos_cube_x - 1) / 4;

          //colonne ou va se poser le cube
          return jeu[ligne][colonne - 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision

        } else if (pos_cube_y > 3 && ligne % 2 == 1 && (pos_cube_x) % 4 == 2 && pos_cube_x > 5) {  //si il peut y avoir un cube a droite sur une ligne impaire
          //colonne--;//on descend le cube
          ligne--;
          colonne = (pos_cube_x) / 4;  //colonne ou va se poser le cube

          return jeu[ligne][colonne - 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision
        } else {
          return true;
        }
      }
    }

  } else {  //sinon
    return true;
  }
  Serial.println("fin2");
}

void exploser(int lig, int col, int coul) {  //methode pour supprimer les boules

  for (int i = 0; i < taille; i++) {
    res_x[i] = 0;
    res_y[i] = 0;
  }


  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      //visite[i][j] = false;ancien
      set(j, i, false);
    }
  }
  int taille_listes = 255;  //apparemment la limte sinon bug de matrice
  //PaireInt res[taille_listes];
  //uint8_t res_x[taille_listes];
  //uint8_t res_y[taille_listes];
  //int taille = 0;  //taille du paquet de retour
  taille = 0;
  //PaireInt file[taille_listes];  //file d'attente de couple; (x,y)
  //uint8_t file_x[255];
  //uint8_t file_y[255];
  int debut = 0;  //la ou on defile
  int fin = 1;    //la ou on enfile

  //visite[lig][col] = true;  //on visite la boule ancien
  set(col, lig, true);

  //file[0].fst = col;
  //file[0].snd = lig;

  file_x[0] = col;
  file_y[0] = lig;

  while (debut != fin) {
    //PaireInt z = file[debut];  //on defile dans a
    //int x = z.fst;
    //int y = z.snd;
    int x = file_x[debut];
    int y = file_y[debut];

    debut = debut + 1;




    //res[taille] = z;  //on ajoute a au retour
    res_x[taille] = x;
    res_y[taille] = y;
    taille++;
    //Serial.println(taille);


    //on calcule tous les voisins de a

    //PaireInt voisins[6];
    uint8_t voisins_x[6];
    uint8_t voisins_y[6];
    int depl;
    if (y % 2 == 0) {
      depl = -1;
    } else {
      depl = 1;
    }
    //voisins[0].fst = x;
    //voisins[0].snd = y - 1;
    voisins_x[0] = x;
    voisins_y[0] = y - 1;

    //voisins[1].fst = x + depl;
    //voisins[1].snd = y - 1;
    voisins_x[1] = x + depl;
    voisins_y[1] = y - 1;

    //voisins[2].fst = x + 1;
    //voisins[2].snd = y;
    voisins_x[2] = x + 1;
    voisins_y[2] = y;

    //voisins[3].fst = x - 1;
    //voisins[3].snd = y;
    voisins_x[3] = x - 1;
    voisins_y[3] = y;

    //voisins[4].fst = x;
    //voisins[4].snd = y + 1;
    voisins_x[4] = x;
    voisins_y[4] = y + 1;

    //voisins[5].fst = x + depl;
    //voisins[5].snd = y + 1;
    voisins_x[5] = x + depl;
    voisins_y[5] = y + 1;



    //on visite tous les voisins de a possibles

    for (int i = 0; i < 6; i++) {
      //PaireInt w = voisins[i];
      //int x = w.fst;
      //int y = w.snd;
      int x = voisins_x[i];
      int y = voisins_y[i];
      //Serial.print("x: ");
      //Serial.println(x);
      //Serial.print("y: ");
      //Serial.println(y);

      if (x <= 14 && x >= 0 && y >= 0 && y <= 16 && !get(x, y) && jeu[y][x] % 32 == coul && fin != taille_listes) {  //ancien !visite[y][x]
        //ajouter w dans file attente
        //visite[y][x] = true;ancien
        set(x, y, true);
        //file[fin].fst=x;
        //file[fin].snd=y;
        file_x[fin] = x;
        file_y[fin] = y;
        fin++;
      }
    }
  }

  if (taille > 2) {
    nb_eclates = nb_eclates + taille;
    clignote = 1;
    /*for (int k = 0; k < taille; k++) {
      //PaireInt pop = res[k];
      //int j = pop.fst;
      //int i = pop.snd;
      int j = res_x[k];
      int i = res_y[k];
      if(jeu[i][j]>=32){
        jeu[i][j] = 32;
      }
      else{
      jeu[i][j] = 0;
      }
      /*if(h==2){
      jeu[i][j] = 0;}
      int auxiliaire2;
      if(h==1){
        auxiliaire2=jeu[i][j];
      }
      else{
        auxiliaire2=0;
      }*/
    /*if (i % 2 == 0) {
        matrix.fillRect(marge_g + j * 3 + j, en_tete + i * 3, 3, 3, couleurs[0]);
      } else {
        matrix.fillRect(marge_g + j * 3 + 2 + j, en_tete + i * 3, 3, 3, couleurs[0]);
      }*/
    //est_pause=true;
    //tps=millis();

    //}
  } else {
    numero_tir++;
    //String ta="t";
    //int tb=numero_tir;
    //String tc=ta+tb;
    Serial2.print(-numero_tir);
    if (numero_tir == nb_tirs) {
      numero_tir = 0;
      Serial.print("mangue");
      pret = false;
      descendre();

    } else {
      transmettre_score();
      pret = true;
      Serial.println("FOU");
      rearmer();
      /*//couleur_cube = random(1, nb_couleur);  //re-armer le canon
      couleur_cube=prochaine_couleur;
      prochaine_couleur=50;
      //prochaine_couleur=prochaine_couleur2;
      if (incl == 0) {
        //pos_cube_x = pos;
        //pos_cube_y = 63 - hauteur - 5;
        matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
      } else if (incl == -1) {
        //pos_cube_x = pos - 5;
        //pos_cube_y = 63 - hauteur - 5;
        matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
        matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
      } else if (incl == 1) {
        //pos_cube_x = pos + 5;
        //pos_cube_y = 63 - hauteur - 5;
        matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
        matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
        matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
      }*/
    }
  }
  Serial.println("fin3");
}

void deplacer_cube() {
  if (en_jeu) {

    //Serial.println(case_libre());
    if (deplacement && case_libre()) {
      for (int i = 0; i < 2; i++) {

        if (incl_cube == 0) {
          matrix.fillRect(pos_cube_x, pos_cube_y - 2, 3, 3, couleurs[couleur_cube * i]);  //boule a envoyer
          if (i == 0) {
            pos_cube_y = pos_cube_y - 1;
          }
        } else if (incl_cube == 1) {
          if (pos_cube_x == 59) {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[couleur_cube * i]);

            if (i == 0) {
              pos_cube_x = pos_cube_x + 1;
              pos_cube_y = pos_cube_y - 1;
              incl_cube = -1;
            }
          } else {



            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x + 2, pos_cube_y - 2, pos_cube_x + 4, pos_cube_y - 2, couleurs[couleur_cube * i]);

            if (i == 0) {
              pos_cube_x = pos_cube_x + 1;
              pos_cube_y = pos_cube_y - 1;
            }
          }
        } else if (incl_cube == -1) {

          if (pos_cube_x == 2) {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[couleur_cube * i]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[couleur_cube * i]);

            if (i == 0) {
              pos_cube_x = pos_cube_x - 1;
              pos_cube_y = pos_cube_y - 1;
              incl_cube = 1;
            }
          } else {
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
    } else if (deplacement) {
      deplacement = false;
      if (ligne == 17) {
        taille2 = 1;
        fin_clignoter[0] = couleur_cube;

        if (incl_cube == 0) {  //effacage du cube mal positionné
          matrix.fillRect(pos_cube_x, pos_cube_y - 2, 3, 3, couleurs[0]);
        } else if (incl_cube == -1) {
          if (pos_cube_x == 2) {

            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[0]);

          } else {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x - 2, pos_cube_y - 2, pos_cube_x, pos_cube_y - 2, couleurs[0]);
          }
        } else if (incl_cube == 1) {
          if (pos_cube_x == 59) {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[0]);

          } else {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x + 2, pos_cube_y - 2, pos_cube_x + 4, pos_cube_y - 2, couleurs[0]);
          }
        }
        matrix.fillRect(marge_g + colonne * 3 + 2 + colonne, en_tete + ligne * 3, 3, 3, couleurs[couleur_cube]);  //repositionnement du cube
        fin_x[0] = colonne;

        //Serial.println("CAVALIER");
        rearmer();
        /*//couleur_cube = random(1, nb_couleur);//re armer le canon
        couleur_cube=prochaine_couleur;
        prochaine_couleur=50;
        //prochaine_couleur=prochaine_couleur2;
        if (incl == 0) {
          //pos_cube_x = pos;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
        } else if (incl == -1) {
          //pos_cube_x = pos - 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        } else if (incl == 1) {
          //pos_cube_x = pos + 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        }*/
        en_jeu = false;
        perdu();
      } else {
        if (jeu[ligne][colonne] >= 32) {

          jeu[ligne][colonne] = 32 + couleur_cube;  //mise a jour du jeu
        } else {
          jeu[ligne][colonne] = couleur_cube;  //mise a jour du jeu
        }



        if (incl_cube == 0) {  //effacage du cube mal positionné
          matrix.fillRect(pos_cube_x, pos_cube_y - 2, 3, 3, couleurs[0]);
        } else if (incl_cube == -1) {
          if (pos_cube_x == 2) {

            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[0]);

          } else {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x - 1, pos_cube_y - 1, pos_cube_x + 1, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x - 2, pos_cube_y - 2, pos_cube_x, pos_cube_y - 2, couleurs[0]);
          }
        } else if (incl_cube == 1) {
          if (pos_cube_x == 59) {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x, pos_cube_y - 2, pos_cube_x + 2, pos_cube_y - 2, couleurs[0]);

          } else {
            matrix.drawLine(pos_cube_x, pos_cube_y, pos_cube_x + 2, pos_cube_y, couleurs[0]);
            matrix.drawLine(pos_cube_x + 1, pos_cube_y - 1, pos_cube_x + 3, pos_cube_y - 1, couleurs[0]);
            matrix.drawLine(pos_cube_x + 2, pos_cube_y - 2, pos_cube_x + 4, pos_cube_y - 2, couleurs[0]);
          }
        }


        if (ligne % 2 == 0) {  //affichage du cube repositionné
          matrix.fillRect(marge_g + colonne * 3 + colonne, en_tete + ligne * 3, 3, 3, couleurs[jeu[ligne][colonne] % 32]);
        } else {
          matrix.fillRect(marge_g + colonne * 3 + 2 + colonne, en_tete + ligne * 3, 3, 3, couleurs[jeu[ligne][colonne] % 32]);
        }

        exploser(ligne, colonne, couleur_cube);

        //boules_isolees();
      }
    }
  }
}



void deplacer(int dirdem, int incldem) {  //deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if (en_jeu) {
    if (estPossible(dirdem, incldem)) {

      for (int i = 0; i < 2; i++) {
        Serial.println(incl);
        Serial.println(incldem);
        if (incl == 0) {
          if (not(deplacement)) {
            matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube * i]);  //boule a envoyer
          }
          matrix.fillRect(pos, matrix.height() - 4 - hauteur, 3, 4, matrix.Color888(255 * i, 0, 0));  //base de la fleche

        } else if (incl == -1) {

          matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos - 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
          matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos - 2, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));
          matrix.drawLine(pos + 2, matrix.height() - 1 - hauteur, pos - 1, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));

          if (not(deplacement)) {
            matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube * i]);  //boule a envoyer en lignes horizontales
            matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube * i]);
            matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube * i]);
          }
        } else if (incl == 1) {
          matrix.drawLine(pos, matrix.height() - 1 - hauteur, pos + 3, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));  //base de la fleche
          matrix.drawLine(pos + 1, matrix.height() - 1 - hauteur, pos + 4, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));
          matrix.drawLine(pos + 2, matrix.height() - 1 - hauteur, pos + 5, matrix.height() - 4 - hauteur, matrix.Color888(255 * i, 0, 0));

          if (not(deplacement)) {
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
}

void rearmer(){//permet de rearmer le canon
  //couleur_cube = random(1, nb_couleur);//re armer le canon
        couleur_cube=prochaine_couleur;
        Serial.print("chargement de :");
        Serial.print(prochaine_couleur);
        //reception=false;
        if(couleur_cube==50){
          couleur_cube=random(1,nb_couleur);
        }
        if(prochaine_couleur2!=50){
          prochaine_couleur=prochaine_couleur2;
          prochaine_couleur2=50;
        }
        else{
        prochaine_couleur=50;
        }
        //prochaine_couleur=prochaine_couleur2;
        if (incl == 0) {
          //pos_cube_x = pos;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
        } else if (incl == -1) {
          //pos_cube_x = pos - 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        } else if (incl == 1) {
          //pos_cube_x = pos + 5;
          //pos_cube_y = 63 - hauteur - 5;
          matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        }
}

void loop() {
  
  if (Serial2.available() > 0) {
    //String a=(Serial2.readString());
    char a=(Serial2.read());
    Serial.print("reception: ");
    Serial.println(a);
    reception=true;
    //Serial.println(a-48);

    if(prochaine_couleur==50){
      prochaine_couleur = a-48;
    }
    else{
      prochaine_couleur2 = a-48;
      //prochaine_couleur2 = a-48;
    }
    //Serial.println(prochaine_couleur);
    //Serial.println(prochaine_couleur2);

    //Serial.println("prochaine_couleur");
    //Serial.println(prochaine_couleur);
    }
  if (Serial.available() > 0) {

    char command = Serial.read();
    Serial.println(command);

    if (!en_jeu) {
      if (command == 'q') {
        Serial2.print('q');
        if(difficulte!=0){
          difficulte--;
        }
      } else if (command == 'd') {
        Serial2.print('d');
        if(difficulte!=2){
          difficulte++;
        }
      } else if (command == ' ') {
        if(fct_debut_bubble){
        initialisation_jeu();
        Serial2.print(' ');
        fct_debut_bubble=false;
        }
        else{
        debut_bubble();
        Serial2.print(' ');
        fct_debut_bubble=true;
        }
      }
    } 
    else {
      if (command == 'a') {
        deplacer(0, -1);
      } else if (command == 'e') {
        deplacer(0, 1);
      } else if (command == 'q') {
        deplacer(-1, 0);
      } else if (command == 'd') {
        deplacer(1, 0);
      } else if (command == 'z' && pret && !deplacement && reception) {
        Serial2.print('z');
        reception=false;
        pret = false;
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
