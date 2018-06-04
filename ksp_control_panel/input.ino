String getinput(){                                                //String that reads input from kRPC over serial
  String ret = "";                                                //new empty string
  for(int x = 0; x<12; x++){                                      //string from kRPC is always 12 characters
    int input = -1;                                               //input of -1 is what is received when no data is sent from kRPC
    do{
      input = Serial.read();                                      //read input from kRPC if input is not -1
    }while(input == -1);

    char cinput = char(input);                                    //casting integer received from kRPC into a char
    ret.concat(cinput);                                           //adding character to the return string
  }
  return ret;                                                     //return completed string
}

String stripLeadingZeroes(String input){                          //receives string input from input function below
  String ret = "";                                                //new empty string
  int start = 0;
  for(int x = 0; x<input.length(); x++){                          //start x at the first character in input, and examine each character in the string
    if(input.charAt(x) != '0'){                                   //if the character is not a zero, the string is started at that character
      start = x;
      break;
    }
  }
  for(int x = start; x<input.length(); x++){                      
    ret.concat(input.charAt(x));                                  //add character at current position to string
  }
  return ret;                                                     //return completed string
}

int displaysize[4]; 
void input() {
  
  
  //altitude
  Serial.print(prepareoutput("601",0));                           //send string 601 to kRPC
  lcd.setCursor(0,0);                                             //set lcd cursor to line 1, position 1
  String altitude = stripLeadingZeroes(String(getinput()));       //get altitude input from kRPC after 601 was sent, passes it to stripLeadingZeros function
  if(altitude.length() < displaysize[0]){
    lcd.clear();
  }
  displaysize[0] = altitude.length();
  lcd.print(altitude);                                            //print altitude at cursor position

  //apoapsis
  Serial.print(prepareoutput("602",0));                           //send string 602 to kRPC
  lcd.setCursor(0,1);                                             //set lcd cursor to line 2, position 1
  String apoapsis = stripLeadingZeroes(String(getinput()));       //get apoapsis input from kRPC after 602 was sent, passes it to stripLeadingZeros function
                                            
  if(apoapsis.length() < displaysize[1]){
    lcd.clear();
  }
  displaysize[1] = apoapsis.length();
  lcd.print(apoapsis);                                            //print apoapsis at cursor position
  //periapsis
  Serial.print(prepareoutput("603",0));                           //send string 603 to kRPC
  lcd.setCursor(8,1);                                             //set lcd cursor to line 2, position 8
  String periapsis = stripLeadingZeroes(String(getinput()));      //get periapsis input from kRPC after 603 was sent, passes it to stripLeadingZeros function
  if(periapsis.length() < displaysize[2]){
    lcd.clear();
  }
  displaysize[2] = periapsis.length();
  lcd.print(periapsis);                                           //print periapsis at cursor position

  //SPEED LOL
  
  Serial.print(prepareoutput("604",0));                           //send string 604 to kRPC
  lcd.setCursor(8,0);                                             //set lcd cursor to line 1, position 8
  String orbital_speed = stripLeadingZeroes(String(getinput()));  //get orbital speed input from kRPC after 604 was sent, passes it to stripLeadingZeros function
  if(orbital_speed.length() < displaysize[3]){
    lcd.clear();
  }
  displaysize[3] = orbital_speed.length();
  lcd.print(orbital_speed);                                       //print orbital speed at cursor position
}
