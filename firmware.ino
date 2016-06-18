#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers
#include <TimerOne.h>  //Needed for display


//#define DEBUG 1
#ifdef DEBUG
#define debugWrite(s) Serial.println((s))

#else
#define debugWrite(s)

#endif

//++++segment-parts++++
#define segA A1
#define segB A3
#define segC 7
#define segD A2
#define segE 4
#define segF A0
#define segG 8

//++++display-select++++
//all of them are PWM
/*#define d1 6
#define d2 9
#define d3 10
#define d4 11
#define points 5  //because pwm*/

const uint8_t digPin[5] {6, 9, 10, 11, 5};

//++++BTNs++++++++++++
//you can use pin 2 and 3 for the BTN for wake up from sleep
#define selBtn 2
#define upBtn 13
#define downBtn 12

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #define STRATEGY_DEFINES
#ifdef STRATEGY_DEFINES
#define actSeg(seg,act) digitalWrite((seg), (act))
#define actDig(dig,act) digitalWrite((dig), !(act))
#define initSeg(seg)   pinMode(seg,OUTPUT); actSeg(seg,LOW));
#define initDig(dig)   pinMode(dig,OUTPUT); actDig(dig,LOW));


//#define actSeg(seg,act) digitalWrite((seg), !(act))
//#define actSeg(dig,act) digitalWrite((dig), (act))
#else
void actSeg(uint8_t seg, bool act) {
  digitalWrite((seg), (act));
}
void actDig(uint8_t dig, bool act) {
  digitalWrite((dig), !(act));
}
void  initSeg(uint8_t seg)   {
  pinMode(seg, OUTPUT);
  actSeg(seg, LOW);
}
void  initDig(uint8_t dig)   {
  pinMode(dig, OUTPUT);
  actDig(dig, LOW);
}
#endif

//###################################################

long seconds = 0;
int minutes = 11;
int hours = 15;

volatile long Old;
volatile long Now;
int menuState = 0;
int menusubState = 0;
int i;          //for-loops
bool displayOn = true;
bool allwaysOn = false;
unsigned int startTime;
unsigned int stopTime;

//+++++++debounce++++++++++++++
#define debounceDelay 50
bool upB;
bool downB;
bool selectB;
bool selbtnState[2] {LOW, LOW};
long lastDebounceSel;
bool upbtnState[2] {LOW, LOW};
long lastDebounceUp;
bool downbtnState[2] {LOW, LOW};
long lastDebounceDown;


//+++++++++logic use buttons++++++++++++
bool btnUp;
bool btnWasUp;
bool btnDown;
bool btnWasDown;
bool btnSelect;
bool btnWasSelect;


#define onTime 4000  //4 seconds is the display on
#define interruptTime 1000 //1000000

#define brightTicks 1   // bright phase, leave 1
#define darkTicks 5     // dark phase 

static uint8_t darkPhase = darkTicks;
//  bright:dark ratio at  3v
//  1:1: 3,2mA per segment  -> 22,4 max      (3,9mA bei 3,2V)
//  1:2: 2,25mA per segment
//  1:3: 1,75mA per segment
//  1:4: 1,4mA per segment
//  1:5: 1,2mA per segment -> 7,5mA max


//for display

volatile char displayOut[5];


//##################################################

/*    __a_
 * f |    |
 *   |__g_| b
 * e |    |
 *   |____| c
 *      d
 */


void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("My Watchg test");
#endif

  for (i = 0; i < 5; i++)
  {
    initDig(digPin[i]);
  }
  /*initDig(d1);
  initDig(d2);
  initDig(d3);
  initDig(d4);
  initDig(points);*/

  initSeg(segA);
  initSeg(segB);
  initSeg(segC);
  initSeg(segD);
  initSeg(segE);
  initSeg(segF);
  initSeg(segG);

  //buttons****

  pinMode(selBtn, INPUT_PULLUP); //This is the main button, tied to INT0
  // digitalWrite(selBtn, HIGH); //Enable internal pull up on button

  pinMode(upBtn, INPUT_PULLUP);
  // digitalWrite(upBtn, HIGH); //Enable internal pull up on button

  pinMode(downBtn, INPUT_PULLUP);
  //digitalWrite(downBtn, HIGH); //Enable internal pull up on button

  POWER_DOWN();

  Timer1.initialize(interruptTime) ; // * 10000);
  Timer1.attachInterrupt(displayR);

  //start display at this point
  Old = millis();
}

