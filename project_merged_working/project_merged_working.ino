#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <EEPROM.h>

#define EEPROM_SIZE 1

#include "fauxmoESP.h"
#define LAMP_1 "lamp two"
fauxmoESP fauxmo;

String buffer;
char buffer1[18432]; // Image size 16kb

#define FIRMWARE "PSF-BDI-GL"
#define FIRMWARE_VERSION "3.3V"
#define PRODUCT "Smart Plug"
#define PRODUCT_VERSION "V1"
#define MODEL "SP/PM-v1"
#define BRAND "Perisync"

WiFiClientSecure espClient;
PubSubClient client(espClient);

String token_no;
String credentials;

//const String endpoint = "http://secret-basin-08997.herokuapp.com/";
const String endpoint = "http://192.168.0.101:12345/";

const String key = "check/mac";

/* Pin definitions */
const int RELAY_PIN_Alexa = 27;
int Button_pin = 0;
const int LED_PIN    = 2;
const int tempPin = 26;

bool Relay_State_alexa;
bool save_state;

void checkPhysicalButton();

int Button_state = HIGH;
bool isFirstConnect = true;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
byte tapCounter;
int timediff;
bool flag1, flag2;
long double presstime, releasetime;
unsigned long int received_new;

const char* ssid = "Pass=123456789";
const char* password = "123456789";
const char* ssid_wifi;
const char* password_wifi;

const char *mqttServer_v1;
const char *mqttUser_v1;
const char *mqttPassword_v1;
int mqttPort;
const char* sub_topic1_c_g;
const char* sub_topic2_c_g;

const char* ca_certificate_v1;
const char* client_certificate_v1;
const char* client_key_v1;

WebServer server(80);
Preferences preferences;

int period = 5000;
unsigned long time_now = 0;

