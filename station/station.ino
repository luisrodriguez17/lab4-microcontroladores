#include <EEPROM.h>
#include <Chrono.h>
#include <stdio.h>
#include <ezOutput.h>
#include <Servo.h>

const int Rc = 10000; // Circuit resistance value
const int Vcc = 5; 
// Pines utilizados
const int temp_sensor = A0;
const int wind_sensor = A2;
const int switch_USART = 22;
const int input_servoV = A15;
const int input_servoH = A14;
const int output_servoV = 9;
const int output_servoH = 10;
const int led_pin = 0;
// Parameters for calculating the temperature
float A = 0.0006973550913078027;
float B = 0.00028800736970464863;
float C = 5.400097451986799e-9;
float K = 2.5; //Disipation factor on mW/C
// Sensors and actuators
float wind_speed = 0; // m/s
int servoV_pose = 0;
int servoH_pose = 0;
// Estructuras
Servo servoH;
Servo servoV;
Chrono timer_eeprom(Chrono::SECONDS);
Chrono timer_usart(Chrono::SECONDS);
int eeprom_address = 0;
ezOutput led(led_pin);


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
 // Function for controling servo
void servo_pose(){
  servoH_pose = analogRead(input_servoH);
  servoV_pose = analogRead(input_servoV);
  servoH_pose = map(servoH_pose, 0, 1023, 0, 180);
  servoV_pose = map(servoV_pose, 0, 1023, 0, 180);   
  servoH.write(servoH_pose);
  servoV.write(servoV_pose);
  
}

void setup() {
  Serial.begin(9600);
  pinMode(switch_USART, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(temp_sensor, INPUT);
  pinMode(wind_sensor, INPUT);
  // Servomotor
  pinMode(input_servoV, INPUT);
  pinMode(input_servoH, INPUT);
  pinMode(output_servoV, OUTPUT);
  pinMode(output_servoV, OUTPUT);
  servoV.attach(output_servoV);
  servoH.attach(output_servoH);
}

void loop() {
  led.loop(); //LED control
  servo_pose(); // Servo Control
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
    led.blink(10,10);
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
  }else{
    led.low();
    }
}
