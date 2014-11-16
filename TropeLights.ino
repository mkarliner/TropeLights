
#include <stdio.h>
#include <IRremote.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);



//Pin Mappings
const int MAG_SENSOR = 2;
const int RECV_PIN = 3;
const int IR_PWR = 4;
const int WHITE_LEDS = 6;
const int MAG_INT = 0;


int flash_duration = 1;

const int threshold = 400;
static FILE uartout = {
  0};

IRrecv irrecv(RECV_PIN);
decode_results results;

//boolean light_sensor() {
//  digitalWrite(led_pin, LOW);
//  int off_value = analogRead(A0); 
//  digitalWrite(led_pin, HIGH);
//  int on_value = analogRead(A0);
//  digitalWrite(led_pin, LOW);
//  if(off_value - on_value > threshold) {
//    return true;
//  } 
//  else {
//    return false;
//  }
//}

//boolean mag_sensor() {
//  int val =  digitalRead(MAG_SENSOR);
//  //printf("Value = %d\n", val);
//  return val == 0;
//
//}


volatile int start_of_revolution = 0;

void mag_int() {
  start_of_revolution++;
}


long flash_period;
unsigned long start_rev=0;
unsigned long old_start_rev=0;

void setup() {
  Serial.begin(9600);
  //Setup the LCD
  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(LOW);
  lcd.home (); 
  //  pinMode(led_pin, OUTPUT);
  pinMode(WHITE_LEDS, OUTPUT);
  pinMode(MAG_SENSOR, INPUT);


  //Setup the IR receiver
  irrecv.enableIRIn(); // Start the receiver
  pinMode(IR_PWR,OUTPUT);
  digitalWrite(IR_PWR,HIGH); // Power IR Rx off pin 2 for neatness.

  //Setup mag sensor interrupt routine
  attachInterrupt(MAG_INT, mag_int, FALLING);

  //  digitalWrite(led_pin, HIGH);

  digitalWrite(WHITE_LEDS, LOW);

  lcd.setCursor ( 0, 0);        // go to the third line
  lcd.print("RPM ");
  lcd.setCursor ( 0, 1 );        // go to the fourth line
  lcd.print("Flash Time");

  // fill in the UART file descriptor with pointer to writer.
  fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

  // The uart is the standard output device STDOUT.
  stdout = &uartout ;
  //  while(mag_sensor() == false);
  //  while(mag_sensor() == true);
}


void loop() {
  char rpm_buf[5];
  static int lcd_status = 0;
  static int white_led_status = 1;

  //while(mag_sensor() == false); // Wait for sensor
  while(start_of_revolution == 0){
  };
  printf("Start of revolution %d\n", start_of_revolution);
  start_of_revolution = 0; //Reset rev
  start_rev = millis();

  int period = millis() - old_start_rev;
  flash_period = (period) / 18;
  old_start_rev = start_rev;
  //delay(50);
  //sprintf(rpm_buf, "%3.1f", (float)60000/period);
  ftoa(rpm_buf, (float)60000/period,  1);
  //  printf("Period %d millis rpm %s, flash period micros %l \n", period, rpm_buf,  flash_period);
  //  Serial.println("");

  lcd.setCursor (11,1);        
  lcd.print(flash_duration, DEC);
  lcd.setCursor ( 12, 0);        // go to the third line
  lcd.print("    ");
  lcd.setCursor ( 12, 0); 
  lcd.print(period);
  if(flash_period > 50 && flash_period < 150 && white_led_status) { 
    lcd.setCursor (5,0);        
    lcd.print(rpm_buf);   
    analogWrite(WHITE_LEDS, 0);
    for(int i=0; i<18; i++) {
      digitalWrite(WHITE_LEDS, HIGH);
      delay(flash_duration);
      digitalWrite(WHITE_LEDS, LOW);
      if(i!=17) {
        delay(flash_period-flash_duration);
      }
    } 
  }  
  else {
    analogWrite(WHITE_LEDS, 10);
  }

  if (irrecv.decode(&results)) {
    int rval = results.value;
    printf("IR code %x\n", rval);
    Serial.println(rval, HEX);
    if(rval == 0x629D) {
      printf("Yahoo\n");
    }
    switch(rval) {
    case 0x629D:  //Up arrow
      flash_duration++;
      break;
    case 0xa857:  //Down arrow
      flash_duration--;
      break;
    case 0x42BD: //Star
      if(lcd_status == 0) {
        lcd.setBacklight(HIGH);
        lcd_status = 1;
      } 
      else {
        lcd.setBacklight(LOW);
        lcd_status = 0;
      }
      break;
    case 0x6897:  //1 
      white_led_status = !white_led_status;
      break;

    default:
      printf("Unknown IR code %x\n", rval);
    }
    printf("Flash duraction %d\n", flash_duration);
    irrecv.resume(); // Receive the next value
    if(flash_duration < 1) {
      flash_duration = 1;
    }
    if(flash_duration > 10) {
      flash_duration = 10;
    }
  }
  //  while(light_sensor() == true);





}










