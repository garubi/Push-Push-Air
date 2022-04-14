// Default preferences
const byte PEDAL1_DEFAULT_KEY_INDEX = 2; // it's the KEY_LEFT_ARROW in the key_options[] sturct
const byte PEDAL2_DEFAULT_KEY_INDEX = 3; // it's the KEY_RIGHT_ARROW in the key_options[] sturct
const String PASSWORD_DEFAULT = "12345678";
const String SSID_DEFAULT = "PushPushAIR2";

#include <Preferences.h>
Preferences preferences;

#include <Pangodream_18650_CL.h>
Pangodream_18650_CL BL;

#include <BleKeyboard.h>
String init_bleName = SSID_DEFAULT;
const char *winit_bleName = init_bleName.c_str();
BleKeyboard bleKeyboard(winit_bleName, "UBI Stage", BL.getBatteryChargeLevel());

#include <Bounce2.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

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

String ssid = "";
String password = "";

// Pin per i Pulsanti
const byte PEDAL1_PIN = 21;
const byte PEDAL2_PIN = 23;

// Led
const byte PEDAL1_LED_PIN = LED_BUILTIN;
const byte PEDAL2_LED_PIN = LED_BUILTIN;
const byte STATUS_LED_PIN = LED_BUILTIN;

// Instantiate a Bounce object
Bounce ped_next = Bounce(); 
// Instantiate another Bounce object
Bounce ped_prev = Bounce(); 

// Chek the battery status every BAT_POLLING_INTERVAL milliseconds
unsigned long batCheckTime;
const int BAT_POLLING_INTERVAL = 5000;

unsigned long btnReadingSettle;

// Manage the status led (it will blink faster as the battery level will go down)
int status_led_off_interval;
int status_led_on_interval;
unsigned long status_led_change_time;
byte status_led_flag;

byte ap_started = false; // true when in configuration mode and Access Point is ON

const char index_html[] PROGMEM = R"rawliteral(
%HEAD_PLACEHODER%
<body>
  <h2>PushPush AIR Configuration v10</h2>
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
    buttons += "<b>Battery level:</b>";
    buttons.concat(BL.getBatteryChargeLevel());
    
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

static void SendKey( byte pedal ){
  if (bleKeyboard.isConnected()) {
    switch( pedal ){
      case PEDAL1_PIN:
          bleKeyboard.press(key_options[preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX)].value);
      break; 
      case PEDAL2_PIN:
          bleKeyboard.press(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].value);
      break; 
    }
    
    Serial.println(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].label);
    delay(100);
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
    pinMode(PEDAL1_PIN, INPUT_PULLUP);
    pinMode(PEDAL2_PIN, INPUT_PULLUP);
    
    ped_next.attach(PEDAL2_PIN);
    ped_prev.attach(PEDAL1_PIN);

    // OUTPUTS /LEDS
    pinMode(PEDAL1_LED_PIN, OUTPUT);
    pinMode(PEDAL2_LED_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    
    Serial.begin(115200);
//    Serial.setDebugOutput(true);
//    Serial.print("ESP32 SDK: ");
//    Serial.println(ESP.getSdkVersion());
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(1000);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    Serial.print("Ped1: ");
    Serial.println(key_options[preferences.getInt("pedal1", PEDAL1_DEFAULT_KEY_INDEX)].label);
    Serial.print("Ped2: ");
    Serial.println(key_options[preferences.getInt("pedal2", PEDAL2_DEFAULT_KEY_INDEX)].label);
    
    status_led_on_interval = 200;
    status_led_flag = LOW;
    
    
    ped_next.update();    
    
    // start the access point to do the configuration when the device is started while pushing pedal 2
    if (ped_next.read() == 0){
        Serial.println("Starting WiFI accessPoint...");
        
          digitalWrite(STATUS_LED_PIN, LOW); 
          
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
          Serial.println("Access Point Started");
    }
    
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    
    server.on("/end", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Connection closed.");
      WiFi.softAPdisconnect(true);
      server.end();
        
      Serial.println("Access Point closed");
      // turn the led off when the Access Point is disconnected
      ap_started = false;
      digitalWrite(STATUS_LED_PIN, HIGH );
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
      request->send_P(200, "text/html", save_html, processor);
    });
    
    
    // To reset to factory presets keep pressed the pedal 1 while turning on the device
    // then press the pedal 2
    ped_prev.update();
    long reset_btn_on_time = millis();
    byte reset_btn_led = 1;
    byte im_resetting = false;

    while (ped_prev.read() == 0){
        Serial.println("Waiting for Pedal 2 to start password reset ");
      digitalWrite(STATUS_LED_PIN, reset_btn_led); 
      if( millis()-reset_btn_on_time > 200 && !im_resetting ){
          digitalWrite(STATUS_LED_PIN, !reset_btn_led);
          reset_btn_led = !reset_btn_led;
        reset_btn_on_time = millis();
      }
       ped_prev.update();
       ped_next.update();

      if (ped_next.fell()){ // the pedal 2 button was pressed: let's start the reset procedure
        Serial.println("Starting password reset ");
         digitalWrite(STATUS_LED_PIN, HIGH); //keep the led on to signal that the reset procedure is starting
         im_resetting = true; //don't blink animore
         
         Serial.println("Reset passowrd to");
         Serial.println(PASSWORD_DEFAULT);
         
         String rpassword = PASSWORD_DEFAULT;
         const char *wrpassword = rpassword.c_str();
         preferences.putString("password", wrpassword);

         delay(3000);

         Serial.println("Password reset done");
         // turn the led off when the write procedure finish
         digitalWrite(STATUS_LED_PIN, LOW);
      }
    }
    
}

