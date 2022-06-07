#define HUMEDAD A1
#define LUZ A3
#define LLUVIA A4
#define RESIST_SERIE 10000
#define BATERIA A5
#include <math.h>

// liberías para manipular PCD8544
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include <EEPROM.h>
#include <Chrono.h>
#include <stdio.h>
#include <ezOutput.h>
#include <LowPower.h>

const int Rc = 10000; //valor de la resistencia
const int Vcc = 5;
const int temp_sensor = A0;
const int wind_sensor = A2;
const int switch_USART = 22;
const int led_pin = 0;
float A = 0.0006973550913078027;
float B = 0.00028800736970464863;
float C = 5.400097451986799e-9;
float K = 2.5; //factor de disipacion en mW/C
float wind_speed = 0; // m/s
Chrono timer_eeprom(Chrono::SECONDS);
Chrono timer_usart(Chrono::SECONDS);
int eeprom_address = 0;
ezOutput led(led_pin);
ezOutput battery(7);  // instancia objeto ezOutput


void memory_save(float temp, float hum, float intensity, float wind_speed, float rain){
   if(eeprom_address >= EEPROM.length()){
      for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      eeprom_address = 0;
      EEPROM.write(eeprom_address, temp);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, hum);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, intensity);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, wind_speed);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, rain);
      eeprom_address += sizeof(float);
   }else{
      EEPROM.write(eeprom_address, temp);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, hum);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, intensity);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, wind_speed);
      eeprom_address += sizeof(float);
      EEPROM.write(eeprom_address, rain);
      eeprom_address += sizeof(float);
   }
    timer_eeprom.restart();
  
  
}

// pines de PCD a arduino: SCLK = 8, DIN = 9, D/C = 10, CS = 11, RST = 12
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 11, 12);

void wakeup()
{  
  // rutina vacía de interrupción de despierto de pin 18
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(1, OUTPUT);
  pinMode(switch_USART, INPUT);
  // Activar display
  display.begin();
  digitalWrite(18, HIGH); // empieza en ALTO para que no esté en low power
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

  // BATERÍA
  float v_bateria = analogRead(BATERIA)*4.8/1023;   // obtiene voltaje de 12 V divido por 1/2.5
  v_bateria = v_bateria*2.5;             //  voltaje batería                                                              
  Serial.println(v_bateria);
  battery.loop();
  
  // batería baja
  if (v_bateria <= 6){          
    battery.blink(500, 250);    // parpadeo led alerta
    digitalWrite(18, LOW); // pin 18 en LOW --> INT no activa, no se despierta de powerdown
    //Serial.println("Batería baja");
  } else {
    battery.low();
    digitalWrite(18, HIGH);  // interrupt que despierta de low power activo
  }

  // LOW POWER MODE: Cuando batería baja, INT pin 18 se desactiva y se mantiene en PowerDown.
  attachInterrupt(digitalPinToInterrupt(18), wakeup, HIGH);  // pin 18 = INT0
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  // se despierta cuando pin 18 es HIGH: interrupción activa (default)
  detachInterrupt(digitalPinToInterrupt(18));         // Disable external pin interrupt en pin 18
   
  delay(500);
  
  // Cambios luis
  led.loop();
  int USART_enable = digitalRead(switch_USART);
  float raw_temp = analogRead(temp_sensor);
  float raw_wind = analogRead(wind_sensor);
  
  // Calculate the temp value
  float V =  raw_temp / 1024 * Vcc;
  float R = (Rc * V ) / (Vcc - V);
  float logR  = log(R);
  float R_th = 1.0 / (A + B * logR + C * logR * logR * logR );
  float kelvin = R_th - V*V/(K * R)*1000;
  float celsius = kelvin - 273.15;
  // Wind Sensor
  float V2 = raw_wind / 1024 * Vcc;
  float wind_speed = 6*V2;
  
  // Saving to EEPROM
  if(timer_eeprom.hasPassed(300)){
      memory_save(celsius, 10.0 , 10.0, wind_speed, 1.0);
  }
  if(USART_enable == 1){
    led.blink(20,20);
    if(timer_usart.hasPassed(6)){
        String temp_buffer = String(celsius, 1);
        //String hum_buffer = String(hummid, 3);
        //String light_buffer = String(lux, 3); 
        String wind_buffer = String(wind_speed, 3);
        //String rain_buffer = String(rain, 3);
        String line = temp_buffer + "/" + wind_buffer;
        Serial.println(line);
        timer_usart.restart();
    }
  }
}
