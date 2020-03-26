/* OTA + Baig Residency */


#include <WiFi.h>
#include <PubSubClient.h>
//#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>

#define FIRMWARE_VERSION 0.13

String user_id = "370";
const char* ssid = "Perisync";
const char* password = "23223889";

//const char* ssid = "DEL_ACC";
//const char* password = "Compaq2106!";

//const char* ssid = "ACT102187940043";
//const char* password = "62152100";

const char* mqttServer = "18.223.169.112";
const char* mqttUser = "nazim";
const char* mqttPassword = "nazim";
const int mqttPort = 8883;
//const int mqttPort = 1883;

const char* client_certificate = \
                                 "-----BEGIN CERTIFICATE-----\n" \
                                 "MIIDMjCCAhqgAwIBAgIBAjANBgkqhkiG9w0BAQsFADAxMSAwHgYDVQQDDBdUTFNH\n" \
                                 "ZW5TZWxmU2lnbmVkdFJvb3RDQTENMAsGA1UEBwwEJCQkJDAeFw0yMDAxMzEwNjAw\n" \
                                 "NDBaFw0zMDAxMjgwNjAwNDBaMCcxFDASBgNVBAMMC3Jhc3BiZXJyeXBpMQ8wDQYD\n" \
                                 "VQQKDAZjbGllbnQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC/Jo42\n" \
                                 "zKlzyDTGU/HwmVuwnnydlQIB/e5znQjVv+hpxnPXK/DcFiJra11xgdR83WmjNe1I\n" \
                                 "lGRBkAoUeaWXPY2gB6TSSMVq2e7zVXctipVmMEAyvLg7qyBBFHr4x9VeGb1bLHX5\n" \
                                 "CG+fqDLyzv2Bw8OG6jKt+xXTXyTWuIzoC9NOisFcUv11omt8Zwt1NYYlmraWPg0l\n" \
                                 "SHE7hqXBTCD+EA87gMKk8OKt2GjYK7jt1qaDxZmfx/T/eP8rx03NRUUyaV7sHGOV\n" \
                                 "nR35MxrrEVTEFA8axwAqz4t4fckJZv/Zhg07wls7sDpcq0ZxBeHPWQVBRREFriMS\n" \
                                 "1MfP5bSN6QLVcQA1AgMBAAGjXzBdMAkGA1UdEwQCMAAwCwYDVR0PBAQDAgWgMBMG\n" \
                                 "A1UdJQQMMAoGCCsGAQUFBwMCMC4GA1UdEQQnMCWCC3Jhc3BiZXJyeXBpggtyYXNw\n" \
                                 "YmVycnlwaYIJbG9jYWxob3N0MA0GCSqGSIb3DQEBCwUAA4IBAQBd93i9WdCDAz7z\n" \
                                 "4wUllduIDxeRYhtBvAWdjLYCSnxyudiVJTvdqjkQ68mO9DAJCTW/YYXFjscU8Cu2\n" \
                                 "iEpF1vkEkQfYP4Ab6nJOp8dn9uMwFbnQRDfm/sg5Zl37eZtnj17KZ5eWbOfBdAFN\n" \
                                 "7QO3eSRlrFTXKg2C2QWPsxhdxOgtV1EzHofe/JXe0cCw3B17MCYZy5ThFhpj067b\n" \
                                 "VbPUQEbCoutbAFX5R4kioEzvNGHRZEV9uV/DE0S59myichiLGl1FZy+4ZOyrgKKK\n" \
                                 "IBuN5LAatZd/Za8ggw3bFOpVoJiBaUyG8VVm5rgPX/89sUTd9bnOB6WPzhQaY6wf\n" \
                                 "Fa1KzBhH\n" \
                                 "-----END CERTIFICATE-----\n";

