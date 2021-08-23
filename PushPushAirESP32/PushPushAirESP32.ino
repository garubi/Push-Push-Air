#include <BleKeyboard.h>
/**
 * see: https://www.pangodream.es/esp32-getting-battery-charging-level
 */
#include <Pangodream_18650_CL.h>

//#define ADC_PIN 34
//#define CONV_FACTOR 1.7
//#define READS 20

Pangodream_18650_CL BL;
/**
 * If you need to change default values you can use it as
 * Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);
 */
 
#define RIGHT_BTN 21
#define LEFT_BTN 23

#define RIGHT_BTN_CODE KEY_RIGHT_ARROW
#define LEFT_BTN_CODE KEY_LEFT_ARROW

BleKeyboard bleKeyboard;

void setup() {
  bleKeyboard.begin();
  
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);

    Serial.begin(115200);
//    Serial.setDebugOutput(true);
//    Serial.print("ESP32 SDK: ");
//    Serial.println(ESP.getSdkVersion());
}

void loop() {
  if (bleKeyboard.isConnected() && !digitalRead(RIGHT_BTN)) {
    bleKeyboard.press(RIGHT_BTN_CODE);
    Serial.println(RIGHT_BTN);
    delay (100);
    bleKeyboard.releaseAll();
  }
  if (bleKeyboard.isConnected() && !digitalRead(LEFT_BTN)) {
    bleKeyboard.press(LEFT_BTN_CODE);
    delay (100);
    Serial.println(LEFT_BTN);
    bleKeyboard.releaseAll();
  }
  Serial.print("Value from pin: ");
  Serial.println(analogRead(34));
  Serial.print("Average value from pin: ");
  Serial.println(BL.pinRead());
  Serial.print("Volts: ");
  Serial.println(BL.getBatteryVolts());
  Serial.print("Charge level: ");
  Serial.println(BL.getBatteryChargeLevel());
  Serial.println("");

  if
  delay(1000);
}
