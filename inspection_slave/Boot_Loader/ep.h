
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


/* SPI FLASH --------------------------------------------------------*/
  #define SPI_nCS_HIGH			*R_P2oHIGH = (1<<7)		
#define SPI_nCS_LOW				*R_P2oLOW = (1<<7)	
#define SPI_nRESET_HIGH		*R_P5oHIGH = (1<<0)		
#define SPI_nRESET_LOW		*R_P5oLOW = (1<<0)	
#define SPI_nWP_HIGH			*R_P5oHIGH = (1<<1)		
#define SPI_nWP_LOW			*R_P5oLOW = (1<<1)
#define SPI_SCLK_HIGH			*R_P5oHIGH = (1<<2)		
#define SPI_SCLK_LOW			*R_P5oLOW = (1<<2)
#define SPI_MOSI_HIGH			*R_P5oHIGH = (1<<3)		
#define SPI_MOSI_LOW			*R_P5oLOW = (1<<3)
#define SPI_MISO					(*R_P5iLEV >> 4) & 0X01
 
 #define	QUAD_PAGE_PROGRAM					0x32
#define	QUAD_READ										0x6B
#define	PAGE_PROGREAM							0x02
#define	READ_DATA										0x0B	
#define	WRITE_ENABLE_F							0x06
#define	STAT_WRITE_ENABLE_F					0x50
#define	WRITE_DISABLE_F							0x04
#define	JEDEC_ID											0x9F
#define	MANUFACTURER_DEVIE_ID				0x90
#define	READ_STAT_REGISTER_1				0x05
#define	READ_STAT_REGISTER_2				0x35
#define	READ_STAT_REGISTER_3				0x15
#define	WRITE_STAT_REGISTER_1				0x01
#define	WRITE_STAT_REGISTER_2				0x31
#define	WRITE_STAT_REGISTER_3				0x11
#define	SECTOR_ERASE								0x20
#define	BLOCK_ERASE_32K							0x52
#define	BLOCK_ERASE_64K							0xD8
#define	CHIP_ERASE									0x60
#define	BLOCK_SECTOR_UNLOCK				0x39
#define	BLOCK_SECTOR_LOCK					0x36

#define	FLASH_MEMORY_BLOCK0				0x000000
#define	FLASH_MEMORY_BLOCK1				0x010000
#define	FLASH_MEMORY_BLOCK2				0x020000
#define	FLASH_MEMORY_BLOCK3				0x030000
#define	FLASH_MEMORY_BLOCK4				0x040000
#define	FLASH_MEMORY_BLOCK5				0x050000
#define	FLASH_MEMORY_BLOCK6				0x060000
#define	FLASH_MEMORY_BLOCK7				0x070000
#define	FLASH_MEMORY_BLOCK8				0x080000
#define	FLASH_MEMORY_BLOCK9				0x090000
#define	FLASH_MEMORY_BLOCK10				0x0A0000
#define	FLASH_MEMORY_BLOCK11				0x0B0000
#define	FLASH_MEMORY_BLOCK12				0x0C0000
#define	FLASH_MEMORY_BLOCK13				0x0D0000
#define	FLASH_MEMORY_BLOCK14				0x0E0000
#define	FLASH_MEMORY_BLOCK15				0x0F0000
#define	FLASH_MEMORY_BLOCK16				0x100000
#define	FLASH_MEMORY_BLOCK17				0x110000
#define	FLASH_MEMORY_BLOCK18				0x120000
#define	FLASH_MEMORY_BLOCK19				0x130000
#define	FLASH_MEMORY_BLOCK20				0x140000
#define	FLASH_MEMORY_BLOCK21				0x150000
#define	FLASH_MEMORY_BLOCK22				0x160000
#define	FLASH_MEMORY_BLOCK23				0x170000
#define	FLASH_MEMORY_BLOCK24				0x180000
#define	FLASH_MEMORY_BLOCK25				0x190000
#define	FLASH_MEMORY_BLOCK26				0x1A0000
#define	FLASH_MEMORY_BLOCK27				0x1B0000
#define	FLASH_MEMORY_BLOCK28				0x1C0000
#define	FLASH_MEMORY_BLOCK29				0x1D0000
#define	FLASH_MEMORY_BLOCK30				0x1E0000
#define	FLASH_MEMORY_BLOCK31				0x1F0000
#define	FLASH_MEMORY_BLOCK32				0x200000
#define	FLASH_MEMORY_BLOCK33				0x210000
#define	FLASH_MEMORY_BLOCK34				0x220000
#define	FLASH_MEMORY_BLOCK35				0x230000
#define	FLASH_MEMORY_BLOCK36				0x240000
#define	FLASH_MEMORY_BLOCK37				0x250000
#define	FLASH_MEMORY_BLOCK38				0x260000
#define	FLASH_MEMORY_BLOCK39				0x270000
#define	FLASH_MEMORY_BLOCK40				0x280000
#define	FLASH_MEMORY_BLOCK41				0x290000
#define	FLASH_MEMORY_BLOCK42				0x2A0000
#define	FLASH_MEMORY_BLOCK43				0x2B0000
#define	FLASH_MEMORY_BLOCK44				0x2C0000
#define	FLASH_MEMORY_BLOCK45				0x2D0000
#define	FLASH_MEMORY_BLOCK46				0x2E0000
#define	FLASH_MEMORY_BLOCK47				0x2F0000
#define	FLASH_MEMORY_BLOCK48				0x300000
#define	FLASH_MEMORY_BLOCK49				0x310000
#define	FLASH_MEMORY_BLOCK50				0x320000
#define	FLASH_MEMORY_BLOCK51				0x330000
#define	FLASH_MEMORY_BLOCK52				0x340000
#define	FLASH_MEMORY_BLOCK53				0x350000
#define	FLASH_MEMORY_BLOCK54				0x360000
#define	FLASH_MEMORY_BLOCK55				0x370000
#define	FLASH_MEMORY_BLOCK56				0x380000
#define	FLASH_MEMORY_BLOCK57				0x390000
#define	FLASH_MEMORY_BLOCK58				0x3A0000
#define	FLASH_MEMORY_BLOCK59				0x3B0000
#define	FLASH_MEMORY_BLOCK60				0x3C0000
#define	FLASH_MEMORY_BLOCK61				0x3D0000
#define	FLASH_MEMORY_BLOCK62				0x3E0000
#define	FLASH_MEMORY_BLOCK63				0x3F0000


#define	FLASH_WRITE_COUNT						256
#define	FLASH_READ_COUNT						256
 
 	typedef struct
	{
		unsigned short PageSize;
		unsigned int PageCount;
		unsigned int SectorSize;
		unsigned int SectorCount;
		unsigned int BlockSize;
		unsigned int BlockCount;
		unsigned int CapacityInKiloByte;
		unsigned char StatusRegister1;
		unsigned char StatusRegister2;
		unsigned char StatusRegister3;
		unsigned char Lock;
	} w25q32_t;
	
	
void byte_data_write(unsigned char data);
unsigned char byte_data_read();
void spi_write(unsigned char cmd,unsigned int addr, unsigned char  *data,int len);
void  spi_read(unsigned char cmd,unsigned int addr,int len);
void spi_stat_read(unsigned char cmd);
void spi_stat_write(unsigned char cmd, unsigned char data);
void write_enable();
void write_disable();
void wait_for_write_end();
void erase_block(unsigned int blockaddr);
void flash_init();
void wait_for_write_end();	
/* SPI FLASH --------------------------------------------------------*/

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
