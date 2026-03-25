//================================================================
// IoT 캡톤디자인: 아두이노 우노 최종 통합 코드
// 아두이노 우노 - 센서 감지, LED/부저 제어, 라즈베리파이로 데이터 송신
//================================================================

#include <SoftwareSerial.h> // 소프트웨어 시리얼

// --- 핀 번호 및 상수 정의 ---
// LED 핀
#define LED_R_PIN 9
#define LED_G_PIN 10
#define LED_B_PIN 11

#define PIR_PIN 4       // PIR 동작 감지 센서
#define MAGNETIC_PIN 6  // 문 열림 감지 센서
#define BUTTON_PIN 7    // 응급 버튼
#define BUZZER_PIN 5    // 부저 (액티브 부저)

// 블루투스 (RX:3, TX:2)
SoftwareSerial BTSerial(3, 2);

// 상수
const long BAUD_RATE = 9600;
const unsigned long MOTION_TIMEOUT = 2000; // 10초

// --- 전역 변수 선언 ---
int magneticState = 1;
int pirState = 0;
int lastMagneticState = 1;
int lastPirState = 0;
unsigned long lastMotionTime = 0;

// 응급 상황 변수
bool isEmergencyActive = false; // 응급 상황 모드 (토글)
int lastButtonState = HIGH;      // 버튼 눌림 감지용 (PULLUP)

// --- LED 제어 함수 ---
void setLedColor(int r, int g, int b) {
  analogWrite(LED_R_PIN, 255 - r);
  analogWrite(LED_G_PIN, 255 - g);
  analogWrite(LED_B_PIN, 255 - b);
}

// --- 초기 설정 함수: setup() ---
void setup() {
  Serial.begin(BAUD_RATE);
  BTSerial.begin(BAUD_RATE);

  // 핀 모드 설정
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); // 부저 핀 출력 설정
  pinMode(MAGNETIC_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 버튼 내부 풀업 사용

  Serial.println("Arduino Uno Final Code Ready (No LCD)");
  lastMotionTime = millis();
}

// --- 메인 루프 함수: loop() ---
void loop() {
  unsigned long currentTime = millis();

  // --- 응급 버튼 확인 및 부저 제어 ---
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    isEmergencyActive = !isEmergencyActive; // 응급 모드 토글
    delay(50); // 디바운싱

    if (isEmergencyActive) {
      BTSerial.println("EMERGENCY");
      Serial.println("EMERGENCY Signal Sent!");
    } else {
      BTSerial.println("SAFE");
      Serial.println("SAFE Signal Sent!");
    }
  }
  lastButtonState = buttonState;

  // 응급 버튼은 '부저'만 제어
  if (isEmergencyActive) {
    digitalWrite(BUZZER_PIN, HIGH); // 부저 켜기
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // 부저 끄기
  }

  // --- 센서 값 읽기 (항상 실행) ---
  magneticState = digitalRead(MAGNETIC_PIN);
  pirState = digitalRead(PIR_PIN);

  // --- LED 상태 제어 (항상 실행) ---
  // (응급 버튼과 상관없이 PIR/Door 상태에 따라 LED 색상 결정)
  if (magneticState == LOW) { // 문이 열렸다면
    setLedColor(0, 0, 255); // 파란색
  } 
  else { // 문이 닫혀있다면
    if (pirState == HIGH) { // 움직임 감지
      setLedColor(0, 255, 0); // 초록색
      lastMotionTime = currentTime;
    } else { // 움직임 없음
      if (currentTime - lastMotionTime > MOTION_TIMEOUT) {
        setLedColor(255, 0, 0); // 빨간색 (주의)
      } else {
        setLedColor(0, 255, 0); // 초록색 (이전 상태 유지)
      }
    }
  }

  // --- 라즈베리파이로 데이터 전송 (항상 실행) ---
  if (pirState != lastPirState || magneticState != lastMagneticState) {
    String dataToSend = String(pirState) + "," + String(magneticState);
    
    BTSerial.println(dataToSend);
    Serial.println(dataToSend);
    
    lastPirState = pirState;
    lastMagneticState = magneticState;
  }
}
