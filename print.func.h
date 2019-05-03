
void printByte(LedControl lc, int addr, byte character []){
  int i = 0;
  for (i = 0; i < 8; i++)
  {
    lc.setRow(addr, i, character[i]);
  }
}
void printNumber(LedControl lc, long number) {
  int s1 = getPositionInt(number, 4);
  int s2 = getPositionInt(number, 3);
  int s3 = getPositionInt(number, 2);
  int s4 = getPositionInt(number, 1);

  if (number >= 1000) {
    printByte(lc, 3, chars[s1]);
  } else {
    printByte(lc, 3, chars[11]);
  }
  if (number >= 100) {
    printByte(lc, 2, chars[s2]);
  } else {
    printByte(lc, 2, chars[11]);
  }
  if (number >= 10) {
    printByte(lc, 1, chars[s3]);
  } else {
    printByte(lc, 1, chars[11]);
  }
  if (number >= 10000) {
    int w = number / 10000;
    for (int a = 0; a < w; a++)
    {
      if (a <= 7) {
        lc.setLed(3, 0, a, true);
      } else break;
    }
  }
  printByte(lc, 0, chars[s4]);
}
void printTime(LedControl lc, long sec) {

  long h = sec / 60 / 60;
  long m = sec / 60 - (h * 60);
  long s = sec - ((m + (h * 60)) * 60);

  int m1 = getPositionInt(m, 2);
  int m2 = getPositionInt(m, 1);
  int s1 = getPositionInt(s, 2);
  int s2 = getPositionInt(s, 1);

  printByte(lc, 3, chars[m1]);
  printByte(lc, 2, chars[m2]);
  printByte(lc, 1, chars[s1]);
  printByte(lc, 0, chars[s2]);

  for (int a = 0; a < h; a++)
  {
    if (a <= 7) {
      lc.setLed(3, 0, a, true);
    } else break;
  }
}