const char* client_key = \
                         "-----BEGIN RSA PRIVATE KEY-----\n" \
                         "MIIEpAIBAAKCAQEAvyaONsypc8g0xlPx8JlbsJ58nZUCAf3uc50I1b/oacZz1yvw\n" \
                         "3BYia2tdcYHUfN1pozXtSJRkQZAKFHmllz2NoAek0kjFatnu81V3LYqVZjBAMry4\n" \
                         "O6sgQRR6+MfVXhm9Wyx1+Qhvn6gy8s79gcPDhuoyrfsV018k1riM6AvTTorBXFL9\n" \
                         "daJrfGcLdTWGJZq2lj4NJUhxO4alwUwg/hAPO4DCpPDirdho2Cu47damg8WZn8f0\n" \
                         "/3j/K8dNzUVFMmle7BxjlZ0d+TMa6xFUxBQPGscAKs+LeH3JCWb/2YYNO8JbO7A6\n" \
                         "XKtGcQXhz1kFQUURBa4jEtTHz+W0jekC1XEANQIDAQABAoIBADWhx70wZ0sJHWWY\n" \
                         "IAX1wXgov6YnLM0KRmFB6jyu/7LN76qrtYTK4RVqpmk2mDwAk6crdkwq9oY+t9VO\n" \
                         "3Fs29gm0m1Y0/M8iHp0/mZS5kXl+iES0Fq0HMqGP8YDj8MXaSvrN2rvo1gjh1Imz\n" \
                         "8gT6vF2aUGXoURNhWqC0XzSsPLmIUMzo9+t2URplCBjUWwsWGTch0sKdrozn/lFJ\n" \
                         "KX5QtGkqdoi7iGS/LFTaSFAHffji/PYRZd++DWhHewPOoSFKB7gw020FSFBh7TCx\n" \
                         "CPCfLFQhakp3gML98xEmOu5XT8jXh1e7ZA/7d8/3Zevp8GIEwA5rEHKnhdtaI/r6\n" \
                         "vJ+udQECgYEA5JW2/AZZXbQL0Z/xVqYgXjsMdwXu0zADHLY7iYY6a1sSKucio2pP\n" \
                         "qCk8B0jRvkKth/eAkkC9CdSxVP+sDfJFgVhWr/PMsdI5eeTKp2bNZphMUPPXwHuV\n" \
                         "ru4RdMJHGP1GMleDrCLh0SoynN4zh/kpjDPPwa/tv5qyoev6zL56iWUCgYEA1hN9\n" \
                         "lo//womfE1OPwbAiPeMWRdBEKTrszCFKsReS/65T+lI+Dc779Qe5GlzLPeXc5u33\n" \
                         "zsPNijf/0VR0ALuOt7vcQsgP8cgM/Jh7q4DqQf8yenq3Qr03inFpFr/lJ5Y0NScv\n" \
                         "V8Vnj4OJZmubfToApekMHc/w4b4jNfqo/vrolpECgYBQrZLfEvKZwwFk5FnO5mWI\n" \
                         "PLkijUKE4W5HxP3whSK7X3JjGB8+pDgnNx6Lx6TXzTroQW/srtLJ1Hseb/rvFTJ4\n" \
                         "+4/lHFyD6poErr7kJMGcU2akL9xXgIcNLdOVbTig1kvfWsoLiGhBZe5lNXvkowmK\n" \
                         "BYNvbkOwm84unlZvy07+jQKBgQCKTaVBdF6j7A1diwrA823r1Z46FEc06+C4MHn0\n" \
                         "12oczR9sBV4ZxbN5pjesFG8p2itQ9kS2zEEqHuui7ZDUOFgH09KFQoMqjTf2qPyc\n" \
                         "3Hh7ksel4UosT9pmEsb175u4a43nGR248svpJ+3IwZR+3s40Yc92IzVYYmKVKQ+t\n" \
                         "S6SgQQKBgQCNiTBS4X+ImZG0MDFvHy68+QFDwJPm1Af2SPL3Oq4r0b+PutCadm4G\n" \
                         "2vGvydv1Quom0ORflQQ0zt9FsK5qz5Gf5Orfq/Z8cCmhcLQc5tuckRXJjOCoAfoj\n" \
                         "ehL5MsIKx5nBYR0rpdV8vv2DAPyovfYMqLEn2DcfZBf/EYbdzNdsCA==\n" \
                         "-----END RSA PRIVATE KEY-----\n";



