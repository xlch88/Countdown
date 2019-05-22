unsigned long _startTime = 0;
void setStartTime() {
  _startTime = millis();
}

long getPassSec() {
  return (millis() - _startTime) / 1000;
}

int getPositionInt(long number, long _position) {
  switch (_position) {
    case 1:
      _position = 1;
      break;
    case 2:
      _position = 10;
      break;
    case 3:
      _position = 100;
      break;
    case 4:
      _position = 1000;
      break;
    case 5:
      _position = 10000;
      break;
    case 6:
      _position = 100000;
      break;
    case 7:
      _position = 1000000;
      break;
    case 8:
      _position = 10000000;
      break;
    case 9:
      _position = 100000000;
      break;
    case 10:
      _position = 1000000000;
      break;
  }

  return number / _position % 10;
}

int split(char dst[][80], char* str, const char* spl)
{
  int n = 0;
  char *result = NULL;
  result = strtok(str, spl);
  while ( result != NULL )
  {
    strcpy(dst[n++], result);
    result = strtok(NULL, spl);
  }
  return n;
}


String result2keyid(int value) {
  Serial.print("KEY: ");
  Serial.println(value);
  if (smallIRL) {
    switch (value) {
      case 69:
        return "1";
        break;
      case 70:
        return "2";
        break;
      case 71:
        return "3";
        break;
      case 68:
        return "4";
        break;
      case 64:
        return "5";
        break;
      case 67:
        return "6";
        break;
      case 7:
        return "7";
        break;
      case 21:
        return "8";
        break;
      case 9:
        return "9";
        break;
      case 22:
        return "*";
        break;
      case 25:
        return "0";
        break;
      case 13:
        return "#";
        break;
      case 24:
        return "up";
        break;
      case 82:
        return "down";
        break;
      case 8:
        return "left";
        break;
      case 90:
        return "right";
        break;
      case 28:
        return "ok";
        break;
    }
  } else {
    switch (value) {
      case 66:
        return "1";
        break;
      case 67:
        return "2";
        break;
      case 15:
        return "3";
        break;
      case 30:
        return "4";
        break;
      case 29:
        return "5";
        break;
      case 28:
        return "6";
        break;
      case 24:
        return "7";
        break;
      case 69:
        return "8";
        break;
      case 76:
        return "9";
        break;
      case 9:
        return "*";
        break;
      case 86:
        return "0";
        break;
      case 21:
        return "#";
        break;
      case 23:
        return "up";
        break;
      case 13:
        return "down";
        break;
      case 12:
        return "left";
        break;
      case 5:
        return "right";
        break;
      case 2:
        return "ok";
        break;
    }
  }
  return String(value);
}