const char* reactimg = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAKAAAACgCAYAAACLz2ctAAAgAElEQVR4Xu2dB7gcVfnGvxMR7CJNwQKIgCUoSG+KgCVgBUT9oxRp0pSmgjSRLh2VFjABNHRUCEVBpKNoABELSEdFJCD2nvk/v3P33Xz3ZLbM7uzu7N7M8yS7d2fmzJk577xfPd8JNn+rP4EsyxY0sxXMbDkzW8bMXm1mS5rZy81sUTNbuc3HdbeZPW1mT5rZE2b2uJk9YmYPmtn9IYR/t9nOyB8WRv4OG9xglmWLmNkaZraqmb219g/Q9WMDjHeZ2Z1mNsvM7gghANgJt00YAGZZBoNtaGYbmNmmZrZ0xUb7MTObaWY3mtn3JwogRxqAWZZNNrP3mtkmZrZ+xQDXqjs3m9nVZnZFCOHeVgcP6/6RA2CWZehwW5rZ5gV0tqqPHzrlpWZ2UQjh/qp3tkj/RgaAWZZtY2Y7mNl6RR7AEB57i5mdFUI4Zwj7Pk+XhxqAWZa91sx2MrPtzWyxURiQAvcw28zONrOpIQSs66HchhKAWZatbmZ7mNknhvKpl9/p88zsqyGEO8pvurctDhUAsyxb28z2NrMtevtYhrZ19MTjQwi3D8sdDAUAa9bs/mb2f8PyYAfczxlmdtQwWM+VBmCWZS81s0PMbK8BD+iwXv4kMzs0hPBsVW+gsgDMsuxTZnZaVR/ckPVrlxDC6VXsc+UAmGUZYTHe3GFzHFdxfH2fcGzvGUIg/FeZrVIAzLLsADM7vDJPZzQ7cmAI4Yiq3FolAFgzMk6dz3p9gwVsuGsVjJSBAzDLsh3N7My+Pfr5F/JPYKcQwtRBPpKBAjDLMhTjnQf5AOZf284IIWDwDWQbCABrCQPTzGydgdz1/IumT+A2M9tuEIkOfQdglmVTzOyq+RjIfwJZlsUdIQSbM2eOTZo0yfSbPv1vHKeN/fzt2yj4nDcJIZAC1retrwDMsozEgTP6dndDeCHA87///S8Cj3///e9/7TnPeU79Tjzg+FHH6vsCCyzQDQBppq96Yd8AmGXZgWZ22BBioq9d/s9//mMCEQAEYDAhv/M3+8RyAinH8Bt/cxzbc5/73G76fVAIoS/usL4AMMuy48xsn26eyEQ5FwaUKAV4bLfffrv96Ec/sgUXXNAWWWQRe8UrXmFLLrmkLbXUUvaSl7xknKgGhNpStiz4DI8LIXy24DmFD+85ALMsw7+3S+GeTdATBD5u/9//Hps8d8cdd8R///rXv+zPf/6z/eEPf7C//OUv9qpXvcpWW20122ijjWyxxcbSIUsEIM2dGkLYrZdD0VMAZlmGvofeN39r8wlIBCNWpfsBRBkl7Ofvp556yq677jq76qqroljeZZddbOONN47syd8pGNu8fN5hPXXT9AyA85mv8yGXFSsrmE8AKVBJ3+MTJjz33HPt/PPPt/e85z0RiAsvvHBdJ+y8F+PO7BkT9gSAVdH5mrkjtC/Vl7wI7HbwfFu+L62+67qyiH/zm9/Yb3/726gDvuxlL7OXv/zl9sIXvjAeBggB5913321f/epX42+f+9zn7A1veEPdWJEuKBblswP9kETXfbt9Jun5pQNwGKzdFJh5QEmByd8paFsNBsdLJ8t7GWRkCAx5/QJc3/rWt+zqq6+2f/7zn1EsA8JVV13V3vWud0VjhHZo43e/+52dffbZ9uCDD9p+++1nK664Yjxe+zkGEd6FhVy6dVwqAIfVz5fHSK3A1e7+1FEs0HnfXgo8HQN4+f63v/0tilrOwQDBIp45c6b96U9/st13390+8IEP1NmQ444//nh78skn7ZhjjrEXv/jF9ZegkQO73XupHbdzCKG02H1pABymCIdcHamiLqYoOCBND+da0uUEAF1f7Ci/XhrF8JEN9nmnNNbw5Zdfbl/72tds0003jbofzAhjIq4POOAAW3vttW3XXXeNzC0m9MDv4j5Li5iUAsBabPe+Lm6oL6emIjRPpBYVs606LhB5cAn4AqeMCg9MiW8+YUBZx+h+elHY9/Of/9xOPPHE6JLZe++9owHC7zfeeKMdfPDBdtJJJ0VxLRVCL0EHOmB6qyuWETsuC4C3VjmxQKDy4BIgYJLf//73UWQhvtCzytx0TQYe3QvHMUyF7obvrhn7wWacf+WVV9o111wTz11hhRXs7W9/uy233HJ1nfT++++P4vYtb3mLfeYzn4ndB7Bf/OIX43195StfiUaLB3UJALwthLBut8+qawBWIaVKg+x1J280eOuP3xFRt912m11//fX20EMPRdABEAbp+c9/fl2f8g83NVRkSXpQe7ajL4g7sRp/40j++9//HsHxvOc9L0Yy1llnnehIXnrppevGgfcBcj59vPfee+3xxx+3W265JeqB6H7ve9/76tf48Y9/HMXtCSecYG97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";

#include "HLW8032.h"
HLW8032 HL;
#define RXD2 25 // dummy pin 

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

int tempVal;
float volts;
float temp;
float itemp;
int measurement = 0;
float Celcius ;
float Rawvalue ;
float RVoltage ;