const char* certv1 = "-----BEGIN CERTIFICATE-----\n"\
                     "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"\
                     "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"\
                     "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"\
                     "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"\
                     "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"\
                     "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"\
                     "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"\
                     "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"\
                     "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"\
                     "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"\
                     "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"\
                     "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"\
                     "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"\
                     "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"\
                     "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"\
                     "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"\
                     "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"\
                     "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"\
                     "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"\
                     "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"\
                     "+OkuE6N36B9K\n"\
                     "-----END CERTIFICATE-----\n"\
                     "";

#define URL_fw_info "https://raw.githubusercontent.com/mohammednazimpasha/otaupdate-esp32/master/ota.json"
#define URL_fw_Bin "https://raw.githubusercontent.com/mohammednazimpasha/otaupdate-esp32/master/ota.bin"
StaticJsonDocument<200> json_packet;

WiFiClientSecure espClientx;
PubSubClient client(espClientx);

char flowRate1_mq[100];
char flowRate2_mq[100];
char flowRate3_mq[100];

char avgflowRate_mq[100];

char totalMilliLitres1_mq[100];
char totalMilliLitres2_mq[100];
char totalMilliLitres3_mq[100];

char waterflow_dyn_c[36];
char waterflow1_dyn_c[36];
char waterflow2_dyn_c[36];
char waterflow3_dyn_c[36];

char wcc_dyn_c[36];
char wcc1_dyn_c[36];
char wcc2_dyn_c[36];
char wcc3_dyn_c[36];

char totalMilliLitres1_dyn_c[48];
char totalMilliLitres2_dyn_c[48];
char totalMilliLitres3_dyn_c[48];

#define SENSOR1  5 // Wss/1 - Plastic - Common
#define SENSOR2  9 // Wss/2 - Brass - Private
#define SENSOR3  19 // Wss/2 - Brass - Kitchen

volatile byte pulseCount1;
volatile byte pulseCount2;
volatile byte pulseCount3;

int a;
int b;
int c;
int i;
int j;

float flowRate1;
float flowRate2;
float flowRate3;

float avgflowRate;

byte pulse1Sec1 = 0;
byte pulse1Sec2 = 0;
byte pulse1Sec3 = 0;

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;

float calibrationFactor1 = 4.5;
float calibrationFactor2 = 5.53;
float calibrationFactor3 = 5.53;

float flowMilliLitres1;
float flowMilliLitres2;
float flowMilliLitres3;

float totalMilliLitres1;
float totalMilliLitres2;
float totalMilliLitres3;

unsigned long time_now = 0;
int period = 6000;

void IRAM_ATTR pulseCounter1()
{
  pulseCount1++;

}

void IRAM_ATTR pulseCounter2()
{
  pulseCount2++;

}

void IRAM_ATTR pulseCounter3()
{
  pulseCount3++;

}

void callback (char* topic, byte *payload, unsigned int length)
{
  Serial.println("# Broker Hammered to......");
  Serial.print(topic);

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;
  String sub_topic1 = user_id + "/" + macValue + "/upload";
  const char* sub_topic1_c = sub_topic1.c_str();
  Serial.println(sub_topic1_c);
  String sub_topic2 = user_id + "/" + macValue + "/restart";
  const char* sub_topic2_c = sub_topic2.c_str();
  Serial.println(sub_topic2_c);

  if (strcmp(topic, sub_topic2_c) == 0)
  {
    Serial.println("Restarting");
    ESP.restart();
  }

  else if (strcmp(topic, sub_topic1_c) == 0)
  {
    Serial.print("performing, check_ota_update function");
    check_ota_update();
  }

}



