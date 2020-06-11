#include <MultiStepper.h>
#include <AccelStepper.h>
#include <EEPROM.h>


//#define PIN_SERIAL_RX 0 - Included to remind why we start at pin 2
//#define PIN_SERIAL_TX 1 - Included to remind why we start at pin 2

#define PIN_STEPPER_DIR 2
#define PIN_STEPPER_STP 3
#define PIN_STEPPER_LIMIT_HOME 4

#define PIN_SENSOR_PRESSURE 5

#define PIN_VALVE_BEER 12
#define PIN_VALVE_RELIEF 13
#define PIN_VALVE_GAS 11

#define PIN_SWITCH_HOME 6   // Home pushbutton (also zeroes position if first homing after boot)
#define PIN_SWITCH_RISE 7   // Rise pushbutton, move from home (bottom) and up to nozzles to fill
#define PIN_SWITCH_PRGE 8   // 
#define PIN_SWITCH_FILL 9   // 
#define PIN_SWITCH_SIZE 10   // Size pushbutton, change can size - selected size should be displayed on oled

#define POSITION_HOME 100
#define POSITION_PURGE 500

#define EEPROM_VER 169
#define EEPROM_SETTINGS_STARTADR 1
#define EEPROM_SETTINGS_ENDADR 12

#define EEPROM_POS_ID 0    // byte value to let us know if board is initialized for fillmeup (eeprom for tuning values are relevant and/or cleared)
                             // Should read 42x4 = 168 on initialized boards.
#define EEPROM_POS_TV330 1 // -5
#define EEPROM_POS_TV440 5 // -8
#define EEPROM_POS_TV500 9 // -12


#define PRESSURE_TOLERANCE 20
#define PRESSURE_AMBIENT 100 // SHOULD BE 100 IN PRODUCTION


//
// Tuning mode:
// After zero-initialized, press home button 5 more times withing 2 seconds.
// OLED will reflect to be in tuning mode
// Select the cansize to tune with the size button, confirm with rise
// Select distance to travel with size, perform with rise (-100, -10, -1, 0 (Save), +1, +10, +100)
// To save (eeprom), select 0 (save) distance, end click rise button to save.
// To abort when size is selected, home?


//
// Initialize stepper
//
AccelStepper stepper = AccelStepper(1, PIN_STEPPER_STP, PIN_STEPPER_DIR);


//
// Variables defining the current state of the machine
//
bool state_isTuning = false;
bool state_isTuning_sizeConfirmed = false;
bool state_isTuning_moving = false;

bool state_isLoading = true;
bool state_isStepperZeroed = false;
bool state_isStepperHoming = false;
bool state_isStepperHome = false;
bool state_isStepperRising = false;
bool state_isStepperRised = false;
bool state_isPurging = false;
bool state_isPurged = false;
bool state_isFilling = false;

int setting_cansize = 500;
long setting_tuningDistance = 0;
long tuning_currentPos = 0;

// Debouncing variables for pushbuttons are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

long setting_tunevalue_330 = 0;
long setting_tunevalue_440 = 0;
long setting_tunevalue_500 = 0;

unsigned long debounce_home = 0;
unsigned long debounce_rise = 0;  
unsigned long debounce_size = 0;
unsigned long debounce_prge = 0;
unsigned long debounce_fill = 0;  
unsigned long debounce_delay = 50;    // the debounce time; increase if the output flickers
int buttonstate_home = LOW;
int buttonstate_rise = LOW;
int buttonstate_size = LOW;
int buttonstate_prge = LOW;
int buttonstate_fill = LOW;
int lastbuttonstate_home = LOW;
int lastbuttonstate_rise = LOW;
int lastbuttonstate_size = LOW;
int lastbuttonstate_prge = LOW;
int lastbuttonstate_fill = LOW;


