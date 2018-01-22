
#ifndef SVTYPESLNX_H
#define SVTYPESLNX_H


#ifdef __GNUC__
#include <sys/types.h>

typedef u_int32_t __uint32;
typedef u_int16_t __uint16;
typedef u_int8_t __uint8 ;
typedef int32_t __int32;
typedef int16_t __int16;
typedef int8_t __int8;
typedef int64_t __int64;
typedef u_int64_t __uint64;


typedef int SV_HANDLE;
typedef int SV_SOCKET;
const int SV_SOCKET_ERROR = -1;

typedef __uint32			DWORD;
typedef __uint16			WORD;
typedef __uint8			BYTE;
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

//определяет sleep идентичный windows
#define winsleep(tms) usleep((tms)*1000)


#endif 

//---------------------------- WIN 32 ---------------------------

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>

//эти типы определяются в mytypes.h, поэтому проверим это
#ifndef __uint64  
typedef unsigned __int64 __uint64; 
typedef unsigned __int32 __uint32; 
typedef unsigned __int16 __uint16; 
typedef unsigned __int8 __uint8; 
typedef unsigned int __uint; 
#endif

typedef HANDLE SV_HANDLE;
typedef SOCKET SV_SOCKET;
const int SV_SOCKET_ERROR = -1;

#define winsleep(tms) Sleep((tms))

#endif  //WIN
//----------------------------- ALL

#define SVNULLSTRUCT memset(this,0,sizeof(*this))

//идентичны sv_time и sv_dtime, вводятся как простые типы
#pragma  pack(1)

struct sv_time2
{
	__uint16 year;
	__uint8 month;
	__uint8 day;
	__uint8 hour;
	__uint8 minute;
	__uint8 second;
	__uint16 milliseconds;
	
	//sv_time(){SVNULLSTRUCT;}
};

struct sv_dtime2
{
    __uint64 val;
};

#pragma pack()


//можно переопределить svdbgprint на другую функцию, по умолч. printf
#ifndef svdbgprint
 #define svdbgprint printf
#endif

#ifdef _SVDBGPRINT 
#define _svdbgprint  svdbgprint("Line: %s:%u\n",__FILE__,__LINE__)
#define _svdbgprintr  svdbgprint("Line: %s:%u r=%d\n",__FILE__,__LINE__,r)
#define _svdbgprinter  if(r!=0) svdbgprint("Line: %s:%u r=%d\n",__FILE__,__LINE__,r)
#else
#define _svdbgprint 
#define _svdbgprintr 
#define _svdbgprinter
#endif

//пример:
//тело: TestSVObjPtr(int a,SVObjPtr& dt) {}
//вызов: TestSVObjPtr(10,SVObjPtr(&a,sizeof(a)));
struct SVObjPtr
{
	void* dt;
	unsigned dtsz;

	SVObjPtr(){dt=0;dtsz=0;}
	SVObjPtr(void* _dt,unsigned _dtsz):dt(_dt),dtsz(_dtsz){}
	bool IsInit(){if(dt==0) return false; else return true;}
};

#endif  //SVTYPESLNX_H
