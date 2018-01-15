#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1 << PB3)  //SI pin 11
#define LATCH (1 << PB2) //RCK pin 10
#define CLOCK (1 << PB5) //SERCK pin 13

// doesnt actually mean anything its just a mulitpleir

/*const int dataPin = 11;
  const int clockPin = 9;
  const int latchPin = 10;*/
const int shiftEnable = 8;
const int clearPin = 7;
const int numLayers = 4;
const int numLEDs = 16; //can onyl be multiple of 8
const int numBytes = numLEDs / 8;
const int refreshRate = 50;

const int ground[numLayers] = {3, 4, 5, 6};
byte cubeLEDs[numLayers][numBytes];

int pos[2] = {0, 0};
int layer = 0;
int prevLayer = 0;
byte zShiftAnimation[4][2] {
  {B10001000, B10001000},
  {B01000100, B01000100},
  {B00100010, B00100010},
  {B00010001, B00010001}
};

byte yShiftAnimation[2][2] {
  {B11111111, B11111111},
  {B00000000, B00000000}
};

byte xShiftAnimation[4][2] {
  {B11110000, B00000000},
  {B00001111, B00000000},
  {B00000000, B11110000},
  {B00000000, B00001111}
};

byte wallScanAnimation[12][2] {
  {B11110000, B00000000},
  {B01110001, B00000000},
  {B00110001, B00010000},
  {B00010001, B00010001},
  {B00000001, B00010011},
  {B00000000, B00010111},
  {B00000000, B00001111},
  {B00000000, B10001110},
  {B00001000, B10001100},
  {B10001000, B10001000},
  {B11001000, B10000000},
  {B11101000, B00000000}
};

void setup()
{
  Serial.begin(115200);
  //Setup Timer interrupts
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = 2e6 / (refreshRate * numLayers); // compare valie
  TCCR1B |= (1 << WGM12);                  // CTC mode
  TCCR1B |= (1 << CS11);                   // 16bit prescaler 2MHz
  TIMSK1 |= (1 << OCIE1A);                 // timer compare interrupt
  interrupts();

  //Setup IO
  SHIFT_REGISTER |= (DATA | LATCH | CLOCK); //Set control pins out
  SHIFT_PORT &= ~(DATA | LATCH | CLOCK);    //Set control pins 0
  SPCR = (1 << SPE) | (1 << MSTR);          //Start SPI as Master

  pinMode(shiftEnable, OUTPUT);
  pinMode(clearPin, OUTPUT);
  for (int i = 0; i < numLayers; i++)
  {
    pinMode(ground[i], OUTPUT);
    digitalWrite(ground[i], 0);
  }

  digitalWrite(clearPin, 1);
  emptyCube();
}

ISR(TIMER1_COMPA_vect)
{
  //Serial.println(layer);
  //Serial.println("Test1");
  digitalWrite(shiftEnable, 1);
  digitalWrite(clearPin, 0);
  digitalWrite(clearPin, 1);
  digitalWrite(ground[prevLayer], 0);
  digitalWrite(ground[layer], 1);
  for (int i = 0; i < numLEDs / 8; i++)
  {
    SHIFT_PORT &= ~LATCH;
    SPDR = cubeLEDs[layer][i];
    while (!(SPSR & (1 << SPIF)))
      ;
    SHIFT_PORT |= LATCH;
    SHIFT_PORT &= ~LATCH;
  }
  digitalWrite(shiftEnable, 0);
  // Serial.print(layer);
  // Serial.print("|");
  // Serial.print(cubeLEDs[layer][0], BIN);
  // Serial.print("|");
  // Serial.print(cubeLEDs[layer][1], BIN);
  // Serial.print("|");
  // Serial.println(ground[layer]);
  prevLayer = layer;
  if (layer < numLayers - 1)
  {
    layer++;
    //Serial.println(layer);
  }
  else
  {
    layer = 0;
  }
  //Serial.println(millis());
}

void loop()
{

  wallScan(2);
  emptyCube();
  zShift(2);
  emptyCube();
  xShift(2);
  emptyCube();
  yShift(2);
  emptyCube();
  droppingLikeFlies();
  emptyCube();
  MakeItRain();
  emptyCube();
  
  delay(100);
}

void makeItRain()
{

}

