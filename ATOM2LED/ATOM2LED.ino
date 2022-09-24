#include <M5Atom.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>

#define PIN 25 

#define OUTER_PIN        26  // M5Atom Lite Grobe Pin
#define OUTER_NUMPIXELS  72  //Number of LED
//#define OUTER_NUMPIXELS  15  // Short LED tape
#define DELAYVAL  1  // Time (in milliseconds) to pause between pixels

const int LEN = 300;
const int MAXMODE = 3; // Number of LED color pattern

//Color pattern by switch command from host.
int root_color[MAXMODE][3] ={{255,0,0},{255,50,0},{255,0,0}};
int node_color[MAXMODE][3] ={{0,0,0},{0,0,0},{0,0,255}};
int brightness[MAXMODE] ={20,50,20};

int store[LEN];
int count;
int modeCnt;

Adafruit_NeoPixel outer_pixels(OUTER_NUMPIXELS, OUTER_PIN, NEO_GRB + NEO_KHZ800); //Boot NeoPixel for 800kHz

void setup() {
  outer_pixels.begin(); //init NeoPixel
  M5.begin(true, true, true);


    Serial.println(WiFi.macAddress()); // Show MAC address to Console.

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() == ESP_OK) {
        Serial.println("ESP-Now Init Success");
    }
    esp_now_register_recv_cb(onReceive);
    count=0;
    modeCnt=0;
}

void onReceive(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
    int modeNum = modeCnt % MAXMODE;
    
    count++;
    if(count>LEN)
        count=0;
    
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println();
    Serial.printf("Last Packet Recv from: %s\n", macStr);
    Serial.printf("Last Packet Recv Data(%d): ", data_len);
    for (int i = 0; i < data_len; i++) {
        //Serial.print(data[i]);
        //Serial.print(" ");
        if (data[i] == 222) {
            //toggleLed();
        }
    }
    if(data[0]=='#'){
      modeCnt++;
    }
    else{
      int currentValue = atoi((const char *)data);
    Serial.print(currentValue);
    store[count]= currentValue;
    int maxVal=0;
    for (int a = 0; a < LEN;  a++)
    {
      if (store[a] > maxVal) {
        maxVal = store[a];
      }
    }
    

    int maxpix = (((double)currentValue)/maxVal)*OUTER_NUMPIXELS;
    Serial.printf("MaxVal:%d MaxPix:%d\n", maxVal,maxpix);
     outer_pixels.clear();
     outer_pixels.setBrightness(brightness[modeNum]);
    for(int i=0; i<maxpix; i++) {
      outer_pixels.setPixelColor(i, outer_pixels.Color(root_color[modeNum][0],root_color[modeNum][1],root_color[modeNum][2]));
    }
    for(int i=maxpix; i<OUTER_NUMPIXELS; i++) {
      outer_pixels.setPixelColor(i, outer_pixels.Color(node_color[modeNum][0],node_color[modeNum][1],node_color[modeNum][2]));
    }

    outer_pixels.show();
    }
    
}

void loop() {
  //the script use onRecieve Callback.
}
