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
void tick_command(char*arg)
{
    if(bTickStatus) {
      char buf[100];
      sprintf(buf, rs::rs(1176), iSecToTick);
      tintin_puts(buf);
    }
    else
      tintin_puts(rs::rs(1177));
}

/************************/
/* the #tickoff command */
/************************/
void tickoff_command(char*arg)
{
    bTickStatus=FALSE;
    tintin_puts2(rs::rs(1178));
}

/***********************/
/* the #tickon command */
/***********************/
void tickon_command(char*arg)
{
    bTickStatus=TRUE;
    tintin_puts2(rs::rs(1179));
}

/************************/
/* the #tickset command */
/************************/
void tickset_command(char*arg)
{
    dwTime0=GetTickCount()/1000; /* we don't prompt! too many ticksets... */
}

/*************************/
/* the #ticksize command */
/*************************/
void ticksize_command(char *arg)
{
  if(*arg!='\0') {
    if(isdigit(*arg)) {
      tick_size=atoi(arg);
      dwTime0=GetTickCount()/1000;
      tintin_puts2(rs::rs(1180));        
    }
  }
  else
    tintin_puts2(rs::rs(1181));
}     