void reconnect ()
{
  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;

  String lwt_s = user_id + "/" + myString + "/" + "lwt";
  const char* lwt = lwt_s.c_str();    //Making lwt topic dynamic

  String lwt_s1 = user_id + "/" + myString + "/" + "!lwt";
  const char* lwt1 = lwt_s1.c_str();    //Making !lwt topic dynamic

  /*Making two subscription topics dynamic*/

  String sub_topic1 = user_id + "/" + macValue + "/upload";
  const char* sub_topic1_c = sub_topic1.c_str();
  Serial.println(sub_topic1_c);
  String sub_topic2 = user_id + "/" + macValue + "/restart";
  const char* sub_topic2_c = sub_topic2.c_str();
  Serial.println(sub_topic2_c);

  while (!client.connected())
  {

    Serial.println("Re-connecting to RabbitMQ...");

    for (j = 0; j < 10; j++)
    {

      Serial.println("Count  ");
      Serial.println(j);

      if (client.connected())
      {
        break;
      }

      if (client.connect("123456", mqttUser, mqttPassword, lwt, 1, 0, "Device disconnected abruptly" ))
      {
        client.publish(lwt1, "Connected just Now");
        Serial.println("Connected");
        client.subscribe(sub_topic2_c);
        client.subscribe(sub_topic1_c);
      }
      else
      {
        Serial.print(i);
        Serial.print("State failure ");
        Serial.print(client.state());
        delay(2000);
      }

    }
    if (!client.connected())
    {
      ESP.restart();
    }
  }

}



void check_ota_update()
{

  WiFiClientSecure *client = new WiFiClientSecure;

  if (client) {
    client -> setCACert(certv1);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
      HTTPClient https;
#ifdef DEBUG_ENABLE
      Serial.print("[HTTPS] begin...\n");
#endif
      if (https.begin(*client, URL_fw_info)) {  // HTTPS
#ifdef DEBUG_ENABLE
        Serial.print("[HTTPS] GET...\n");
#endif
        // start connection and send HTTP header
        int httpCode = https.GET();
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
#ifdef DEBUG_ENABLE
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
#endif
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            DeserializationError error = deserializeJson(json_packet, payload);
            if (!error) {
              String version = json_packet["version"];
              if (version.equals(String(FIRMWARE_VERSION)) )
              {
                Serial.println("Device already on latest firmware version");
              }
              else //todo: check if valid firmware version
              {
                Serial.println("performing ota update..");
                perform_ota();
              }
            }
            json_packet.clear();
#ifdef DEBUG_ENABLE
            Serial.println(payload);
#endif
          }
        } else {
#ifdef DEBUG_ENABLE
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
#endif
        }

        https.end();
      } else {
#ifdef DEBUG_ENABLE
        Serial.printf("[HTTPS] Unable to connect\n");
#endif
      }

    }

    delete client;
  } else {
#ifdef DEBUG_ENABLE
    Serial.println("Unable to create client");
#endif
  }
}



