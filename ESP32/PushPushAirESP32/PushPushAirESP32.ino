/*
    Name: Push Push AIR2
    Author: Stefano Garuti
    Link: https://github.com/garubi/Push-Push-Air
    Hardware version: 2.0
*/

#include "customize.h"

/* ****** CUSTOMIZATION INSTRUCTION ****** */
/*
The PIN and LED assignement is done in a separate file so this script can be left untouched 

- uncomment the above #include "customize.h"
- create a customize.h file in the same folder of this script
- Put the following code in the customize.h file and change it as you need:
*/

/* code to copy and paste: */
/*
#define BATTERY_POWERED false // set to false if your devices doesn't have a battery otherwise omit it or set to true

#define PEDAL1_PIN 23
#define PEDAL2_PIN 21

#define PEDAL1_LED_PIN 2 // Blink when pedal/button 1 is pressed
#define PEDAL2_LED_PIN 2 // Blink when pedal/button 2 is pressed
#define STATUS_LED_PIN 2 // Blink to signal device's statuses

#define LED_ON HIGH
#define LED_OFF LOW
*/

/* ****** *************************** ****** */

const char SoftwareVersion[] = "3.0.0";

/* Pedals/Buttons connections */
#ifndef PEDAL1_PIN
    #define PEDAL1_PIN 23
#endif

#ifndef PEDAL2_PIN
    #define PEDAL2_PIN 21
#endif

/* Leds connections */
#ifndef PEDAL1_LED_PIN
    #define PEDAL1_LED_PIN LED_BUILTIN
#endif

#ifndef PEDAL2_LED_PIN
    #define PEDAL1_LED_PIN LED_BUILTIN
#endif

#ifndef STATUS_LED_PIN
    #define PEDAL1_LED_PIN LED_BUILTIN
#endif

#ifndef LED_ON
    #define LED_ON LED_ON
#endif

#ifndef LED_OFF
    #define LED_OFF HIGH
#endif


/* Is battery operated? */
#ifndef BATTERY_POWERED
    #define BATTERY_POWERED true
#endif

#include <Bounce2.h>
Bounce ped_2 = Bounce(); 
Bounce ped_1 = Bounce(); 

// Manage the status led (it will blink faster as the battery level will go down)
int status_led_off_interval;
int status_led_on_interval;
unsigned long status_led_change_time;
byte status_led_flag;

/* Device's configurable preferences */
#include <Preferences.h>
Preferences preferences;
// Default preferences :
const byte PEDAL1_DEFAULT_KEY_INDEX = 2; // it's the KEY_LEFT_ARROW in the key_options[] sturct
const byte PEDAL2_DEFAULT_KEY_INDEX = 3; // it's the KEY_RIGHT_ARROW in the key_options[] sturct
const String PASSWORD_DEFAULT = "12345678";
const String SSID_DEFAULT = "PushPushAIR2";

/* Battery charge checking */
#if BATTERY_POWERED
    #include <Pangodream_18650_CL.h>
    Pangodream_18650_CL BL;
    unsigned long batCheckTime;
    const int BAT_POLLING_INTERVAL = 5000; // Chek the battery status every BAT_POLLING_INTERVAL milliseconds
    // Calculate the the led's blinking frequency based on the battery charge levele
    int batteryChargeLedOffInterval(){
            if( BL.getBatteryChargeLevel() < 1 ){
                return 100;
            }
            return 100 * BL.getBatteryChargeLevel();
    }
#endif

/* The device's will appear as a Bluethooth keyboard */
#include <BleKeyboard.h>
String init_bleName = SSID_DEFAULT;
const char *winit_bleName = init_bleName.c_str();
#if BATTERY_POWERED
    BleKeyboard bleKeyboard(winit_bleName, "UBI Stage", BL.getBatteryChargeLevel());
#else
    BleKeyboard bleKeyboard(winit_bleName, "UBI Stage" );
#endif

