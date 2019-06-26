boolean getServerTime() {
  char server[] = "192.168.3.88";
  String result = "";
  WiFiEspClient client;

  if (client.connect(server, 30008)) {
    client.print("getTime");
    delay(100);
  }
  while (client.available()) {
    char c = client.read();
    result += c;
  }
  if (!client.connected()) {
    client.stop();
  }
  client.stop();

  char *end;

  long serverTime = static_cast<long>(strtol(result.c_str(), &end, 10));

  if (serverTime != 0) {
    struct tm *t;
    serverTime += 28800;

    t = localtime(&serverTime);

    Time t2(t->tm_year + 1870, t->tm_mon + 1, t->tm_mday + 1, t->tm_hour, t->tm_min, t->tm_sec, t->tm_wday + 1);
    rtc.time(t2);
    return true;
  }
  return false;
}

void displayTime(boolean dp, int hm = 0) {
  Time t = rtc.time();
  tm.setPoint(7, dp);
  tm.setPoint(6, dp);
  tm.setPoint(4, dp);
  tm.setPoint(2, dp);

  //tm.displayHex(7, getPositionInt(hm, 2));
  tm.displayHex(6, getPositionInt(t.sec, 1));
  tm.displayHex(5, getPositionInt(t.sec, 2));
  tm.displayHex(4, getPositionInt(t.min, 1));
  tm.displayHex(3, getPositionInt(t.min, 2));
  tm.displayHex(2, getPositionInt(t.hr, 1));
  tm.displayHex(1, getPositionInt(t.hr, 2));
}
int getNowLedIndex(float _time1, float _time2) {
  float bfb = _time1 / _time2;
  if (bfb > 0.896) {
    return 7;
  } else if (bfb > 0.75) {
    return 6;
  } else if (bfb > 0.625) {
    return 5;
  } else if (bfb > 0.5) {
    return 4;
  } else if (bfb > 0.375) {
    return 3;
  } else if (bfb > 0.25) {
    return 2;
  } else if (bfb > 0.125) {
    return 1;
  } else {
    return 0;
  }
}
