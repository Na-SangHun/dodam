//================================================================
// IoT 캡스톤디자인: 비접촉식 고독사 방지 시스템
// 역할: 아두이노 메가 - 환경 센서 데이터 수집 및 전송 허브 (가스 감지 알람 추가)
//================================================================

// --- 1. 라이브러리 포함 ---
#include <DHT.h>      // 온습도 센서 라이브러리
#include <Wire.h>     // I2C 통신 라이브러리   // 조도 센서 라이브러리
#include <MQ135.h>    // 유해가스 센서 라이브러리
#include <Servo.h> // 서브모터 센서 라이브러리

// --- 2. 핀 번호 및 상수 정의 ---
// 센서 핀
#define DHT11_PIN      2      // DHT11 센서 DATA 핀
#define MQ135_PIN      A12    // MQ135 센서 AOUT 핀
#define SERVO_PIN      4 // 서브모터 핀 
// 통신 및 주기 설정
const long BAUD_RATE = 9600;
const int SENSOR_READ_PERIOD = 2000; // 2초 주기로 데이터 전송

// --- 3. 센서 객체 및 전역 변수 선언 ---
// 센서 객체
DHT dht11(DHT11_PIN, DHT11);
MQ135 mq135(MQ135_PIN);
Servo myservo;

// 센서 값 저장 변수
int TEMP = 0;
int RH = 0;
int GAS = 0;

// 비동기 처리용 시간 변수
unsigned long t_now = 0;
unsigned long t_prev_sensor = 0;

// --- 4. 초기 설정 함수: setup() ---
void setup() {
  Serial.begin(BAUD_RATE);      // PC와의 통신 시작 (디버깅용)
  Serial1.begin(BAUD_RATE);     // 라즈베리파이(HC-06)와의 통신 시작
  

  // 센서 초기화
  dht11.begin();
  Wire.begin();
  myservo.attach(SERVO_PIN);
  myservo.write(70);
  t_prev_sensor = millis();
}

// --- 5. 메인 루프 함수: loop() ---
void loop() {
  t_now = millis();

  // --- 2초 주기: 전체 센서 데이터 취합 및 전송 ---
  if (t_now - t_prev_sensor >= SENSOR_READ_PERIOD) {
    // 아날로그 및 I2C 센서 값 읽기
    TEMP = dht11.readTemperature();
    RH = dht11.readHumidity();

    if (isnan(TEMP) || isnan(RH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    
  
    GAS = mq135.getCorrectedPPM(TEMP, RH);

    // <<-- [추가된 부저 알람 로직] -->>
    // 가스 수치가 3000을 초과하는지 확인
    if (GAS >= 1180) {
      myservo.write(160);
      delay(10);
    } else {
      // 부저를 끔
      myservo.write(70);
      delay(10);
    }
    // ------------------------------------
    
    // 데이터 포장 (char 배열 사용)
    char data_buffer[50]; 
    sprintf(data_buffer, "%d,%d,%d",
            TEMP, RH,  GAS);

    // 데이터 전송
    Serial.println(data_buffer);
    Serial1.println(data_buffer);

    // 이전 동작시간 갱신
    t_prev_sensor = t_now;
  }
}