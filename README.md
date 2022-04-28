# Push Push Air

_[this page is a work in progress, and for the moment it's little more than a notepad... it will become a well documentend Readme, soon or later]_

A little (but expandable) battery powered pedalboard that, emulating a Bluethooth keyboard, send keystrokes (per example it can turn pages on a PDF reader... ). 

Made with an ESP32
Configurable via WiFI (you can choose the keystrokes that you want to send)

## Usage ##
- Turn on your PushPush AIR
- Go to the Bluethooth manager on your mobile, tablet or computer and start the search for new devices
- In a few seconds it should appear "PushPushAIR2" (or the new name of the device if you already changed it[see below] )
- Click/Tap on the discovered PushPushAIR to pair it with your mobile device
- PushPushAIR will appear and act as a remote Bluethooth keyboard
- Out of the box PushPushAIR is configured to send the key cursor left and right. Check the app that you want to control and, if it needs different keys, go to the Configuration section and change the keystrokes sent.

## Configuration ##
- Turn on the PPA while pushing the right-most pedal.
- The status led will be steady blue.
- Now connect from a computer or mobile device to the wifi network "Push Push AIR" and insert the passord
- Once the connection is etablished, browse to 192.168.4.1 to use the configuration page.
- On the configuration page you can set:
 - The device name that idetify the Push Push AIR on WiFI and Bluethooth (you need to restart the device in order to render effective the name's change)
 - The passowrd to access the configuration page
 - The characters sent by the pedals
 
## Reset the password to the factory one ##
 If you change the password of the Configuration page and you forget it, you can revert to the factory one: `12345678`
 **How to reset:**
 - Turn on the PPA while pushing both pedals. The Status led starts blink fast
 - Keep pressed both the pedals for 5 seconds, untile the Status led turn steady on
 - Wait 2/3 seconds untile the led turns off.
 - The reset is now complete. 
 
 
 
 
 
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
 
 BleKeyboard 0.3.2beta
 Preferences
 Bounce2