void get_credentials() {

  HTTPClient http;

 // http.begin("http://secret-basin-08997.herokuapp.com/register/product");
  http.begin("http://192.168.0.101:12345/register/product");


  DynamicJsonDocument credentialsDocument(6000);

  Serial.println(credentials);

  DeserializationError err = deserializeJson(credentialsDocument, credentials);
  if (err) {
    Serial.print("ERROR: ");
    Serial.println(err.c_str());
    return;
  }
  preferences.begin("namespace", false);

  const char *url = credentialsDocument["url"];
  mqttServer_v1 = url;
  preferences.putString("server", mqttServer_v1);

  uint16_t port = credentialsDocument["port"];
  preferences.putInt("mqttport", port);

  const char *username = credentialsDocument["username"];
  mqttUser_v1 = username;
  Serial.println(mqttUser_v1);
  preferences.putString("uname", mqttUser_v1);

  const char *password = credentialsDocument["password"];
  mqttPassword_v1 = password;
  Serial.println(mqttPassword_v1);
  preferences.putString("pword", mqttPassword_v1);

  const char* ca_certificate = credentialsDocument["ca_certificate"];
  Serial.println(ca_certificate);

  const char* client_certificate = credentialsDocument["client_certificate"];
  Serial.println(client_certificate);
  preferences.putString("ccert", client_certificate);

  const char* client_key = credentialsDocument["client_key"];
  Serial.println(client_key);
  preferences.putString("ckey", client_key);

  //espClient.setCACert(ca_certificate); // probabaly heap issue ; 
  espClient.setCertificate(client_certificate);
  espClient.setPrivateKey(client_key);

  client.setServer(url, port);

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");
  String lwt_s = user_id + "/" + myString + "/" + "lwt";
  const char* lwt = lwt_s.c_str();
  String clientid_s = user_id + "-" + myString;
  const char* clientid = clientid_s.c_str();

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT Broker...");

    if (client.connect(clientid, username, password, lwt, 0, 0, "Device is disconnected" ))
    {
      Serial.println("Connected");
      char macValue[100];
      uint64_t chipid = ESP.getEfuseMac();
      uint32_t high = chipid >> 32;
      uint32_t low = chipid;
      sprintf(macValue, "%08x%08x", high, low);
      String myString = macValue;
      preferences.begin("namespace", false);
      String user_id = preferences.getString("uid");
      String sub_topic1 = user_id + "/" + macValue + "/relay";
      const char* sub_topic1_c = sub_topic1.c_str();
      sub_topic1_c_g = sub_topic1_c;
      Serial.println(sub_topic1_c);
      client.subscribe(sub_topic1_c);
      String sub_topic2 = user_id + "/" + macValue + "/reset";
      const char* sub_topic2_c = sub_topic2.c_str();
      sub_topic2_c_g = sub_topic2_c;
      client.subscribe(sub_topic2_c);
    }
    else
    {
      Serial.print("state failure  ");
      Serial.println(client.state());
      break;
    }
  }
  Serial.println("Connected");
}

