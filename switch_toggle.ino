// [아두이노 통합 코드: 온습도 센서 + 조도 센서 + RGB LED + 부저 + 토글 음소거 스위치]
#include <DHT.h>

#define DHTPIN 2          // DHT11 온습도 센서 데이터 핀 (디지털 2번)
#define DHTTYPE DHT11     // 센서 종류

const int redPin = 9;     // RGB LED - 빨강 (D9)
const int greenPin = 10;  // RGB LED - 초록 (D10)
const int bluePin = 11;   // RGB LED - 파랑 (D11)
const int buzzerPin = 8;  // 피에조 부저 (D8)
const int switchPin = 4;  // 토글 스위치 모듈 (D4)
const int cdsPin = A0;    // 조도 센서 아날로그 핀 (A0)

DHT dht(DHTPIN, DHTTYPE);

const float tempThreshold = 28.0; // 경고 기준 온도 (28도 이상 시 경고)
const int lightThreshold = 300;   // 조도 기준치 (어두워지면 낮아짐)

bool buzzerMuted = false;         // 현재 부저 음소거 상태 (false: 소리 켜짐, true: 음소거)
int lastSwitchState = HIGH;       // 스위치 이전 상태 기억용

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP); // 내부 풀업 저항 활성화 (모듈에 따라 필요)
}

void loop() {
  // 센서 안정화를 위해 2초 대기
  delay(2000);

  // 1. 아두이노 내부 가동 시간 계산 (시:분:초)
  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;

  // 2. 온습도 값 읽기
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT 센서에서 값을 읽어오지 못했습니다. 배선을 확인해주세요!");
    return;
  }

  // 3. 조도 센서 값 읽기
  int rawCds = analogRead(cdsPin);

  // 4. 토글 스위치 입력 감지 (버튼을 '누르는 순간'을 포착)
  int currentSwitchState = digitalRead(switchPin);
  
  // 이전에는 안 눌려있다가(HIGH) 방금 눌렸을 때(LOW)
  if (lastSwitchState == HIGH && currentSwitchState == LOW) {
    buzzerMuted = !buzzerMuted; // 상태를 반전시킴 (true <-> false)
    delay(50); // 간단한 채터링(노이즈) 방지 딜레이
  }
  lastSwitchState = currentSwitchState; // 현재 상태를 다음 루프에 전달

  String ledStatus = "";
  String buzzerStatus = "";

  // 5. 우선순위 제어: 온도 경고(28도 이상) 발생 시
  if (temperature >= tempThreshold) {
    setColor(255, 0, 0); // 빨간색 LED (위험/경고)
    ledStatus = "빨강 (온도 경고/위험)";

    // 음소거 상태가 아닐 때만 부저 울림
    if (!buzzerMuted) {
      tone(buzzerPin, 1000, 300); // 1000Hz 경고음
      buzzerStatus = "울림 (ON)";
    } else {
      noTone(buzzerPin);
      buzzerStatus = "음소거 켜짐 (OFF by 스위치)";
    }
  } 
  // 6. 온도가 정상일 때 조도 상태 확인
  else {
    noTone(buzzerPin);
    buzzerStatus = "정지 (OFF)";

    if (rawCds < lightThreshold) {
      // 어두워진 경우: 노란색 켜기 (빨강 + 초록 혼합)
      setColor(255, 255, 0); 
      ledStatus = "노란색 (어두움/야간 모드)";
    } else {
      // 밝은 평소 상태: 초록색 켜기
      setColor(0, 255, 0); 
      ledStatus = "초록색 (안전/쾌적)";
    }
  }

  // 7. 시리얼 모니터 통합 출력
  Serial.print("실행 시간 [ ");
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
  Serial.print(" ]  |  ");
  
  Serial.print("온도: ");
  Serial.print(temperature);
  Serial.print(" °C  |  조도: ");
  Serial.print(rawCds);
  Serial.print("  |  LED: ");
  Serial.print(ledStatus);
  Serial.print("  |  부저: ");
  Serial.print(buzzerStatus);
  Serial.print("  |  음소거모드: ");
  Serial.println(buzzerMuted ? "ON" : "OFF");
}

// RGB 색상 제어 함수
void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}