#include <TimerFour.h>
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
volatile uint8_t file_couleurs[5];

color couleurs[7] = { matrix.Color888(0, 0, 0), matrix.Color888(255, 0, 255), matrix.Color888(0, 0, 255), matrix.Color888(255, 125, 0), matrix.Color888(0, 255, 0), matrix.Color888(255, 255, 0), matrix.Color888(0, 255, 255) };


volatile int level = 1;
volatile int coup_restant;

bool essai_visible = true;
volatile bool jeu_demarrer = false;

String score = "0";  // score
String score_temp = "0"; // score temporaire servant à la gestion de l'affichage du score
String ajout_score = "0"; // nombre de point gagné par le joueur
String ajout_score_temp = "0"; // nombre de point gagné par le joueur, temporaire servant à la gestion de l'affichage du score
String best_score = "0";

void setup() {
  randomSeed(analogRead(0));
  matrix.begin();
  Serial.begin(9600);
  Serial2.begin(9600); // Initialise la communication série
  delay(1100);
  initAnim(); // animation de lancement du jeu
  initInterface(); // initialise l'interface de jeu
  Timer3.initialize(100000);
  Timer3.attachInterrupt(majScore);
  Timer4.initialize(500000);  //clignote toutes les demi secondes
  Timer4.attachInterrupt(dernierEssai);
}