void post_product_info() {
  HTTPClient http;

//  http.begin("http://secret-basin-08997.herokuapp.com/register/product");
  http.begin("http://192.168.0.101:12345/register/product");


  char buffer2[2048];

  StaticJsonDocument<2000> tokenDocument;
  StaticJsonDocument<256> tokendoc;

  Serial.println(token_no);
  deserializeJson(tokendoc, token_no);
  String token = tokendoc["token"];
  Serial.println(token);

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;

  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");

  tokenDocument["macvalue"] = myString;

  tokenDocument["userid"] = user_id;

  tokenDocument["token"] = token;

  tokenDocument["Product_Name"] = "Home Smart Plug";

  tokenDocument["Model_No"] = "HLI";

  tokenDocument["Device_ID"] = "1000.86F3EC";

  tokenDocument["Firmware"] = FIRMWARE;

  tokenDocument["Firmware_Version"] = FIRMWARE_VERSION;

  serializeJson(tokenDocument, buffer2);
  Serial.println(buffer2);
  int httpCode = http.POST(buffer2);  //Make the request

  if (httpCode > 0) { //Check for the returning code

    credentials = http.getString();
    Serial.println(httpCode);
    Serial.println(credentials);
  }

  else {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources

}

void callback(char* topic, byte *payload, unsigned int length) {
  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");

  Serial.println("-------new message from broker-----");
  Serial.print("channel:");
  Serial.println(topic);
  Serial.print("data:");

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  String sub_topic1 = user_id + "/" + macValue + "/relay";
  const char* sub_topic1_c = sub_topic1.c_str();
  Serial.println(sub_topic1_c);
  String sub_topic2 = user_id + "/" + macValue + "/reset";
  const char* sub_topic2_c = sub_topic2.c_str();
  Serial.println(sub_topic2_c);

  if (strcmp(topic, sub_topic1_c) == 0) {
    int len = Serial.write(payload, length);
    if (len == 2) {
      Relay_State_alexa = !Relay_State_alexa;
      digitalWrite(RELAY_PIN_Alexa, HIGH);
      client.publish(sub_topic1_c, "The device is ON");
      Serial.println("RELAY_PIN_Alexa is ON");
      EEPROM.write(0, Relay_State_alexa);
      EEPROM.commit();
      Button_state = LOW;
    }
    else if (len == 3) {
      Relay_State_alexa = !Relay_State_alexa;
      digitalWrite(RELAY_PIN_Alexa, LOW);
      client.publish(sub_topic1_c, "The device is OFF");
      Serial.println("RELAY_PIN_Alexa is OFF");
      EEPROM.write(0, Relay_State_alexa);
      EEPROM.commit();
      Button_state = LOW;
    }
    else if (len == 5) {
      char Relay_State_alexa_mq[10];
      String Relay_State_alexa_str = String(Relay_State_alexa);
      Relay_State_alexa_str.toCharArray(Relay_State_alexa_mq, Relay_State_alexa_str.length() + 1);
      client.publish(sub_topic1_c, Relay_State_alexa_mq);
    }
  }

  else if (strcmp(topic, sub_topic2_c) == 0) {
    client.publish(sub_topic2_c, "Topic is reset");
    int len = Serial.write(payload, length);
    if (len > 0) {
      esp_restart();
      delay(2000);
    }
  }
}


void alexa() {

  fauxmo.createServer(true);
  fauxmo.setPort(80); // The TCP port must be 80 for gen3 devices (default is 1901)
  fauxmo.enable(true);
  fauxmo.addDevice(LAMP_1); // Add virtual devices
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value)
  {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    if ( (strcmp(device_name, LAMP_1) == 0) )
    {
      // this just sets a variable that the main loop() does something about
      Serial.println("RELAY 1 switched by Alexa");
      //digitalWrite(RELAY_PIN_Alexa, !digitalRead(RELAY_PIN_Alexa));

      if (state)
      {
        digitalWrite(RELAY_PIN_Alexa, HIGH);
        client.publish("esp/test", "The device is on");
        digitalWrite(LED_PIN, HIGH);

      } else
      {
        digitalWrite(RELAY_PIN_Alexa, LOW);
        client.publish("esp/test", "The device is off");
        digitalWrite(LED_PIN, LOW);

      }
    }
  });
}

void on_restart() {
  Serial.println("restarting");
  ESP.restart();
}