void loop(void)
{
    if( ap_started != true ){ // we are not in config mode.
        if(status_led_flag == HIGH && millis() > status_led_change_time ){
          status_led_change_time = millis() + status_led_on_interval;
          status_led_flag = LOW;
          digitalWrite(STATUS_LED_PIN, status_led_flag );
        }
        if(status_led_flag == LOW && millis() > status_led_change_time ){
          status_led_change_time = millis() + status_led_off_interval;
          status_led_flag = HIGH;
          digitalWrite(STATUS_LED_PIN, status_led_flag );
        }
    }
    else{
        digitalWrite(STATUS_LED_PIN, LOW );
    }

    if(  millis() > btnReadingSettle ){
      btnReadingSettle = millis() + 50;
      static uint8_t pedalNEXTStateLast = HIGH;
      static uint8_t pedalPREVStateLast = HIGH;
      uint8_t pedalState;
      ped_next.update();
      ped_prev.update();
    
      pedalState = ped_next.read();
      if (pedalState != pedalNEXTStateLast) {
          pedalNEXTStateLast = pedalState;
          
          if (pedalState == LOW ) {
              Serial.print("Pushed pedal 1");
              SendKey( PEDAL1_PIN );
          }
          digitalWrite(PEDAL1_LED_PIN, pedalState );
      }
  
      pedalState = ped_prev.read();
      if (pedalState != pedalPREVStateLast) {
          pedalPREVStateLast = pedalState;
  
          if (pedalState == LOW ) {
              Serial.print("Pushed pedal 2");
              SendKey( PEDAL2_PIN );
          }
          digitalWrite(PEDAL2_LED_PIN, pedalState );
      }
    }

  if(  millis() > batCheckTime ){
    batCheckTime = millis() + BAT_POLLING_INTERVAL;
    // Serial.print("Value from pin: ");
    // Serial.println(analogRead(34));
    // Serial.print("Average value from pin: ");
    // Serial.println(BL.pinRead());
    // Serial.print("Volts: ");
    // Serial.println(BL.getBatteryVolts());
    // Serial.print("Charge level: ");
    // Serial.println(BL.getBatteryChargeLevel());
    // Serial.println("");
    status_led_off_interval = 100 * BL.getBatteryChargeLevel();
    bleKeyboard.setBatteryLevel(BL.getBatteryChargeLevel());
  }

}
