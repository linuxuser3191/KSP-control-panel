void switch_initialization() {
  led_test();                                    //function to test all LEDs
  switch_test();                                 //function to read all SWITCH positions
}

void led_test() {
  int i = 0;

  for (i = 0; i <= 15; i++) {                     //turn each LED on individually, then off; 0-15 is 2 registers (see constructor in ksp_control_panel)
    srLED.set(i, HIGH);
    delay(250);
    srLED.set(i, LOW);
    delay(250);
  }
  
  srLED.setAllHigh();                             //turn all LEDs on together
  delay(1000);

  srLED.setAllLow();                              //turn all LEDs off
  delay(1000);
}

void switch_test() {
  int i = 0;
  
  for (i = 0; i <= 15; i++) {                     //check each switch position
    uint8_t sr1 = srSW.get(i);                    //sr1 is set to each 4021 pin sequentially

    if (sr1 != 0 && i != 3 && i != 7) {           //if switch is high, print switch position, except 3 and 7, which are wired high always
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(sr1);
      //delay(3000);
    }else if( i ==3 || i == 7 && sr1 == 0) {      //switch 3 and 7 are set to high always, so print switch position if it is low
      Serial.print(i);
      Serial.print(" = ");
      Serial.println(sr1);
    }
  }
}
