# Push Push Air

_[this page is a work in progress, and for the moment it's little more than a notepad... it will become a well documentend Readme aniway, soon or later]_

A little (but expandable) battery powered pedalboard that, emulating a Bluethooth keyboard, send keystrokes (per example it can turn pages on a PDF reader... ). 

Made with an ESP32
Configurable via WiFI (you can choose the keystrokes that you want to send)



Libraries used:
```
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
 ```
 
 BleKeyboard 0.3.0
 Preferences
 Bounce2