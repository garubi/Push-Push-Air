#include <BleKeyboard.h>
BleKeyboard bleKeyboard;

#include <Bounce2.h>

/**
 * for 18650 battery level check
 * see: https://www.pangodream.es/esp32-getting-battery-charging-level
 *  
 * If you need to change default values you can use it as
 * Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);
 * #define ADC_PIN 34
 * #define CONV_FACTOR 1.7
 * #define READS 20
 */
#include <Pangodream_18650_CL.h>
Pangodream_18650_CL BL;

//Pulsanti
const byte PEDALNEXT_PIN = 21;
const byte PEDALPREV_PIN = 23;

//Led
const byte PEDALNEXT_LED = LED_BUILTIN;
const byte PEDALPREV_LED = LED_BUILTIN;

//Tasti da emulare
const byte PEDALNEXT_KEY = KEY_RIGHT_ARROW;
const byte PEDALPREV_KEY = KEY_LEFT_ARROW;


const byte PED_NEXT = PEDALNEXT_PIN;
const byte PED_PREV = PEDALPREV_PIN;

// Instantiate a Bounce object
Bounce ped_next = Bounce(); 
// Instantiate another Bounce object
Bounce ped_prev = Bounce(); 

unsigned long batCheckTime;
const int BAT_POLLING_INTERVAL = 5000;

unsigned long btnReadingSettle;

static void SendKey( byte pedal ){
  if (bleKeyboard.isConnected()) {
    switch( pedal ){
      case PED_NEXT:
          bleKeyboard.press(PEDALNEXT_KEY);
      break; 
      case PED_PREV:
          bleKeyboard.press(PEDALPREV_KEY);
      break; 
    }
    
    Serial.println(pedal);
//    delay(100);
//    bleKeyboard.releaseAll();
  }
}

void setup(void)
{
    bleKeyboard.begin();
    
    // BUTTONS / INPUTS
    pinMode(PEDALNEXT_PIN, INPUT_PULLUP);
    pinMode(PEDALPREV_PIN, INPUT_PULLUP);
    
    ped_next.attach(PEDALNEXT_PIN);
    ped_prev.attach(PEDALPREV_PIN);

    // OUTPUTS /LEDS
    pinMode(PEDALNEXT_LED, OUTPUT);
    pinMode(PEDALPREV_LED, OUTPUT);

    Serial.begin(115200);
//    Serial.setDebugOutput(true);
//    Serial.print("ESP32 SDK: ");
//    Serial.println(ESP.getSdkVersion());
}

void loop(void)
{
    if( btnReadingSettle + 50 < millis() ){
      btnReadingSettle = millis();
      static uint8_t pedalNEXTStateLast = 0;
      static uint8_t pedalPREVStateLast = 0;
      uint8_t pedalState;
      ped_next.update();
      ped_prev.update();
    
      pedalState = ped_next.read();
      if (pedalState != pedalNEXTStateLast) {
          pedalNEXTStateLast = pedalState;
  
          if (pedalState == LOW ) {
              SendKey( PED_NEXT );
          }
          digitalWrite(PEDALNEXT_LED, pedalState );
      }
  
      pedalState = ped_prev.read();
      if (pedalState != pedalPREVStateLast) {
          pedalPREVStateLast = pedalState;
  
          if (pedalState == LOW ) {
              SendKey( PED_PREV );
          }
          digitalWrite(PEDALPREV_LED, pedalState );
      }
    }

  if( batCheckTime + BAT_POLLING_INTERVAL < millis() ){
    batCheckTime = millis();
  Serial.print("Value from pin: ");
  Serial.println(analogRead(34));
  Serial.print("Average value from pin: ");
  Serial.println(BL.pinRead());
  Serial.print("Volts: ");
  Serial.println(BL.getBatteryVolts());
  Serial.print("Charge level: ");
  Serial.println(BL.getBatteryChargeLevel());
  Serial.println("");
  // delay(1000);
  }

}
