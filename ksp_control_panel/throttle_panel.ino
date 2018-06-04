const int throttlePin = A12;                                  //throttle is analog12 (gray)
const int motorForward = 52;                                  //motor 1 forward pin from L293D chip
const int motorBackward = 50;                                 //motor 1 backward pin from L293D chip
const int fullThrottleSW = 7;                                 //pin 7 on first 4021B shift register
const int killThrottleSW = 3;                                 //pin 3 on first 4021B shift register

int minimumthrottletosend = 35;                               //if these value changes, need to update formula and min/max values in python code
int maximumthrottletosend = 1010;

int count = 0;
int polltime = 100;                                           //how often to send throttle data to kRPC
uint8_t fullthrottlecheck = 1;                                //check full throttle switch position
uint8_t killthrottlecheck = 1;                                //check kill throttle switch position
long sentlastthrottle = millis();                             //time last throttle data was sent to kRPC

boolean allowsend = true;                                     //allows us to send data to kRPC
int throttleValue;                                            //variable to read analog throttle value

String prepareoutput(String command,int input) {              //prepareoutput receives string command input, and integer throttle input
  String inputasstring = String(input);                       //analog throttle data sent after "500.00000000xxxx" where xxxx is throttle value
  String tosend = command;
  tosend.concat(".");                                         //add a period between command and throttle data
  while (tosend.length() + inputasstring.length()< 16) {      //CHARACTER SIZE THAT PYTHON ACCEPTS. CHANGE THE 16 if you are pulling off more bytes
    tosend.concat(0);                                         //add zeros until you hit 16 character length
  }
  tosend.concat(inputasstring);
  return tosend;                                              //return completed string
}

void throttle_kill(){                                         //kill throttle switch
  throttleoutput(0);                                          //send zero to throttle output
  digitalWrite(motorBackward, HIGH);                          //set motor backward to full and motor forward to off to move motor
  digitalWrite(motorForward, LOW);
  do {
    throttleValue = analogRead(throttlePin);                  //only apply power to motor while throttle is above minimum throttle threshold
  } while (throttleValue > minimumthrottletosend);
  
  throttle_manual();                                          //set throttle to manual
  allowsend = false;                                          //turn off sending throttle data
}
void throttle_full(){                                         //full throttle switch
  throttleValue = analogRead(throttlePin);                    //read analog throttle data
  throttleoutput(maximumthrottletosend);                      //stop sending throttle data when above max throttle to send

  digitalWrite(motorForward, HIGH);                           //set motor forward to full and motor backward to off to move motor
  digitalWrite(motorBackward, LOW);
  do{    
    throttleValue = analogRead(throttlePin);                  //only apply power to motor while throttle is below maximum throttle threshold
  } while (throttleValue < maximumthrottletosend);
  
  throttle_manual();                                          //set throttle to manual
  allowsend = false;                                          //turn off sending throttle data
}

void throttle_manual(){                                       //disengage throttle motor
  //ALLOW MANUAL MOVEMENT
  digitalWrite(motorBackward, LOW);                           //turn motors off to allow manual throttle movement
  digitalWrite(motorForward, LOW);
}

void throttle_setup(){
  pinMode(motorForward, OUTPUT);                              //set motor pins as outputs
  pinMode(motorBackward, OUTPUT);                 
  //set throttle motor to off initially
  digitalWrite(motorForward, LOW);                            //turn motors off to allow manual throttle movement
  digitalWrite(motorBackward, LOW);

  Serial.print(prepareoutput("501",0));                       //send command to kRPC to read in-game throttle state
  int input = getinput().toInt();                             //read in-game throttle and set to input
  throttleValue = analogRead(throttlePin);                    //read analog throttle data
  if( input > throttleValue){                                 //if game data is greater than analog data, move throttle forward to match within 10
    digitalWrite(motorForward, HIGH);
    digitalWrite(motorBackward, LOW);
    do{
      throttleValue = analogRead(throttlePin); 
    }while(abs(input - throttleValue) > 10);
    digitalWrite(motorForward, LOW);
    digitalWrite(motorBackward, LOW);
  }else{                                                      //if game data is less than analog data, move throttle backwards to match within 10
    digitalWrite(motorForward, LOW);
    digitalWrite(motorBackward, HIGH);
    do{
      throttleValue = analogRead(throttlePin); 
    }while(abs(input - throttleValue) > 10);
    digitalWrite(motorForward, LOW);
    digitalWrite(motorBackward, LOW);
  }
}

void throttleoutput(int output){                            //accepts integers to kill or full throttle and sends it to output string
  String tooutput = prepareoutput("500",output);
  if(DEBUG){                                                //debug adds print lines for reading serial output
    Serial.println(tooutput);
  }else{
    Serial.print(tooutput);
  }
}

void throttle_panel() {
 if (count > 500){                                          //read full and kill throttle switch
    fullthrottlecheck = srSW.get(fullThrottleSW);
    killthrottlecheck = srSW.get(killThrottleSW);
  } else {
    count++;
    if(count %100 == 0){
      //Serial.print(count);
    }
  }
  
  if (fullthrottlecheck == 0){                              //full and kill throttle switches are high by default, if either set to 0, set motor as needed
    throttle_full();
  }
  if (killthrottlecheck == 0){
    throttle_kill();
  }
    
  throttleValue = analogRead(throttlePin);                                  
  if (allowsend && throttleValue > minimumthrottletosend && throttleValue < maximumthrottletosend) {               //send analog throttle value if above minimum and less than maximum
    throttleoutput(throttleValue);
    
    sentlastthrottle = millis();                            //time that last throttle data was sent
    allowsend = false;                                      //stop sending throttle data
  }
  
  if (millis() - sentlastthrottle > polltime){              //if current time minus the time last throttle data was sent is greater than polltime
    allowsend = true;                                       //allow throttle data to be sent
  }  
}
