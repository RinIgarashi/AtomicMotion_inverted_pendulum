#include <Arduino.h>
#include <AsyncUDP.h>
#include <M5ModuleHMI.h>
#include <M5Unified.h>
#include <WiFi.h>

// 通信の設定(WiFi, UDP)
const char *ssid = "ssid";
const char *password = "password";

uint16_t rover_port = 5000;

AsyncUDP udp;

// HMIモジュール
M5ModuleHMI hmi;

class HMIData {
 public:
  int32_t encVal;
  int32_t incVal;
  bool prevBtnS;
  bool prevBtn1;
  bool prevBtn2;
  bool currBtnS;
  bool currBtn1;
  bool currBtn2;
  void update() {
    encVal = hmi.getEncoderValue();
    incVal = hmi.getIncrementValue();
    prevBtnS = currBtnS;
    prevBtn1 = currBtn1;
    prevBtn2 = currBtn2;
    currBtnS = hmi.getButtonS();
    currBtn1 = hmi.getButton1();
    currBtn2 = hmi.getButton2();
  }
  bool btnSWasPressed() { return prevBtnS && !currBtnS; }
  bool btn1WasPressed() { return prevBtn1 && !currBtn1; }
  bool btn2WasPressed() { return prevBtn2 && !currBtn2; }
  bool btnSWasReleased() { return !prevBtnS && currBtnS; }
  bool btn1WasReleased() { return !prevBtn1 && currBtn1; }
  bool btn2WasReleased() { return !prevBtn2 && currBtn2; }
};

HMIData hmiData;

uint8_t incrementIndex = 0;
float increment[4] = {0.01, 0.1, 1.0, 10.0};

uint8_t editParamIndex = 1;
float editParam[4] = {90.0, 10, 0, 0.5};  // targetAngle, Kp, Ki, Kd

void sendUDP() {
  uint8_t msg[16];
  memcpy(msg, editParam, sizeof(editParam));
  udp.broadcastTo(msg, sizeof(msg), rover_port);
}

void setup() {
  auto cfg = M5.config();
  cfg.internal_spk = false;
  M5.begin(cfg);

  M5.Display.setFont(&fonts::AsciiFont8x16);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  hmi.begin(&Wire, HMI_ADDR, 21, 22, 100000);
  hmi.resetCounter();
}

void loop() {
  hmiData.update();

  if (hmiData.btnSWasPressed()) {
    incrementIndex = (incrementIndex + 1) % 4;
  }

  if (hmiData.btn1WasPressed()) {
    editParamIndex = (editParamIndex + 1) % 4;
  }

  editParam[editParamIndex] += increment[incrementIndex] * float(hmiData.incVal) / 5;

  if (!hmiData.currBtn2) {
    sendUDP();
    M5.Display.setCursor(0, 80);
    M5.Display.printf("Sent UDP packet");
  }

  M5.Display.setCursor(0, 0);
  M5.Display.printf("  TargetAngle:%7.3f\n", editParam[0]);
  M5.Display.printf("  Kp         :%7.3f\n", editParam[1]);
  M5.Display.printf("  Ki         :%7.3f\n", editParam[2]);
  M5.Display.printf("  Kd         :%7.3f\n", editParam[3]);

  M5.Display.printf(" (Increment  :%6.2f)\n", increment[incrementIndex]);

  M5.Display.setCursor(0, 16 * editParamIndex);
  M5.Display.print(">");

  delay(10);
}