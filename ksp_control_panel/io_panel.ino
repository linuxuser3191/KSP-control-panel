//SWITCH status
boolean sasSW = false;                            //SAS panel sas mode toggle switch status
boolean abortSW = false;                          //Staging panel abort pushbutton status
boolean armSW = false;                            //Staging panel arm toggle switch

//SWITCH variables
int sSAS = 2;                                     //pin 2 on first 4021b shift register
int bSAS = 8;                                     //pin 0 on second 4021b shift register
int bMANEUVER = 9;                                //pin 1 on second 4021b shift register
int bPROGRADE = 10;                               //pin 2 on second 4021b shift register
int bRETROGRADE = 11;                             //pin 3 on second 4021b shift register
int bNORMAL = 12;                                 //pin 4 on second 4021b shift register
int bANTINORMAL = 13;                             //pin 5 on second 4021b shift register
int bRADIALOUT = 14;                              //pin 6 on second 4021b shift register
int bRADIALIN = 15;                               //pin 7 on second 4021b shift register
int bTARGET = 0;                                  //pin 0 on first 4021b shift register
int bANTITARGET = 1;                              //pin 1 on first 4021b shift register
int bEXECUTE = 6;                                 //pin 6 on first 4021b shift register
int sARM = 5;                                     //pin 5 on first 4021b shift register
int bABORT = 4;                                   //pin 4 on first 4021b shift register

//variables for loops
int i = 0;                                        //used in io_panel to poll switches
int j = 0;                                        //used to reset abort button
int k = 0;                                        //used to send one sas switch output
int m = 0;                                        //used to reset arm switch

//variables for button debounce
long debouncetime = 300;                          //cant hit each button more than approx 3x each second
long debounce[16];                                //YOU NEED TO CHANGE THIS NUMBER IF YOU ADD MORE BUTTONS

void initialize_debounce(){                       //debounce prevents multiple commands sent from a single button push
  long timeinit = millis();                       //timeinit is set to current time 
  for(int x = 0; x < sizeof(debounce)/sizeof(long); x++){         //for each element in debounce array, take timeinit and set to each element
    debounce[x] = timeinit;
  }
}

String prepareforoutput(int input){               //each switch returns a string of an int, this function adds a . and additional 0's up to 16 chars
  String tosend = String(input);                  //output to serial looks like "300.000000000000"
  tosend.concat(".");                             //add the period after the 3 digits
  while(tosend.length() < 16){                    //CHARACTER SIZE THAT PYTHON ACCEPTS. CHANGE THE 16 if you are pulling off more bytes
    tosend.concat(0);                             //add zeros until the string is 16 chars
  }
  return tosend;
}

String io_panel() {
  for (i = 0; i <= 15; i++) {                     //CHANGE HERE FOR NUMBER OF BUTTONS TOO
    uint8_t sw = srSW.get(i);                     //sw is set to each pin on 4021 shift registers
    long timenow = millis();                      //timenow is set to current time
    if (sw != 0 && timenow - debounce[i] > debouncetime) {        //if the switch is high and it passes debounce time, set new debounce time
      debounce[i] = timenow;
      if (i == sARM && armSW == false) {          //if arm switch is high and status is false
        if (m == 0) {
          armSW = true;                           //set arm switch status to true, only runs once until reset
          m = 1;
        }
      }
      if (armSW == true) {                        //if arm switch status is true
        if (i == bEXECUTE) {                      //if execute button is pressed
          return prepareforoutput(301);           //send 301 to string output
        }
      }
      if (i == bABORT && abortSW == false) {      //if abort button is pressed and abort status is false
        if (j == 0) {
          abortSW = true;                         //set abort status to true, only runs once until reset
          j = 1;
          return prepareforoutput(302);           //send 302 to string output
        }
      }

      if (i == sSAS && sasSW == false) {          //if sas switch is on and sas switch status is false
        if (k == 0) {
          sasSW = true;                           //set sas switch status true, only runs once until reset
          k = 1;
          return prepareforoutput(400);           //send 400 to string output
        }
      }
      if (sasSW == true) {                        //if SAS panel sas toggle switch is HIGH, check SAS panel mode momentary pushbuttons
        if (i == bSAS) {                          //if stability assist button is pressed
          return prepareforoutput(401);           //send 401 to string output
        } else if (i == bMANEUVER) {              //if maneuver button is pressed
          return prepareforoutput(402);           //send 402 to string output
        } else if (i == bPROGRADE) {              //if prograde button is pressed
          return prepareforoutput(403);           //send 403 to string output
        } else if (i == bRETROGRADE ) {           //if retrograde button is pressed
          return prepareforoutput(404);           //send 404 to string output
        } else if (i == bNORMAL) {                //if normal button is pressed
          return prepareforoutput(405);           //send 405 to string output
        } else if (i == bANTINORMAL) {            //if antinormal button is pressed
          return prepareforoutput(406);           //send 406 to string output
        } else if (i == bRADIALOUT) {             //if radial out button is pressed
          return prepareforoutput(407);           //send 407 to string output
        } else if (i == bRADIALIN) {              //if radial in button is pressed
          return prepareforoutput(408);           //send 408 to string output
        } else if (i == bTARGET) {                //if target button is pressed
          return prepareforoutput(409);           //send 409 to string output
        } else if (i == bANTITARGET) {            //if antitarget button is pressed
          return prepareforoutput(410);           //send 410 to string output
        }
      }
    }
    if (i == bABORT && sw == 0) {                 //if abort button is reset
      if (j == 1) {
        abortSW = false;                          //set abort status to false, only runs once
        j = 0;
      }
    }
    if (i == sSAS && sw == 0) {                   //if sas switch is reset
      if (k == 1) {
        sasSW = false;                            //set sas switch status to false, only runs once
        k = 0;
        return prepareforoutput(411);             //sends 411 to string output
      }
    }
    if (i==sARM && sw == 0) {                     //if arm switch is reset
      if (m == 1) {
        armSW = false;                            //set arm switch status to false, only runs once
        m = 0;
      }
    }
  }
  return "";                                      //return empty string if no buttons were pushed
}
