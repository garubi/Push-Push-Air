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
  <title>ESP32 WEB SERVER</title>
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
  <h2>ESP32 WEB SERVER</h2>
  %SELECT_PLACEHODER%
  %BUTTONPLACEHOLDER%
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
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Output - GPIO 32</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"32\" " + outputState(32) + "><span class=\"slider\"></span></label>";

    buttons += "<h4>Output - GPIO 25</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"25\" " + outputState(25) + "><span class=\"slider\"></span></label>";

    buttons += "<h4>Output - GPIO 27</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"27\" " + outputState(27) + "><span class=\"slider\"></span></label>";

    buttons += "<h4>Output - GPIO 13</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"13\" " + outputState(13) + "><span class=\"slider\"></span></label>";

    return buttons;
  }
  if(var == "SELECT_PLACEHODER"){
    String buttons = "";
    buttons += "<b>Battery level:</b>";
    
    buttons += "<h4>Pedal 1</h4><select><option>Select...</option></select>";

    buttons += "<h4>Pedal 2</h4><select><option>Select...</option></select>";

    return buttons;
  }
  return String();
}

String outputState(int output){
    return "";
  // if(digitalRead(output)){
  //   return "checked";
  // }
  // else {
  //   return "";
  // }
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
