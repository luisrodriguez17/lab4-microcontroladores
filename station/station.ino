#define HUMEDAD A1
#define LUZ A3
#define LLUVIA A4
#define RESIST_SERIE 10000
#include <math.h>

// liberías para manipular PCD8544
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// pines de PCD a arduino: SCLK = 8, DIN = 9, D/C = 10, CS = 11, RST = 12
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 11, 12);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(1, OUTPUT);

  // Activar display
  display.begin();
}

void loop() {
  // switch deshabilitar/habilitar PCD
  int switchLCD = digitalRead(A15);

  // Sensor humedad
  int humid = analogRead(HUMEDAD);
  humid = map(humid, 0, 1023, 0, 100);  // convertir tensión a 0-100 %
  //Serial.print("Humedad: "); Serial.println(humid);
  //delay(500);

  // Sensor luz
  float vLuz = analogRead(LUZ)*5.0/1023.0;
  float rLuz = vLuz*RESIST_SERIE/(5-vLuz);
  float lux = pow(rLuz, -1.162)*pow(10.0, 5.935);  // fórmula interpolada de R a lux

  // sensor lluvia
  int lluvia = digitalRead(LLUVIA);

  // Imprimir en LCD
  display.clearDisplay();  //clears display each time loop starts over
  display.setCursor(0,0);
  display.print("Humedad: "); display.println(humid);  // muestra humedad
  display.print(lux); display.println(" lux");  // muestra luz en lux
  if (!lluvia){
    display.println("Hay lluvia");
  }
  else {
    display.println("No hay lluvia");
  }

  // deshabilita despliegue de info (muestra display vacío)
  if (switchLCD){
    display.clearDisplay();
    display.display();  
  }
  else {
    display.display();
  }
   
  delay(500);

}
