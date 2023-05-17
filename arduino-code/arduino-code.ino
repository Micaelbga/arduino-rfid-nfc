#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define RST_PIN   9     // Configurable, see typical pin layout above
#define SS_PIN    10    // Configurable, see typical pin layout above

int ledVerde = 7;
int ledVermelho = 6;
int ledAmarelo = 5;
int pinButton = 4;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

SoftwareSerial arduino(0,1);

String receiveDataFromNodemcu(){
    char c;
    String data;
    c = 0;
    data = "";
   
    while(arduino.available() > 0){
      c = arduino.read();
      if(c == 'B'){
        break;
      }else{
        data += c;
        delay(1);
      }
    }
    return data;
}

String readCard(){
  String uid;
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    delay(50);
    return "";
  }

  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += mfrc522.uid.uidByte[i];
  }
  return uid;
}

void transferDataToNodemcu(String data, String character){
  arduino.flush();
  arduino.print(data);
  arduino.print(character);
  arduino.flush();
  delay(500); 
}

void triggerLed(int codStatus, bool isInverse = false){
    if(!isInverse){
      switch(codStatus){
        case 200:
          digitalWrite(ledVerde,HIGH);
          delay(1000);
          digitalWrite(ledVerde,LOW); 
          break;
          
        case 404:
          digitalWrite(ledVermelho,HIGH);
          delay(1000);
          digitalWrite(ledVermelho,LOW); 
          break;
          
        default:
          break;
      } 
    }else{
      switch(codStatus){
        case 200:
          digitalWrite(ledVermelho,HIGH);
          delay(1000);
          digitalWrite(ledVermelho,LOW); 
          break;
          
        case 404:
          digitalWrite(ledVerde,HIGH);
          delay(1000);
          digitalWrite(ledVerde,LOW); 
          break;
            
        default:
          break;
      }
    }
}

void setup() {
  arduino.begin(9600); 
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(pinButton, INPUT);
}

void loop() {
  String uid;
  String codStatus = "0";

  if(digitalRead(pinButton)){
    int stateButton = 1;
    digitalWrite(ledAmarelo,HIGH);
    while(stateButton){
      uid = readCard();
      if(uid != ""){
        transferDataToNodemcu(uid, "C");
        codStatus = receiveDataFromNodemcu();
        digitalWrite(ledAmarelo, LOW);
        triggerLed(codStatus.toInt(), true);
        stateButton = 0;
        uid = "";
      }
    }
  }

  uid = readCard(); //retorna pro inicio do loop se nenhum cartão for lido
  if(uid != ""){
    transferDataToNodemcu(uid, "A");
  }
  
  codStatus = receiveDataFromNodemcu();
  
  if(codStatus != "0"){
    triggerLed(codStatus.toInt());
  }

}