/* The keys we can send.. */
struct Key_options {
    String label; //char can also be a fixed length string like char fruit[16];
    uint8_t value;
} key_options[] = {
    {"KEY_UP_ARROW" , KEY_UP_ARROW},
    {"KEY_DOWN_ARROW" , KEY_DOWN_ARROW},
    {"KEY_LEFT_ARROW" , KEY_LEFT_ARROW},
    {"KEY_RIGHT_ARROW" , KEY_RIGHT_ARROW},
    {"KEY_BACKSPACE" , KEY_BACKSPACE},
    {"KEY_RETURN" , KEY_RETURN},
    {"KEY_ESC" , KEY_ESC},
    {"KEY_DELETE" , KEY_DELETE},
    {"KEY_PAGE_UP" , KEY_PAGE_UP},
    {"KEY_PAGE_DOWN" , KEY_PAGE_DOWN},
    {"KEY_HOME" , KEY_HOME},
    {"KEY_END" , KEY_END},
};
const int key_options_num = sizeof(key_options) / sizeof(key_options[0]);

/* WiFI and webserver settings for preferences configuration */
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
String ssid = "";
String password = "";
byte ap_started = false; // true when in configuration mode and Access Point is ON
const unsigned int CONFIG_MAX_IDLE_TIME = 1000 * 60 * 5; // millisecons of configuration inactivity after that we turn off the access point
unsigned long lastConfigActivityTime;

const char index_html[] PROGMEM = R"rawliteral(
%HEAD_PLACEHODER%
<body>
  <h2>PushPush AIR2 Configuration</h2>
  <form action="/save">
  %SELECT_PLACEHODER%
  <br>
  <button type="submit">Save</button>
  </form>
</body>
</html>
)rawliteral";

const char save_html[] PROGMEM = R"rawliteral(
%HEAD_PLACEHODER%
<body>
  <h2>PushPush AIR Configuration v10</h2>
  <h3>Configuration saved</h3>
  <form action="/">
    <button type="submit">Edit Again</button>
  </form>
  <form action="/end">
    <button type="submit">Finish</button>
  </form>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  if(var == "HEAD_PLACEHODER"){
    String head = "";
    head += "<!DOCTYPE HTML><html>";
    head += "<head>";
      head += "<title>PushPush AIR Configuration v1</title>";
      head += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
      head += "<link rel=\"icon\" href=\"data:,\">";
      head += "<style>";
        head += "html {font-family: Arial; display: inline-block; text-align: center;}";
        head += "h3 {color:red}";
        head += "p {font-size: 3.0rem;}";
        head += "body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}";
      head += "</style>";
    head += "</head>"  ;
    return head;
  }
  if(var == "SELECT_PLACEHODER"){
    String buttons = "";
    #if BATTERY_POWERED
        buttons += "<b>Battery level:</b>";
        buttons.concat(BL.getBatteryChargeLevel());
    #else
        buttons += "<em>This device has no battery</em>";
    #endif
    buttons.concat("<h4>Device Name:</h4><input name=\"devicename\" value=\"");
    buttons.concat(preferences.getString("ssid", SSID_DEFAULT));
    buttons.concat("\" type=\"text\" maxlength=\"15\">");
    buttons += "<br><small>You have to restart PushPush AIR in order to the new name becomes effective</small>";
    
    buttons.concat("<h4>Password:</h4><input name=\"password\" value=\""); 
    buttons.concat(preferences.getString("password", PASSWORD_DEFAULT));
    buttons.concat("\" type=\"text\">");
    buttons += "<br><small>If you forget the password you can reset to the default "+ PASSWORD_DEFAULT +" by pressing the two pedals while turning on the Push Push Air</small>";
        
    buttons += "<h4>Pedal 1:</h4><select name=\"pedal1\">" + optionsList(1) + "</select>";
    // 
    buttons += "<h4>Pedal 2:</h4><select name=\"pedal2\">" + optionsList(2) + "</select>";

    return buttons;
  }
  return String();
}

String optionsList(byte pedal){
    int selected;
    if( pedal == 1 ){
        selected = preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX);
    }
    else{
        selected = preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX);
    }
    String list = "";
    for(int prs = 0; prs < key_options_num; prs++){
        list.concat("<option value=\"");
        list.concat(prs);
        list.concat("\" ");
        if( selected == prs ){
            list.concat("selected");
        }
        list.concat(">");
        list.concat(key_options[prs].label);
        list.concat("</option>");
    }
    return list;
}

void shutDownServer(){
    WiFi.softAPdisconnect(true);
    server.end();

    Serial.println("Access Point closed");
    // turn the led off when the Access Point is disconnected
    ap_started = false;
    digitalWrite(STATUS_LED_PIN, LED_OFF );
}

