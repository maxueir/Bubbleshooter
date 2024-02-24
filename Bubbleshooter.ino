
 
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

DFRobot_RGBMatrix matrix(A, B, C, D, E, CLK, LAT, OE, false, WIDTH, _HIGH);

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.fillRect(0,matrix.height()-3, 3, 3, matrix.Color888(255, 0, 0));
  matrix.drawPixel(1,matrix.height()-4, matrix.Color888(255, 0, 255));
  matrix.drawLine(matrix.width()/2 -1,matrix.height()-1,10,20, matrix.Color888(255, 0, 255));
}

void loop(){
  // Coordonnées de début et de fin de la ligne
  int x0 = 24;
  int y0 = 5;
  int x1 = 56;
  int y1 = 12;

  // Dessine la ligne
  matrix.drawLine(x0, y0, x1, y1, matrix.Color888(255, 0, 0));  // Ligne rouge

  // Affiche les coordonnées des pixels allumés
  Serial.println("Pixels allumés sur la ligne :");
  for (int x = x0; x <= x1; x++) {
    int y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);  // Equation de la droite
    if (y >= 0 && y < matrix.height()) {
      matrix.drawPixel(x, y, matrix.Color888(0, 255, 0));  // Pixel vert pour visualisation
      Serial.print("(");
      Serial.print(x);
      Serial.print(", ");
      Serial.print(y);
      Serial.println(")");
    }
    delay(300);
  }

  delay(5000);  // Attente pour observer les pixels allumés
  matrix.fillScreen(matrix.Color888(0, 0, 0));  // Efface l'écran
  delay(1000);  // Délai avant la prochaine itération
}
