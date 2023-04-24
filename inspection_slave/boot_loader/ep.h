
/* #define SECTOR_1	((volatile unsigned char*)0x00010000)
#define SECTOR_2	((volatile unsigned char*)0x00020000)
#define SECTOR_3	((volatile unsigned char*)0x00030000)
#define SECTOR_4	((volatile unsigned char*)0x00040000)
#define SECTOR_5	((volatile unsigned char*)0x00050000)
#define SECTOR_6	((volatile unsigned char*)0x00060000)
#define SECTOR_7	((volatile unsigned char*)0x00070000) */

#define PACKET_1K_SIZE          ((unsigned int)1024)
#define PACKET_DATA_INDEX       ((unsigned int)4)
#define PACKET_TRAILER_SIZE     ((unsigned int)2)
#define FILE_NAME_LENGTH        ((unsigned int)64)
#define FILE_SIZE_LENGTH        ((unsigned int)128)

#define SOH                     ((unsigned char)0x01)  /* start of 128-byte data packet */
#define STX                     ((unsigned char)0x02)  /* start of 1024-byte data packet */
#define EOT                     ((unsigned char)0x04)  /* end of transmission */
#define ACK                     ((unsigned char)0x06)  /* acknowledge */
#define NAK                     ((unsigned char)0x15)  /* negative acknowledge */
#define CA                      ((unsigned int)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((unsigned char)0x43)  /* 'C' == 0x43, request 16-bit CRC */
#define NEGATIVE_BYTE           ((unsigned char)0xFF)

#define ABORT1                  ((unsigned char)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((unsigned char)0x61)  /* 'a' == 0x61, abort by user */

#define MAX_ERRORS              ((unsigned int)5)

/* Exported constants --------------------------------------------------------*/
/* Packet structure defines */
#define PACKET_HEADER_SIZE      ((unsigned int)3)
#define PACKET_DATA_INDEX       ((unsigned int)4)
#define PACKET_START_INDEX      ((unsigned int)1)
#define PACKET_NUMBER_INDEX     ((unsigned int)2)
#define PACKET_CNUMBER_INDEX    ((unsigned int)3)
#define PACKET_TRAILER_SIZE     ((unsigned int)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((unsigned int)128)
#define PACKET_1K_SIZE          ((unsigned int)1024)


typedef enum 
{
  HAL_OK       		= 0x01,
  HAL_ERROR    	= 0x00,
  HAL_BUSY     	= 0x02,
  HAL_TIMEOUT	= 0x03
} HAL_StatusTypeDef;

typedef enum
{
  COM_OK       = 0x01,
  COM_ERROR    = 0x00,
  COM_ABORT    = 0x02,
  COM_TIMEOUT  = 0x03,
  COM_DATA     = 0x04,
  COM_LIMIT    = 0x05
} COM_StatusTypeDef;

#define DEBUG_UART_PORT UART0
#define UART_SUB_HANDLE UART1
