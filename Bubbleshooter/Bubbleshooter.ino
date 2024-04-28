
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

color couleurs[7] = { matrix.Color888(0, 0, 0), matrix.Color888(255, 0, 255), matrix.Color888(0, 0, 255), matrix.Color888(255, 50, 0), matrix.Color888(0, 135, 0), matrix.Color888(255, 255, 0), matrix.Color888(0, 255, 255) }; 
volatile bool deplacement = false;  //booleen pour indiquer si la boule est en deplacement
volatile bool pret = true;          //booleen pour indiquer si la boule est prete a etre deplacee
volatile bool en_jeu = false;//indique si un jeu est en cours
volatile bool reception=false;//indique si on a recu la prochaine bille
volatile bool fct_debut_bubble=true;//indique si on est dans la fonction debut bubble

void setup() {
  randomSeed(analogRead(0));
  matrix.begin();
  Serial.begin(9600);
  Serial2.begin(9600);
  delay(1100);  // nécessaire pour que l'animation de lancement du jeu ne se fasse qu'une fois

  Timer3.initialize(25000);  //definit l'intervalle un 0 en -
  Timer3.attachInterrupt(deplacer_cube);
  Timer4.initialize(500000);  //clignote toutes les demi secondes
  Timer4.attachInterrupt(clignoter);
  debut_bubble();
  
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
    Serial2.print(score);

    nb_eclates = 0;
    score = 0;
  }
}

void clignoter() {  //permet de faire clignoter les billes avant qu'elles n'explosent
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
        rearmer();//re-armer le canon
        clignote = 0;
        
      }
    }
  } else if (!en_jeu) {

    for (int k = 0; k < taille2; k++) {
      
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

void boules_isolees() {  //peut etre reduit a un pb de graphe -> chaque bille est reliee a ses 6 voisins au max et il s'agit de voir si il existe un chemin entre chaque bille et la ligne -1 (a chercher dans cours de graphe et voir pour la complexite)

//forward propagation; on prends le probleme à l'envers; on regarde toutes les billes accessibles depuis la ligne -1

  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      set(j, i, false);
    }
  }

  int debut = 0;  // la ou on defile
  int fin = 0;    // la ou on enfile

  for (int i = 0; i < 15; i++) {
    if (jeu[0][i] % 32 != 0) {
      set(i, 0, true);
      file_x[fin] = i;
      file_y[fin] = 0;
      fin++;
    }
  }

  while (debut != fin) {
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
    
    voisins_x[1] = x + depl;
    voisins_y[1] = y - 1;
    
    voisins_x[2] = x + 1;
    voisins_y[2] = y;
    
    voisins_x[3] = x - 1;
    voisins_y[3] = y;
    
    voisins_x[4] = x;
    voisins_y[4] = y + 1;
    
    voisins_x[5] = x + depl;
    voisins_y[5] = y + 1;

    for (int i = 0; i < 6; i++) {
      int x = voisins_x[i];
      int y = voisins_y[i];

      if (x <= 14 && x >= 0 && y >= 0 && y <= 16 && !get(x, y) && jeu[y][x] % 32 != 0) {  
        //ajouter w dans file attente
        set(x, y, true);
        file_x[fin] = x;
        file_y[fin] = y;
        fin++;
      }
    }
  }

  taille = 0;
  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      if (!get(j, i) && jeu[i][j] % 32 != 0) {
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
    rearmer();//re-armer le canon
    }
}