void droppingLikeFlies()
{
  randomSeed(micros());
  for (int j = 0; j < 2; j++)
  {
    cubeLEDs[3][j] = B11111111;
  }
  int usedBits[2][8] = {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}};
  for (int i = 0; i < 16; i++)
  {
    int droppingByte = random() % 2;
    int droppingBit = random() % 8;
    while (usedBits[droppingByte][droppingBit] == 0)
    {
      droppingByte = random() % 2;
      droppingBit = random() % 8;
    }
    unsigned int timerA = millis();
    unsigned int prevTimerA = timerA;
    while (true)
    {
      usedBits[droppingByte][droppingBit] = 0;
      timerA = millis();
      if ((timerA - prevTimerA) > 100)
      {
        bitClear(cubeLEDs[3][droppingByte], droppingBit);
        bitSet(cubeLEDs[2][droppingByte], droppingBit);
      }

      if ((timerA - prevTimerA) > 200)
      {
        bitClear(cubeLEDs[2][droppingByte], droppingBit);
        bitSet(cubeLEDs[1][droppingByte], droppingBit);
      }

      if ((timerA - prevTimerA) > 300)
      {
        bitClear(cubeLEDs[2][droppingByte], droppingBit);
        bitSet(cubeLEDs[1][droppingByte], droppingBit);
      }

      if ((timerA - prevTimerA) > 400)
      {
        bitClear(cubeLEDs[1][droppingByte], droppingBit);
        bitSet(cubeLEDs[0][droppingByte], droppingBit);
        break;
      }
    }
  }
}

void MakeItRain() {
  randomSeed(micros());
  for (int j = 0; j < 2; j++)
  {
    for (int i = 0; i < 4; i++)
    {
      cubeLEDs[i][j] = B00000000;

    }
  }
  int usedBits[4][2][8] = {
    {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}},
    {{1, 1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 1, 1, 1, 1}}
  };

  unsigned int timerA = millis();
  unsigned int prevTimerA = timerA;
  unsigned int timerB = millis();
  unsigned int prevTimerB = timerB;
  int counter = 0;
  while (1)
  {
    timerA = millis();
    timerB = millis();
    //Serial.println(times);
    /*if (counter > times) {
      break;
    }*/
    if ((timerA - prevTimerA) > 100)
    {
      counter++;
      //Serial.println("ya");
      prevTimerA = millis();
      int droppingByte = random() % 2;
      int droppingBit = random() % 8;
      int droppingLayer = (random() % 4);
      int create = random() % 8;

      if (create)
      {
        bitSet(cubeLEDs[3][droppingByte], droppingBit);
        usedBits[3][droppingByte][droppingBit] = 0;
      }

      while (usedBits[droppingLayer][droppingByte][droppingBit] == 1)
      {
        droppingByte = random() % 2;
        droppingBit = random() % 8;
        droppingLayer = random() % 4;
      }
      usedBits[droppingLayer][droppingByte][droppingBit] = 1;
      if (droppingLayer == 0)
      {
        bitClear(cubeLEDs[droppingLayer][droppingByte], droppingBit);
      } else {
        bitClear(cubeLEDs[droppingLayer][droppingByte], droppingBit);
        bitSet(cubeLEDs[droppingLayer - 1][droppingByte], droppingBit);
        usedBits[droppingLayer - 1][droppingByte][droppingBit] = 0;
      }


    }
  }
}

void myDelay(int time)
{
}

void zShift(int times)
{
  for (int l = 0; l < times; l++)
  {
    for (int k = 0; k < 4; k++)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[i][j] = zShiftAnimation[k][j];
        }
      }
      delay(100);
    }
    for (int k = 3; k >= 0; k--)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[i][j] = zShiftAnimation[k][j];
        }
      }
      delay(100);
    }
  }
}

void xShift(int times)
{
  for (int l = 0; l < times; l++)
  {
    for (int k = 0; k < 4; k++)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[i][j] = xShiftAnimation[k][j];
        }
      }
      delay(100);
    }
    for (int k = 3; k >= 0; k--)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[i][j] = xShiftAnimation[k][j];
        }
      }
      delay(100);
    }
  }
}

void yShift(int times)
{
  for (int l = 0; l < times; l++)
  {
    for (int k = 0; k < 4; k++)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[k - 1][j] = yShiftAnimation[1][j];
          cubeLEDs[k][j] = yShiftAnimation[0][j];
        }
      }
      delay(100);
    }
    for (int k = 3; k >= 0; k--)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[k + 1][j] = yShiftAnimation[1][j];
          cubeLEDs[k][j] = yShiftAnimation[0][j];
        }
      }
      delay(100);
    }
  }
}

void wallScan(int times)
{
  for (int l = 0; l < times; l++)
  {
    for (int k = 0; k < 12; k++)
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 2; j++)
        {
          cubeLEDs[i][j] = wallScanAnimation[k][j];
        }
      }
      delay(100);
    }
  }
}

void emptyCube()
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      cubeLEDs[i][j] = B00000000;
    }
  }
}


