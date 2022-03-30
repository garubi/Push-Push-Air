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

#include <BleKeyboard.h>
//BleKeyboard bleKeyboard;
BleKeyboard bleKeyboard("PushPush AIR", "UBI Stage", BL.getBatteryChargeLevel());

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
// Setting network credentials
const char* ssid = "PushPush AIR b2";
const char* password = "12345678";

// Pin per i Pulsanti
const byte PEDALNEXT_PIN = 21;
const byte PEDALPREV_PIN = 23;

// Led
const byte PEDALNEXT_LED = LED_BUILTIN;
const byte PEDALPREV_LED = LED_BUILTIN;
const byte STATUS_LED = LED_BUILTIN;

// Tasti da emulare
const byte PEDALNEXT_KEY = KEY_RIGHT_ARROW;
const byte PEDALPREV_KEY = KEY_LEFT_ARROW;

const byte PED_NEXT = PEDALNEXT_PIN;
const byte PED_PREV = PEDALPREV_PIN;

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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>PushPush AIR Configuration v1</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>PushPush AIR Configuration v10</h2>
  <form action="/save">
  %SELECT_PLACEHODER%
  <br>
  <button type="submit">Save</button>
  </form>
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  if(var == "SELECT_PLACEHODER"){
    String buttons = "";
    buttons += "<b>Battery level:</b>";
    buttons.concat(BL.getBatteryChargeLevel());
    
    buttons += "<h4>Device Name:</h4><input name=\"devicename\" type=\"text\">";
    
    buttons += "<h4>Password:</h4><input name=\"password\" type=\"text\">";
    buttons += "<br><small>If you forget the password you can reset to the default '12345678' by pressing the two pedals while turning on the Push Push Air</small>";
        
    buttons += "<h4>Pedal 1:</h4><select name=\"pedal1\">" + optionsList() + "</select>";
    // 
    buttons += "<h4>Pedal 2:</h4><select name=\"pedal2\">" + optionsList() + "</select>";

    return buttons;
  }
  return String();
}

String optionsList(){
    String list = "";
    for(int prs = 0; prs < key_options_num; prs++){
        list.concat("<option value=\"");
        list.concat(prs);
        list.concat("\">" + key_options[prs].label + "</option>");
    }
    return list;
}

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
    delay(100);
    bleKeyboard.releaseAll();
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
    pinMode(STATUS_LED, OUTPUT);
    
    Serial.begin(115200);
//    Serial.setDebugOutput(true);
//    Serial.print("ESP32 SDK: ");
//    Serial.println(ESP.getSdkVersion());

    status_led_off_interval = 100 * BL.getBatteryChargeLevel();
    status_led_on_interval = 200;
    status_led_flag = LOW;
    
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html, processor);
    });
    
    server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("devicename")){
            String devicename;
            devicename = request->getParam("devicename")->value();
            Serial.println("devicename");
            Serial.println( devicename );
        }
        if(request->hasParam("password")){
            String password;
            password = request->getParam("password")->value();
            Serial.println("password");
            Serial.println( password );
        }
        if(request->hasParam("pedal1")){
            String pedal1;
            pedal1 = request->getParam("pedal1")->value();
            Serial.println("pedal1");
            Serial.println( pedal1 );
        }
        if(request->hasParam("pedal2")){
            String pedal2;
            pedal2 = request->getParam("pedal2")->value();
            Serial.println("pedal2");
            Serial.println( pedal2 );
        }
    
      request->send(200, "text/plain", "Saved!");
    });
    
    // Start server
    server.begin();
}

void loop(void)
{

    if(status_led_flag == HIGH && millis() > status_led_change_time ){
      status_led_change_time = millis() + status_led_on_interval;
      status_led_flag = LOW;
      digitalWrite(STATUS_LED, status_led_flag );
    }
    if(status_led_flag == LOW && millis() > status_led_change_time ){
      status_led_change_time = millis() + status_led_off_interval;
      status_led_flag = HIGH;
      digitalWrite(STATUS_LED, status_led_flag );
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