void setup() {
  
  Serial.begin(9600);  // Start the Serial monitor with speed of 9600 Bauds

  Serial.println("setup()");

  // REMEMBER
  // INPUT_PULLUP: High when open (floating, pulled up), low when pressed (grounded)
  pinMode(PIN_STEPPER_LIMIT_HOME, INPUT_PULLUP);    
  pinMode(PIN_SWITCH_HOME, INPUT_PULLUP);
  pinMode(PIN_SWITCH_RISE, INPUT_PULLUP);  
  pinMode(PIN_SWITCH_SIZE, INPUT_PULLUP);  
  pinMode(PIN_SWITCH_PRGE, INPUT_PULLUP);  
  pinMode(PIN_SWITCH_FILL, INPUT_PULLUP);  
  
  pinMode(PIN_STEPPER_DIR, OUTPUT);  
  pinMode(PIN_STEPPER_STP, OUTPUT);  

  pinMode(PIN_VALVE_BEER, OUTPUT);  
  pinMode(PIN_VALVE_RELIEF, OUTPUT);  
  pinMode(PIN_VALVE_GAS, OUTPUT);  

  state_isLoading = false;

  if(EEPROM.read(EEPROM_POS_ID) != EEPROM_VER) {
    for (int i = EEPROM_SETTINGS_STARTADR; i <= EEPROM_SETTINGS_ENDADR; i++)
      EEPROM.update(i, 0);

    EEPROM.update(EEPROM_POS_ID, EEPROM_VER);
  }

  EEPROM.get(EEPROM_POS_TV330, setting_tunevalue_330);
  EEPROM.get(EEPROM_POS_TV440, setting_tunevalue_440);
  EEPROM.get(EEPROM_POS_TV500, setting_tunevalue_500);

  if(setting_tunevalue_330 == 0 && setting_tunevalue_440 == 0 && setting_tunevalue_500 == 0) {
    Serial.println("setup() - straight to tuning mode");
    state_isTuning = true;
  } else {
    Serial.print("setup() - tuned steps value for 330,440,500: ");
    Serial.print(setting_tunevalue_330);
    Serial.print(", ");
    Serial.print(setting_tunevalue_440);
    Serial.print(", ");
    Serial.print(setting_tunevalue_500);
    Serial.println("");
  }
  screen_update();
}



void loop() {
  read_buttons();  
}


//
// screen_update()
//
//
//
void screen_update() {

  Serial.print("screen_update() - can size ");
  Serial.print(setting_cansize);
  Serial.println("");
  
  if(state_isTuning) {

    if(state_isTuning_moving) {
      Serial.print("screen_update() - tuning moving ");    
      Serial.print(setting_tuningDistance);
      Serial.println(" steps");
    } else if(state_isTuning_sizeConfirmed) {
      Serial.print("screen_update() - tuning, distance ");
      Serial.print(setting_tuningDistance);
      Serial.print(" steps");

      if(setting_tuningDistance == 0)
        Serial.print(" (save)");

      Serial.println("");
    } else {
      Serial.println("screen_update() - tuning, confirm can size");    
    }
  } else if(state_isStepperHoming) { 
    Serial.println("screen_update() - going home");   
   } else if(state_isStepperHome) { 
    Serial.println("screen_update() - is home");   
 } else if(state_isStepperRising) { 
    Serial.println("screen_update() - going up");   
  } else if(state_isStepperRised) {
    Serial.println("screen_update() - is up");   
  }


}



//
// buttonpress_home()
//
// Move to home, and if necessary zero-init position
//
//
unsigned long tuning_homepress_firstmillis = 0;
int tuning_homepress_count = 0;
void buttonpress_home() {

  Serial.println("buttonpress_home()");

  if(state_isStepperRising || state_isStepperHoming || state_isTuning) { // Do not allow size change while rising.
    //Serial.println("buttonpress_home() - homing not allowed in current state");
    return;
  }

  
  if(!state_isTuning && state_isStepperHome && (tuning_homepress_firstmillis == 0)) {
    // Start the  possibility enter tuning mode.
    tuning_homepress_count++;
    tuning_homepress_firstmillis = millis();
    //Serial.println("buttonpress_home() - pressed while at home - press five more time within two seconds to enter tuning mode...");
  } else if(!state_isTuning && state_isStepperHome && tuning_homepress_firstmillis > 0) {

    unsigned long remainingTime = 10000 - (millis() - tuning_homepress_firstmillis);
    if(remainingTime > 10000) {
      // Overflowed uint so its larger, chance to enter tuning mode wasted.
      //Serial.print("buttonpress_home() - time windows to enter tuning mode exceeded");
      tuning_homepress_firstmillis = 0;
      tuning_homepress_count = 0;
    } else if (tuning_homepress_count <= 4) {
      // Still within time window
      tuning_homepress_count++;
      
      //Serial.print("buttonpress_home() - counting to enter tuning mode, got ");
      //Serial.print(tuning_homepress_count);
      //Serial.print(" clicks, ");
      //Serial.print(remainingTime);
      //Serial.println(" ms remaining.");
      
      if((tuning_homepress_count == 5) && ((millis() - tuning_homepress_firstmillis) < 10000)) {
        //Serial.println("buttonpress_home() - entering tuning mode");
        state_isTuning = true;
        screen_update();
      }
    }
  } else {
    //Serial.println("buttonpress_home() - invoking stepper_home()");
    if(state_isStepperRised) {
      digitalWrite(PIN_VALVE_RELIEF, HIGH);
      delay(1000);
      digitalWrite(PIN_VALVE_RELIEF, LOW);
    }

    stepper_home();
  }

  screen_update();

}


