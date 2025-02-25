#include <Arduino.h>
#include <AsyncUDP.h>
#include <M5AtomicMotion.h>
#include <M5Unified.h>
#include <MadgwickAHRS.h>
#include <WiFi.h>

// 通信の設定(WiFi, UDP)
const char *ssid = "ssid";
const char *password = "password";

AsyncUDP udp;
uint16_t teleplot_port = 47269;
uint16_t rover_port = 5000;

// Madgwickフィルタ
Madgwick madgwick;

// モータードライバ
M5AtomicMotion atomicMotion;

void motorControl(int8_t left, int8_t right) {
  atomicMotion.setServoAngle(0, 90 + left);
  atomicMotion.setServoAngle(2, 90 + right);
}

// PID制御パラメータ
float targetAngle = 88.0;
float Kp = 10.0;
float Ki = 0.0;
float Kd = 0.5;

// PID制御パラメータの受信
void udpRecvCallback(AsyncUDPPacket &packet) {
  if (packet.length() != 16) return;
  uint8_t data[16];
  packet.read(data, 16);

  float dataf[4];
  memcpy(dataf, data, sizeof(dataf));

  targetAngle = dataf[0];
  Kp = dataf[1];
  Ki = dataf[2];
  Kd = dataf[3];

  M5.Display.setCursor(0, 0);
  M5.Display.printf("target:%7.3f\n", targetAngle);
  M5.Display.printf("Kp    :%7.3f\n", Kp);
  M5.Display.printf("Ki    :%7.3f\n", Ki);
  M5.Display.printf("Kd    :%7.3f\n", Kd);
}

// core0で実行される関数(主にmadgwickの更新)
void core0(void *pvParameters);

// core1で実行される関数(主にPID制御)
void core1(void *pvParameters);
bool core1Enable = false;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  USBSerial.begin(115200);

  M5.Display.setFont(&fonts::AsciiFont8x16);

  // WiFi接続
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // UDPの初期化
  if (udp.listen(IP_ANY_TYPE, rover_port)) {
    udp.onPacket(udpRecvCallback);
  }

  // モータードライバの初期化
  while (!atomicMotion.begin(&M5.In_I2C, M5_ATOMIC_MOTION_I2C_ADDR, SDA, SCL, 100000)) {
    delay(1000);
  }
  motorControl(0, 0);

  xTaskCreateUniversal(core0, "core0", 8192, NULL, 1, NULL, 0);
  // xTaskCreateUniversal(core1, "core1", 8192, NULL, 1, NULL, 1);
}

// メインループ
void loop() {}

void core0(void *pvParameters) {
  // Madgwickフィルタの初期化
  uint32_t madgwickSampleCycle = 10;  // [ms]
  madgwick.begin(1000 / madgwickSampleCycle);

  // UDPパケット
  AsyncUDPMessage msg;

  uint32_t prevMillis = millis();

  // ループ
  while (1) {
    M5.update();
    M5.Imu.update();
    if (millis() - prevMillis >= madgwickSampleCycle) {
      prevMillis = millis();
      auto data = M5.Imu.getImuData();
      madgwick.updateIMU(data.gyro.x, data.gyro.y, data.gyro.z, data.accel.x, data.accel.y, data.accel.z);

      msg.flush();
      msg.printf("3D|attitude:S:cube:P:0:0:0:Q:%f:%f:%f:%f:H:3:W:2:D:1:C:#9F9F9F",  //
                 madgwick.getQ1(), madgwick.getQ2(), madgwick.getQ3(), madgwick.getQ0());
      udp.broadcastTo(msg, teleplot_port);

      msg.flush();
      msg.printf("targetAngle:%f\nroll:%f\nKp:%f\nKi:%f\nKd:%f", targetAngle, madgwick.getRoll(), Kp, Ki, Kd);
      udp.broadcastTo(msg, teleplot_port);

      delay(1);
    }
    if (M5.BtnA.wasPressed() && !core1Enable) {
      xTaskCreateUniversal(core1, "core1", 8192, NULL, 1, NULL, 1);
      core1Enable = true;
    }
  }
}

void core1(void *pvParameters) {
  float dt, roll, P, I, D, prevP, U;

  I = 0;

  // UDPパケット
  AsyncUDPMessage msg;

  uint32_t prevMicros = micros();

  while (1) {
    dt = (micros() - prevMicros) / 1000000.0;
    prevMicros = micros();

    roll = madgwick.getRoll();
    P = targetAngle - roll;
    I += P * dt;
    D = (P - prevP) / dt;
    prevP = P;

    U = Kp * P + Ki * I + Kd * D;

    U = constrain(U, -90, 90);

    // アンチワインドアップ
    if (abs(I) > 100) {
      I = 0;
    }

    if ((roll < 30) || (150 < roll)) {
      U = 0;
    }

    motorControl(int8_t(U), int8_t(-U));

    msg.flush();
    msg.printf("dt:%f\nU:%d", dt, int8_t(U));
    udp.broadcastTo(msg, teleplot_port);

    delay(1);
  }
}