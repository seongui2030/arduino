// [아두이노 온습도 센서 & RGB LED 통합 코드]
// 핀 설정: RGB LED (R:9, G:10, B:11) / 디지털 온습도 센서 (S: 2번 핀)

#include <DHT.h>

#define DHTPIN 2          // DHT11 온습도 센서 데이터 핀 (디지털 2번)
#define DHTTYPE DHT11     // 센서 종류

const int redPin = 9;     // RGB LED - 빨강 (D9)
const int greenPin = 10;  // RGB LED - 초록 (D10)
const int bluePin = 11;   // RGB LED - 파랑 (D11)

DHT dht(DHTPIN, DHTTYPE);

const float tempThreshold = 28.0; // 경고 기준 온도 (28도 이상 시 빨간 불)

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
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

  // 센서 읽기 오류 확인
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT 센서에서 값을 읽어오지 못했습니다. 배선을 확인해주세요!");
    return;
  }

  // 3. 온도에 따른 RGB LED 제어 및 상태 문자열 설정
  String ledStatus = "";
  if (temperature >= tempThreshold) {
    setColor(255, 0, 0); // 28도 이상: 빨간색 (경고)
    ledStatus = "빨강 (경고/위험)";
  } else {
    setColor(0, 255, 0); // 28도 미만: 초록색 (안전)
    ledStatus = "초록 (안전/쾌적)";
  }

  // 4. 시리얼 모니터 출력 (실행 시간 + 온습도 + LED 상태)
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
  Serial.print(" °C  |  습도: ");
  Serial.print(humidity);
  Serial.print(" %  |  LED 상태: ");
  Serial.println(ledStatus);
}

// RGB 색상 제어 함수 (공통 캐소드 기준)
void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}