//
// buttonpress_rise()
//
// Move to filling position
//
//
void buttonpress_rise() {

  Serial.println("buttonpress_rise()");

  if(state_isStepperRising || state_isStepperHoming) // Do not allow size change while rising.
    return;

  if(state_isTuning) {

    if(!state_isTuning_sizeConfirmed) {
      
      // Confirm can size
      state_isTuning_sizeConfirmed = true;

    } else {

      // Perform travel
      if(setting_tuningDistance == 0) {
        
        //Save current position for given cansize
        switch(setting_cansize) {
          case 330:
            EEPROM.put(EEPROM_POS_TV330, tuning_currentPos);
            break;
          case 440:
            EEPROM.put(EEPROM_POS_TV440, tuning_currentPos);
            break;
          case 500:
            EEPROM.put(EEPROM_POS_TV500, tuning_currentPos);
            break;
        }

        state_isTuning_sizeConfirmed = false; // Back to size select
      } else {

        tuning_currentPos += setting_tuningDistance;

        Serial.print("Moving to ");
        Serial.print(tuning_currentPos);
        Serial.println("");


        state_isTuning_moving = true;
        screen_update();
        stepper.setMaxSpeed(1000.0);     // Set Max Speed of Stepper (Faster for regular movements)
        stepper.setAcceleration(500.0);  // Set Acceleration of Stepper  

        stepper.moveTo(tuning_currentPos);  // Set the position to move to
        stepper.runToPosition();

        state_isTuning_moving = false;        
      }
      
      
    }

  } else {
    // Normal operation, rise stepper to selected can size
    stepper_rise();
  }

  screen_update();  
}


//
// buttonpress_size()
//
// Toggle through available sizes
//
//


void buttonpress_size() {

  Serial.println("buttonpress_size()");

  int sensorVal=analogRead(A0);
  Serial.print("Sensor Value: ");
  Serial.print(sensorVal);
  Serial.println("");

  if((sensorVal > (PRESSURE_AMBIENT - PRESSURE_TOLERANCE)) && (sensorVal < (PRESSURE_AMBIENT + PRESSURE_TOLERANCE))) {
    Serial.println("ambient pressure");
  }
  
  // Theres 4v to operate on between lower (0.5V and upper 4.5V bound).
  // Analog reading of ~102 equals 0.5v (0 bar), hence we interpret everything under 120 as ambient pressure
  // Analog reading of ~921 equals 4.5v (5 bar), we should never reach this unless stress testing.

  // 820 Points between bounds, 5 bars => 164 points per bar
  // 102 ~ 100 = 0.5v = 0.0MPa = 0 bar
  // 184 ~ 180 = 0.5 bar
  // 266 ~ 270 = 1 bar
  // 348 ~ 350 = 1.5 bar
  // 430 ~ 430 = 2 bar
  // 594 ~ 590 = 3 bar
  // 758 ~ 760 = 4 bar
  // 921 ~ 920 = 4.5v = 0.5MPa = 5 bar




   // float pressure_psi = ((float)voltage - 0.5) * 25.0;
  
  if(state_isStepperRising) { // Do not allow size change while rising.
    Serial.println("buttonpress_size() - not allowed to change size while rising");
    return;
  }
  

  if(state_isTuning && state_isTuning_sizeConfirmed) {
    // When tuning given size, provide distance control instead of can size
    if(setting_tuningDistance == -10000) {
      setting_tuningDistance = -1000;
    } else if(setting_tuningDistance == -1000) {
      setting_tuningDistance = -100;
    } else if(setting_tuningDistance == -100) {
      setting_tuningDistance = -1;
    } else if(setting_tuningDistance == -1) {
      setting_tuningDistance = 0;
    } else if(setting_tuningDistance == 0) {
      setting_tuningDistance = 1;
    } else if(setting_tuningDistance == 1) {
      setting_tuningDistance = 100;
    } else if(setting_tuningDistance == 100) {
      setting_tuningDistance = 1000;
    } else if(setting_tuningDistance == 1000) {
      setting_tuningDistance = 10000;
    } else {
      setting_tuningDistance = -10000;
    }

    Serial.print("buttonpress_size() - tuning distance set to ");
    Serial.print(setting_tuningDistance);
    Serial.println("");
    
  } else {
    
    // Regular mode (and tuning cofnirm cansize), select can size.
    if(setting_cansize == 500) {
      setting_cansize = 330;
    } else if(setting_cansize == 330) {
      setting_cansize = 440;
    } else {
      setting_cansize = 500;
    }

    Serial.print("buttonpress_size() - size set to ");
    Serial.print(setting_cansize);
    Serial.println("");

  }

  screen_update();
}