void descendre() {    //fonction pour faire descendre le jeu et créer une nvlle ligne
  bool verif = true;  //verifie si une bille fait perdre le joueur

  for (int i = 17; i >= 0; i--) {
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
}

void perdu() {  //appellée lorsque une bille est trop basse ( -> partie perdue)
  //a completer
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
  taille2=0;
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


    ligne = (pos_cube_y - 1) / 3;
    if (ligne % 2 == 0) {
      colonne = pos_cube_x / 4;

      if (pos_cube_x == 1 || (pos_cube_x - 1) % 4 == 1 || (pos_cube_x - 1) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
      } else if ((pos_cube_x - 1) % 4 == 0) {  //si il est entre deux cubes
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
      } else {  //si il est a droite
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
      }


    } else {

      colonne = (pos_cube_x - 1) / 4;
      if (pos_cube_x == 59 || (pos_cube_x + 1) % 4 == 2 || (pos_cube_x + 1) % 4 == 3) {  // si il est en 59 ou a droite ou en face
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
      } else if ((pos_cube_x + 1) % 4 == 0) {  //si il est entre deux cubes
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
      } else {  //si il est a gauche
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
      }

    }
    
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
        
        return ((pos_cube_y > 2 + en_tete) && jeu[ligne][colonne-1] % 32 == 0);
      } else {
        return true;
      }
    } else {
      
      if (pos_cube_y % 3 == 0) {
        ligne = (pos_cube_y - 1) / 3;
        if (ligne % 2 == 0) {
          
          colonne = (pos_cube_x + 3) / 4;

          if (pos_cube_x + 3 == 60 || (pos_cube_x + 2) % 4 == 1 || (pos_cube_x + 2) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x + 2) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
          } else {  //si il est a droite
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
          }
        } else {
          
          colonne = (pos_cube_x + 2) / 4;
          if (pos_cube_x + 3 == 59 || pos_cube_x + 3 == 60 || (pos_cube_x + 4) % 4 == 2 || (pos_cube_x + 4) % 4 == 3) {  // si il est en 59 ou a droite ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x + 4) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
          } else {  //si il est a gauche
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
          }
        }
      } else {  //verifier juste a droite du cube
      
        ligne = (pos_cube_y - 1) / 3;
        if (pos_cube_y > 3 && ligne % 2 == 0 && (pos_cube_x) % 4 == 2 && pos_cube_x < 53) {  //si il peut y avoir un cube a droite sur une ligne paire

          ligne--;
          colonne = (pos_cube_x) / 4;//colonne ou va se poser le cube
          
          return jeu[ligne][colonne + 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision

        } else if (pos_cube_y > 3 && ligne % 2 == 1 && (pos_cube_x) % 4 == 0 && pos_cube_x < 55) {  //si il peut y avoir un cube a droite sur une ligne impaire
          
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
      
      if (pos_cube_y % 3 == 0) {
        ligne = (pos_cube_y - 1) / 3;
        if (ligne % 2 == 0) {
          colonne = (pos_cube_x - 3) / 4;

          if (pos_cube_x - 3 == 0 || (pos_cube_x - 4) % 4 == 1 || (pos_cube_x - 4) % 4 == 2) {  // si il est en 1 ou a gauche ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x - 4) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne - 1] % 32 == 0);
          } else {  //si il est a droite
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne - 1] % 32 == 0);
          }
        } else {
          colonne = (pos_cube_x - 4) / 4;
          if (pos_cube_x - 3 == 0 || pos_cube_x - 3 == 1 || (pos_cube_x - 2) % 4 == 2 || (pos_cube_x - 2) % 4 == 3) {  // si il est en 59 ou a droite ou en face
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0);
          } else if ((pos_cube_x - 2) % 4 == 0) {  //si il est entre deux cubes
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne] % 32 == 0 && jeu[ligne - 1][colonne + 1] % 32 == 0);
          } else {  //si il est a gauche
            return ((pos_cube_y > 2 + en_tete) && jeu[ligne - 1][colonne + 1] % 32 == 0);
          }
        }
      } else {  //verifier juste a droite du cube
        ligne = (pos_cube_y - 1) / 3;
        if (pos_cube_y > 3 && ligne % 2 == 0 && (pos_cube_x) % 4 == 0 && pos_cube_x > 7) {  //si il peut y avoir un cube a droite sur une ligne paire

          ligne--;
          colonne = (pos_cube_x - 1) / 4;//colonne ou va se poser le cube

          return jeu[ligne][colonne - 1] % 32 == 0;  //colonne sur laquelle le cube va faire une collision

        } else if (pos_cube_y > 3 && ligne % 2 == 1 && (pos_cube_x) % 4 == 2 && pos_cube_x > 5) {  //si il peut y avoir un cube a droite sur une ligne impaire
         
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
}

void exploser(int lig, int col, int coul) {  //methode pour supprimer les boules

  for (int i = 0; i < taille; i++) {
    res_x[i] = 0;
    res_y[i] = 0;
  }


  for (int i = 0; i < 17; i++) {
    for (int j = 0; j < 15; j++) {
      set(j, i, false);
    }
  }
  int taille_listes = 255;
  taille = 0;//taille du paquet de retour
  
  int debut = 0;  //la ou on defile
  int fin = 1;    //la ou on enfile

  set(col, lig, true);

  file_x[0] = col;
  file_y[0] = lig;

  while (debut != fin) {
    int x = file_x[debut];
    int y = file_y[debut];

    debut = debut + 1;

    res_x[taille] = x;//on ajoute au retour
    res_y[taille] = y;//on ajoute au retour
    taille++;


    //on calcule tous les voisins de a

    uint8_t voisins_x[6];//6 voisins au max
    uint8_t voisins_y[6];
    int depl;
    if (y % 2 == 0) {
      depl = -1;
    } else {
      depl = 1;
    }
    voisins_x[0] = x;
    voisins_y[0] = y - 1;
    
    voisins_x[1] = x + depl;
    voisins_y[1] = y - 1;
    
    voisins_x[2] = x + 1;
    voisins_y[2] = y;
    
    voisins_x[3] = x - 1;
    voisins_y[3] = y;
    
    voisins_x[4] = x;
    voisins_y[4] = y + 1;
    
    voisins_x[5] = x + depl;
    voisins_y[5] = y + 1;



    //on visite tous les voisins de a possibles

    for (int i = 0; i < 6; i++) {
      int x = voisins_x[i];
      int y = voisins_y[i];

      if (x <= 14 && x >= 0 && y >= 0 && y <= 16 && !get(x, y) && jeu[y][x] % 32 == coul && fin != taille_listes) {  //ancien !visite[y][x]
        //ajouter x,y dans file attente
        set(x, y, true);
        file_x[fin] = x;
        file_y[fin] = y;
        fin++;
      }
    }
  }

  if (taille > 2) {
    nb_eclates = nb_eclates + taille;
    clignote = 1;
    
  } else {
    numero_tir++;
    
    
    if (numero_tir == nb_tirs) {
      numero_tir = 0;
      Serial2.print(-numero_tir);
      pret = false;
      descendre();

    } else {
      Serial2.print(-numero_tir);
      transmettre_score();
      pret = true;
      rearmer();//re-armer le canon
      
    }
  }
}

void deplacer_cube() {
  if (en_jeu) {

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
        rearmer();//re armer le canon
        
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
      }
    }
  }
}



