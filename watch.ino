#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers
#include <TimerOne.h>  //Needed for display

//  http://schueler.bulme.at/~philipp.gretzl/

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

//++++display-select-pins++++
const uint8_t digPin[5] {6, 9, 10, 11, 5};

//++++BTNs++++++++++++
//you can use pin 2 and 3 for the BTN for wake up from sleep
#define selBtnPin 2
#define upBtnPin 13
#define downBtnPin 12

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

//###################################################

unsigned long seconds = 0;
uint8_t minutes = 11;
uint8_t hours = 15;

volatile unsigned long Old;
volatile unsigned long Now;
int menuState = 0;
int menusubState = 0;

bool displayOn = true;

//+++++++debounce++++++++++++++
#define debounceDelay 50
//#define PAPI 1

//+++++++++logic use buttons++++++++++++
/*bool btnUp;
bool btnDown;
bool btnSelect;*/


#ifdef DEBUG
#define onTime 10000  //10 seconds is the display on
#else
#define onTime 3000  //3 seconds is the display on
#endif

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


//display buffer
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

  for (int i = 0; i < 5; i++)
  {
    initDig(digPin[i]);
  }

  initSeg(segA);
  initSeg(segB);
  initSeg(segC);
  initSeg(segD);
  initSeg(segE);
  initSeg(segF);
  initSeg(segG);

  //buttons****

  pinMode(selBtnPin, INPUT_PULLUP); //This is the main button, tied to INT0
  // digitalWrite(selBtn, HIGH); //Enable internal pull up on button
  pinMode(upBtnPin, INPUT_PULLUP);
  // digitalWrite(upBtn, HIGH); //Enable internal pull up on button
  pinMode(downBtnPin, INPUT_PULLUP);
  //digitalWrite(downBtn, HIGH); //Enable internal pull up on button

  POWER_DOWN();

  Timer1.initialize(interruptTime) ; // * 10000);
  Timer1.attachInterrupt(displayR);

  //start display at this point
  Now = Old = millis();

  /*drawPoints('.');
  digWrite(0, 'V');
  digWrite(1, '2');
  digWrite(2, '0');
  digWrite(3, ' ');
  delay(1000);*/
}

//############################################
#include "debounce.h"  //Needed for debounce
Btn upBtn(upBtnPin , 250);
Btn downBtn(downBtnPin , 250);
Btn selBtn(selBtnPin);


void debounce()
{
  upBtn.debounce(Now);
  downBtn.debounce(Now);
  selBtn.debounce(Now);
}


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
        displaySettings();
        break;
      case 3:
        menuState = 0;
        break;
    }
  }
}

void checkAll()
{
  debugWrite("checkAll");

  Now = millis();
  debounce();

  if (upBtn.pulse() || downBtn.pulse() || selBtn.pulse())
  {
    Old = Now;
  }
  else
  {
    delay(debounceDelay);
  }


  if (Now >= Old + onTime)
  {
    debugWrite("checkAll->sleep");
    displayOn = false;
    enterSleepMode();
    debugWrite("checkAll->wake up");
    Old = Now = millis();
    return;     // return to mai-loop
  }


  if (menusubState == 0)
  {
    if (downBtn.pulse())
    {
      menuState++;
    }
    if (upBtn.pulse())
    {
      menuState--;
    }
  }
}



//++++++++++++++++++++++++++++++++++
void writeTime()
{
  drawPoints(((Now / 500) % 2) ? ':' : ' ');
  if (hours / 10 != 0)
  {
    digWrite(0, (hours / 10));
  }
  else
  {
    digWrite(0, ' ');
  }
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
      digWrite(2, minutes / 10);
      digWrite(3, minutes % 10);
      if (upBtn.pulse())
      {
        if (minutes > 0) minutes = minutes - 1;
        else minutes = 59;
      }
      if (downBtn.pulse())
      {
        minutes = (minutes + 1) % 60;
      }
      break;
    case 2:
      //set hours
      drawPoints(':');
      digWrite(0, hours  / 10);
      digWrite(1, hours  % 10);
      digWrite(2, '-');
      digWrite(3, '-');
      if (upBtn.pulse())
      {
        if (hours > 0) hours = hours - 1;
        else hours = 23;
      }
      if (downBtn.pulse())
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
  if (selBtn.pulse())
  {
    menusubState++;
  }
}

void displaySettings()
{
  switch (menusubState)
  {
    case 0:
      drawPoints(' ');
      digWrite(0, 'D');
      digWrite(1, 'I');
      digWrite(2, 'S');
      digWrite(3, 'P');
      break;
    case 1:
      //set minutes
      drawPoints(':');
      digWrite(0, 'D');
      digWrite(1, 'B');
      digWrite(2, ' ');
      digWrite(3, darkPhase);
      if (downBtn.pulse())
      {
        if (darkPhase < 5)
        {
          darkPhase++;
        }
      }
      if (upBtn.pulse())
      {
        if (darkPhase > 2)
        {
          darkPhase--;
        }
      }
      break;
    case 2:
      menuState = menusubState = 0;
      break;
  }
  if (selBtn.pulse())
  {
    menusubState++;
  }
}