void perform_ota()
{
  WiFiClientSecure clientOTA;
  clientOTA.setCACert(certv1);
  // Reading data over SSL may be slow, use an adequate timeout
  clientOTA.setTimeout(100);
  httpUpdate.setLedPin(25);
  httpUpdate.rebootOnUpdate(true);
  t_httpUpdate_return ret = httpUpdate.update(clientOTA, URL_fw_Bin);
  // Or:
  //t_httpUpdate_return ret = httpUpdate.update(client, "server", 443, "file.bin");
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

void water_flow() {

  char macValue[100];
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t high = chipid >> 32;
  uint32_t low = chipid;
  sprintf(macValue, "%08x%08x", high, low);
  String myString = macValue;

  currentMillis = millis();

  if (currentMillis - previousMillis > interval)
  {

    detachInterrupt(SENSOR1);
    detachInterrupt(SENSOR2);
    detachInterrupt(SENSOR3);

    pulse1Sec1 = pulseCount1;
    pulseCount1 = 0;

    pulse1Sec2 = pulseCount2;
    pulseCount2 = 0;

    pulse1Sec3 = pulseCount3;
    pulseCount3 = 0;

    flowRate1 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec1) / calibrationFactor1;
    flowRate2 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec2) / calibrationFactor2;
    flowRate3 = ((1000.0 / (millis() - previousMillis)) * pulse1Sec3) / calibrationFactor3;
    avgflowRate = (flowRate1 + flowRate2 + flowRate3) / 3;

    flowMilliLitres1 = (flowRate1 / 60);
    totalMilliLitres1 += flowMilliLitres1;

    flowMilliLitres2 = (flowRate2 / 60);
    totalMilliLitres2 += flowMilliLitres2;
    Serial.print(totalMilliLitres2);

    flowMilliLitres3 = (flowRate3 / 60);
    totalMilliLitres3 += flowMilliLitres3;

    a++;
    b++;
    c++;

    String flowRate1_str = String(flowRate1);
    flowRate1_str.toCharArray(flowRate1_mq, flowRate1_str.length() + 1);

    String flowRate2_str = String(flowRate2);
    flowRate2_str.toCharArray(flowRate2_mq, flowRate2_str.length() + 1);

    String flowRate3_str = String(flowRate3);
    flowRate3_str.toCharArray(flowRate3_mq, flowRate3_str.length() + 1);

    String avgflowRate_str = String(avgflowRate);
    avgflowRate_str.toCharArray(avgflowRate_mq, avgflowRate_str.length() + 1);

    String totalMilliLitres1_str = String(totalMilliLitres1);
    totalMilliLitres1_str.toCharArray(totalMilliLitres1_mq, totalMilliLitres1_str.length() + 1);

    String totalMilliLitres2_str = String(totalMilliLitres2);
    totalMilliLitres2_str.toCharArray(totalMilliLitres2_mq, totalMilliLitres2_str.length() + 1);

    String totalMilliLitres3_str = String(totalMilliLitres3);
    totalMilliLitres3_str.toCharArray(totalMilliLitres3_mq, totalMilliLitres3_str.length() + 1);

    previousMillis = millis();

    if (a >= 60)
    {
      a = 0;

      String waterflow_dyn = user_id + "/" + myString + "/" + "waterflow";
      waterflow_dyn.toCharArray(waterflow_dyn_c, waterflow_dyn.length() + 1);
      String waterflow1_dyn = user_id + "/" + myString + "/" + "waterflow/1";
      waterflow1_dyn.toCharArray(waterflow1_dyn_c, waterflow1_dyn.length() + 1);
      String waterflow2_dyn = user_id + "/" + myString + "/" + "waterflow/2";
      waterflow2_dyn.toCharArray(waterflow2_dyn_c, waterflow2_dyn.length() + 1);
      String waterflow3_dyn = user_id + "/" + myString + "/" + "waterflow/3";
      waterflow3_dyn.toCharArray(waterflow3_dyn_c, waterflow3_dyn.length() + 1);

      Serial.print("a#waterflow/1: ");
      Serial.print(flowRate1);
      Serial.print("..L/min");
      Serial.print("\t");

      Serial.print("a#waterflow/2: ");
      Serial.print(flowRate2);
      Serial.print("L/min");
      Serial.print("\t");

      Serial.print("a#waterflow/3: ");
      Serial.print(flowRate3);
      Serial.print("L/min");
      Serial.print("\t");

      Serial.println("\n");

      client.publish(waterflow_dyn_c, avgflowRate_mq);
      client.publish(waterflow1_dyn_c, flowRate1_mq);
      client.publish(waterflow2_dyn_c, flowRate2_mq);
      client.publish(waterflow3_dyn_c, flowRate3_mq);

      Serial.println(waterflow_dyn_c);

    }


    if (b >= 5)
    {
      b = 0;

      String wss_dyn = user_id + "/" + myString + "/" + "wcc";
      wss_dyn.toCharArray(wcc_dyn_c, wss_dyn.length() + 1);
      String wss1_dyn = user_id + "/" + myString + "/" + "wcc/1";
      wss1_dyn.toCharArray(wcc1_dyn_c, wss1_dyn.length() + 1);
      String wss2_dyn = user_id + "/" + myString + "/" + "wcc/2";
      wss2_dyn.toCharArray(wcc2_dyn_c, wss2_dyn.length() + 1);
      String wss3_dyn = user_id + "/" + myString + "/" + "wcc/3";
      wss3_dyn.toCharArray(wcc3_dyn_c, wss3_dyn.length() + 1);


      Serial.print("b#Flow rate1: ");
      Serial.print(flowRate1);
      Serial.print("L/min");
      Serial.print("\t");

      Serial.print("b#Flow rate2: ");
      Serial.print(flowRate2);
      Serial.print("L/min");
      Serial.print("\t");

      Serial.print("b#Flow rate3: ");
      Serial.print(flowRate3);
      Serial.print("L/min");
      Serial.print("\t");

      Serial.println("\n");

      client.publish(wcc_dyn_c, avgflowRate_mq);
      client.publish(wcc1_dyn_c, flowRate1_mq);
      client.publish(wcc2_dyn_c, flowRate2_mq);
      client.publish(wcc3_dyn_c, flowRate3_mq);
      Serial.println(wcc3_dyn_c);
    }


    if (c >= 60)
    {

      c = 0;
      String totalMilliLitres1_dyn = user_id + "/" + myString + "/" + "totalMilliLitres/1";
      totalMilliLitres1_dyn.toCharArray(totalMilliLitres1_dyn_c, totalMilliLitres1_dyn.length() + 1);
      String totalMilliLitres2_dyn = user_id + "/" + myString +  "/" + "totalMilliLitres/2";
      totalMilliLitres2_dyn.toCharArray(totalMilliLitres2_dyn_c, totalMilliLitres2_dyn.length() + 1);
      String totalMilliLitres3_dyn = user_id + "/" + myString + "/" + "totalMilliLitres/3";
      totalMilliLitres3_dyn.toCharArray(totalMilliLitres3_dyn_c, totalMilliLitres3_dyn.length() + 1);
      client.publish(totalMilliLitres1_dyn_c, totalMilliLitres1_mq);
      client.publish(totalMilliLitres2_dyn_c, totalMilliLitres2_mq);
      client.publish(totalMilliLitres3_dyn_c, totalMilliLitres3_mq);
      Serial.println(totalMilliLitres1_dyn_c);

      totalMilliLitres1 = 0;
      totalMilliLitres2 = 0;
      totalMilliLitres3 = 0;

    }
    attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
    attachInterrupt(digitalPinToInterrupt(SENSOR3), pulseCounter3, FALLING);
  }
}

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  for (i = 0; i <= 50; i++)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, password);
      delay(1000);
      Serial.print(".");
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi freshly connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  else
  {
    ESP.restart();
  }


  espClientx.setCertificate(client_certificate);
  espClientx.setPrivateKey(client_key);

  client.setServer(mqttServer, mqttPort);
  reconnect ();

  client.setCallback(callback);

  pinMode(SENSOR1, INPUT_PULLUP);
  pinMode(SENSOR2, INPUT_PULLUP);
  pinMode(SENSOR3, INPUT_PULLUP);

  pulseCount1 = 0;
  flowRate1 = 0.0;

  pulseCount2 = 0;
  flowRate2 = 0.0;

  pulseCount3 = 0;
  flowRate3 = 0.0;

  avgflowRate = 0;
  previousMillis = 0;

  a = 0;
  b = 0;
  c = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR2), pulseCounter2, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENSOR3), pulseCounter3, FALLING);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  water_flow();
  client.loop();
}
