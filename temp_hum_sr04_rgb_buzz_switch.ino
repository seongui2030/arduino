// [테스트용: 스위치 입력 없이 자동 타이머 및 도트 매트릭스 카운트다운 코드]
#include <DHT.h>
#include <LedControl.h>

#define DHTPIN 2          // DHT11 온습도 센서 (D2)
#define DHTTYPE DHT11     

const int redPin = 9;     // RGB LED - 빨강 (D9)
const int greenPin = 10;  // RGB LED - 초록 (D10)
const int bluePin = A3;   // RGB LED - 파랑 (A3)
const int buzzerPin = 8;  // 피에조 부저 (D8)

const int muteSwitchPin = 4;   // 기존 음소거 스위치 (D4)
const int cdsPin = A0;         // 조도 센서 (A0)
const int trigPin = 5;         // 초음파 Trig (D5)
const int echoPin = 6;         // 초음파 Echo (D6)

// 도트 매트릭스 핀 (DIN=11, CLK=13, CS=12)
LedControl lc = LedControl(11, 13, 12, 1); 

DHT dht(DHTPIN, DHTTYPE);

const float tempThreshold = 28.0; 
const int lightThreshold = 300;   
const int distanceThreshold = 20; 

bool buzzerMuted = false;         
int lastMuteState = HIGH;       

// 타이머 및 카운트다운 변수 (시작하자마자 자동으로 활성화)
bool timerActive = true;         
unsigned long timerStartTime = 0; 
bool countdownActive = false;     
int currentCountdown = 9;         
unsigned long lastCountdownTime = 0;

// 8x8 도트 매트릭스 숫자 폰트 (0~9)
byte numbers[10][8] = {
  {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C}, // 0
  {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E}, // 1
  {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x66, 0x7E}, // 2
  {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x06, 0x66, 0x3C}, // 3
  {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x0C}, // 4
  {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x06, 0x66, 0x3C}, // 5
  {0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C}, // 6
  {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30}, // 7
  {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x66, 0x3C}, // 8
  {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x66, 0x3C}  // 9
};

void displayDigit(int digit) {
  if (digit < 0 || digit > 9) return;
  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, numbers[digit][i]);
  }
}

void clearMatrix() {
  lc.clearDisplay(0);
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  pinMode(muteSwitchPin, INPUT_PULLUP);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 도트 매트릭스 초기화
  lc.shutdown(0, false);
  lc.setIntensity(0, 8); 
  clearMatrix();

  timerStartTime = millis(); // 전원 켜지자마자 타이머 시작 시간 기록
}

void loop() {
  // 1. 초음파 거리 측정
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;

  // 2. 온습도 및 조도 값 읽기
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int rawCds = analogRead(cdsPin);

  // 3. 음소거 스위치 (D4)
  int currentMuteState = digitalRead(muteSwitchPin);
  if (lastMuteState == HIGH && currentMuteState == LOW) {
    buzzerMuted = !buzzerMuted; 
    delay(50);
  }
  lastMuteState = currentMuteState;

  // 4. 자동 타이머 및 도트 매트릭스 카운트다운 처리
  String timerStatus = "대기 중";
  if (timerActive) {
    unsigned long elapsedSeconds = (millis() - timerStartTime) / 1000;

    if (!countdownActive) {
      clearMatrix();
      if (elapsedSeconds >= 10) {
        countdownActive = true;       
        currentCountdown = 9;         
        lastCountdownTime = millis();
        displayDigit(currentCountdown);
      } else {
        timerStatus = String(10 - elapsedSeconds) + "초 후 카운트다운";
      }
    } 
    else {
      timerStatus = "카운트다운 중: " + String(currentCountdown);
      displayDigit(currentCountdown);

      if (millis() - lastCountdownTime >= 1000) {
        lastCountdownTime = millis();
        currentCountdown--;

        if (currentCountdown < 0) {
          tone(buzzerPin, 1500, 1000); // 0이 되면 1초간 경보음
          // 카운트다운이 끝나고 다시 처음부터 반복되게 하려면 아래 주석을 해제하세요
          // timerStartTime = millis();
          // countdownActive = false;
          // currentCountdown = 9;
          timerActive = false;
          clearMatrix();
        }
      }
    }
  }

  // 5. LED 및 부저 제어 로직
  String ledStatus = "";
  String buzzerStatus = "";

  if (distance > 0 && distance <= distanceThreshold) {
    setColor(255, 0, 0); 
    ledStatus = "빨강 (20cm 이내 접근)";
    if (!buzzerMuted && !timerActive) {
      tone(buzzerPin, 1200, 150); 
    }
  }
  else if (!isnan(temperature) && temperature >= tempThreshold) {
    setColor(255, 0, 0); 
    ledStatus = "빨강 (온도 경고)";
    if (!buzzerMuted && !timerActive) {
      tone(buzzerPin, 1000, 300); 
    }
  } 
  else if (rawCds < lightThreshold) {
    setColor(255, 255, 0); 
    ledStatus = "노란색 (야간 모드)";
  } 
  else {
    setColor(0, 255, 0); 
    ledStatus = "초록색 (안전)";
  }

  // 6. 시리얼 모니터 출력
  Serial.print("거리: ");
  Serial.print(distance);
  Serial.print("cm | 타이머: ");
  Serial.print(timerStatus);
  Serial.print(" | 매트릭스 숫자: ");
  Serial.println(countdownActive ? String(currentCountdown) : "-");

  delay(100);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}