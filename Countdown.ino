#define RELAY 21   //继电器控制引脚
#define IA1a 39
#define IA1b 38
#define Trig 41
#define Echo 42
#define RELAY1 18
#define RELAY2 32
#define RELAY3 A0

#include "LedControl.h"
#include "WiFiEsp.h"
#include "DS1302.h"
#include "TM1638lite.h"
#include <time.h>
//#include <TimerOne.h>
//#include <MsTimer2.h>
//#include "IRremote.h"
#include "IRLremote.h"

DS1302 rtc(39, 38, 37);
TM1638lite tm(36, 35, 34);
WiFiEspServer server(4848);
CNec IRLremote;
LedControl lc1 = LedControl(22, 24, 23, 4);
LedControl lc2 = LedControl(25, 27, 26, 4);
LedControl lc3 = LedControl(30, 28, 29, 4);
LedControl lc4 = LedControl(33, 31, 32, 4);

long time1 = 0;
long time2 = 0;
int mode = -1;
int reallyMode = -1; //mode = 3
int reallyMode2 = -1; //mode = 4

boolean isStart           = 0;
boolean isStartCamera     = 0;
boolean isStopCamera      = 0;
boolean isStartPWM        = 0;
boolean isStopPWM         = 0;
boolean doend             = 0;
boolean isDelaySound      = 0;
boolean isLedProgressBar  = 0;
boolean isTakeStart       = 0;
boolean isJoin            = 0;
boolean isEndSound        = 0;
long endSoundLoop         = 60;
boolean smallIRL          = 0;
boolean puted             = 0;

int menuIndex             = 0;
int menuMode              = 1;

int halfTime = 0;
int halfTime2 = 0;

int oldButtons = 0;

int clockTime[6] = {
  2019, /* Y */
  4,    /* M */
  30,   /* D */
  13,   /* H */
  48,   /* m */
  23    /* s */
};

boolean endSoundEd        = 1;
//int ms_100 = 0;


#include "char.h"
#include "helper.func.h"
#include "print.func.h"
#include "timeShow.h"

