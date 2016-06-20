//###########POWER_DOWN###########################

void POWER_DOWN()
{
  //Power down various bits of hardware to lower power usage
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  //sleep_enable();

  //Shut off ADC, TWI, SPI, Timer0, Timer1++++++++++++++++++
  ADCSRA &= ~(1 << ADEN); //Disable ADC
  ACSR = (1 << ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1 << AIN1D) | (1 << AIN0D); //Disable digital input buffer on AIN1/0

  power_twi_disable();  //I2C
  power_spi_disable();  //SPI
#ifndef DEBUG
  power_usart0_disable(); //Needed for serial.print
#endif

  //power_timer0_disable(); //Needed for delay and millis()
  //power_timer1_disable();

  //power_timer2_disable(); //Needed for asynchronous 32kHz operation
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //Timer setup+++++++++++++++++++++++++++++++++++++++++++++++
  //Setup TIMER2
  TCCR2A = 0x00;
  //TCCR2B = (1<<CS22)|(1<<CS20); //Set CLK/128 or overflow interrupt every 1s
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); //Set CLK/1024 or overflow interrupt every 8s
  ASSR = (1 << AS2); //Enable asynchronous operation
  TIMSK2 = (1 << TOIE2); //Enable the timer 2 interrupt

  //Setup external INT0 interrupt
  EICRA = (1 << ISC01); //Interrupt on falling edge
  EIMSK = (1 << INT0); //Enable INT0 interrupt

  //System clock futzing
  //CLKPR = (1<<CLKPCE); //Enable clock writing
  //CLKPR = (1<<CLKPS3); //Divid the system clock by 256



  //  showTime(); //Show the current time for a few seconds

  sei(); //Enable global interrupts

}

void enterSleepMode()
{

  power_timer0_disable(); //Needed for delay and millis()

  power_timer1_disable();
  writeNumber(0);
  actDig(digPin[0], LOW);
  actDig(digPin[1], LOW);
  actDig(digPin[2], LOW);
  actDig(digPin[3], LOW);
  actDig(digPin[4], LOW);
  //Timer1.detachInterrupt();

  do
  {
    sleep_mode();
  } while (!displayOn);

  power_timer0_enable();
  //Timer1.attachInterrupt(displayR);
  power_timer1_enable();

  /*pinMode(upBtn, INPUT);

  pinMode(downBtn, INPUT);*/
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++
SIGNAL(TIMER2_OVF_vect) {
  seconds += 8; //We sleep for 8 seconds instead of 1 to save more power
  //seconds++; //Use this if we are waking up every second

  //Update the minutes and hours variables
  minutes += seconds / 60; //Example: seconds = 2317, minutes = 58 + 38 = 96
  seconds %= 60; //seconds = 37
  hours += minutes / 60; //12 + (96 / 60) = 13
  minutes %= 60; //minutes = 36

  while (hours > 23) hours -= 24;
}

//The interrupt occurs when you push the button
SIGNAL(INT0_vect) {

  if (!displayOn)   //for stting menuState to 0 only after sleep
  {
    menuState = 0;
    menusubState = 0;
    displayOn = true;
  }
}