void buttonpress_prge() {

  Serial.println("buttonpress_prge()");


  // Pressure test, if fails complain and abort
  if(state_isPurging) {
    // Stop
    digitalWrite(PIN_VALVE_RELIEF, LOW);
    digitalWrite(PIN_VALVE_GAS, LOW);
    state_isPurging = state_isPurged = false;
  } else {
    state_isPurging = true;

/*
    // Achieve pressure, 3 attempts
    for(int i = 0; i <= 3; i++) {
      digitalWrite(PIN_VALVE_GAS, HIGH);
      delay(500);
      digitalWrite(PIN_VALVE_GAS, LOW);

      if(isPressurized()) {
        break;
      }       
    }
    
    if(!isPressurized()) {
      Serial.println("buttonpress_prge() failed to pressurize");
      state_isPurging = state_isPurged = false;
      return;
    }   */    
    
    // Purge...
    digitalWrite(PIN_VALVE_GAS, HIGH);
    digitalWrite(PIN_VALVE_RELIEF, HIGH);

    delay(3000);
    
    digitalWrite(PIN_VALVE_GAS, LOW);
    digitalWrite(PIN_VALVE_RELIEF, LOW);

    state_isPurging = false;
    state_isPurged = true;
  }
}



void buttonpress_fill() {

  Serial.println("buttonpress_fill()");

  // Is filling? Stop!
  if(state_isFilling) {
    Serial.println("buttonpress_fill() stopping!");
    digitalWrite(PIN_VALVE_BEER, LOW);
    digitalWrite(PIN_VALVE_RELIEF, LOW);
    state_isFilling = false;
  } else {
    Serial.println("buttonpress_fill() filling beer");
    digitalWrite(PIN_VALVE_BEER, HIGH);
    delay(500);
    digitalWrite(PIN_VALVE_RELIEF, HIGH);
    state_isFilling = true;
  }

  /*
   * 
   *   } else if(!state_isPurged) {
    Serial.println("buttonpress_fill() NOT PURGED!");
    return;  
  } else if(isPressurized()) {
   */
}



bool isPressurized() {

    /*
    if((sensorVal > (PRESSURE_AMBIENT - PRESSURE_TOLERANCE)) && (sensorVal < (PRESSURE_AMBIENT + PRESSURE_TOLERANCE))) {
      Serial.println("ambient pressure");
    }*/
    
    // Theres 4v to operate on between lower (0.5V and upper 4.5V bound).
    // Analog reading of ~102 equals 0.5v (0 bar), hence we interpret everything under 120 as ambient pressure
    // Analog reading of ~921 equals 4.5v (5 bar), we should never reach this unless stress testing.
  
    // 820 Points between bounds, 5 bars => 164 points per bar
    // 102 ~ 100 = 0.5v = 0.0MPa = 0 bar
    // 184 ~ 180 = 0.5 bar
    // 266 ~ 270 = 1 bar
    // 348 ~ 350 = 1.5 bar
    // 430 ~ 430 = 2 bar
    // 594 ~ 590 = 3 bar
    // 758 ~ 760 = 4 bar
    // 921 ~ 920 = 4.5v = 0.5MPa = 5 bar


    
  int sum = 0;
  int prev = 0;
  for(int i = 0; i < 5; i++) {
    int sensorVal=analogRead(A0);
    Serial.print("isPressurized(): Sensor Value: ");
    Serial.print(sensorVal);
    Serial.println("");

    if(sensorVal < (PRESSURE_AMBIENT - PRESSURE_TOLERANCE)) {
      Serial.println("isPressurized(): Below ambient, returning false ");
      return false;
    } else if(sensorVal > 300) {
      Serial.println("isPressurized(): way above 1 bar, unreasonably high, returning false");
      return false;
    } 
    
    if(i > 0) {
      if((sensorVal > (prev - PRESSURE_TOLERANCE)) && (sensorVal < (prev + PRESSURE_TOLERANCE))) {
        // All good, keep going
      } else {
        Serial.println("isPressurized(): ambient or too high readings, returning false");
        return false;
      }
    }

    sum += sensorVal; 

    prev = sensorVal;
      
    delay(50);
  }

  int avg = sum / 5;

  Serial.print("isPressurized(): avg value: ");
  Serial.print(avg);
  Serial.println("");

  if(avg > (PRESSURE_AMBIENT + PRESSURE_TOLERANCE) && avg < 300) {
    Serial.println("isPressurized(): good average, is pressurized");
    return true;
  } else {
    Serial.println("isPressurized(): bad average, ret false");
    return false;
  }
}
