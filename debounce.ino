//#######DEBOUNCE##############
void debounce()
{
  Now = millis();

  if (Now - debounceDelay > lastDebounceSel)
  {
    selbtnState[0] = !digitalRead(selBtn);
    if (selbtnState[0] != selbtnState[1])
    {
      lastDebounceSel = Now;
      selbtnState[1] = selbtnState[0];

    }
    else
    {
      selectB = selbtnState[0];
    }
  }

  if (Now - debounceDelay > lastDebounceUp)
  {
    upbtnState[0] = !digitalRead(upBtn);

    if (upbtnState[0] != upbtnState[1])
    {
      lastDebounceUp = Now;
      upbtnState[1] = upbtnState[0];
    }
    else
    {
      upB = upbtnState[0];
    }
  }

  if (Now - debounceDelay > lastDebounceDown)
  {
    downbtnState[0] = !digitalRead(downBtn);

    if (downbtnState[0] != downbtnState[1])
    {
      lastDebounceDown = Now;
      downbtnState[1] = downbtnState[0];
    }
    else
    {
      downB = downbtnState[0];
    }
  }

  if (upB || downB || selectB)
  {
    Old = Now;
  }

  btnUp = false;
  btnDown = false;
  btnSelect = false;

  if (!upB)
  {
    btnWasUp = false;
  }
  if (!downB)
  {
    btnWasDown = false;
  }
  if (!selectB)
  {
    btnWasSelect = false;
  }

  if ( !btnWasUp && upB)
  {
    btnUp = true;
    btnWasUp = true;
  }
  else
  {
    btnUp = false;
  }

  if (!btnWasDown && downB)
  {
    btnDown = true;
    btnWasDown = true;
  }
  else
  {
    btnDown = false;
  }

  if (!btnWasSelect && selectB)
  {
    btnSelect = true;
    btnWasSelect = true;
  }
  else
  {
    btnSelect = false;
  }
}