void send_wifilist() {
  
  server.sendHeader("Access-Control-Allow-Origin", "*");

  Serial.println("Setup done");

  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    const int capacity = JSON_ARRAY_SIZE(10) + 6 * JSON_OBJECT_SIZE(50); //10
    DynamicJsonDocument scannerdoc(6000); //6000
    JsonObject obj1 = scannerdoc.createNestedObject();
    JsonObject obj2 = scannerdoc.createNestedObject();
    JsonObject obj3 = scannerdoc.createNestedObject();
    JsonObject obj4 = scannerdoc.createNestedObject();
    JsonObject obj5 = scannerdoc.createNestedObject();
    JsonObject obj6 = scannerdoc.createNestedObject();

    Serial.print(n);
    Serial.println(" networks found - reactimg ");

    obj1["ssid"] = WiFi.SSID(0);
    obj1["rssid"] = WiFi.RSSI(0);
    obj1["encryption"] = WiFi.encryptionType(0);
    obj1["productname"] = PRODUCT;
    obj1["random1"] = MODEL;
    obj1["random2"] = BRAND;


    obj2["ssid"] = WiFi.SSID(1);
    obj2["rssid"] = WiFi.RSSI(1);
    obj2["encryption"] = WiFi.encryptionType(1);
    obj2["productname"] = PRODUCT;
    obj2["random1"] = MODEL;
    obj2["random2"] = BRAND;


    obj3["ssid"] = WiFi.SSID(2);
    obj3["rssid"] = WiFi.RSSI(2);
    obj3["encryption"] = WiFi.encryptionType(2);
    obj3["productname"] = PRODUCT;
    obj3["random1"] = MODEL; 
    obj3["random2"] = BRAND;


    obj4["ssid"] = WiFi.SSID(3);
    obj4["rssid"] = WiFi.RSSI(3);
    obj4["encryption"] = WiFi.encryptionType(3);
    obj4["productname"] = PRODUCT;
    obj4["random1"] = MODEL;
    obj4["random2"] = BRAND;

    obj5["ssid"] = WiFi.SSID(4);
    obj5["rssid"] = WiFi.RSSI(4);
    obj5["encryption"] = WiFi.encryptionType(4);
    obj5["productname"] = PRODUCT;
    obj5["random1"] = MODEL;
    obj5["random2"] = BRAND;

    obj6["image"] = reactimg;

    serializeJson(scannerdoc, buffer1);
    Serial.println(buffer1);
  }
  server.send(200, "application/json", buffer1);
}

void handleBody() { 
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST,GET,PUT");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");

  if (server.hasArg("plain") == false) 
  { 
    server.send(200, "application/json", "Body not received");
    return;
  }

  String message;
  message += server.arg("plain");
  preferences.begin("namespace", false);
  Serial.println(message);
  StaticJsonDocument<512> wifidoc;
  deserializeJson(wifidoc, message);
  const char* ssidc = wifidoc["ssid"];
  preferences.putString("username", ssidc);
  const char* passwordc = wifidoc["pwd"];
  preferences.putString("password", passwordc);
  String userid = wifidoc["userid"];

  preferences.putString("uid", userid);

  preferences.end();

  Serial.println(ssidc);
  Serial.println(passwordc);
  Serial.println(userid);

  int i;

  for (i = 0; i <= 10; i++)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssidc, passwordc);
      delay(1000);
      Serial.print(".");
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to the WiFi network");

  }

  HTTPClient http;
 // http.begin("http://secret-basin-08997.herokuapp.com/check/mac"); //Specify the URL
  http.begin("http://192.168.0.101:12345/check/mac");

  http.addHeader("Content-Type", "application/json");
  StaticJsonDocument<256> testDocument;

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  const char *msg = myString.c_str(); //revert ???
  testDocument["macvalue"] = myString;
  
  serializeJson(testDocument, buffer);
  Serial.println(buffer);
  int httpResponseCode = http.POST(buffer);
  if (httpResponseCode > 0) {
    token_no = http.getString(); //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(token_no);           //Print request answer
    post_product_info();

  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  get_credentials();

  wl_status_t status1 = WiFi.status();
  if (status1 == WL_CONNECTED)
  {
    char macValue[100];
    uint64_t chipid = ESP.getEfuseMac();
    uint32_t high = chipid >> 32;
    uint32_t low = chipid;
    sprintf(macValue, "%08x%08x", high, low);
    String myString = macValue;
    server.send(200, "application/json", myString);
    Serial.println("mac sent");
  }

  if (status1 == WL_DISCONNECTED)
  {
    server.send(200, "application/json", "Wifi Connection failed");
  }
}