/* send the keypress via BLE */
static void SendKey( byte pedal ){
  if (bleKeyboard.isConnected()) {
    switch( pedal ){
      case PEDAL1_PIN:
          bleKeyboard.press(key_options[preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX)].value);
          Serial.println(key_options[preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX)].label);
      break; 
      case PEDAL2_PIN:
          bleKeyboard.press(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].value);
          Serial.println(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].label);
      break; 
    }
    bleKeyboard.releaseAll();
  }
}

void setup(void)
{
    preferences.begin("pushpush-config", false);
    
    String bleName = preferences.getString("ssid", SSID_DEFAULT);
    const char *wbleName = bleName.c_str();
    bleKeyboard.setName(wbleName);
    bleKeyboard.begin();
    
    // BUTTONS / INPUTS
    ped_2.attach(PEDAL2_PIN, INPUT_PULLUP);
    ped_1.attach(PEDAL1_PIN, INPUT_PULLUP);
    ped_2.interval(50);
    ped_1.interval(50);
    // OUTPUTS /LEDS
    pinMode(PEDAL1_LED_PIN, OUTPUT);
    pinMode(PEDAL2_LED_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    
    Serial.begin(115200); // for feedback and debug
//    Serial.setDebugOutput(true);
//    Serial.print("ESP32 SDK: ");
//    Serial.println(ESP.getSdkVersion());

    /* At startup, the led will be on for 1 second */
    digitalWrite(STATUS_LED_PIN, LED_ON);
    delay(1000);
    digitalWrite(STATUS_LED_PIN, LED_OFF);
    
    Serial.println("Welcome to PushPushAir2");
    Serial.print("Firmware version: ");
    Serial.println(SoftwareVersion);
    Serial.print("Ped1 sends: ");
    Serial.println(key_options[preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX)].label);
    Serial.print("Ped2 sends: ");
    Serial.println(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].label);
    Serial.print("Password: ");
    Serial.println(preferences.getString("password", PASSWORD_DEFAULT));
    
    #if BATTERY_POWERED
        status_led_off_interval = batteryChargeLedOffInterval();
    #else
        status_led_off_interval = 100*100;
    #endif

    status_led_on_interval = 200;
    status_led_flag = LED_ON;
    
    // start the access point to do the configuration when the device is started while pushing pedal 2
    ped_1.update();
    ped_2.update(); 

    
    Serial.print("ped 1: ");
    Serial.println(ped_1.read());   
    Serial.print("ped 2: ");
    Serial.println(ped_2.read());

    
    
    if (ped_2.read() == 0 && ped_1.read() == 0){
        // To reset to factory password press the pedal 1 and pedal 2 while turning on the device
        // the status led blinks fast
        // keep the buttons pressed for 5 seconds until the led stays on
        // now the password is reset to the factory one.
        long reset_btn_on_time = millis();
        long reset_wait = millis();
        byte reset_btn_led = LED_OFF;
        byte password_reset_done = false;
        
        Serial.println("Waiting for password factory reset. Keep the buttons pressed for 5 seconds");
        while (ped_2.read() == 0 && ped_1.read() == 0 && ! password_reset_done ) {
            if( millis() - reset_wait < 5000 ){
                if( millis()-reset_btn_on_time > 50){
                    reset_btn_led = !reset_btn_led;
                    digitalWrite(STATUS_LED_PIN, reset_btn_led);
                    reset_btn_on_time = millis();
                    Serial.print(".");
                }
            }
            else{
                digitalWrite(STATUS_LED_PIN, LED_ON); //keep the led on to signal that the reset procedure is starting
                Serial.println("Reset passowrd to");
                Serial.println(PASSWORD_DEFAULT);
                String rpassword = PASSWORD_DEFAULT;
                const char *wrpassword = rpassword.c_str();
                preferences.putString("password", wrpassword);
                delay(2000);
                Serial.println("Password reset done");
                password_reset_done = true; 
                digitalWrite(STATUS_LED_PIN, LED_OFF);
            }
            ped_1.update();
            ped_2.update();
        }
    }
    
    if (ped_2.read() == 0 && ped_1.read() != 0 ){
        Serial.println("Starting WiFI accessPoint...");
        
          digitalWrite(STATUS_LED_PIN, LED_ON); 
          
          Serial.println("Setting AP (Access Point)…");
          ssid = preferences.getString("ssid", SSID_DEFAULT); 
          password = preferences.getString("password", PASSWORD_DEFAULT);
          
          const char *wpassword = password.c_str();
          const char *wssid = ssid.c_str();
          
          WiFi.softAP(wssid, wpassword);
          
          IPAddress IP = WiFi.softAPIP();
          Serial.print("AP IP address: ");
          Serial.println(IP);
          Serial.print("AP SSID: ");
          Serial.println(wssid);
          Serial.print("AP password: ");
          Serial.println(wpassword);
          
          // Start server
          server.begin();
          ap_started = true;
          lastConfigActivityTime = millis();
          Serial.println("Access Point Started");
          
          /* Web configuration resposnse pages and action */
          server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send_P(200, "text/html", index_html, processor);
              lastConfigActivityTime = millis();
          });
          
          server.on("/end", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send(200, "text/plain", "Connection closed.");
              shutDownServer();
          });
          
          server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request){
              if(request->hasParam("devicename")){
                  String devicename;
                  devicename = request->getParam("devicename")->value();
                  Serial.println("devicename");
                  Serial.println( devicename );
                  preferences.putString("ssid", devicename);
              }
              if(request->hasParam("password")){
                  String getpassword;
                  getpassword = request->getParam("password")->value();
                  Serial.println("password");
                  Serial.println( getpassword );
                  preferences.putString("password", getpassword);
              }
              if(request->hasParam("pedal1")){
                  String pedal1;
                  pedal1 = request->getParam("pedal1")->value();
                  Serial.println("pedal1");
                  Serial.println( pedal1 );
                  preferences.putInt("pedal1", pedal1.toInt());
              }
              if(request->hasParam("pedal2")){
                  String pedal2;
                  pedal2 = request->getParam("pedal2")->value();
                  Serial.println("pedal2");
                  Serial.println( pedal2 );
                  preferences.putInt("pedal2", pedal2.toInt());
          
              }
              lastConfigActivityTime = millis();
              request->send_P(200, "text/html", save_html, processor);
          });
    }
    
    
}

