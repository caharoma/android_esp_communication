#include <WiFi.h>
#include <string.h>
#include <WiFiAP.h>
#include "driver/gpio.h"
#include <WebServer.h>
#include "FS.h"
#include "SPIFFS.h"

#define LED1 13
#define LED2 12
#define LED3 14
#define LED4 27

#define RESET_BTN 35

char* ssid = "";
char* password = "";
const int leds[] = {LED1,LED2,LED3,LED4};

int x = 1;
int y = 0;
bool wifiConfigured = false;

const char* cfgFilePath = "/config.txt";
File cfgFile;


const char index_html[] PROGMEM =R"rawliteral(
<!DOCTYPE HTML><html>
<head>
<script>
function button_click(){
  let ssid = document.getElementById('ssid').value;
  let pass = document.getElementById('pass').value;
  let button = document.getElementById('button');
  let url = "/set?s="+ssid+"&p="+pass;

  button.disabled = true;

  let resp = fetch(url);
  if(resp.ok){
    button.textContent = "Done";
  }
}
</script>
</head>
<body>
<b>ssid</b> </br>
<input type="text" id="ssid" size="20" required></input> </br>
<b>password</b> </br>
<input type="text" id="pass" size="20" required></input> </br>
<button type="button" id="button" onclick="button_click()">Save</button>

</body>
</html>)rawliteral";


WebServer server(80);

WiFiServer wifiServer(6886);

void render_index(){
  server.send(200, "text/html", index_html);
}

void set_cfg(){
  const char *s = server.arg(0).c_str();
  const char *p = server.arg(1).c_str();
  gpio_set_level((gpio_num_t)leds[2], 1);

  Serial.println(server.arg(0));
  Serial.println(s);
  Serial.println(p);

  cfgFile = SPIFFS.open(cfgFilePath, FILE_WRITE);
  String data = server.arg(0) + (String)";" + server.arg(1);
  cfgFile.print(data.c_str());
  cfgFile.close();
  
  Serial.println(data.c_str());
  gpio_set_level((gpio_num_t)leds[3], 1);
  server.send(200, "text/html", "OK");
  ESP.restart();
}

void setup_routing(){
  server.on("/", render_index);
  server.on("/set", set_cfg);
  server.begin();
}



void setup() {
  Serial.begin(115200);
  for (int i=0; i<4; i++){
    gpio_pad_select_gpio(leds[i]);
    gpio_set_direction((gpio_num_t)leds[i], GPIO_MODE_OUTPUT);
    //gpio_set_direction((gpio_num_t)RESET_BTN, GPIO_MODE_INPUT);
    pinMode(RESET_BTN, INPUT);
  }

  if (!SPIFFS.begin(true)){
    gpio_set_level((gpio_num_t)leds[1], 1);
  }



  cfgFile = SPIFFS.open(cfgFilePath, FILE_READ);
  if(!cfgFile){
    wifiConfigured = false;
    Serial.println("no cfg");
  }
  else{
    wifiConfigured = true;
    char* data = (char*)cfgFile.readString().c_str();
    Serial.println(data);
    if (strlen(data) < 1){
      wifiConfigured = false;
      gpio_set_level((gpio_num_t)leds[0], 0);
    }
    else{

      Serial.println("setting up");
      Serial.println(data);
      char str[strlen(data)];
      strcpy(str, data);
      Serial.println(str);
      char* d = strtok(str, ";");
      int r = 0;
      while (d != NULL){
        if(r == 0){
          ssid = d;
        }
        else{
          password = d;
        }
        r++;
        d = strtok(NULL, ";");
      }

      Serial.println(ssid);
      Serial.println(password);

      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED){
        delay(500);
      }
      wifiServer.begin();
      gpio_set_level((gpio_num_t)leds[1], 1);

    }
  }

  if (wifiConfigured == false){
    WiFi.softAP("test");
  }
  
  setup_routing();

  gpio_set_level((gpio_num_t)leds[0], 1);

  for (int i=0; i<4; i++){
    gpio_set_level((gpio_num_t)leds[i], 0);
  }
}



int LAST_LED = 0;
const long interval = 100;
unsigned long lastMillis = millis();
int LAST_INFO = 0;

void loop() {
  if(digitalRead(RESET_BTN) == HIGH){
    Serial.println("-------delete");
    SPIFFS.remove(cfgFilePath);
    ESP.restart();
  }
  if(wifiConfigured == false){
    server.handleClient();
  }
  else{
    WiFiClient wifiClient = wifiServer.available();
    while (wifiClient.connected()){
      if(wifiClient.available()){

        byte buffer[4];
        for(int i=0; i<4; i++){
          buffer[i] = wifiClient.read();
        }

        if (buffer[0] != byte('-') && buffer[0]!= byte('+')){
          continue;
        }

        int m = (int)((buffer[1]-byte('0'))*100)+((buffer[2]-byte('0'))*10)+(buffer[3]-byte('0'));
        

        if(buffer[0] == byte('-')){
          m = -m;
        }

        Serial.println(m);

        if(m != 0){ 
          LAST_INFO = m;
        }
        unsigned long curentMillis = millis();



        if (curentMillis - lastMillis >= interval - abs(m)){ 
          if (LAST_INFO != 0){
            for (int i=0; i<4; i++){
              gpio_set_level((gpio_num_t)leds[i], 0);
            }
            if (LAST_INFO > 0){
              gpio_set_level((gpio_num_t)leds[LAST_LED], 1);
              LAST_LED++;
              if(LAST_LED > 3){
                LAST_LED = 0;
              }
            }
            else{
              gpio_set_level((gpio_num_t)leds[LAST_LED], 1);
              LAST_LED--;
              if(LAST_LED < 0){
                LAST_LED = 3;
              }
            }
          }
          lastMillis = millis();
        }
      }
    }
  }
}