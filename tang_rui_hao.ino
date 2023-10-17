#include <ThingsCloudWiFiManager.h>
#include <ThingsCloudMQTT.h>
#include <FastLED.h>            // 此示例程序需要使用FastLED库

//======================================================
// 设置 ssid / password，连接到你的 WiFi AP
const char *ssid = "myh";
const char *password = "123456789";
// 在 ThingsCloud 控制台的设备详情页中，复制以下设备连接信息
// https://console.thingscloud.xyz
#define THINGSCLOUD_MQTT_HOST "sh-1-mqtt.iot-api.com"
#define THINGSCLOUD_DEVICE_ACCESS_TOKEN "mwt8niybmmhx0y1l"
#define THINGSCLOUD_PROJECT_KEY "QN2QwMFQRC"
//======================================================

ThingsCloudMQTT client(
  THINGSCLOUD_MQTT_HOST,
  THINGSCLOUD_DEVICE_ACCESS_TOKEN,
  THINGSCLOUD_PROJECT_KEY);

// 设置指示灯的输入引脚
#define RELAY_PIN 5

#define DIR 4 //方向信号
#define PUL 2 //脉冲信号

#define NUM_LEDS 30             // LED灯珠数量
#define DATA_PIN 13              // Arduino输出控制信号引脚
#define LED_TYPE WS2812         // LED灯带型号
#define COLOR_ORDER GRB         // RGB灯珠中红色、绿色、蓝色LED的排列顺序

uint8_t max_bright = 255;       // LED亮度控制变量，可使用数值为 0 ～ 255， 数值越大则光带亮度越高
 
CRGB leds[NUM_LEDS];            // 建立光带leds

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  // 初始断开继电器，输出高电平
  digitalWrite(RELAY_PIN, LOW);

  pinMode(DIR, OUTPUT);
  pinMode(PUL, OUTPUT);
  //digitalWrite(DIR, LOW);//设置步进电机转动方向
  LEDS.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // 初始化光带 
  FastLED.setBrightness(max_bright);

  // 允许 SDK 的日志输出
  client.enableDebuggingMessages();

  // 连接 WiFi AP
  client.setWifiCredentials(ssid, password);
}

void liang(){           //灯带亮
  for (int i=0;i<=29;i++){
    leds[i]= CRGB::White;
    FastLED.show();                // 更新LED色彩
    delay(50);                     // 等待500毫秒
  }
}

void mie(){           //灯带灭
  for (int i=0;i<=29;i++){
    leds[i]= CRGB::Black;
    FastLED.show();                // 更新LED色彩
    delay(50);                     // 等待500毫秒
  }
}

void turn(int count)
{
    for(int i=0;i<count;i++)
    {
        digitalWrite(DIR, LOW);//步进电机顺时针方向
        digitalWrite(PUL, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL, LOW);
        delayMicroseconds(500);    
    }
}

void antiturn(int count)
{
    for(int i=0;i<count;i++)
    {
        digitalWrite(DIR, HIGH);//步进电机逆时针方向
        digitalWrite(PUL, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL, LOW);
        delayMicroseconds(500);    
    }
}

void handleAttributes(const JsonObject &obj) {
  if (obj.containsKey("alarm_status")) {        //步进电机控制
    // 接收到下发的 relay 属性
    if (obj["alarm_status"] == true) {
      // 闭合继电器，输出低电平
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("relay switch to ON");
      turn(17000);
      delay(10000);
      antiturn(17000);
    } else {
      // 断开继电器，输出高电平
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("relay switch to OFF");
      //antiturn(30000);
    }
  }
  if (obj.containsKey("dengdai_state")) {
    // 接收到下发的 relay 属性
    if (obj["dengdai_state"] == true) {
      // 闭合继电器，输出低电平
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("relay switch to ON");
      liang();
    } else {
      // 断开继电器，输出高电平
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("relay switch to OFF");
      mie();
    }
  }
}
// 必须实现这个回调函数，当 MQTT 连接成功后执行该函数。
void onMQTTConnect() {
  // 订阅获取属性的回复消息
  client.onAttributesGetResponse([](const String &topic, const JsonObject &obj) {
    if (obj["result"] == 1) {
      handleAttributes(obj["attributes"]);
    }
  });
  // 订阅云平台下发属性的消息
  client.onAttributesPush([](const JsonObject &obj) {
    handleAttributes(obj);
  });
  // 读取设备在云平台上的属性，用于初始化继电器状态。
  // 云平台回复的属性会进入上边 client.onAttributesGetResponse 接收的消息。
  client.getAttributes();
}

void loop() {
  client.loop();
}
