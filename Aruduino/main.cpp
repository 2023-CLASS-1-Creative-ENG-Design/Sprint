@성민 #include<Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>

    bool isBluetoothConnected = false; // 자동 운전과 블루투스 제어 모드 전환을 위한 변수
SoftwareSerial bt_connection(5, 4);    // 블루투스 모듈 연결 핀
int throttle, steering, data, junk = 0;
bool end = false;
int speed[4] = {0, 85, 170, 255};

int sensor_trig_pin[2] = {3, 13}; // trig1,trig2   [0]:전방센서,[1]:하방센서
int sensor_echo_pin[2] = {2, 12}; // echo1,echo2
Servo MyServo1;                   // 전방센서 서브모터
int angle1 = 90;                  // 전방센서 서브모터 각도
int state = 10;                   // 서브모터 각도 단위
int speedIndex = 1;               // 서브 모터 속도 설정 인덱스

int Dir1Pin_A = 7;   // 모터A 방향 제어핀1
int Dir2Pin_A = 8;   // 모터A 방향 제어핀2
int SpeedPin_A = 6;  // 모터A 속도 제어 핀(PWM)
int Dir1Pin_B = 9;   // 모터B 방향 제어핀1
int Dir2Pin_B = 10;  // 모터B 방향 제어핀2
int SpeedPin_B = 11; // 모터B 속도 제어 핀(PWM)
int buzzor_pin = A4;

void stop()
{
    digitalWrite(Dir1Pin_A, LOW);
    digitalWrite(Dir2Pin_A, LOW);
    analogWrite(SpeedPin_A, 0);
    digitalWrite(Dir1Pin_B, LOW);
    digitalWrite(Dir2Pin_B, LOW);
    analogWrite(SpeedPin_B, 0);
}
void go_front()
{
    digitalWrite(Dir1Pin_A, HIGH);
    digitalWrite(Dir2Pin_A, LOW);
    analogWrite(SpeedPin_A, 170);
    digitalWrite(Dir1Pin_B, HIGH);
    digitalWrite(Dir2Pin_B, LOW);
    analogWrite(SpeedPin_B, 170);
}
void go_back()
{
    digitalWrite(Dir1Pin_A, LOW);
    digitalWrite(Dir2Pin_A, HIGH);
    analogWrite(SpeedPin_A, 170);
    digitalWrite(Dir1Pin_B, LOW);
    digitalWrite(Dir2Pin_B, HIGH);
    analogWrite(SpeedPin_B, 170);
}
void rotate_left()
{
    digitalWrite(Dir1Pin_A, LOW);
    digitalWrite(Dir2Pin_A, HIGH);
    analogWrite(SpeedPin_A, 170);
    digitalWrite(Dir1Pin_B, HIGH);
    digitalWrite(Dir2Pin_B, LOW);
    analogWrite(SpeedPin_B, 170);
}
void rotate_right()
{
    digitalWrite(Dir1Pin_A, HIGH);
    digitalWrite(Dir2Pin_A, LOW);
    analogWrite(SpeedPin_A, 170);
    digitalWrite(Dir1Pin_B, LOW);
    digitalWrite(Dir2Pin_B, HIGH);
    analogWrite(SpeedPin_B, 170);
}
// 해당 초음파센서 거리 측정 후 cm반환 함수
long dist(int trig, int echo)
{
    long duration, distance;
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    duration = pulseIn(echo, HIGH);
    distance = duration * 340 / 2 / 10000;
    return distance;
}
// 부저 소리 출력
void sound(int buzzer)
{
    tone(buzzer, 261.6);
    delay(100);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 311.1);
    delay(100);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 329.6);
    delay(100);
    noTone(buzzer);
    delay(100);
    tone(buzzer, 349.2);
    delay(100);
    noTone(buzzer);
    delay(100);
}
// 전방 센서로 장애물 감지 시 각도에 따른 처리
void movePattern()
{
    go_back();  // 후진
    delay(500); // 0.5초
    // 좌측에 장애물 감지 시 우회
    if (angle1 >= 90)
    {
        Serial.println("왼쪽 장애물 처리");
        rotate_right();
        delay(500);
    }
    // 우측 장애물 감지 시 우회
    else
    {
        Serial.println("오른쪽 장애물 처리");
        rotate_left();
        delay(500);
    }
    // 다시 서브모터 각도 정면으로 재조정
    angle1 = 90;
    MyServo1.write(angle1);
    delay(100);
}
void auto_drive()
{
    Serial.println("===================");
    MyServo1.write(angle1);
    delay(50);

    // 하부 센서 거리측정
    int lower_dist = dist(sensor_trig_pin[1], sensor_echo_pin[1]);
    Serial.println(lower_dist);
    // 하부 센서 바닥 미감지 시 장애물 우회
    if (lower_dist > 10 && lower_dist < 100)
    {
        // 후진 후 우회전
        sound(buzzor_pin);
        Serial.println("전방 낭떠러지");
        stop();
        delay(500);
        Serial.println("후진");
        go_back();
        delay(1000);
        Serial.println("우회전");
        rotate_right();
        delay(500);
    }
    else
        Serial.println("전방 낭떠러지 아님");

    // 전방 센서 거리측정
    int front_dist = dist(sensor_trig_pin[0], sensor_echo_pin[0]);
    Serial.println(front_dist);
    // 전방 센서 장애물 감지 시 장애물 우회
    if (front_dist > 0 && front_dist < 20)
    {
        sound(buzzor_pin);
        movePattern();
    }
    Serial.println("직진");
    go_front();
    // 회전 각도 조정
    if (angle1 == 140)
        state = -10;
    else if (angle1 == 40)
        state = 10;
    angle1 += state;
}
// 초기화
void setup()
{
    Serial.begin(9600);
    bt_connection.begin(9600);
    Serial.println("start");
    pinMode(Dir1Pin_A, OUTPUT);  // 모터A 방향 제어핀1
    pinMode(Dir2Pin_A, OUTPUT);  // 모터A 방향 제어핀2
    pinMode(SpeedPin_A, OUTPUT); // 모터A 속도 제어 핀(PWM)
    pinMode(Dir1Pin_B, OUTPUT);  // 모터B 방향 제어핀1
    pinMode(Dir2Pin_B, OUTPUT);  // 모터B 방향 제어핀2
    pinMode(SpeedPin_B, OUTPUT); // 모터B 속도 제어 핀(PWM)
    for (int i = 0; i < 2; i++)
    {
        pinMode(sensor_trig_pin[i], OUTPUT); // 초음파센서 trig,echo핀
        pinMode(sensor_echo_pin[i], INPUT);
    }
    pinMode(A0, OUTPUT);
    MyServo1.attach(A0); // 전방 센서 서브모터 연결 핀
    MyServo1.write(angle1);
    delay(1000);
}

