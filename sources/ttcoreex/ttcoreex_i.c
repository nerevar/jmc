/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Wed Apr 27 16:43:10 2016
 */
/* Compiler settings for D:\github\jmc\jmc-msvs6\sources\ttcoreex\ttcoreex.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IJmcSite = {0xB41BA78F,0x8ED5,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


const IID IID_IJmcObj = {0xDDF78224,0x8F80,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


const IID LIBID_TTCOREEXLib = {0xB41BA781,0x8ED5,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


const CLSID CLSID_JmcSite = {0xB41BA790,0x8ED5,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


const IID DIID__IJmcObjEvents = {0xDDF78226,0x8F80,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


const CLSID CLSID_JmcObj = {0xDDF78225,0x8F80,0x11D4,{0x92,0xAE,0x00,0x01,0x02,0x7E,0xD9,0x04}};


#ifdef __cplusplus
}
#endif

