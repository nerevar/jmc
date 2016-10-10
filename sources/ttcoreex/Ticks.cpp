/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: ticks.c - functions for the ticker stuff                    */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"


/*********************/
/* the #tick command */
/*********************/
void tick_command(wchar_t*arg)
{
    if(bTickStatus) {
      wchar_t buf[100];
      swprintf(buf, rs::rs(1176), iSecToTick);
      tintin_puts(buf);
    }
    else
      tintin_puts2(rs::rs(1177));
}

/************************/
/* the #tickoff command */
/************************/
void tickoff_command(wchar_t*arg)
{
    bTickStatus=FALSE;
    tintin_puts2(rs::rs(1178));
}

/***********************/
/* the #tickon command */
/***********************/
void tickon_command(wchar_t*arg)
{
    bTickStatus=TRUE;
    tintin_puts2(rs::rs(1179));
}

/************************/
/* the #tickset command */
/************************/
void tickset_command(wchar_t*arg)
{
    dwTime0=GetTickCount()/1000; /* we don't prompt! too many ticksets... */
}

/*************************/
/* the #ticksize command */
/*************************/
void ticksize_command(wchar_t *arg)
{
  if(*arg!='\0') {
    if(iswdigit(*arg)) {
      tick_size=_wtoi(arg);
      dwTime0=GetTickCount()/1000;
      tintin_puts2(rs::rs(1180));        
    }
  }
  else
    tintin_puts2(rs::rs(1181));
}     

