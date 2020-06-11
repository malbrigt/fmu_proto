void read_buttons() {


  //
  // Home button pressed? (Debounce method from https://www.arduino.cc/en/tutorial/debounce)
  //
  int reading_home  = digitalRead(PIN_SWITCH_HOME);   // current value of pushbutton input pin
  if (reading_home != lastbuttonstate_home) {         // reading changed since last loop?
    debounce_home = millis();                         // reset the debouncing timer
    //Serial.println("loop() - reading_home changed, waiting for debounce check");
  }
  if ((millis() - debounce_home) > debounce_delay) {  // stable reading over debounce_delay milliseconds?
    if (reading_home != buttonstate_home) {           // have the state changed?
      buttonstate_home = reading_home;                // set new state

      if (buttonstate_home == LOW)                    // if state is low (pressed, remember we're on pullups)
        buttonpress_home();                           // perform action
    }
  }
  lastbuttonstate_home = reading_home;

  //
  // Rise button pressed? (Debounce method from https://www.arduino.cc/en/tutorial/debounce)
  //
  int reading_rise  = digitalRead(PIN_SWITCH_RISE);   // current value of pushbutton input pin
  if (reading_rise != lastbuttonstate_rise)  {        // reading changed since last loop?
    debounce_rise = millis();                         // reset the debouncing timer
    //Serial.println("loop() - reading_rise changed, waiting for debounce check");
  }
  if ((millis() - debounce_rise) > debounce_delay) {  // stable reading over debounce_delay milliseconds?
    if (reading_rise != buttonstate_rise) {           // have the state changed?
      buttonstate_rise = reading_rise;                // set new state

      if (buttonstate_rise == LOW)                    // if state is low (pressed, remember we're on pullups)
        buttonpress_rise();                           // perform action
    }
  }  
  lastbuttonstate_rise = reading_rise;


  //
  // Size button pressed? (Debounce method from https://www.arduino.cc/en/tutorial/debounce)
  //
  int reading_size  = digitalRead(PIN_SWITCH_SIZE);   // current value of pushbutton input pin
  if (reading_size != lastbuttonstate_size) {         // reading changed since last loop?
    debounce_size = millis();                         // reset the debouncing timer
    //Serial.println("loop() - reading_size changed, waiting for debounce check");
  }

  if ((millis() - debounce_size) > debounce_delay) {  // stable reading over debounce_delay milliseconds?
    if (reading_size != buttonstate_size) {           // have the state changed?
      buttonstate_size = reading_size;                // set new state

      if (buttonstate_size == LOW)                    // if state is low (pressed, remember we're on pullups)
        buttonpress_size();                           // perform action
    }
  }  
  lastbuttonstate_size = reading_size;  


  //
  // prge button pressed? (Debounce method from https://www.arduino.cc/en/tutorial/debounce)
  //
  int reading_prge  = digitalRead(PIN_SWITCH_PRGE);   // current value of pushbutton input pin
  if (reading_prge != lastbuttonstate_prge) {         // reading changed since last loop?
    debounce_prge = millis();                         // reset the debouncing timer
    //Serial.println("loop() - reading_prge changed, waiting for debounce check");
  }

  if ((millis() - debounce_prge) > debounce_delay) {  // stable reading over debounce_delay milliseconds?
    if (reading_prge != buttonstate_prge) {           // have the state changed?
      buttonstate_prge = reading_prge;                // set new state

      if (buttonstate_prge == LOW)                    // if state is low (pressed, remember we're on pullups)
        buttonpress_prge();                           // perform action
    }
  }  
  lastbuttonstate_prge = reading_prge;  



  //
  // fill button pressed? (Debounce method from https://www.arduino.cc/en/tutorial/debounce)
  //
  int reading_fill  = digitalRead(PIN_SWITCH_FILL);   // current value of pushbutton input pin
  if (reading_fill != lastbuttonstate_fill) {         // reading changed since last loop?
    debounce_fill = millis();                         // reset the debouncing timer
    //Serial.println("loop() - reading_fill changed, waiting for debounce check");
  }

  if ((millis() - debounce_fill) > debounce_delay) {  // stable reading over debounce_delay milliseconds?
    if (reading_fill != buttonstate_fill) {           // have the state changed?
      buttonstate_fill = reading_fill;                // set new state

      if (buttonstate_fill == LOW)                    // if state is low (pressed, remember we're on pullups)
        buttonpress_fill();                           // perform action
    }
  }  
  lastbuttonstate_fill = reading_fill;    
  
}
