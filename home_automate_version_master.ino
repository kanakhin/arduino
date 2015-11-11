#include <Bounce2.h>
#include <EEPROM.h>
#include <Wire.h>

const byte address = 77; //4d hex
const byte cnt = 4;
const byte pin_cnt = 19;

int pins[] = {14,15,16,17};
int leds[] = {3,4,5,6};

byte init_leds[cnt] ;
byte init_buttons[cnt];
int button_states[cnt];
bool disabled_buttons[cnt];
Bounce debouncers[cnt];
unsigned long buttonPressTimeStamps[cnt];
boolean changed[cnt];

bool new_command = false;
bool start_request = false;

byte request_type = 1;
byte first_command = 0;
byte current_port = 0;

void setup(){
  Serial.begin(9600);
  
  for(byte i=0; i<cnt; i=i+1) {
    button_states[i] = 0;
    
    byte value = EEPROM.read(leds[i]);
    if(value==11) {
      init_leds[i] = LOW ;
    }else{
      init_leds[i] = HIGH ;
    }
    init_buttons[i] = HIGH;
    buttonPressTimeStamps[i] = 0;
    changed[i] = false;

    debouncers[i] = Bounce();

    pinMode(pins[i], INPUT);
    pinMode(leds[i], OUTPUT);
    
    digitalWrite(pins[i], init_buttons[i]);
    digitalWrite(leds[i], init_leds[i]);
    
    debouncers[i].attach( pins[i] );
    debouncers[i].interval(5);
  }
  
  Wire.begin(address);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void answer_to_master(byte data) {
  Wire.write(data);
}

void do_action(byte command, byte port) {
  Serial.println(command);
  Serial.println(port);
  byte value = 0;
  byte dvalue = 0;
  switch (command) {
    case 1:
      start_request = true;
      request_type = 1;
      current_port = 0;
      
      break;
    case 2:
      start_request = true;
      request_type = 2;
      current_port = 0;
      if (disabled_buttons[port] == true) {
        answer_to_master(1);
      }else{
        answer_to_master(0);
      }
      break;
    case 3:
      value = EEPROM.read(port);
      if(value==11) {
        digitalWrite(port, HIGH);
        EEPROM.write(port, 10);
      } else {
        digitalWrite(port, LOW);
        EEPROM.write(port, 11);
      }
      
      break;
    case 4:
      dvalue = EEPROM.read(port);
      if(dvalue==11) {
        EEPROM.write(port, 10);
      } else {
        EEPROM.write(port, 11);
      }
      
      break;
    case 5:
      for (byte i=0; i<cnt; i = i + 1) {
        digitalWrite(leds[i], LOW);
        EEPROM.write(leds[i], 11);
      }
      
      break;
    case 6:
      for (byte i=0; i<cnt; i = i + 1) {
        digitalWrite(leds[i], HIGH);
        EEPROM.write(leds[i], 10);
      }
      
      break;
    default:
      
    break;
  }
}

unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}

String decToHex(byte decValue, byte desiredStringLength) {
  
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  
  return hexString;
}

void receiveEvent(int howMany) {
  byte bytes = Wire.available();
  int x = 0;
  for (byte i=1; i <= bytes; i=i+1) {
    x = Wire.read();
  }
  
  Serial.println(x);

  if (x == 1 or x == 2 or x == 5 or x == 6) {
    do_action(x, 0);
  } else {
    if ( x > 200) {
      do_action (4, x - 200);
    } else {
      do_action (3, x - 100);
    }
  }  
}

void requestEvent() {
  if (request_type == 1) {
    byte value = EEPROM.read(leds[current_port]);
    if(value==11) {
      answer_to_master(1);
    } else {
      answer_to_master(0);
    }
    
    current_port = current_port + 1;
  } else if (request_type == 2) {
    byte dvalue = EEPROM.read(pins[current_port]);
    if(dvalue==11) {
      answer_to_master(1);
    } else {
      answer_to_master(0);
    }

    current_port = current_port + 1;
  }
}

void loop(){
  for(byte i=0; i<cnt; i=i+1){
    byte dvalue = EEPROM.read(pins[i]);
    if(dvalue!=11) {
      changed[i] = debouncers[i].update();
      
      if ( changed[i] ) {
        int value = debouncers[i].read();
        if ( value == HIGH) {
           button_states[i] = 0;   
       } else {
             byte value = EEPROM.read(leds[i]);
             if(value==11) {
               digitalWrite(leds[i], HIGH );
               EEPROM.write(leds[i], 10);
             }else{
               digitalWrite(leds[i], LOW);
               EEPROM.write(leds[i], 11);
             }
             
             button_states[i] = 1;
             buttonPressTimeStamps[i] = millis();     
       }
      }
   
      if ( button_states[i] == 1 ) {
        if ( millis() - buttonPressTimeStamps[i] >= 200 ) {
            button_states[i] = 2;
        }
      }
    }
  }
} 
