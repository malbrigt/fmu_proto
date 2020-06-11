


//
// stepper_home()
//
//
//
void stepper_home() {

  Serial.println("stepper_home()");

  // Simulation
  state_isStepperHoming = true;
  screen_update();

  /*
  delay(2000);
  
  state_isStepperHoming = false;
  state_isStepperHome = true;
  state_isStepperRising = state_isStepperRised = false;  
  */
  
  state_isStepperHoming = true;

  if(!state_isStepperZeroed) {
    
    //
    // Initialize zero position of the machine by lowering until limit switch is activated.
    // Then slowly rise until it deactivates, and move to home position. 
    //
    // Ensures reliable starting position everytime.
    //
        
    Serial.println("Initializing zero position (bumping limit switch)");

    stepper.setMaxSpeed(500.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
    stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
    
    long pos_homing=1; 
    while (digitalRead(PIN_STEPPER_LIMIT_HOME)) {  // Make the Stepper move CCW until the switch is activated   
 

      stepper.moveTo(pos_homing);  
      stepper.run();
      pos_homing++;


      delay(1);      
    }

    Serial.println("First bump complete, moving away from limit switch until it deactivates");

    stepper.setCurrentPosition(0);  // Set the current position as zero for now
    stepper.setMaxSpeed(500.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
    stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
    
    pos_homing=-1;
    while (!digitalRead(PIN_STEPPER_LIMIT_HOME)) { // Make the Stepper move CW until the switch is deactivated
     stepper.moveTo(pos_homing);  // Set the position to move to
      pos_homing--;  // Decrease by 1 for next move if needed
      stepper.run();  // Start moving the stepper

      delay(5);
    }
  
    stepper.setCurrentPosition(0);
    tuning_currentPos = 0;

    Serial.println("Zeroing complete");

    state_isStepperZeroed = true;
  }

  Serial.println("Stepper moving to home position");

  stepper.setMaxSpeed(5000.0);     // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(2000.0);  // Set Acceleration of Stepper  

  stepper.moveTo(0);
  stepper.runToPosition();

  state_isStepperHoming = false;
  state_isStepperHome = true;
  state_isStepperRising = state_isStepperRised = false;
  
}





//
// stepper_rise()
//
//
//
void stepper_rise() {

  Serial.println("stepper_rise()");



  state_isStepperRising = true;
  state_isStepperHome = state_isStepperHoming = false;

  Serial.println("Rising cans fast to safe high position...");

  stepper.setMaxSpeed(5000.0);     // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(2000.0);  // Set Acceleration of Stepper  

  // Move to upper safe position (500ml start-purge pos), and check if cancelled before going further
  long safepos = setting_tunevalue_500 > 0 ? (setting_tunevalue_500 - POSITION_PURGE) : (setting_tunevalue_500 + POSITION_PURGE);
  stepper.moveTo(safepos);
  stepper.runToPosition();


  int target_pos = setting_tunevalue_500;
  if(setting_cansize == 330) {
      target_pos = setting_tunevalue_330;
  } else if(setting_cansize == 440) {
      target_pos = setting_tunevalue_440;
  }

  long purgepos = target_pos > 0 ? (target_pos - POSITION_PURGE) : (target_pos + POSITION_PURGE);
  stepper.moveTo(purgepos);
  stepper.runToPosition();

 
  stepper.setMaxSpeed(500.0);     // Slower you slut
  stepper.setAcceleration(500.0);

  stepper.moveTo(target_pos);
  stepper.runToPosition();
  
  state_isStepperRising = false;
  state_isStepperRised = true;
  
}
