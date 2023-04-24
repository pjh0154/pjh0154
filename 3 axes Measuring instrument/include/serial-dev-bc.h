#ifndef __SERIAL_DEV_BC_H__
#define __SERIAL_DEV_BC_H__

#include <windows.h>
#include <stdio.h>
#include <string.h>
 
class serial_bc
{
public:
 
    char port_name[20];

    HANDLE        hWatchThread;    // 쓰레드 핸들
    DWORD       dwThreadID ;    // 쓰레드 ID
    OVERLAPPED  osWrite, osRead ;    // Overlapped I/O를 위한 구조체
    
    // member function
    int open_port(int port_number,int baud, int dataBit, int stopBit, int parity);
    BOOL WriteCommBlock(LPSTR lpByte , DWORD dwBytesToWrite);
 
    int serial_close(void);
};

#endif