// 블루투스 통신 원격 제어 함수
void control_with_bluetooth()
{
    // 어플리케이션 패드의 현재 throttle(앞뒤), steering(좌우)값 계산
    if (bt_connection.available())
    {
        data = bt_connection.read();
        if (data < 43 || data > 92)
        {
            throttle = 0;
            steering = 0;
        }
        else
        {
            throttle = ((data - 44) % 7) - 3;
            steering = ((data - 44) / 7) - 3;
        }
        int leftSpeed = constrain(abs(throttle + steering), 0, 3);
        int rightSpeed = constrain(abs(throttle - steering), 0, 3);

        if (throttle > 0)
        {
            // 전진 명령
            Serial.println("front");
            digitalWrite(Dir1Pin_A, HIGH);
            digitalWrite(Dir2Pin_A, LOW);
            digitalWrite(Dir1Pin_B, HIGH);
            digitalWrite(Dir2Pin_B, LOW);
            analogWrite(SpeedPin_A, speed[leftSpeed]);
            analogWrite(SpeedPin_B, speed[rightSpeed]);
        }
        else if (throttle < 0)
        {
            // 후진 명령
            Serial.println("back");
            digitalWrite(Dir1Pin_A, LOW);
            digitalWrite(Dir2Pin_A, HIGH);
            digitalWrite(Dir1Pin_B, LOW);
            digitalWrite(Dir2Pin_B, HIGH);
            analogWrite(SpeedPin_A, speed[leftSpeed]);
            analogWrite(SpeedPin_B, speed[rightSpeed]);
        }
        else if (steering < 0)
        {
            // 좌회전 명령
            Serial.println("left");
            digitalWrite(Dir1Pin_A, LOW);
            digitalWrite(Dir2Pin_A, HIGH);
            digitalWrite(Dir1Pin_B, HIGH);
            digitalWrite(Dir2Pin_B, LOW);
            analogWrite(SpeedPin_A, speed[leftSpeed]);
            analogWrite(SpeedPin_B, speed[rightSpeed]);
        }
        else if (steering > 0)
        {
            // 우회전 명령
            Serial.println("right");
            digitalWrite(Dir1Pin_A, HIGH);
            digitalWrite(Dir2Pin_A, LOW);
            digitalWrite(Dir1Pin_B, LOW);
            digitalWrite(Dir2Pin_B, HIGH);
            analogWrite(SpeedPin_A, speed[leftSpeed]);
            analogWrite(SpeedPin_B, speed[rightSpeed]);
        }
        else
        {
            // 정지 명령
            Serial.println("stop");
            digitalWrite(Dir1Pin_A, LOW);
            digitalWrite(Dir2Pin_A, LOW);
            digitalWrite(Dir1Pin_B, LOW);
            digitalWrite(Dir2Pin_B, LOW);
            analogWrite(SpeedPin_A, speed[leftSpeed]);
            analogWrite(SpeedPin_B, speed[rightSpeed]);
        }
    }
}

void loop()
{
    if (bt_connection.available())
    {
        isBluetoothConnected = true;
        control_with_bluetooth();
    }
    else
    {
        if (isBluetoothConnected)
        {
            // 블루투스 연결이 끊어진 경우
            isBluetoothConnected = false;
            stop();
        }
        else
        {
            auto_drive();
        }
    }
}