// Libraries being used in the sketch
#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>


// Keypad
#define buzzerPin 3
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {22, 23, 24, 25};
byte colPins[COLS] = {28, 29, 30,31};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
const String password = "1234"; // Desired password is input here
String inputPassword;


// Stepper
const int stepsPerRev = 2038;
Stepper mystepper = Stepper(stepsPerRev, 36, 37, 38, 39);


// RFID scanner
#define garageLED 7
#define SS_PIN 53
#define RST_PIN 5
RFID rfid(SS_PIN, RST_PIN);

String rf_card;


// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);


// Servo
#define servoPIN 45
Servo myservo;


// Photoresistor
#define photoLED 8
const int photoRES = A0;


// Defined delays
#define accessDelay 2000
#define deniedDelay 3000



void setup() {
  // Initialise LEDs
  pinMode(garageLED, OUTPUT);
  pinMode(photoLED, OUTPUT);
  pinMode(photoRES, INPUT);

  pinMode(buzzerPin, OUTPUT);

  // Initialise communication
  Serial.begin(9600);
  SPI.begin();
  rfid.init();


  // Initilaise servo
  myservo.attach(servoPIN);
  delay(500);
  myservo.write(0);

  // Initialse LCD
  lcd.init();
  lcd.backlight();
  accessInitialState();
}

// Loop has code for keypad contained within and calls the photoresistor LED and garage controller functions constantly
void loop() {
  static int inputCount = 1;
  char key = keypad.getKey();
  if (key){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Input Code:");
    if(key == '*') {
      inputPassword = "";
    }
    else if(key == '#') {
      if(password == inputPassword) {
        accessOK();
        inputCount = 1;
        openDoor();
        delay(accessDelay);
        closeDoor();
        accessInitialState();
        
      }
      else {
        accessNotOK();
        inputCount = 1;
        tone(buzzerPin, 400);
        delay(deniedDelay);
        noTone(buzzerPin);
        accessInitialState();
      }
      inputPassword = "";
    }
    else {
      inputPassword += key;
      lcd.setCursor(0, 1);
      lcd.print(inputPassword);
    }
  }
  LEDController();
  garageController();
}




// RFID Controller for garage

void garageController(){
if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      String pass ="163 73 55 252";
  
      rf_card = String(rfid.serNum[0]) + " " + String(rfid.serNum[1]) + " " + String(rfid.serNum[2]) + " " + String(rfid.serNum[3]);
      Serial.println(rf_card);
      if (rf_card == pass) {
        accessOK();
        myservo.write(100);

        delay(accessDelay);
        accessInitialState();
        myservo.write(0);
      
        //door_servo();

      }else{
      digitalWrite(garageLED, HIGH);
      accessNotOK();

      delay(deniedDelay);

      digitalWrite(garageLED, LOW);
      accessInitialState();
  }
    }
  }
}


void openDoor(){
  mystepper.setSpeed(5);
  mystepper.step(stepsPerRev);
}

void closeDoor(){
  mystepper.setSpeed(10);
  mystepper.step(-stepsPerRev);
}



// Photoresistor Code

void LEDController(){
  int autoLED_Val = analogRead(photoRES);
  if(autoLED_Val < 70){
    digitalWrite(photoLED, HIGH);
  } else{
    digitalWrite(photoLED, LOW);
  }
}



// Access states

void accessInitialState(){
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Awaiting");
  lcd.setCursor(6 , 1);
  lcd.print("Input...");
}

void accessOK(){
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Access");
  lcd.setCursor(4, 1);
  lcd.print("Granted!");
}

void accessNotOK(){
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Access");
  lcd.setCursor(4, 1);
  lcd.print("Denied!");
}
