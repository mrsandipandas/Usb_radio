#include <mega328p.h>
#include <delay.h>
#include "eeprom.h"
#include "flags.h"


// FLAGS storage 32 bits
unsigned long int     RFlag = 0;      //Flags storage in RAM. Volatile.

//*****************************************************************************
/* Used to set a boolean flag */
//*****************************************************************************
void SetFlag(unsigned char flag) 
{   
  if (flag < 32)
    RFlag = RFlag | (1<<(unsigned long int)flag);
  else
  {
    flag = flag - 32;
    EFlag = EFlag | (1<<(unsigned long int)flag);
  }
}          

//*****************************************************************************
/* Used to clear a boolean flag */
//*****************************************************************************
void ClearFlag(unsigned char flag) 
{
  if (flag < 32)
    RFlag = RFlag & ~(1<<(unsigned long int)flag);
  else
  {
    flag = flag - 32;
    EFlag = EFlag & ~(1<<(unsigned long int)flag);
  }
}

//*****************************************************************************
/* Used to return the value of a flag */
//*****************************************************************************
unsigned char CheckFlag(unsigned char flag) 
{
  if (flag < 32)
  {
    if (RFlag & (1<<(unsigned long int)flag))
      return(1);
    else
      return(0);
  }
  else
  {
    flag = flag - 32;
    if (EFlag & (1<<(unsigned long int)flag))
      return(1);
    else
      return(0);
  }
}
                 

