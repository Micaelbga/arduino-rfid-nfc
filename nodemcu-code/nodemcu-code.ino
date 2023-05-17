#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

const char *ssid = ""; // NAME WI-FI
const char *password = ""; // PASSWORD WI-FI
String ip = ""; //IP LOCALSERVER DB

String databaseUrl = "http://"+ ip +":3000/api/";

String setFunction = "";

WiFiClient client;

SoftwareSerial nodemcu(3,1);

void connectToWiFi(){
  Serial.println("Connecting to WiFi...");
  Serial.println("...");
  WiFi.begin(ssid,password);
  int retries = 0;
  while((WiFi.status() != WL_CONNECTED) && (retries < 15)){
    retries ++;
    delay(500);
    Serial.print(".");
  }
  if(retries > 14){
    Serial.println("WiFi connection failed.");
  }
  if((WiFi.status() == WL_CONNECTED)){
    Serial.println("WiFi connected!");
    Serial.println("IP andress: ");
    Serial.print(WiFi.localIP());
    Serial.println();
  }
}

String receiveDataFromArduino(){
    char c;
    String data;
    c = 0;
    data = "";
   
    while(nodemcu.available() > 0){
      c = nodemcu.read();

      if(c == 'C'){
        setFunction = c;
        break;
      }
      
      if(c == 'A'){
        break;
      }
      
      if(c != 'A' || c != 'C'){
        data += c;
        delay(1);
      }
    }
    return data;
}

void transferDataToArduino(int codStatus){
  nodemcu.flush();
  nodemcu.print(codStatus);
  nodemcu.print("B");
  nodemcu.flush();
  delay(500);
}

int getCartaoByUid(String uid){
  HTTPClient http;
  http.begin(client,databaseUrl+"cartao/getcartao");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int res = http.POST(uid);
  
  http.end();
  return res;
}

void postCartao(String uid){
  HTTPClient http;
  http.begin(client,databaseUrl+"cartao/postcartao" );
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
  http.POST(uid);
  http.end();
}

void postMovimentacao(String uid){
  HTTPClient http;
  http.begin(client,databaseUrl+"movimentacao/postmovimentacao" );
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
  http.POST(uid);
  http.end();
}

void setup(){
  Serial.begin(57600);
  nodemcu.begin(9600);
  
  connectToWiFi();
  
}

void loop(){
    int codStatus;
    String uid;

    uid = receiveDataFromArduino();

    if(uid == ""){
      return;
    }

    if(setFunction == "C"){
      codStatus = getCartaoByUid(uid);
      transferDataToArduino(codStatus);
      if(codStatus == 404){
        postCartao(uid);
      }
      setFunction = "";
      uid = "";
    }
    
    if(uid != ""){
      codStatus = getCartaoByUid(uid);
      transferDataToArduino(codStatus);
      if(codStatus ==200){
        postMovimentacao(uid);
      }
      uid = "";
    }
}
