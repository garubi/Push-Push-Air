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

### Configuration ###
- Turn on the PPA while pushing the right-most pedal. Keep the pedal pressed for at least 2 seconds.
- The status led will be steady blue.
- Now connect from a computer or mobile device to the wifi network "Push Push AIR" and insert the password
- Once the connection is etablished, browse to 192.168.4.1 to use the configuration page.
- On the configuration page you can set:
 - The device name that idetify the Push Push AIR on WiFI and Bluethooth (you need to restart the device in order to render effective the name's change)
 - The passowrd to access the configuration page
 - The characters sent by the pedals
- Click on Save to store your new configuration
- Please remember to click on "Finish" when you are done with the setup: in this way the WiFI access point will be closed (the led turns off). This is important for two reasons:
 - It will save on battery charge
 - No one (even if he has your password) will be able to change the settings during your performance ;-)
 
Please note that after 5 minutes from the last click on the buttons in the configuration page, the Push Push AIR WiFi access point will turn off automatically
 
### Reset the password to the factory one ###
 If you change the password of the Configuration page and you forget it, you can revert to the factory one: `12345678`
 **How to reset:**
 - Turn on the PPA while pushing both pedals. The Status led starts blink fast
 - Keep pressed both the pedals for 5 seconds, untile the Status led turn steady on
 - Wait 2/3 seconds untile the led turns off.
 - The reset is now complete. 
 
## Making it ##
I used a board that have the 18650 battery and charger included like this:
https://wiki.geekworm.com/index.php/WEMOS_ESP32_Board_with_18650_Battery_Holder
see References folder for pictures and pinout
 
**Libraries used:**
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
 
 **Compiling**
 in Arduino IDE select the 
 _WeMos WiFI&Bluethooth Battery_
 
 as Partition Scheme select:
 _Huge APP (3MB No OTA/1MB SPIFF)
 