void button_press() {
  int reading = digitalRead(Button_pin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != Button_state) {
      Button_state = reading;
    }
  }

  if (Button_state == 0 && flag2 == 0)
  {
    presstime = millis();
    flag1 = 0;
    flag2 = 1;
    tapCounter++;
  }
  if (Button_state == 1 && flag1 == 0)
  {
    releasetime = millis();
    flag1 = 1;
    flag2 = 0;

    timediff = releasetime - presstime;
  }

  if ((millis() - presstime) > 400 && Button_state == 1)
  {
    if (tapCounter == 1)
    {
      if (timediff >= 400)
      {
        Serial.println("Hold");
        hold();
      }
      else
      {
        Serial.println("single tap");
      }
    }
    else if (tapCounter == 2 )
    {
      if (timediff >= 400)
      {
        Serial.println("single tap and hold");
      }
      else
      {
        Serial.println("double tap");
      }
    }
    else if (tapCounter == 3)
    {
      Serial.println("triple tap");
    }
    else if (tapCounter == 4)
    {
      Serial.println("four tap");
    }
    tapCounter = 0;
  }
  lastButtonState = reading;
}


void hold()
{
  Serial.print("Setting AP (Access Point)…");

  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  WiFi.disconnect();

  server.on("/wifilist", send_wifilist);
  server.on("/body", handleBody);
  server.on("/restart", on_restart);

  server.begin();
  Serial.println("Server listening");
}

void checkPhysicalButton()
{
  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");
  String sub_topic1 = user_id + "/" + macValue + "/relay";
  const char* sub_topic1_c = sub_topic1.c_str();
  //Serial.println(sub_topic1_c);
  String sub_topic2 = user_id + "/" + macValue + "/reset";
  const char* sub_topic2_c = sub_topic2.c_str();
  //Serial.println(sub_topic2_c);
  if (digitalRead(Button_pin) == LOW) {
    if (Button_state != LOW) {
      Relay_State_alexa = !Relay_State_alexa;

      if (Relay_State_alexa == HIGH) {
        digitalWrite(RELAY_PIN_Alexa, Relay_State_alexa);
        digitalWrite(LED_PIN, Relay_State_alexa);
        //Serial.println(sub_topic1_c_g);
        client.publish(sub_topic1_c, "The device is ON");
        Serial.println("The device is ON");
        EEPROM.write(0, Relay_State_alexa);
        EEPROM.commit();
      }
      else if (Relay_State_alexa == LOW) {
        digitalWrite(RELAY_PIN_Alexa, Relay_State_alexa);
        digitalWrite(LED_PIN, Relay_State_alexa);
        client.publish(sub_topic1_c, "The device is OFF");
        Serial.println("The device is OFF");
        EEPROM.write(0, Relay_State_alexa);
        EEPROM.commit();
      }
    }
    Button_state = LOW;
  }
  else {
    Button_state = HIGH;
  }
}

void sensor()
{
  char itemp_mq[100];
  char temp_mq[100];
  // Hall Effect Sensor
  measurement = 0;
  measurement = hallRead();
  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");
  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  // Internal MCU Temp sending to mqtt
  itemp = ((temprature_sens_read() - 32) / 1.8);
  String itemp_str = String(itemp);
  itemp_str.toCharArray(itemp_mq, itemp_str.length() + 1);
  String etemp = user_id + "/" + myString + "/etemp";
  const char* etemp_c = etemp.c_str();
  client.publish(etemp_c, itemp_mq);

  // External Temp sending to mqtt
  tempVal = analogRead(tempPin);
  volts = tempVal / 1023.0;
  temp = (volts - 0.5) * 100 ;
  String temp_str = String(temp);
  temp_str.toCharArray(temp_mq, temp_str.length() + 1);
  String ptemp = user_id + "/" + myString + "/ptemp";
  const char* ptemp_c =  ptemp.c_str();
  client.publish(ptemp_c, temp_mq);
}