void deplacer(int dirdem, int incldem) {  //deplacer la fleche d'envoi en inclinaison -1=gauche 0=rester 1=droite et en position -1=gauche 0=rester 1=droite
  if (en_jeu) {
    if (estPossible(dirdem, incldem)) {

      for (int i = 0; i < 2; i++) {
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
        couleur_cube=prochaine_couleur;//rearmer le canon

        if(couleur_cube==50){//si jamais on perd le court de la file, on prends des billes aléatoires
          couleur_cube=random(1,nb_couleur);
        }
        if(prochaine_couleur2!=50){//permet de gerer l'eventualite où on connait la valeur des deux billes suivantes
          prochaine_couleur=prochaine_couleur2;
          prochaine_couleur2=50;
        }
        else{
        prochaine_couleur=50;
        }
        if (incl == 0) {
          matrix.fillRect(pos, matrix.height() - 8 - hauteur, 3, 3, couleurs[couleur_cube]);  //boule a envoyer
        } else if (incl == -1) {
          matrix.drawLine(pos - 5, matrix.height() - 6 - hauteur, pos - 3, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos - 6, matrix.height() - 7 - hauteur, pos - 4, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos - 7, matrix.height() - 8 - hauteur, pos - 5, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        } else if (incl == 1) {
          matrix.drawLine(pos + 7, matrix.height() - 6 - hauteur, pos + 5, matrix.height() - 6 - hauteur, couleurs[couleur_cube]);  //boule a envoyer en lignes horizontales
          matrix.drawLine(pos + 8, matrix.height() - 7 - hauteur, pos + 6, matrix.height() - 7 - hauteur, couleurs[couleur_cube]);
          matrix.drawLine(pos + 9, matrix.height() - 8 - hauteur, pos + 7, matrix.height() - 8 - hauteur, couleurs[couleur_cube]);
        }
}

void loop() {
  
  if (Serial2.available() > 0) {
    char a=(Serial2.read());
    reception=true;

    if(prochaine_couleur==50){
      prochaine_couleur = a-48;
    }
    else{
      prochaine_couleur2 = a-48;
    }
    }
  if (Serial.available() > 0) {

    char command = Serial.read();

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
