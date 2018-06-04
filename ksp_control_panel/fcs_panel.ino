const int yawPin = A10;                             //yaw is analog10 (white)
const int pitchPin = A13;                           //pitch is analog13 (purple)
const int rollPin = A9;                             //roll is analog9 (black)

int fcspolltime = 50;                               //how often we send joystick data to serial bus

int yawValue;                                       //variable for reading vehicle yaw
int pitchValue;                                     //variable for reading vehicle pitch
int rollValue;                                      //variable for reading vehicle roll

String formatfcs(int i){                            //each attitude position requires at least 4 chars to send to python
  String ret = String(i);                           //string input from analog data
  while(ret.length() < 4){                          //if string length is less than 4
    ret = "0" + ret;                                //add zeros to front, ie 0023, 0486, 1016
  }
  return ret;
}

long sentlastfcs = millis();                        //variable to keep track last time we sent fcs data, use with polltime

void fcs_panel() {
  yawValue = analogRead(yawPin);                    //read analog yaw value
  pitchValue = analogRead(pitchPin);                //read analog pitch value
  rollValue = analogRead(rollPin);                  //read analog roll value

  //example fcs output to serial is "!0yyyy.pppp.rrrr where y,p,r is the 4 digit analog value
  String toprint = "!0";                            //each fcs output begins with !0
  toprint.concat(formatfcs(yawValue));              //add yaw to string output
  toprint.concat(".");                              //period separates yaw from pitch
  toprint.concat(formatfcs(pitchValue));            //add pitch to string output
  toprint.concat(".");                              //period separates pitch from roll
  toprint.concat(formatfcs(rollValue));             //add roll to string output

  if(millis() - sentlastfcs > fcspolltime){         //if time since last fcs data sent is greater than poll time, send fcs data
    if(toprint.length() == 16){                     //only send fcs data if string length is 16 chars long
      if(DEBUG){                                    //debug prints lines for easier serial monitor reading
        Serial.println(toprint);
      }else{
        Serial.print(toprint);                      //otherwise send print statements for python
      }
      sentlastfcs = millis();                       //sets most recent fcs data sent to current time
    }
  }
}
