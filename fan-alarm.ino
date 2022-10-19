// C++ code
//
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>

time_t time;
char buffer[5];

int display_time = 0000; // integers for printing to LCD display
int display_static = 0000;
int display_alarm = 1200;

int mode = 0; // device mode: 0=normal, 1=set_mins, 2=set_hours, 3=alarm_mins, 4=alarm_hours

bool pressed = false; // assorted booleans
bool alarm = false;
bool pm = false;

LiquidCrystal_I2C lcd(0x27,16,2);

void setup()
{
  Serial.begin(9600); // debug stuff
  pinMode(LED_BUILTIN, OUTPUT);

  setTime(0,0,0,1,1,2022); // time setup

  lcd.init();
  lcd.backlight();
  update_led(display_time);

  pinMode(2, OUTPUT); // these are for the LEDs
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, INPUT); // these are for the buttons
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(11, OUTPUT); // these are for the motor
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  
  analogWrite(11, 255); // initial motor settings
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
}

void update_led(int display_val)
{
  // set am/pm value
  pm = display_val >= 1200;
  
  // convert to 12-hour time
  if (display_val >= 1300) {
    display_val -= 1200;
  } else if (display_val < 100) {
    display_val += 1200;
  }

  // update LCD display
  lcd.clear();
  sprintf(buffer,"%02d:%02d",display_val/100,display_val%100);
  lcd.print(buffer);
  lcd.print(pm?" PM":" AM");
  lcd.setCursor(0,1);
  switch(mode){
    case 0:
      break;
    case 1:
      lcd.print("Set minutes");
      break;
    case 2:
      lcd.print("Set hours");
      break;
    case 3:
      lcd.print("Set alarm minutes");
      break;
    case 4:
      lcd.print("Set alarm hours");
      break;
    default:
      lcd.print("MODE ERROR");
  }

  // print time to serial monitor
  sprintf(buffer,"%04d %s\n",display_val,pm?"PM":"AM");
  Serial.print(buffer);

  // print alarm status to serial monitor
  if (alarm){
    Serial.print("alarm is active!\n");
  }
}

void loop()
{
  delay(10);

  if (minute()!=minute(time)) {
    // update time variable
    time = now();
    display_time = minute()+100*hour(time);

    // activate alarm if necessary
    if (display_time == display_alarm) {
      alarm = true;
      digitalWrite(12, HIGH);
    }

    // update displayed time
    if (mode == 0){
      update_led(display_time);
    }
  }

  if (!pressed) {
    // increment button
    if (digitalRead(6) == HIGH) {
      pressed = true;
      if (mode == 1) {
        // increase current minutes by 1
        display_static += 1;
        if (display_static%100 >= 60){
          display_static -= 60;
        }
        update_led(display_static);
      } else if (mode == 2){
        // increase current hours by 1
        display_static += 100;
        if (display_static >= 2400){
          display_static -= 2400;
        }
        update_led(display_static);
      } else if (mode == 3){
        // increase alarm minutes by 1
        display_alarm += 1;
        if (display_alarm%100 >= 60){
          display_alarm -= 60;
        }
        update_led(display_alarm);
      } else if (mode == 4){
        // increase alarm hours by 1
        display_alarm += 100;
        if (display_alarm >= 2400){
          display_alarm -= 2400;
        }
        update_led(display_alarm);
      }
    }

    // set-time mode button
    if (digitalRead(5) == HIGH) {
      pressed = true;
      if (mode == 0) {
        mode = 1;
        display_static = display_time;
        Serial.print("set time mode enabled\n");
        update_led(display_static);
      } else if (mode == 1){
        mode = 2;
        update_led(display_static);
      } else {
        setTime(display_static/100,display_static%100,0,1,1,2022);
        Serial.print("time set\n");
        mode = 0;
      }
    }

    // set-alarm mode button
    if (digitalRead(7) == HIGH) {
      pressed = true;
      if (mode == 0) {
        mode = 3;
        Serial.print("set alarm mode enabled\n");
        update_led(display_alarm);
      } else if (mode == 3){
        mode = 4;
        update_led(display_static);
      } else {
        Serial.print("alarm set\n");
        mode = 0;
      }
    }

    // alarm-off button
    if (digitalRead(8) == HIGH && alarm) {
      alarm = false;
      digitalWrite(12, LOW);
    }
  }
  if (digitalRead(7) == LOW && (digitalRead(6) == LOW && digitalRead(5) == LOW)) {
    pressed = 0;
  }

  // am/pm LED
  if (pm) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }

  // set-time mode LED
  if (mode == 1 || mode == 2) {
    digitalWrite(3, HIGH);
  } else {
    digitalWrite(3, LOW);
  }

  // set-alarm mode LED
  if (mode == 3 || mode == 4) {
    digitalWrite(4, HIGH);
  } else {
    digitalWrite(4, LOW);
  }
}