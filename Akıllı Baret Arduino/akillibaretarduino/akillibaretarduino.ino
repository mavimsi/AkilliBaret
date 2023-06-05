
#include "FirebaseESP8266.h"
#include <MPU6050.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <DFRobot_DHT11.h>

DFRobot_DHT11 DHT; //DHT nesnesi oluşturduk ısı nem için

MPU6050 sensor;  // sensor nesnesi oluşturduk düşme için

int16_t ivmeX , ivmeY , ivmeZ; //aşağı yukarı sağa sola hareket değerleri için
const int dusmedeger = 30000;  // Düşme eşik değeri


int DHT11_PIN=D0; //Isı nem sensör pini


#define gazsensorpin A0 // Gaz sensör pini
int gazsensordeger;


int trigPin=D5;  //Mesafe sensörü için
int echoPin=D6;


int buton=D7; //Buton için 
int butonAktif=0;


int buzzerPin=D4;  //buzzer ses çıkması için


//sensör verisinin geliş süresi ve aradaki mesafeyi göstermek için
long sure;
int uzaklik; 



#define FIREBASE_HOST "firebase database URL"
#define FIREBASE_AUTH "firebase database API anahtarı"
#define WIFI_SSID "bağlanacak wifi adresi"
#define WIFI_PASSWORD "bağlanacak wifi şifresi"

FirebaseData veritabanim;


void setup() {
 Serial.begin(115200);
 Wire.begin();
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Ağ Bağlantısı Oluşturuluyor");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("IP adresine bağlanıldı: ");
  Serial.println(WiFi.localIP());
  Serial.println();


 //Firebase bağlantısı başlatıyoruz. Bağlantı kesilirse tekrar bağlanmasını istiyoruz.

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.reconnectWiFi(true);


  sensor.initialize(); //ivme  sensörünü başlattık



//Ultrasonik mesafe sensörlerinin pinlerini ayarlıyoruz.
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);

//buzzer için pin ayarlaması
  pinMode(buzzerPin, OUTPUT); 

//buton için pin ayarlaması
  pinMode(buton,INPUT);

}

void loop() {


// İvme sensörü için kodlar

sensor.getAcceleration(&ivmeX, &ivmeY, &ivmeZ); //sensör değerlerini okuyarak değerlerini değişkenlere atadık.

 Serial.print("ivme X = ");
  Serial.println(ivmeX);

  Serial.print("ivme Y = ");
  Serial.println(ivmeY);

  Serial.print("ivme Z = ");
  Serial.println(ivmeZ);

  delay(1000);

  //eşik değerinden yüksek olunca

 if (abs(ivmeX)>dusmedeger || abs(ivmeY)>dusmedeger || abs(ivmeZ)>dusmedeger) {
    if(Firebase.setInt(veritabanim,"/dustu",1)){
        Serial.println("Integer tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("Integer tipinde veri gönderimi başarısız");
       }
    Serial.print("düşme var ");
    delay(25000);
    
  }else {  
    if(Firebase.setInt(veritabanim,"/dustu",3)){
        Serial.println("Integer tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("Integer tipinde veri gönderimi başarısız");
       }
   Serial.print("düşme yok ");
    }






//Buton içinkodlar

if(digitalRead(buton)== HIGH)
{
  Serial.print("Dikkat Tehlike Butonu!!!");
  digitalWrite(buzzerPin,HIGH);
  delay(50);
  digitalWrite(buzzerPin,LOW);
  delay(50);
  butonAktif=1;
   if(Firebase.setInt(veritabanim,"/butonbasili",butonAktif))
       {
        Serial.println("Integer tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("Integer tipinde veri gönderimi başarısız");
       }
 } else
      {
      Serial.print("Durum Normal!!!");
      digitalWrite(buzzerPin,LOW);
      butonAktif=0;
      if(Firebase.setInt(veritabanim,"/butonbasili",butonAktif))
       {
        Serial.println("Integer tipinde veri gönderimi başarılı");
       }
        else{ 
        Serial.println("Integer tipinde veri gönderimi başarısız");
        } 
 }



  
//Ultrasonik sensör için kodlar

digitalWrite(trigPin,LOW);
delayMicroseconds(2);
digitalWrite(trigPin,HIGH);
delayMicroseconds(10);
digitalWrite(trigPin,LOW);

sure = pulseIn(echoPin, HIGH);
uzaklik = sure*0.034/2;

Serial.print("Uzaklık:");
Serial.println(uzaklik);
Serial.print("cm");

delay(50);

if(Firebase.setInt(veritabanim,"/barettak",uzaklik)){
        Serial.println("Integer tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("Integer tipinde veri gönderimi başarısız");
       }

if(uzaklik<15 && uzaklik>0)
 {
  Serial.print("Baret takılı");
 }
  else{
      Serial.print("Takılı değil!!!");
      }






// Gaz sensörü için kodlar 300 gibi bir değer yeterli olacaktır.


  gazsensordeger=analogRead(gazsensorpin);

  Serial.print("Gaz Sensör Değeri: ");
  Serial.println(gazsensordeger);
  
  if(gazsensordeger>200)
 {
  Serial.print("Dikkat Tehlike!!!");
  digitalWrite(buzzerPin,HIGH); // değer büyükse ses çıkması için
  delay(100);
  digitalWrite(buzzerPin,LOW);
  delay(100);  
 }
  else{
      Serial.print("Durum Normal!!!");
      digitalWrite(buzzerPin,LOW);
      }

//Firebase için gaz sensör değeri
  if(Firebase.setInt(veritabanim,"/geazsensordeger",gazsensordeger)){
        Serial.println("String tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("String tipinde veri gönderimi başarısız");
       }



  



  //Isı nem sensör kodları 

  
  DHT.read(DHT11_PIN);
  Serial.print("temp:");
  Serial.print(DHT.temperature);
  Serial.print("  humi:");
  Serial.println(DHT.humidity);
  delay(1000);


//Isı belli bir dereceden büyükse uyarı
  if(DHT.temperature > 30)
  {
  Serial.print("Dikkat Tehlike!!!");
  digitalWrite(buzzerPin,HIGH); // değer büyükse ses çıkması için
  delay(100);
  digitalWrite(buzzerPin,LOW);
  delay(100);  
 }
  else{
      Serial.print("Durum Normal!!!");
      digitalWrite(buzzerPin,LOW);
      }


//Nem belli bir değerden büyükse uyarı
  if(DHT.humidity > 70)
  {
  Serial.print("Dikkat Tehlike!!!");
  digitalWrite(buzzerPin,HIGH); // değer büyükse ses çıkması için
  delay(100);
  digitalWrite(buzzerPin,LOW);
  delay(100);  
 }
  else{
      Serial.print("Durum Normal!!!");
      digitalWrite(buzzerPin,LOW);
      }
 

  //Firebase için ısı ve nem değeri gönderilen bölüm
  
if(Firebase.setInt(veritabanim,"/gelennemdeger",DHT.humidity)){
        Serial.println("String tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("String tipinde veri gönderimi başarısız");
       }

if(Firebase.setInt(veritabanim,"/gelenisideger",DHT.temperature)){
        Serial.println("String tipinde veri gönderimi başarılı");
       }
       else{ 
        Serial.println("String tipinde veri gönderimi başarısız");
       }

  
}
