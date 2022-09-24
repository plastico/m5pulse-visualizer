/*----------------------------------------------
 *  M5Stick Pulse02 Serial Sensor Recieve Sample
 *    #2019/09/15 - #2019/09/17 - #2020/08/17
 *    Serial 19200ボー
 *    
 *    Board Select -> M5Stick
 *    Writer       -> AVRISP MkII
 -----------------------------------------------*/
#include <M5StickC.h>
#include <WiFi.h>
#include <esp_now.h>

#define MAXADRANGE  4096
#define DELIMITCODE  0x0a       // Delimit Code 

#define SWEEPMAG    6 
#define LCDWIDTH    160
#define LCDHEIGHT   80
#define PLSPOSX     32
#define PLSPOSY     2
#define BTN_A_PIN 37
#define BTN_B_PIN 39

// INPUT_PULLUPが有効かは不明だが、有効という前提で定義
#define BTN_ON  LOW
#define BTN_OFF HIGH

static  int sweepptr = 0;
static  int lastY = 0;
static  int plsrate = 0;
static  int posx = 0;
static  float fConv;  
uint8_t slaveAddress[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // 受信機のMACアドレスに書き換えます
esp_now_peer_info_t slave;
uint8_t prev_btn_a = BTN_OFF;
uint8_t btn_a      = BTN_OFF;

void setup() {

    M5.begin();
    pinMode(BTN_A_PIN, INPUT_PULLUP);
//---
    M5.Axp.ScreenBreath(8);         // LCD輝度を抑える 7-15
    M5.Lcd.setRotation(1);          // 回転をDefaultに戻す
    M5.Lcd.fillScreen(BLACK);
//--- 省電力でWifiOFFする場合に下記入れる
    //WiFi.mode(WIFI_OFF);
//--
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);    
    Serial2.begin(19200, SERIAL_8N1, 32, 33);   // Grove connector RX=32  TX=33
  // コマンドを何回か送らないと認識しない場合があるので5回繰り返す
    for (byte i=0; i<5; i++){       
      Serial2.print("@RG2");           // Gain=2  4->2
      Serial2.write(0x0a);
    }
    for (byte i=0; i<5; i++){
      Serial2.print("@OF30");           // Offset
      Serial2.write(0x0a);
    }
    fConv = (float)LCDHEIGHT/(float)MAXADRANGE;

 // ESP-NOW初期化
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() == ESP_OK) {
      Serial.println("ESPNow Init Success");
      M5.Lcd.print("ESPNow Init Success\n");
/*
      slave.peer_addr[0]=(uint8_t)0x24;
      slave.peer_addr[1]=(uint8_t)0x0a;
      slave.peer_addr[2]=(uint8_t)0xc4;
      slave.peer_addr[3]=(uint8_t)0xf9;
      slave.peer_addr[4]=(uint8_t)0x9f;
      slave.peer_addr[5]=(uint8_t)0x91;
  */
      slave.peer_addr[0]=(uint8_t)0xff;
      slave.peer_addr[1]=(uint8_t)0xff;
      slave.peer_addr[2]=(uint8_t)0xff;
      slave.peer_addr[3]=(uint8_t)0xff;
      slave.peer_addr[4]=(uint8_t)0xff;
      slave.peer_addr[5]=(uint8_t)0xff;
      slave.channel = 0;
      slave.encrypt = false;

      esp_err_t addStatus = esp_now_add_peer(&slave);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
      }
    } else {
      Serial.println("ESPNow Init Failed");
      M5.Lcd.print("ESPNow Init Failed\n");
      ESP.restart();
    
      //esp_now_peer_info_t slave;
      //memcpy(peerInfo.peer_addr, slaveAddress, 6);

      
  }
      
}

void loop() {
String   strInput;  
int      val,posy;
char     buf[250];
uint8_t bs[250];
 M5.update();

 btn_a = digitalRead(BTN_A_PIN);
 if(prev_btn_a == BTN_OFF && btn_a == BTN_ON){
            sprintf(buf, "#");
            memcpy(bs, buf, strlen(buf));
    esp_err_t result = esp_now_send(slave.peer_addr, bs, strlen(buf));
             
  }
  else{
    

 if(Serial2.available() > 0) {
    strInput = Serial2.readStringUntil(DELIMITCODE);
    if (strInput[0]=='#'){      // 脈波間隔データ
      strInput[0]=' ';          // Spaceに置き換える
      if (strInput[1]=='-'){
        strInput[1]=' ';        // Spaceに置き換える
        M5.Lcd.setTextColor(RED);
      }else{
        M5.Lcd.setTextColor(GREEN);
      }
      val = strInput.toInt();     
      if (val>0) plsrate = 60000/val;
      M5.Lcd.fillRect(PLSPOSX, PLSPOSY, 32, 32, BLACK);      
    }else{
      sweepptr++;
      
      if (sweepptr>=SWEEPMAG){
        sweepptr = 0;
        val = strInput.toInt();
        

            sprintf(buf, "%d", val);
            memcpy(bs, buf, strlen(buf));
            Serial.print(buf);
            Serial.print(" : "); 
            esp_err_t result = esp_now_send(slave.peer_addr, bs, strlen(buf));
              
        //esp_err_t result = esp_now_send(slave.peer_addr, data, sizeof(data));
        M5.Lcd.drawLine(posx, 0, posx, LCDHEIGHT-1, BLACK);
        posy = LCDHEIGHT - (int)((float)val*fConv);
        if (posx > 0) {
          M5.Lcd.drawLine(posx-1, lastY, posx, posy, WHITE);
        }else{
          M5.Lcd.drawLine(posx, lastY, posx, posy, WHITE);
        }
        lastY = posy;
        posx++;
        if (posx >= LCDWIDTH) posx=0;

        //Serial.print("Send Status: ");
        switch (result)
        {
        case ESP_OK:
            Serial.println("Success");
            break;
        case ESP_ERR_ESPNOW_NOT_INIT:
            Serial.println("ESPNOW not Init.");
            break;
        case ESP_ERR_ESPNOW_ARG:
            Serial.println("Invalid Argument");
            break;
        case ESP_ERR_ESPNOW_INTERNAL:
            Serial.println("Internal Error");
            break;
        case ESP_ERR_ESPNOW_NO_MEM:
            Serial.println("ESP_ERR_ESPNOW_NO_MEM");
            break;
        case ESP_ERR_ESPNOW_NOT_FOUND:
            Serial.println("Peer not found.");
            break;

        default:
            Serial.println("Not sure what happened");
            break;
        }
      }
    }
    M5.Lcd.setCursor(PLSPOSX, PLSPOSY);
    M5.Lcd.println(plsrate);
  }    
  }
  delay(1);
  prev_btn_a =btn_a;
   
}

//-----------
