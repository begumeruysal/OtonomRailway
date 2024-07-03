// Sabit tanımlamalar: Pin numaraları ve ölçüm aralıkları
int trigPin = 7;        // Ultrasonik sensörün trig pini
int echoPin = 6;        // Ultrasonik sensörün echo pini
int enA = 2;            // Motor sürücüsünün Enable A pin numarası
int enB = 3;
int in1 = 4;            // Motor sürücüsünün IN1 pin numarası
int in2 = 5;            // Motor sürücüsünün IN2 pin numarası
int in3 = 11 ;
int in4 = 12 ;
int buzzerPin = 8;
int led = 46;
int kapiLed = 10;       // Kapı LED tanımlaması
int maximumRange = 50;  // Maksimum algılama mesafesi (cm)
int minimumRange = 0;
int minDistance = 10;   // Motorun duracağı minimum mesafe (cm)
int S0 = 40;
int S1 = 41;
int S2 = 42;
int S3 = 43;
int sensorOut = 44;
int hallSensorPin = 38;  // Hall sensörünün bağlı olduğu pin
int esp32CamPin = 45;    // ESP32-CAM'den gelen sinyal için pin

 // Hall sensörünün başlangıç durumu 
int state,kirmizi_f, yesil_f, mavi_f;
String espSignal = "";

void setup() {
  // Pin modlarının ayarlanması
  pinMode(trigPin, OUTPUT);   // Trig pini çıkış olarak ayarlandı
  pinMode(echoPin, INPUT);    // Echo pini giriş olarak ayarlandı
  pinMode(enA, OUTPUT);       // Motor sürücüsünün Enable A pin modu ayarlandı
  pinMode(in1, OUTPUT);       // Motor sürücüsünün IN1 pin modu ayarlandı
  pinMode(in2, OUTPUT);       // Motor sürücüsünün IN2 pin modu ayarlandı
  pinMode(enB, OUTPUT);       // Motor sürücüsünün Enable A pin modu ayarlandı
  pinMode(in3, OUTPUT);       // Motor sürücüsünün IN1 pin modu ayarlandı
  pinMode(in4, OUTPUT); 
  pinMode(buzzerPin, OUTPUT); // Buzzer kontrol pini çıkış olarak ayarlanır
  pinMode(led, OUTPUT); //LED'in çıkış elemanı olduğunu belirtiyoruz
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT); 
  pinMode(hallSensorPin, INPUT);  // Hall sensör pinini giriş olarak ayarla
  pinMode(kapiLed, OUTPUT); // Kapı LED pinini çıkış olarak ayarla
  pinMode(esp32CamPin, INPUT); // ESP32-CAM sinyal pinini giriş olarak ayarla
  
  Serial.begin(9600); //9600 Baundluk bir seri haberleşme başlatıyoruz
  Serial1.begin(115200);
  // Motor sürücüsünün Enable A pininin başlangıç durumu
  digitalWrite(enA, HIGH);    // Enable A pinine yüksek seviye uygulandı
  digitalWrite(enB, HIGH); 
  digitalWrite(S0,HIGH); //%20
  digitalWrite(S1,LOW);
}

void loop() {
  // ESP32-CAM'den gelen sinyalleri oku
  int esp32CamState = digitalRead(esp32CamPin);
  Serial.print("ESP32-CAM sinyal durumu: "); // Sinyal durumunu kontrol etmek için
  Serial.println(esp32CamState);
  // Renk sensörü okumaları
  renkAlgila();

  state = digitalRead(hallSensorPin);
  int mesafe = olcumuHesapla();

    // Motor hızı ayarlanıyor
  //motorHiziAyarla(mesafe); 
   // Renk algılamaya göre hareketi belirle
  if (esp32CamState == 1 ||kirmizi_f < yesil_f && kirmizi_f < mavi_f && state == LOW) {
    // Kırmızı algılandı, dur
    motorDur();
    if (state == LOW) {
    // Hall efekti sensöründen LOW geliyorsa kapı LED'ini yak
      digitalWrite(kapiLed, HIGH);
      delay(5000); // 2 saniye bekleyin
      digitalWrite(kapiLed, LOW); // Beklemeden sonra LED'i söndür
    } else {
      digitalWrite(kapiLed, LOW);
    }
  }
  else if (yesil_f < kirmizi_f && yesil_f < mavi_f) {
    digitalWrite(kapiLed, LOW); 
    // Yeşil algılandı, mesafe kontrolü yap
    if (mesafe > minDistance) {
      motorHiziAyarla(mesafe);
    } else {
      motorDur(); // Engel varsa dur
    }
  } 
  else {
    // Diğer durumlar (mavi veya başka renk)
    motorHiziAyarla(mesafe);
    digitalWrite(kapiLed, LOW); 
  }

  int olculenMesafe = mesafeOlcul(maximumRange, minimumRange); // Mesafe ölçülür
  melodiCal(olculenMesafe * 10); // Ölçülen mesafeye göre melodi çalınır

  int isik = analogRead(A0); //Işık değişkenini A0 pinindeki LDR ile okuyoruz
  Serial.println(isik); //Okunan değeri seri iletişim ekranına yansıtıyoruz
  if (isik <=250) { //Okunan ışık değeri ...'den büyük ise
    digitalWrite(led, HIGH); //LED yanmasın
  }
  else  { //Okunan ışık değeri ...'den küçük ise
    digitalWrite(led, LOW); //LED yansın
  }


}