void hlw() {
  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  preferences.begin("namespace", false);
  String user_id = preferences.getString("uid");
  float voltage = HL.GetVol() * 0.001;
  char voltage_mq[100];
  String voltage_str = String(voltage);
  voltage_str.toCharArray(voltage_mq, voltage_str.length() + 1);
  String voltage1 = user_id + "/" + myString + "/vol";
  const char* vol = voltage1.c_str();
  client.publish(vol, voltage_mq);

  float active_power = HL.GetActivePower() * 100;
  char active_power_mq[100];
  String active_power_str = String(active_power);
  active_power_str.toCharArray(active_power_mq, active_power_str.length() + 1);
  String active_power1 = user_id + "/" + myString + "/wat";
  const char* wat = active_power1.c_str();
  client.publish(wat, active_power_mq);

  float current = HL.GetCurrent();
  char current_mq[100];
  String current_str = String(current);
  current_str.toCharArray(current_mq, current_str.length() + 1);
  String current1 = user_id + "/" + myString + "/amp";
  const char* amp = current1.c_str();
  client.publish(amp, current_mq);

}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  button_press();

  preferences.begin("namespace", false);
  String ssid_p = preferences.getString("username");
  const char* ssid = ssid_p.c_str();
  String pwd_p = preferences.getString("password");
  const char* pwd = pwd_p.c_str();
  HL.begin(Serial1, RXD2);

  Serial.println("Connecting to WiFi..");
  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
    break;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
  Serial.println("Connected to the WiFi network");
  }

  EEPROM.begin(EEPROM_SIZE);
  Relay_State_alexa = EEPROM.read(0);

  Serial.println(preferences.getString("server"));
  String server = preferences.getString("server");
  const char* server_c = server.c_str();

  Serial.println(preferences.getInt("mqttport"));
  int mqport = preferences.getInt("mqttport");

  Serial.println(preferences.getString("uname"));
  String usname = preferences.getString("uname");
  const char* usname_c = usname.c_str();

  Serial.println(preferences.getString("pword"));
  String pasword = preferences.getString("pword");
  const char* pasword_c = pasword.c_str();

  Serial.println(preferences.getString("ccert"));
  String ccert = preferences.getString("ccert");
  const char* ccert_c = ccert.c_str();

  Serial.println(preferences.getString("ckey"));
  String ckey = preferences.getString("ckey");
  const char* ckey_c = ckey.c_str();

  String user_id = preferences.getString("uid");

  espClient.setCertificate(ccert_c);
  espClient.setPrivateKey(ckey_c);
  client.setServer(server_c, mqport);

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;

  String lwt_s = user_id + "/" + myString + "/" + "lwt";
  const char* lwt = lwt_s.c_str();
  String clientid_s = user_id + "-" + myString;
  const char* clientid = clientid_s.c_str();

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT Broker...");
    if (client.connect(clientid, usname_c, pasword_c, lwt, 0, 0, "Device has been disconnected abruptly" ))
    {
      Serial.println("Connected");
      char macValue[100];
      uint64_t chipid = ESP.getEfuseMac();
      uint32_t high = chipid >> 32;
      uint32_t low = chipid;
      sprintf(macValue, "%08x%08x", high, low);
      String myString = macValue;
      String sub_topic1 = user_id + "/" + macValue + "/relay";
      const char* sub_topic1_c = sub_topic1.c_str();
      sub_topic1_c_g = sub_topic1_c;
      Serial.println(sub_topic1_c);
      client.subscribe(sub_topic1_c);
      String sub_topic2 = user_id + "/" + macValue + "/reset";
      const char* sub_topic2_c = sub_topic2.c_str();
      sub_topic2_c_g = sub_topic2_c;
      client.subscribe(sub_topic2_c);
    }
    else
    {
      Serial.print("state failure  ");
      Serial.print(client.state());
      break;
    }
  }

  client.setCallback(callback);
  pinMode(LED_PIN, OUTPUT);
  pinMode(Button_pin, INPUT_PULLUP);
  pinMode(RELAY_PIN_Alexa, OUTPUT);
  digitalWrite(RELAY_PIN_Alexa, Relay_State_alexa);

}


void loop()
{
  HL.SerialReadLoop();
  server.handleClient();
  button_press();
 
  if (millis() > time_now + period) {
    time_now = millis();
    sensor();
    if (HL.SerialRead == 1) {
      hlw();
    }
  }
  client.loop();
  checkPhysicalButton();
  fauxmo.handle();
}
