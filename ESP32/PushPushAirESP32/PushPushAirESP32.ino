#include <IotWebConf.h>
#include <IotWebConfUsing.h> // This loads aliases for easier class names.

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "PushPush AIR";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "12345678";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "beta1"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 22

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Method declarations.
void handleRoot();
// -- Callback methods.
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);

DNSServer dnsServer;
WebServer server(80);

char stringParamValue[STRING_LEN];
char intParamValue[NUMBER_LEN];
char floatParamValue[NUMBER_LEN];
char checkboxParamValue[STRING_LEN];
char chooserParamValue[STRING_LEN];

static char chooserValues[][STRING_LEN] = { "red", "blue", "darkYellow" };
static char chooserNames[][STRING_LEN] = { "Red", "Blue", "Dark yellow" };

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
// -- You can also use namespace formats e.g.: iotwebconf::TextParameter
IotWebConfTextParameter stringParam = IotWebConfTextParameter("String param", "stringParam", stringParamValue, STRING_LEN);
IotWebConfParameterGroup group1 = IotWebConfParameterGroup("group1", "");
IotWebConfNumberParameter intParam = IotWebConfNumberParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "20", "1..100", "min='1' max='100' step='1'");
// -- We can add a legend to the separator
IotWebConfParameterGroup group2 = IotWebConfParameterGroup("c_factor", "Calibration factor");
IotWebConfNumberParameter floatParam = IotWebConfNumberParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN,  nullptr, "e.g. 23.4", "step='0.1'");
IotWebConfCheckboxParameter checkboxParam = IotWebConfCheckboxParameter("Check param", "checkParam", checkboxParamValue, STRING_LEN,  true);
IotWebConfSelectParameter chooserParam = IotWebConfSelectParameter("Choose param", "chooseParam", chooserParamValue, STRING_LEN, (char*)chooserValues, (char*)chooserNames, sizeof(chooserValues) / STRING_LEN, STRING_LEN);

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

    Serial.println("Starting up...");
  group1.addItem(&intParam);
  group2.addItem(&floatParam);
  group2.addItem(&checkboxParam);
  group2.addItem(&chooserParam);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addSystemParameter(&stringParam);
  iotWebConf.addParameterGroup(&group1);
  iotWebConf.addParameterGroup(&group2);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.getApTimeoutParameter()->visible = true;
    // -- Initializing the configuration.
    iotWebConf.init();
  
    // -- Set up required URL handlers on the web server.
    server.on("/", handleRoot);
    server.on("/config", []{ iotWebConf.handleConfig(); });
    server.onNotFound([](){ iotWebConf.handleNotFound(); });
  
    Serial.println("Ready.");
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
//    Serial.print("Value from pin: ");
//    Serial.println(analogRead(34));
//    Serial.print("Average value from pin: ");
//    Serial.println(BL.pinRead());
//    Serial.print("Volts: ");
//    Serial.println(BL.getBatteryVolts());
//    Serial.print("Charge level: ");
//    Serial.println(BL.getBatteryChargeLevel());
//    Serial.println("");
    status_led_off_interval = 100 * BL.getBatteryChargeLevel();
    bleKeyboard.setBatteryLevel(BL.getBatteryChargeLevel());
  }
  
 // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>IotWebConf 03 Custom Parameters</title></head><body>Hello world!";
  s += "<ul>";
  s += "<li>String param value: ";
  s += stringParamValue;
  s += "<li>Int param value: ";
  s += atoi(intParamValue);
  s += "<li>Float param value: ";
  s += atof(floatParamValue);
  s += "<li>CheckBox selected: ";
//  s += checkboxParam.isChecked();
  s += "<li>Option selected: ";
  s += chooserParamValue;
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{
  Serial.println("Validating form.");
  bool valid = true;

/*
  int l = webRequestWrapper->arg(stringParam.getId()).length();
  if (l < 3)
  {
    stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }
*/
  return valid;
}