void initAnim() { // animation de lancement du jeu
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

void initInterface() { // interface pendant le choix de la difficulté par le joueur 
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
  choixDifficulte();
  majScore();
  
  
}

void majScore() { // met à jour la score en temps réel
  // effacement du rectangle contenant l'affichage du score
  if (ajout_score.toInt()>0) {
    ajout_score_temp = String(ajout_score.toInt() - 10);
    score_temp = String(score.toInt() + 10);
    for (int i = 0; i < score.length(); i++) {
      matrix.drawChar(matrix.width() - (7*(i+1)), 22, score[score.length()-i-1], matrix.Color333(0, 0, 0), 0, 1);
    }
    for (int i = 0; i < score_temp.length(); i++) {
      matrix.drawChar(matrix.width() - (7*(i+1)), 22, score_temp[score_temp.length()-i-1], matrix.Color333(255, 0, 0), 0, 1);
    }
    score = score_temp;
    ajout_score = ajout_score_temp;
    // verification si le meilleur score du joueur doit etre mis a jour 
    if (score.toInt()>=best_score.toInt()) {
      best_score = score;
      for (int i = 0; i < score.length(); i++) {
        matrix.drawChar(matrix.width() - (7*(i+1)), 40, score[score.length()-i-1], matrix.Color333(0, 0, 0), 0, 1);
      }
      for (int i = 0; i < best_score.length(); i++) {
        matrix.drawChar(matrix.width() - (7*(i+1)), 40, score_temp[score_temp.length()-i-1], matrix.Color333(255, 0, 0), 0, 1);
      }

    }
  }
}

void choixDifficulte() { // modifie la difficulté de la partie (variable "level") et la jauge de difficulté dans l'interface
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

void nbEssai() { // met à jour la nombre d'essai restant pour joueur
  if(level==0){
    if(coup_restant==4){
  for(int i=0;i<4;i++){
    if(i==3){
      matrix.fillRect(2, 13+10*i, 7, 7, matrix.Color333(255, 255, 255));
    }
    else{
      matrix.fillRect(3, 14+10*i, 5, 5, matrix.Color333(255, 255, 255));
    }
    
  }}
  else if(coup_restant==3){
    matrix.fillRect(3, 14, 5, 5, matrix.Color333(0, 0, 0));
  }
  else if(coup_restant==2){
    matrix.fillRect(3, 24, 5, 5, matrix.Color333(0, 0, 0));
  }
  else{
    matrix.fillRect(3, 34, 5, 5, matrix.Color333(0, 0, 0));
  }
  }
  else if(level==1){
  if(coup_restant==3){
  for(int i=0;i<3;i++){
    if(i==2){
      matrix.fillRect(2, 18+10*i, 7, 7, matrix.Color333(255, 255, 255));
    }
    else{
      matrix.fillRect(3, 19+10*i, 5, 5, matrix.Color333(255, 255, 255));
    }
    
  }}
  else if(coup_restant==2){
    matrix.fillRect(3, 19, 5, 5, matrix.Color333(0, 0, 0));
  }
  else{
    matrix.fillRect(3, 29, 5, 5, matrix.Color333(0, 0, 0));
  }
  }
  else{
  if(coup_restant==2){
  for(int i=0;i<2;i++){
    if(i==1){
      matrix.fillRect(2, 23+10*i, 7, 7, matrix.Color333(255, 255, 255));
    }
    else{
      matrix.fillRect(3, 24+10*i, 5, 5, matrix.Color333(255, 255, 255));
    }
    
  }}
  else{
    matrix.fillRect(3, 24, 5, 5, matrix.Color333(0, 0, 0));
  }
  }
}

void dernierEssai() { // fait clignoter le dernier rectangle en bas du nombre d'essai s'il ne reste qu'un seul essai au joueur (variable "coup_restant" = 1) et fait clignoter la prochaine bille de l'utilisateur dans le rectangle du bas 
  if(jeu_demarrer){
  if (essai_visible) {
    matrix.fillRect(35 , matrix.height() - 7, 3, 3, couleurs[file_couleurs[0]]);
    
  }
  else {
    matrix.fillRect(35 , matrix.height() - 7, 3, 3, couleurs[0]);
    
  }

  if(coup_restant==1){
    if(level==0){
      if(essai_visible){
        matrix.fillRect(2, 43, 7, 7, matrix.Color333(255, 255, 255));
      }
      else{
        matrix.fillRect(2, 43, 7, 7, matrix.Color333(0, 0, 0));
      }
    }
    else if(level==1){
      if(essai_visible){
        matrix.fillRect(2, 38, 7, 7, matrix.Color333(255, 255, 255));
      }
      else{
        matrix.fillRect(2, 38, 7, 7, matrix.Color333(0, 0, 0));
      }
    }
    else{
      if(essai_visible){
        matrix.fillRect(2, 33, 7, 7, matrix.Color333(255, 255, 255));
      }
      else{
        matrix.fillRect(2, 33, 7, 7, matrix.Color333(0, 0, 0));
      }
      
    }
  }
  essai_visible = !essai_visible;

  }

}

void creerFile(){ // créer une nouvelle file de couleur pour les billes
  for(int i=0;i<5;i++){
    file_couleurs[i]=random(1,nb_couleur);//creation de la file
    matrix.fillRect(35 + (5 * i), matrix.height() - 7, 3, 3, couleurs[file_couleurs[i]]);// affichage des carré à venir
  }
  // fleche
  matrix.drawLine(4, matrix.height() - 6, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  matrix.drawLine(28 - 3, matrix.height() - 6 - 3, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  matrix.drawLine(28 - 3, matrix.height() - 6 + 3, 28, matrix.height() - 6, matrix.Color333(7, 7, 0));
  Serial2.print(file_couleurs[0]); // envoi de la couleur de la 1ere bille
}

int pop(){ // renvoi le numéro de la couleur de la bille prochaine qu'aura le joueur une fois son coup joué 
  int res=file_couleurs[1];
  for(int i=0;i<5;i++){
    if(i==4){
      file_couleurs[i]=random(1,nb_couleur);//update de la file
      matrix.fillRect(35 + (5 * i), matrix.height() - 7, 3, 3, couleurs[file_couleurs[i]]);// affichage des carré à venir
    }
    else{
      file_couleurs[i]=file_couleurs[i+1];//update de la file
      matrix.fillRect(35 + (5 * i), matrix.height() - 7, 3, 3, couleurs[file_couleurs[i]]);// affichage des carré à venir
    }
    
  }
  return res;
}


void loop() {
  if (Serial2.available() > 0 && jeu_demarrer) { // vrai si o a reçu un caractère sur la liaison série
    String transmit = Serial2.readString(); // met dans lu le caractère lu
    Serial.print("Interface a reçu: ");
    Serial.println(transmit); // Affiche le message reçu

    if (transmit[0] == 'z') {
      Serial2.print(pop()); // envoi de la prochaine couleur de cube
      if (transmit.length()>1) { // récupération du nombre de tir réaliser
        char num_tir_temp = transmit.charAt(2);
        int num_tir = String(num_tir_temp).toInt();
        coup_restant = 4 - level - num_tir;
        nbEssai();
      }
    }
    else if (transmit == " ") {
      jeu_demarrer = false;
      coup_restant = 4 - level;
      score="0";
      // effacer le rectangle du nombre de coup restant
      matrix.fillRect(2, 13, 7, 38, matrix.Color333(0, 0, 0));
      // effacer le rectangle de la file de bille
      matrix.fillRect(2, matrix.height()-9, 59, 7, matrix.Color333(0, 0, 0));
    }
    else {
      bool isInt = true; // Supposons que c'est un entier jusqu'à preuve du contraire
      for (int i = 0; i < transmit.length(); i++) {
        if (!isdigit(transmit[i])) { // Si un caractère n'est pas un chiffre
            isInt = false; // La chaîne n'est pas un entier
            break; // Sortir de la boucle
        }
      }
      if (isInt) {
        ajout_score = transmit;
      }
    }
  }
  else if (Serial2.available() > 0 && !jeu_demarrer) { // vrai si o a reçu un caractère sur la liaison série
    char transmit = Serial2.read(); // met dans lu le caractère lu
    //Serial.print("Interface a reçu: ");
    //Serial.println(transmit); // Affiche le message reçu

    if (transmit=='q' && level>0) {
      level--;
      choixDifficulte();
    }
    else if (transmit=='d' && level<2) {
      level++;
      choixDifficulte();
    }
    else if (transmit == ' ') {
      jeu_demarrer = true;
      coup_restant = 4 - level; // affichage du nombre de coup restant en fonction de la difficulté choisit
      String nb = String(coup_restant);
      String scor = "Score";
      for (int i = 0; i<scor.length(); i++) {
        matrix.drawChar(13 + (6*i), 13, scor[i], matrix.Color333(255, 0, 0), 0, 1);
      }
      matrix.drawPixel(45,15,matrix.Color333(255, 0, 0));
      matrix.drawPixel(45,19,matrix.Color333(255, 0, 0));
      String best = "Best";
      for (int i = 0; i<best.length(); i++) {
        matrix.drawChar(13 + (6*i), 32, best[i], matrix.Color333(255, 0, 0), 0, 1);
      }
      matrix.drawPixel(39,34,matrix.Color333(255, 0, 0));
      matrix.drawPixel(39,38,matrix.Color333(255, 0, 0));
      if (score == "0") {
        matrix.drawChar(matrix.width() - 7, 22, score[0], matrix.Color333(255, 0, 0), 0, 1);
      }
      if (best_score == "0") {
        matrix.drawChar(matrix.width() - 7, 40, best_score[0], matrix.Color333(255, 0, 0), 0, 1);
      }
      // initialisation de la file de couleur de cube
      creerFile();

      if(level==0){
        matrix.fillRect(2, 13, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 23, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 33, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 43, 7, 7, matrix.Color333(255, 255, 255));
      }
      else if(level==1){
        matrix.fillRect(2, 18, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 28, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 38, 7, 7, matrix.Color333(255, 255, 255));
      }
      else{
        matrix.fillRect(2, 23, 7, 7, matrix.Color333(255, 255, 255));
        matrix.fillRect(2, 33, 7, 7, matrix.Color333(255, 255, 255));
      }
    }
  }
}