void loop(void)
{
    if( ap_started != true ){ // we are not in config mode.
        if(status_led_flag == LED_OFF && millis() > status_led_change_time ){
          status_led_change_time = millis() + status_led_on_interval;
          status_led_flag = LED_ON;
          digitalWrite(STATUS_LED_PIN, status_led_flag );
        }
        if(status_led_flag == LED_ON && millis() > status_led_change_time ){
          status_led_change_time = millis() + status_led_off_interval;
          status_led_flag = LED_OFF;
          digitalWrite(STATUS_LED_PIN, status_led_flag );
        }
    }
    else{
        digitalWrite(STATUS_LED_PIN, LED_ON );
        if(millis() - lastConfigActivityTime > CONFIG_MAX_IDLE_TIME ){
            // we are on the same WebConfig page for more than CONFIG_MAX_IDLE_TIME
            // so we shut down the server
            Serial.println("Web Configuration timeout");
            shutDownServer();
        }
    }


    uint8_t pedalState;
    ped_2.update();
    ped_1.update();
    if ( ped_2.changed() ) { 
        pedalState = ped_2.read();
        if (pedalState == LED_ON ) {
          Serial.print("Pushed pedal 2");
          SendKey( PEDAL2_PIN );
        }
        digitalWrite(PEDAL2_LED_PIN, pedalState );
    }

    if ( ped_1.changed() ) { 
        pedalState = ped_1.read();
        if (pedalState == LED_ON ) {
            Serial.print("Pushed pedal 1");
            SendKey( PEDAL1_PIN );
        }
        digitalWrite(PEDAL1_LED_PIN, pedalState );
    }

    #if BATTERY_POWERED
        /* every BAT_POLLING_INTERVAL we check the battery charge */
        if(  millis() > batCheckTime ){
            batCheckTime = millis() + BAT_POLLING_INTERVAL;
            Serial.print("Volts: ");
            Serial.println(BL.getBatteryVolts());
            Serial.print("Charge level: ");
            Serial.println(BL.getBatteryChargeLevel());
        
            status_led_off_interval = batteryChargeLedOffInterval();
            bleKeyboard.setBatteryLevel(BL.getBatteryChargeLevel());
        }
    #endif

}