void setup() {
  Serial.begin(9600);
  Serial3.begin(115200);

  pinMode(IA1a, OUTPUT);
  pinMode(IA1b, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  pinMode(43, OUTPUT);
  pinMode(3, INPUT);
  pinMode(21, INPUT);
  pinMode(48, INPUT);

  digitalWrite(RELAY, HIGH);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(43, HIGH);

  initLED(lc1);
  initLED(lc2);
  initLED(lc3);
  initLED(lc4);

  if (!IRLremote.begin(2))
    Serial.println(F("You did not choose a valid pin."));

  initTimeShow();

  if (digitalRead(48) == HIGH) {
    initWifi();
  } else {
    printByte(lc4, 3, chars[29]);
    printByte(lc4, 2, chars[21]);
    printByte(lc4, 1, chars[21]);
    printByte(lc4, 0, chars[11]);
    printByte(lc3, 3, chars[27]);
    printByte(lc3, 2, chars[19]);
    printByte(lc3, 1, chars[13]);
    printByte(lc3, 0, chars[12]);
  }

  printByte(lc1, 3, chars[17]);
  printByte(lc1, 2, chars[18]);
  printByte(lc1, 1, chars[19]);
  printByte(lc1, 0, chars[20]);
}

long _irrecv_startTime = 0;
long _doing_startTime = 0;
long _menu_startTime = 0;
long _endsound_startTime = 0;
long _buttons_startTime = 0;
void loop() {
  long now = millis();

  //TCPServer();

  if (now - _doing_startTime >= 500) {
    _doing_startTime = now;
    doing();
  }

  if (IRLremote.available()) {
    auto data = IRLremote.read();
    if (data.command != 0) {
      keydown(result2keyid(data.command));
    }
  }

  if (now - _buttons_startTime >= 50) {
    _buttons_startTime = now;
    int buttons = tm.readButtons();
    if (buttons != 0 && oldButtons != buttons) {
      switch (buttons) {
        case 192:
          tm.reset();
          tm.displayText("Load...");
          delay(500);
          if (getServerTime()) {
            tm.displayText("SUCCESS");
          } else {
            tm.displayText("FAILED");
          }
          delay(500);
          tm.reset();
          break;

        case 3:
          tm.reset();
          if (smallIRL = !smallIRL) {
            tm.displayText("LITE");
          } else {
            tm.displayText("FULL");
          }
          delay(500);
          tm.reset();
          break;
        case 1:
          keydown("up");
          break;
        case 2:
          keydown("down");
          break;
        case 4:
          keydown("left");
          break;
        case 8:
          keydown("right");
          break;
        case 16:
          keydown("ok");
          break;
        case 32:
          keydown("+10");
          break;
        case 64:
          keydown("-10");
          break;
        case 128:
          keydown("*");
          break;
      }
    }
    oldButtons = buttons;
  }

  if (endSoundEd == 0) {
    if (halfTime) {
      digitalWrite(43, LOW);
    } else {
      digitalWrite(43, HIGH);
    }
    if (now - _endsound_startTime >= endSoundLoop * 1000) {
      endSoundEd = 1;
      digitalWrite(43, HIGH);
    }
  }

  if (mode > 0) {
    if (mode == 3) {
      if (!puted && digitalRead(40) == LOW) { //等待放入磁铁
        printByte(lc4, 3, chars[16]);
        printByte(lc4, 2, chars[15]);
        printByte(lc4, 1, chars[20]);
        printByte(lc4, 0, chars[11]);
      } else if (!puted && digitalRead(40) == HIGH) { //已经放入磁铁
        puted = 1;
      } else if (puted && digitalRead(40) == LOW) { //拿走了磁铁
        mode = reallyMode;
        initTime(mode, String(time1).c_str(), 0, isStartPWM, isStopPWM, isStartCamera, isStopCamera, 0, isDelaySound, isLedProgressBar, isEndSound, endSoundLoop);
        return;
      } else if (puted && digitalRead(40) == HIGH) { //等待拿走
        printByte(lc4, 3, chars[20]);
        printByte(lc4, 2, chars[18]);
        printByte(lc4, 1, chars[26]);
        printByte(lc4, 0, chars[12]);
      }

      delay(50);
      printByte(lc1, 3, chars[10]);
      delay(50);
      printByte(lc1, 3, chars[11]);

    } else if (mode == 4) {
      digitalWrite(Trig, LOW);
      delayMicroseconds(2);
      digitalWrite(Trig, HIGH);
      delayMicroseconds(10);
      digitalWrite(Trig, LOW);

      float cm = (float(pulseIn(Echo, HIGH, 1000000)) * 17 ) / 1000;

      printNumber(lc2, cm);

      if (cm < 20 && cm != 0) {
        if (isStartCamera) {
          isStartCamera = 0;
          camera();
        }
        if (reallyMode2 == 3) {
          initTime(reallyMode, String(time1).c_str(), 1, isStartPWM, isStopPWM, isStartCamera, isStopCamera, 0, isDelaySound, isLedProgressBar, isEndSound, endSoundLoop);
        } else {
          initTime(reallyMode2, String(time1).c_str(), 0, isStartPWM, isStopPWM, isStartCamera, isStopCamera, 0, isDelaySound, isLedProgressBar, isEndSound, endSoundLoop);
        }
        return;
      } else {
        delay(50);
        printByte(lc1, 3, chars[10]);
        delay(50);
        printByte(lc1, 3, chars[11]);
      }
    } else {
      if ((mode == 1 || mode == 5) && doend) { //倒计时
        mode = -1; //END

        printByte(lc1, 2, chars[12]);
        printByte(lc1, 1, chars[13]);
        printByte(lc1, 0, chars[14]);

        if (isStopCamera) {
          delay(2000);
          camera();
        }

        if (isStopPWM) {
          digitalWrite(IA1a, LOW);
          //analogWrite(IA1b, 60);
          digitalWrite(IA1b, HIGH);
          delay(2000);
          digitalWrite(IA1b, LOW);
        }

        if (isEndSound) {
          endSoundEd = 0;
          _endsound_startTime = millis();
        }
      }
    }
  } else if (mode == -2) {
    if (now - _menu_startTime >= 200) {
      _menu_startTime = now;
    }

    if (halfTime) {
      setMenuHover(menuIndex, 0);
      lc3.setIntensity(2, 0);
      lc3.setIntensity(1, 0);
      lc3.setIntensity(0, 0);
    } else {
      setMenuHover(menuIndex, 1);
      lc3.setIntensity(2, 1);
      lc3.setIntensity(1, 1);
      lc3.setIntensity(0, 1);
    }
  }
}

void keydown(String key) {
  endSoundEd = 1;
  digitalWrite(43, LOW);
  delay(3);
  digitalWrite(43, HIGH);

  Serial.println(key);
  if (key == "*") {
    mode = -2;
  } else {
    if (mode == -2) {
      if (key == "up") {
        setMenuHover(menuIndex, 0);
        menuIndex--;
      } else if (key == "down") {
        setMenuHover(menuIndex, 0);
        menuIndex++;
      } else if ((key == "left" || key == "right") && menuIndex == 0) {
        if (key == "left")
          if (time1 != 0)
            time1 = time1 / 2;
        if (key == "right")
          time1 = time1 * 2;
      } else if ((key == "1" || key == "2" || key == "3" || key == "4" || key == "5" || key == "6" || key == "7" || key == "8" || key == "9" || key == "0") && menuIndex == 0) {
        time1 = time1 * 10;
        char* end;
        time1 += static_cast<int>(strtol(key.c_str(), &end, 10));
      } else if (key == "#") {
        if (time1 != 0)
          time1 = time1 / 10;
      } else if (key == "ok") {
        switch (menuIndex) {
          case 0:
            if (time1 == 0 && menuMode == 1) {
              printByte(lc4, 3, chars[12]);
              printByte(lc4, 2, chars[62]);
              printByte(lc4, 1, chars[62]);
              printByte(lc4, 0, chars[11]);
              delay(1000);
              break;
            }
            initTime(menuMode, String(time1).c_str(), isTakeStart, isStartPWM, isStopPWM, isStartCamera, isStopCamera, isJoin, isDelaySound, isLedProgressBar, isEndSound, endSoundLoop);
            return;
            break;
          case 1:
            if (menuMode == 1) {
              menuMode = 2;
            } else {
              menuMode = 1;
            }
            break;
          case 2:
            isStartPWM = !isStartPWM;
            break;
          case 3:
            isStopPWM = !isStopPWM;
            break;
          case 4:
            isStartCamera = !isStartCamera;
            break;
          case 5:
            isStopCamera = !isStopCamera;
            break;
          case 6:
            isJoin = !isJoin;
            break;
          case 7:
            isTakeStart = !isTakeStart;
            break;
          case 8:
            isDelaySound = !isDelaySound;
            break;
          case 9:
            isLedProgressBar = !isLedProgressBar;
            break;
          case 10:
            isEndSound = !isEndSound;
            break;
        }
      } else if (key == "+10") {
        time1 += 10;
      } else if (key == "-10") {
        time1 -= 10;
        if (time1 < 0) {
          time1 = 0;
        }
      }
    } else {
      return;
    }
  }

  if (menuIndex < 0) {
    menuIndex = 10;
  } else if (menuIndex > 10) {
    menuIndex = 0;
  }

  int page = 0;

  if (menuIndex > 9) {
    page = 1;
  } else page = 0;

  printNumber(lc4, time1);
  if (menuMode == 1) {
    printByte(lc3, 3, chars[38]);
  } else {
    printByte(lc3, 3, chars[37]);
  }

  if (page == 0) {
    printByte(lc2, 3, chars[1]);
    printByte(lc2, 2, chars[2]);
    printByte(lc2, 1, chars[3]);
    printByte(lc2, 0, chars[4]);
    printByte(lc1, 3, chars[5]);
    printByte(lc1, 2, chars[6]);
    printByte(lc1, 1, chars[7]);
    printByte(lc1, 0, chars[8]);

    if (isStartPWM)       lc2.setLed(3, 0, 0, 1);  else  lc2.setLed(3, 0, 0, 0);
    if (isStopPWM)        lc2.setLed(2, 0, 0, 1);  else  lc2.setLed(2, 0, 0, 0);
    if (isStartCamera)    lc2.setLed(1, 0, 0, 1);  else  lc2.setLed(1, 0, 0, 0);
    if (isStopCamera)     lc2.setLed(0, 0, 0, 1);  else  lc2.setLed(0, 0, 0, 0);
    if (isJoin)           lc1.setLed(3, 0, 0, 1);  else  lc1.setLed(3, 0, 0, 0);
    if (isTakeStart)      lc1.setLed(2, 0, 0, 1);  else  lc1.setLed(2, 0, 0, 0);
    if (isDelaySound)     lc1.setLed(1, 0, 0, 1);  else  lc1.setLed(1, 0, 0, 0);
    if (isLedProgressBar) lc1.setLed(0, 0, 0, 1);  else  lc1.setLed(0, 0, 0, 0);
  } else if (page == 1) {
    printByte(lc2, 3, chars[9]);
    printByte(lc2, 2, chars[11]);
    printByte(lc2, 1, chars[11]);
    printByte(lc2, 0, chars[11]);
    printByte(lc1, 3, chars[11]);
    printByte(lc1, 2, chars[11]);
    printByte(lc1, 1, chars[11]);
    printByte(lc1, 0, chars[11]);

    if (isEndSound)       lc2.setLed(3, 0, 0, 1);  else  lc2.setLed(3, 0, 0, 0);
  }

  switch (menuIndex) {
    case 0:
      printByte(lc3, 2, chars[59]);
      printByte(lc3, 1, chars[60]);
      printByte(lc3, 0, chars[61]);
      break;
    case 1:
      if (menuMode == 1) {
        printByte(lc3, 2, chars[31]);
        printByte(lc3, 1, chars[32]);
        printByte(lc3, 0, chars[33]);
      } else {
        printByte(lc3, 2, chars[34]);
        printByte(lc3, 1, chars[35]);
        printByte(lc3, 0, chars[36]);
      }
      break;
    case 2:
      printByte(lc3, 2, chars[39]);
      printByte(lc3, 1, chars[40]);
      printByte(lc3, 0, chars[41]);
      break;
    case 3:
      printByte(lc3, 2, chars[42]);
      printByte(lc3, 1, chars[40]);
      printByte(lc3, 0, chars[41]);
      break;
    case 4:
      printByte(lc3, 2, chars[43]);
      printByte(lc3, 1, chars[44]);
      printByte(lc3, 0, chars[45]);
      break;
    case 5:
      printByte(lc3, 2, chars[46]);
      printByte(lc3, 1, chars[44]);
      printByte(lc3, 0, chars[45]);
      break;
    case 6:
      printByte(lc3, 2, chars[47]);
      printByte(lc3, 1, chars[48]);
      printByte(lc3, 0, chars[49]);
      break;
    case 7:
      printByte(lc3, 2, chars[50]);
      printByte(lc3, 1, chars[51]);
      printByte(lc3, 0, chars[52]);
      break;
    case 8:
      printByte(lc3, 2, chars[53]);
      printByte(lc3, 1, chars[54]);
      printByte(lc3, 0, chars[55]);
      break;
    case 9:
      printByte(lc3, 2, chars[56]);
      printByte(lc3, 1, chars[57]);
      printByte(lc3, 0, chars[58]);
      break;
    case 10:
      printByte(lc3, 2, chars[63]);
      printByte(lc3, 1, chars[64]);
      printByte(lc3, 0, chars[65]);
      break;
  }
}
void setMenuHover(int index, boolean dp) {
  int level = 0;
  if (dp) {
    level = 1;
  } else {
    level = 0;
  }

  if (index > 9) {
    index -= 8;
  }

  switch (index) {
    case 0:
      lc4.setIntensity(0, level);
      lc4.setIntensity(1, level);
      lc4.setIntensity(2, level);
      lc4.setIntensity(3, level);
      break;
    case 1:
      lc3.setIntensity(3, level);
      break;
    case 2:
      lc2.setIntensity(3, level);
      break;
    case 3:
      lc2.setIntensity(2, level);
      break;
    case 4:
      lc2.setIntensity(1, level);
      break;
    case 5:
      lc2.setIntensity(0, level);
      break;
    case 6:
      lc1.setIntensity(3, level);
      break;
    case 7:
      lc1.setIntensity(2, level);
      break;
    case 8:
      lc1.setIntensity(1, level);
      break;
    case 9:
      lc1.setIntensity(0, level);
      break;
  }
}

void doing() {
  halfTime = !halfTime;
  halfTime2 = halfTime;
  displayTime((halfTime2 == 1));

  if ((mode == 1 || mode == 2 || mode == 5) && !doend) {
    long passSec = getPassSec();

    if (halfTime == 1) {
      if (mode == 1) { //倒计时
        printTime(lc4, time1 - passSec);
        printNumber(lc3, time1 - passSec);
        printNumber(lc2, passSec);
        if (time1 - passSec <= 0) {
          doend = 1;
        }
      } else if (mode == 2) {
        printTime(lc4, passSec);
        printNumber(lc3, passSec);
      } else if (mode == 5) {
        boolean isEnd = true;
        Time t = rtc.time();

        if (clockTime[0] != t.yr && clockTime[0] != -1) {
          isEnd = false;
        } else if (clockTime[1] != t.mon && clockTime[1] != -1) {
          isEnd = false;
        } else if (clockTime[2] != t.date && clockTime[2] != -1) {
          isEnd = false;
        } else if (clockTime[3] != t.hr && clockTime[3] != -1) {
          isEnd = false;
        } else if (clockTime[4] != t.min && clockTime[4] != -1) {
          isEnd = false;
        } else if (clockTime[5] != t.sec && clockTime[5] != -1 && t.sec < clockTime[5]) {
          isEnd = false;
        }

        if (isEnd) doend = 1;
      }
    }

    int LedIndex = getNowLedIndex(time1 - passSec, time1);
    if (!halfTime2) {
      if (mode == 1) {
        if (isLedProgressBar) {
          for (int leds = 0; leds <= 7; leds++) {
            if (leds < LedIndex) {
              tm.setLED(leds, 1);
            } else if (leds == LedIndex) {
              tm.setLED(leds, 1);
            } else {
              tm.setLED(leds, 0);
            }
          }
        }
      } else if (mode == 2) {
        if (isLedProgressBar) {
          for (int leds = 0; leds <= 7; leds++) {
            tm.setLED(leds, 0);
          }
        }
      }

      printByte(lc1, 3, chars[11]);

      if (mode == 1 || mode == 2)
        lc4.setLed(2, 0, 7, true);
      if (isDelaySound) {
        digitalWrite(RELAY, HIGH);
        Serial.println("滴");
      }
    } else {
      if (mode == 1) {
        if (isLedProgressBar) {
          for (int leds = 0; leds <= 7; leds++) {
            if (leds < LedIndex) {
              tm.setLED(leds, 1);
            } else if (leds == LedIndex) {
              tm.setLED(leds, 0);
            } else {
              tm.setLED(leds, 0);
            }
          }
        }
        printByte(lc1, 3, chars[38]);
      } else if (mode == 2) {
        if (isLedProgressBar) {
          for (int leds = 0; leds <= 7; leds++) {
            tm.setLED(leds, 1);
          }
        }
        printByte(lc1, 3, chars[37]);
      } else if (mode == 5) {
        printByte(lc1, 3, chars[67]);
      }

      if (mode == 1 || mode == 2)
        lc4.setLed(2, 0, 7, false);
      if (isDelaySound) {
        digitalWrite(RELAY, LOW);

        Serial.println("嗒");
      }
    }
  }
}

void TCPServer() {
  WiFiEspClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String webHeader = "";

    long _startMillis = millis();
    while (client.connected() && millis() - _startMillis < 1000) {
      if (client.available()) {
        char c = client.read();
        webHeader += c;

        if (c == '\n' && currentLineIsBlank) {
          char *str = webHeader.c_str();
          char dst[10][80];
          char* end;
          int strnum = split(dst, str, "|");

          if (strnum != 13) {
            Serial.print("FailstrNum:");
            Serial.println(strnum);
            client.stop();
            break;
          }

          initTime(
            static_cast<int>(strtol(dst[0], &end, 10)),
            dst[1],
            static_cast<int>(strtol(dst[2], &end, 10)),
            static_cast<int>(strtol(dst[3], &end, 10)),
            static_cast<int>(strtol(dst[4], &end, 10)),
            static_cast<int>(strtol(dst[5], &end, 10)),
            static_cast<int>(strtol(dst[6], &end, 10)),
            static_cast<int>(strtol(dst[7], &end, 10)),
            static_cast<int>(strtol(dst[8], &end, 10)),
            static_cast<int>(strtol(dst[9], &end, 10)),
            static_cast<int>(strtol(dst[10], &end, 10)),
            static_cast<int>(strtol(dst[11], &end, 10))
          );

          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    //delay(10);

    // close the connection:
    client.stop();
  }
}
void camera() {
  digitalWrite(RELAY1, LOW);
  delay(400);
  digitalWrite(RELAY1, HIGH);
}
void initTime(int smode, char* stime, int _isTakeStart, int _isStartPWM, int _isStopPWM, int _isStartCamera, int _isStopCamera, int _isJoin, int _isDelaySound, int _isLedProgressBar, int _isEndSound, int _endSoundLoop) {
  Serial.println(stime);
  mode = smode;

  char dst[10][80];
  char* end;
  if (mode == 5) {
    int strnum = split(dst, stime, ".");
    clockTime[0] = static_cast<int>(strtol(dst[0], &end, 10));
    clockTime[1] = static_cast<int>(strtol(dst[1], &end, 10));
    clockTime[2] = static_cast<int>(strtol(dst[2], &end, 10));
    clockTime[3] = static_cast<int>(strtol(dst[3], &end, 10));
    clockTime[4] = static_cast<int>(strtol(dst[4], &end, 10));
    clockTime[5] = static_cast<int>(strtol(dst[5], &end, 10));
  } else {
    time1 = static_cast<int>(strtol(stime, &end, 10));
  }

  time2 = 0;
  doend = 1;

  isStart = 0;
  isStartCamera = _isStartCamera;
  isStopCamera = _isStopCamera;
  isStartPWM = _isStartPWM;
  isStopPWM = _isStopPWM;
  isDelaySound = _isDelaySound;
  isLedProgressBar = _isLedProgressBar;
  isTakeStart = _isTakeStart;
  isJoin = _isJoin;
  isEndSound = _isEndSound;
  endSoundLoop = _endSoundLoop;

  digitalWrite(43, HIGH);
  endSoundEd = 1;

  if (!isLedProgressBar) {
    for (int leds = 0; leds <= 7; leds++) {
      tm.setLED(leds, 0);
    }
  }

  if (_isTakeStart) {
    reallyMode = mode;
    mode = 3;
  }
  if (_isJoin) {
    reallyMode2 = mode;
    mode = 4;
  }

  initLED(lc1);
  initLED(lc2);
  initLED(lc3);
  initLED(lc4);

  if (mode == 1) { //倒计时
    printByte(lc1, 2, chars[31]);
    printByte(lc1, 1, chars[32]);
    printByte(lc1, 0, chars[33]);
  } else  if (mode == 2) { //正计时
    printByte(lc1, 2, chars[34]);
    printByte(lc1, 1, chars[35]);
    printByte(lc1, 0, chars[36]);
  } else if (mode == 3) { //Take
    puted = 0;
  } else if (mode == 4) {
    printByte(lc4, 3, chars[17]);
    printByte(lc4, 2, chars[18]);
    printByte(lc4, 1, chars[19]);
    printByte(lc4, 0, chars[20]);

    printByte(lc3, 3, chars[30]);
    printByte(lc3, 2, chars[29]);
    printByte(lc3, 1, chars[19]);
    printByte(lc3, 0, chars[13]);
  } else if (mode == 5) {
    if (clockTime[0] != -1) {
      printByte(lc4, 3, chars[getPositionInt(clockTime[0], 4)]);
      printByte(lc4, 2, chars[getPositionInt(clockTime[0], 3)]);
      printByte(lc4, 1, chars[getPositionInt(clockTime[0], 2)]);
      printByte(lc4, 0, chars[getPositionInt(clockTime[0], 1)]);
    } else {
      printByte(lc4, 3, chars[66]);
      printByte(lc4, 2, chars[66]);
      printByte(lc4, 1, chars[66]);
      printByte(lc4, 0, chars[66]);
    }

    if (clockTime[1] != -1) {
      printByte(lc3, 3, chars[getPositionInt(clockTime[1], 2)]);
      printByte(lc3, 2, chars[getPositionInt(clockTime[1], 1)]);
    } else {
      printByte(lc3, 3, chars[66]);
      printByte(lc3, 2, chars[66]);
    }

    if (clockTime[2] != -1) {
      printByte(lc3, 1, chars[getPositionInt(clockTime[2], 2)]);
      printByte(lc3, 0, chars[getPositionInt(clockTime[2], 1)]);
    } else {
      printByte(lc3, 1, chars[66]);
      printByte(lc3, 0, chars[66]);
    }

    if (clockTime[3] != -1) {
      printByte(lc2, 3, chars[getPositionInt(clockTime[3], 2)]);
      printByte(lc2, 2, chars[getPositionInt(clockTime[3], 1)]);
    } else {
      printByte(lc2, 3, chars[66]);
      printByte(lc2, 2, chars[66]);
    }

    if (clockTime[4] != -1) {
      printByte(lc2, 1, chars[getPositionInt(clockTime[4], 2)]);
      printByte(lc2, 0, chars[getPositionInt(clockTime[4], 1)]);
    } else {
      printByte(lc2, 1, chars[66]);
      printByte(lc2, 0, chars[66]);
    }

    if (clockTime[5] != -1) {
      printByte(lc1, 1, chars[getPositionInt(clockTime[5], 2)]);
      printByte(lc1, 0, chars[getPositionInt(clockTime[5], 1)]);
    } else {
      printByte(lc1, 2, chars[68]);
      printByte(lc1, 1, chars[69]);
      printByte(lc1, 0, chars[70]);
    }

    lc4.setLed(0, 0, 7, 1);
    lc3.setLed(2, 0, 7, 1);
    lc3.setLed(0, 0, 7, 1);
    lc2.setLed(2, 0, 7, 1);
    lc2.setLed(0, 0, 7, 1);
    lc1.setLed(0, 0, 7, 1);


  }

  doend = 0;

  if (mode == 1 || mode == 2) {
    isStart = 1;
    if (isStartCamera && !_isStartCamera) {
      camera();
    }
    if (isStartPWM) {
      digitalWrite(IA1a, LOW);
      //analogWrite(IA1b, 60);
      digitalWrite(IA1b, HIGH);
      delay(2000 );
      digitalWrite(IA1b, LOW);
    }
  }

  setStartTime();
}
void initLED(LedControl lc) {
  int ledDevices = lc.getDeviceCount();
  for (int address = 0; address < ledDevices; address++) {
    lc.shutdown(address, false);
    lc.setIntensity(address, 0);
    lc.clearDisplay(address);
  }
}
void initWifi() {
  char ssid[] = "DowntimeTech_2.4G";
  char pass[] = "2817436a";

  printByte(lc4, 3, chars[19]);
  printByte(lc4, 2, chars[13]);
  printByte(lc4, 1, chars[19]);
  printByte(lc4, 0, chars[20]);

  WiFi.init(&Serial3);

  if (WiFi.status() == WL_NO_SHIELD) {
    printByte(lc3, 3, chars[21]);
    printByte(lc3, 2, chars[18]);
    printByte(lc3, 1, chars[20]);
    printByte(lc3, 0, chars[27]);

    while (true) {
      printByte(lc2, 3, chars[28]);
      delay(200);
      printByte(lc2, 3, chars[11]);
      delay(200);
    }
  } else {
    printByte(lc4, 3, chars[17]);
    printByte(lc4, 2, chars[19]);
    printByte(lc4, 1, chars[21]);
    printByte(lc4, 0, chars[19]);
  }


  int status = WL_IDLE_STATUS;     // the Wifi radio's status
  int tryCount = 0;
  while ( status != WL_CONNECTED) {
    tryCount++;
    printNumber(lc3, tryCount);
    printByte(lc3, 3, chars[24]);

    status = WiFi.begin(ssid, pass);
  }

  String ip = WiFi.localIP().toCharArray();
  char *str = ip.c_str();
  char dst[8][80];
  char* end;
  split(dst, str, ".");

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  printNumber(lc3, static_cast<int>(strtol(dst[2], &end, 10)));
  printNumber(lc2, static_cast<int>(strtol(dst[3], &end, 10)));
  printByte(lc3, 3, chars[25]);
  printByte(lc2, 3, chars[22]);

  server.begin();
}
void initTimeShow() {
  //Timer1.stop();

  Time t = rtc.time();


  for (int leds = 0; leds <= 7; leds++) {
    tm.setLED(leds, 0);
  }
  tm.reset();

  //  int s = t.sec;
  //  while (s == t.sec) {
  //    t = rtc.time();
  //  }
  //
  //  Timer1.initialize(100000);
  //  Timer1.attachInterrupt( doing );
}