//############################################

void loop() {
  checkAll();

  if (displayOn)
  {
    switch (menuState)
    {
      case -1:
        menuState = 2;
        break;
      case 0:
        writeTime();
        break;
      case 1:
        setTime();
        break;
      case 2:
        drawPoints(' ');
        digWrite(0, '-');
        digWrite(1, 'P');
        digWrite(2, 'G');
        digWrite(3, '-');
        break;
      case 3:
        menuState = 0;
        break;
    }
  }
}

/*void stopwatch()
{
  switch (menusubState)
  {
    case 0:
      drawPoints(' ');
      digWrite(0, 'S');
      digWrite(1, 'T');
      digWrite(2, 'O');
      digWrite(3, 'P');
      break;
    case 1:
      allwaysOn = true;
      startTime = Now;
      menusubState++;
      break;
    case 2:           //does not work
      stopTime = Now - startTime;
      drawPoints(':');
      i = stopTime / 1000;
      digWrite(1, (i / 60) / 10);
      digWrite(2, (i / 60) % 10);
      digWrite(3, (i % 60) / 10);
      digWrite(4, (i % 60) % 10);
      break;
    case 3:
      allwaysOn = false;
      drawPoints(':');
      i = stopTime / 1000;
      digWrite(1, (i / 60) / 10);
      digWrite(2, (i / 60) % 10);
      digWrite(3, (i % 60) / 10);
      digWrite(4, (i % 60) % 10);
      break;
    case 4:
      menusubState = 0;
      break;
  }

  if (btnSelect)
  {
    menusubState++;
  }

}*/

void checkAll()
{
  debugWrite("checkAll");
  if (Now >= Old + onTime && !allwaysOn)
  {
    debugWrite("checkAll->sleep");
    displayOn = false;
    enterSleepMode();
    debugWrite("checkAll->wake up");
    return;
  }

  debounce();


  if (menusubState == 0)
  {
    if (btnDown)
    {
      menuState++;
    }
    if (btnUp)
    {
      menuState--;
    }
  }
}



//++++++++++++++++++++++++++++++++++
void writeTime()
{
  drawPoints(':');
  digWrite(0, (hours / 10));
  digWrite(1, (hours % 10));
  digWrite(2, (minutes / 10));
  digWrite(3, (minutes % 10));
}

void setTime()
{
  switch (menusubState)
  {
    case 0:
      drawPoints(' ');
      digWrite(0, 'S');
      digWrite(1, 'E');
      digWrite(2, 'T');
      digWrite(3, ' ');
      break;
    case 1:
      //set minutes
      drawPoints(':');
      digWrite(0, '-');
      digWrite(1, '-');
      digWrite(2, (minutes % 60) / 10);
      digWrite(3, (minutes % 60) % 10);
      if (btnUp)
      {
        minutes = (minutes - 1) % 60;
      }
      if (btnDown)
      {
        minutes = (minutes + 1) % 60;
      }
      if (minutes < 0)
      {
        minutes = 60;
      }
      break;
    case 2:
      //set hours
      drawPoints(':');
      digWrite(0, (hours % 60) / 10);
      digWrite(1, (hours % 60) % 10);
      digWrite(2, '-');
      digWrite(3, '-');
      if (btnUp)
      {
        hours = (hours - 1) % 24;
      }
      if (btnDown)
      {
        hours = (hours + 1) % 24;
      }
      if (hours < 0)
      {
        hours = 24;
      }
      break;
    case 3:
      menuState = menusubState = 0;
      break;
  }
  if (btnSelect)
  {
    menusubState++;
  }
}
