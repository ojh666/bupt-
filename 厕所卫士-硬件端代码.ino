#include <ESP8266.h>
#include <Wire.h>
#include <I2Cdev.h> 

#define INTERVAL_SENSOR   5000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      5000             //定义发送时间

int Sensor = 12;
const int TrigPin = 8; //发送超声波
const int EchoPin = 9; //接收超声波
float cm;
float sensor_tem, sensor_hum, sensor_lux;  
char  sensor_tem_c[7], sensor_hum_c[7], sensor_lux_c[7] ; 
char  buf[10];
String jsonToSend;
String postString;  

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();

float tempOLED, humiOLED, lightnessOLED;

#define INTERVAL_OLED 1000

unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器


#ifdef ESP32
#error "This code is not recommended to run on the ESP32 platform! Please check your Tools->Board setting."
#endif

/**
**CoreUSB UART Port: [Serial1] [D0,D1]
**Core+ UART Port: [Serial1] [D2,D3]
**/
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1284P__) || defined (__AVR_ATmega644P__) || defined(__AVR_ATmega128RFA1__)
#define EspSerial Serial1
#define UARTSPEED  115200
#endif

/**
**Core UART Port: [SoftSerial] [D2,D3]
**/
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); /* RX:D2, TX:D3 */

#define EspSerial mySerial
#define UARTSPEED  9600
#endif

#define SSID        "HUAWEI P30 Pro"
#define PASSWORD    "sxb123zxc"
#define DEVICEID    "575179617" //OneNet上的设备ID
String apiKey = "AsEh2aZWBXfEd52BpzRuHUTC1=c=";

#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_NET      5000  

ESP8266 wifi(&EspSerial);

void setup(void)
{
  pinMode(10, OUTPUT);
  Serial.begin(115200);
  pinMode(Sensor, INPUT);
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  while (!Serial); // wait for Leonardo enumeration, others continue immediately
  Serial.print(F("setup begin\r\n"));
  delay(100);

  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version: "));
  Serial.println(wifi.getVersion().c_str());


  if (wifi.setOprToStation()) {
    Serial.print(F("to station ok\r\n"));
  } else {
    Serial.print(F("to station err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));
    Serial.print(F("IP: "));
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  Serial.print(F("setup end\r\n"));
}
  int interval=15000;
  int a = 0;
  int val = digitalRead(Sensor);   
  void loop(){
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔
  {  
    getSensorData();          
    //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
       // 产生一个10us的高电平脉冲去触发TrigPin
       digitalWrite(TrigPin, LOW);
       delayMicroseconds(2);
       digitalWrite(TrigPin, HIGH);
       delayMicroseconds(10);
       digitalWrite(TrigPin, LOW);
       // 检测脉冲宽度，并计算出距离
       cm = pulseIn(EchoPin, HIGH) / 58.0;
       if((a!=3)&&(cm<50)&&(val=0)){
       for(int i = 0;i<3;i++){
       digitalWrite(10,HIGH);
       // 产生一个10us的高电平脉冲去触发TrigPin
       digitalWrite(TrigPin, LOW);
       delayMicroseconds(2);
       digitalWrite(TrigPin, HIGH);
       delayMicroseconds(10);
       digitalWrite(TrigPin, LOW);
       // 检测脉冲宽度，并计算出距离
       cm = pulseIn(EchoPin, HIGH) / 58.0;
       Serial.println(cm);  
       if(cm>=50){digitalWrite(10,LOW);i=3;}
       else{a=i+1;}
       delay(5000); 
      }
  } 
    digitalWrite(10,LOW);
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
}
void getSensorData(){  
    if(a==3){sensor_lux = 1;}
    else{sensor_lux = 0;}
    sensor_tem = 5;  
    sensor_hum = 4;     
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    dtostrf(sensor_lux, 3, 1,+ sensor_lux_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Humidity\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Light\":";
    dtostrf(sensor_lux,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
}