void renkAlgila() {
  // Kırmızı renk filtresi
  digitalWrite(S2, LOW);   
  digitalWrite(S3, LOW);     
  kirmizi_f = pulseIn(sensorOut, LOW);  
  kirmizi_f = map(kirmizi_f, 32, 287, 0, 100);  // Kalibrasyon değerleri
  Serial.print("Kirmizi= ");   
  Serial.print(kirmizi_f);   
  Serial.print("  ");  
  delay(100);      

  // Yeşil renk filtresi
  digitalWrite(S2, HIGH);   
  digitalWrite(S3, HIGH);     
  yesil_f = pulseIn(sensorOut, LOW);  
  yesil_f = map(yesil_f, 32, 339, 0, 100);  
  Serial.print("Yesil= ");   
  Serial.print(yesil_f);   
  Serial.print("  ");   
  delay(100);    

  // Mavi renk filtresi
  digitalWrite(S2, LOW);   
  digitalWrite(S3, HIGH);     
  mavi_f = pulseIn(sensorOut, LOW); 
  mavi_f = map(mavi_f, 10, 111, 0, 100);     
  Serial.print("Mavi= ");   
  Serial.print(mavi_f);   
  Serial.println("  ");   
  delay(100);   
}

void motorDur() {
  analogWrite(in1, 0);
  analogWrite(in2, 0);
  analogWrite(in3, 0);
  analogWrite(in4, 0);
}

// Ultrasonik sensör ile mesafe ölçümü yapar
int olcumuHesapla() {
  long sure;
  
  // Ultrasonik sensörün sinyal gönderme işlemi
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Echo pininden gelen sinyalin süresi ölçülüyor
  sure = pulseIn(echoPin, HIGH);
  
  // Süre mesafeye çevriliyor (cm)
  int mesafe = sure / 58.2;
  
  return mesafe;
}

// Motor hızını ayarlar
void motorHiziAyarla(int mesafe) {
  int motorHizi = 0;
  
  // Mesafe değerine göre motor hızını belirle
  if (mesafe >= maximumRange) {
   analogWrite(in1, 55);
    analogWrite(in2, 0); // Max hız
    analogWrite(in3, 55);
    analogWrite(in4, 0); // Max hız
  } else if (mesafe <= minDistance) {
    analogWrite(in1, 0);
    analogWrite(in2, 0);    // Dur
    analogWrite(in3, 0);
    analogWrite(in4, 0);    // Dur
  } else {
    // Minimum mesafe ve maksimum mesafe arasında hızı ayarla
    motorHizi = map(mesafe, minDistance, maximumRange, 25, 100);
    analogWrite(in1, motorHizi);
    analogWrite(in2, 0); 
    analogWrite(in3, motorHizi);
    analogWrite(in4, 0); 
  } 
}

int mesafeOlcul(int maksMesafe, int minMesafe) {
  long sure, mesafe;
  digitalWrite(trigPin, LOW);      // Tetik pini düşük yapılır
  delayMicroseconds(2);             // Kısa bir bekleme
  digitalWrite(trigPin, HIGH);      // Tetik pini yüksek yapılır
  delayMicroseconds(10);            // 10 mikrosaniye beklenir
  digitalWrite(trigPin, LOW);       // Tetik pini tekrar düşük yapılır
  sure = pulseIn(echoPin, HIGH);    // Yankı pininden yüksek sinyal alınıncaya kadar bekleme
  mesafe = sure / 58.2;              // Süreyi mesafeye dönüştürmek için hesaplama yapılır
  delay(50);                         // 50 milisaniye beklenir
  if (mesafe >= maksMesafe || mesafe <= minMesafe) // Ölçülen mesafe maksimum veya minimum aralıkta değilse
    return 0;                        // 0 döndürülür
  return mesafe;                     // Ölçülen mesafe döndürülür
}

void melodiCal(int gecikmeZamani) {
  tone(buzzerPin, 1140);            // Buzzer ile belirli frekansta ses çalınır
  delay(gecikmeZamani);             // Belirli bir süre beklenir
  noTone(buzzerPin);                // Buzzer sesi durdurulur
  delay(gecikmeZamani);             // Belirli bir süre daha beklenir
}

