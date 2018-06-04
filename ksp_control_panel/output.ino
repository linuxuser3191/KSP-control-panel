//LED register 1
  int ledSAS =         0;                //pin 0 on first 74hc595 shift register
  int ledMANEUVER =    1;                //pin 1 on first 74hc595 shift register
  int ledPROGRADE =    2;                //pin 2 on first 74hc595 shift register
  int ledRETROGRADE =  3;                //pin 3 on first 74hc595 shift register
  int ledNORMAL =      4;                //pin 4 on first 74hc595 shift register
  int ledANTINORMAL =  5;                //pin 5 on first 74hc595 shift register
  int ledRADIALOUT =   6;                //pin 6 on first 74hc595 shift register
  int ledRADIALIN =    7;                //pin 7 on first 74hc595 shift register

  //LED register 2
  int ledEXECUTE =    14;                //pin 6 on second 74hc595 shift register
  int ledTARGET =      8;                //pin 0 on second 74hc595 shift register
  int ledANTITARGET =  9;                //pin 1 on second 74hc595 shift register
  
String toaction="";                      //string to action to kRPC
String action = "";                      //string to hold output action

void output() {
  action = io_panel();                   //runs io_panel and sets the return string to action
  if (action != "") {                    //if output action isn't blank, send it to kRPC
    kRPC(action);
    leds();                              //send output to set leds
    action = "";                         //reset output action to blank, so action loop is reset
  }
}

void kRPC(String toaction) {             //kRPC receives string variable from io_panel, throttle_panel, fcs_panel
  //lcd.setCursor(0,0);                    //set lcd status display cursor to line 1, character position 1
  //lcd.print(toaction);                   //lcd display prints action
  if(toaction.length() == 16){           //if string length is 16 characters, print string to kRPC
    if(DEBUG){                           //debug sends print lines for easier reading in serial output
      Serial.println(toaction);
    }else{
      Serial.print(toaction);
    }
  }
  toaction = "";                         //reset toaction string so only one output is sent to kRPC
}

void leds() {                            //TODO - update integers to strings
  
  int intaction = action.toInt();
  /*
  lcd.setCursor(8,0); 
  lcd.print(intaction);
  */
  if (intaction == 301) {                   //if output action string is 301, blink execute light; need to figure out a blink without delaying input
    srLED.set(ledEXECUTE, HIGH);
    srLED.set(ledEXECUTE, LOW);
  } else if (intaction == 400) {            //if output action string is 400, turn all leds off, then set sas led on
    srLED.setAllLow();
    srLED.set(ledSAS, HIGH);
  } else if (intaction == 401) {            //if output action string is 401, turn all leds off, then set sas led on
    srLED.setAllLow();
    srLED.set(ledSAS, HIGH);    
  } else if (intaction == 402) {            //if output action string is 402, turn all leds off, then set maneuver led on
    srLED.setAllLow();
    srLED.set(ledMANEUVER, HIGH);    
  } else if (intaction == 403) {            //if output action string is 403, turn all leds off, then set prograde led on
    srLED.setAllLow();
    srLED.set(ledPROGRADE, HIGH);    
  } else if (intaction == 404) {            //if output action string is 404, turn all leds off, then set retrograde led on
    srLED.setAllLow();
    srLED.set(ledRETROGRADE, HIGH);    
  } else if (intaction == 405) {            //if output action string is 405, turn all leds off, then set normal led on
    srLED.setAllLow();
    srLED.set(ledNORMAL, HIGH);    
  } else if (intaction == 406) {            //if output action string is 406, turn all leds off, then set antinormal led on
    srLED.setAllLow();
    srLED.set(ledANTINORMAL, HIGH);    
  } else if (intaction == 407) {            //if output action string is 407, turn all leds off, then set radial out led on
    srLED.setAllLow();
    srLED.set(ledRADIALOUT, HIGH);    
  } else if (intaction == 408) {            //if output action string is 408, turn all leds off, then set radial in led on
    srLED.setAllLow();
    srLED.set(ledRADIALIN, HIGH);    
  } else if (intaction == 409) {            //if output action string is 409, turn all leds off, then set target led on
    srLED.setAllLow();
    srLED.set(ledTARGET, HIGH);    
  } else if (intaction == 410) {            //if output action string is 410, turn all leds off, then set antitarget led on
    srLED.setAllLow();
    srLED.set(ledANTITARGET, HIGH);    
  } else if (intaction == 411) {            //if output action string is 411, turn all leds off, then turn all leds off
    srLED.setAllLow();
  }
}
