#include "cantus.h"

//#define SDCARD_STORAGE 

//#define UFI_DEBUG
#ifdef UFI_DEBUG
	#define DEBUGUFI	debugprintf
#else
	#define DEBUGUFI(...)
#endif

#define END_LEN		0x07		// Endpoint Dscriptor Length

#define DSCR_STRING     3   // Descriptor type: String 


const BYTE	Dev_Descp_Mass[] =
{
    DEV_LEN,			    // bLength: Size of descriptor
    0x01,		        // bDescriptorType: Device
    0x00,0x01,			// bcdUSB: USB 1.1
    0x00,				// bDeviceClass: none
    0x00,				// bDeviceSubClass: none
    0x00,				// bDeviceProtocol: none
    EP0_LEN,			    // bMaxPacketSize0: 16 bytes
    0xDC,0x0A,		    // idVendor:  - ADC
    0x20,0x00,			// idProduct:
    0x01,0x00,			// bcdDevice: device release
    0x00,				// iManufacturer:
    0x00,				// iProduct:
    0x00,				// iSerialNumber: none
    0x01
};				// bNumConfigurations: 1


//----------------------------------------------------------------------------------------
// CONFIGURATION DESCRIPTOR
//--------------------------------------------------------------------------------------
const BYTE	Cfg_Descp_Mass[] = {0x09,				// bLength: Size of descriptor
                         0x02,		        // bDescriptorType: Configuration
                         33,0x00,		// wTotalLength: Cfg+Ifc+Class+Ep = 32 bytes
                         0x01,				// bNumInterfaces: 1 interface
                         0x01,				// bConfigurationValue: 1
                         0x00,				// iConfiguration: none
                         0x60,				// bmAttributes: Self power
                         0x32,				// MaxPower: 100mA

//------------------------------------------------------------------------------------------------
// INTERFACE DESCRIPTOR
//------------------------------------------------------------------------------------------------
                         0x09,				// bLength: Size of descriptor
                         0x04,		        // bDescriptorType: Interface
                         0x00,				// bInterfaceNumber: #1
                         0x00,				// bAlternateSetting: #0
                         0x02,				// bNumEndpoints: 2
                         0x08,				// mass storage class
                         0x06,				// SCSI transparent
                         0x50,				// Bulk Only Transfer
                         0x00,				// iInterface: none

//-----------------------------------------------------------------------------------------------------------
// ENDPOINT 1 DESCRIPTOR  (BULK OUT)
//-----------------------------------------------------------------------------------------------------------
                         7,			// bLength: Size of descriptor
                         0x05,				// bDescriptorType: Endpoint
                         0x01,				// bEndpointAddress: OUT, EP1
                         0x02,				// bmAttributes: Bulk
                         EP1_LEN,0x00,		// wMaxPacketSize:
                         0x00,	            // bInterval: 255ms  , ignored for Bulk

//-----------------------------------------------------------------------------------------------------------
// ENDPOINT 2 DESCRIPTOR  (BULK IN)
//-----------------------------------------------------------------------------------------------------------
                         7,			// bLength: Size of descriptor
                         0x05,				// bDescriptorType: Endpoint
                         0x82,				// bEndpointAddress: IN, EP2
                         0x02,				// bmAttributes: Bulk
                         EP2_LEN,0x00,		// wMaxPacketSize:
                         0x00,	            // bInterval: 255ms, ignored for Bulk
//-----------------------------------------------------------------------------------------------------------
// ENDPOINT 3 DESCRIPTOR  (Interrupt IN)
//-----------------------------------------------------------------------------------------------------------

                         7,			// bLength: Size of descriptor
                         0x05,
                         0x83,
                         0x03,
                         02,0x00,
                         0x01,
                        };


const BYTE StringDscr_Mass[] = {   4,          // String descriptor length
                             DSCR_STRING,
                             9,4,

                             16,         // String descriptor length
                             DSCR_STRING,
                             'A',0,
                             'D',0,
                             'C',0,
                             'h',0,
                             'i',0,
                             'p',0,
                             's',0,

                             18,         // Descriptor length
                             DSCR_STRING,
                             'S',0,
                             't',0,
                             'o',0,
                             'r',0,
                             'a',0,
                             'g',0,
                             'e',0,
                             ' ',0
                         };



#define TEST_UNIT_READY             	0x00
#define REQUEST_SENSE               	0x03
#define INQUIRY                     	0x12
#define READ_FORMAT			            0x23
#define STOP_START_UNIT             	0x1B
#define PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define READ_CAPACITY               	0x25
#define READ_10                     	0x28
#define VERIFY_10                   	0x2F
#define MODE_SELECT_06			        0x15
#define MODE_SENSE_06 			        0x1A
#define WRITE_10			            0x2A

#define BCODE	0
#define BSTRUC	1
#define SHALF	2
#define LLBA	3
#define ROOTE	4
#define L0x60	5
#define	LFAT	6
#define LFAT2	7

#define RESIDUE_ADDR	0xff80
#define RESIDUE_ADDR_B	0xffc0

union byte2
{
    unsigned short bw;

    struct
    {
        unsigned char bl;
        unsigned char bh;
    } wordsp;
};

union BYTE4
{
    unsigned long b4;

    struct
    {
        unsigned char ll;
        unsigned char lh;
        unsigned char hl;
        unsigned char hh;
    } longsp;
};

struct ReadCommand
{
    unsigned char OpertionCode;
    unsigned char LUN;
    unsigned long LBA;
    unsigned char res1;
    unsigned short TransLen;
    unsigned char res2;
    unsigned char res3;
    unsigned char res4;
};

struct ReadFormat
{
    unsigned char OperationCode;
    unsigned char LUN;
    unsigned char res1;
    unsigned char res2;
    unsigned char res3;
    unsigned char res4;
    unsigned char res5;
    unsigned short AllocLen;
    unsigned char res6;
    unsigned char res7;
    unsigned char res8;
};
struct CBW
{
    unsigned char Sign[4];
    unsigned char Tag[4];
    unsigned long DTL;
    unsigned char Flag;
    unsigned char LUN;
    unsigned char CBWCBLen;
};

struct CapaList
{
    // capacity list header
    unsigned char res1;
    unsigned char res2;
    unsigned char res3;
    unsigned char CapaLen;

    //current/maximum capacity descriptor
    unsigned long CNOB;
    unsigned char DescCode;
    unsigned short CBlkLen;
    unsigned char res4;

    //formattable capacity descriptor
    unsigned long FNOB;
    unsigned char res5;
    unsigned short FBlkLen;
    unsigned char res6;

};


unsigned char send_csw;

U8 bulk_buf[64] = {0};
struct CBW *wrapp;
unsigned char removal_prevent = 0;

unsigned char InquiryData[36] =
{
    0x00,                                           // Device class
    0x80,                                           // RMB bit is set by inquiry data
    0x02,                                           // Version
    0x01,                                           // Data format = 1
    0x00, 0x00, 0x00, 0x00,
    'A', 'D', 'C', 'H', 'I', 'P', 'S', ' ', // Manufacturer
    'M', 'a', 's', 's', ' ', 'S', 't', 'o', // Product(Zip 100)
    'r', 'a', 'g', 'e', ' ', ' ', ' ', ' ',
    '1', '.', '0', '0'                          // Revision
};

unsigned char ReadFromatCapa[12] =
{
    0x00,0x00,0x00,0x08,  0x00,0x00,0x40,0x00,0x02,0x00,0x02,0x00
};   	// 8M

unsigned char ReadCapa[8] = {0,}; // end sector addres(4byte),pagesize(4byte), warning:big endian
unsigned char ModeParam[12] =
{
    0x0b, //Mode data length
    0x00, // media type
    0x00, //device-specific parameter
    0x08, // block description length
    0x00, //PS=1, page code=0x06
    0x00,0x3f,0xff,0x00, 		// page lengthWCD, logical block size 8M
    0x00,0x02,0x00      		// Logical block size : 512byte(1st Generation)

};


unsigned char SenseData[18] =
{
    0xf0,0x00,0x05,0x00,
    0x00,0x00,0x00,0x0b,
    0x00,0x00,0x00,0x00,
    0x24,0x00,0x00,0x00,
    0x00,0x00
};


void sendCSW(void)
{
    int i;
    U8 buf[64];
    i = 0;
    buf[i++] = 'U';
    buf[i++] = 'S';
    buf[i++] = 'B';
    buf[i++] = 'S';
    buf[i++] = bulk_buf[4];
    buf[i++] = bulk_buf[5];
    buf[i++] = bulk_buf[6];
    buf[i++] = bulk_buf[7];
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = send_csw;
    write_usb(buf,13);
}
#ifndef SDCARD_STORAGE
static void updatenandsectors(unsigned long sector,U32 sectorcnt)
{
	ALIGN4 U8 tempbuf[2048];
	ALIGN4 U8 buf[2048] ;
	U32 pageperblock = nand_get_pageperblock();
	U32 pagesize = nand_get_pagesize();
	int blocknum;
	int pagenum;
	int sectorperpage = pagesize/512;
	U32 startblock = (sector/sectorperpage)/pageperblock;
	U32 endblock = (sector+sectorcnt)/sectorperpage/pageperblock;
	U32 startsector = sector;
	U32 endsector=sector+sectorcnt-1;
	int i;
	for(blocknum=startblock;blocknum<endblock+1;blocknum++)
	{
		if(nand_phy_eraseblock(0)==FALSE)
		{
			debugstring("block 0 erase error\r\n");
			return;
		}
		//backup 
		U32 zeropagenum =0;
		if(pagesize==512)
		{
			for(pagenum=blocknum*pageperblock;pagenum<blocknum*pageperblock+pageperblock;pagenum++)
			{
				if( (pagenum >= startsector) 
					&&  (pagenum <= endsector) )
				{
					for(i=0;i<512;)
					{
						read_usb_full(&buf[i]);
						i+=64;
					}
					nand_phy_writepage(zeropagenum,buf);
				}
				else
				{
					nand_readpage(pagenum,tempbuf);
					nand_phy_writepage(zeropagenum,tempbuf);
				}
				zeropagenum++;
			}
		}
		else//2048 page
		{
			U32 startpage = startsector/4;
			U32 endpage = endsector/4;
			for(pagenum=blocknum*pageperblock;pagenum<blocknum*pageperblock+pageperblock;pagenum++)
			{
				if( (pagenum >= startpage) 
					&&  (pagenum <= endpage) )
				{
					U32 sectornum=pagenum*4;//start sector number in page
					U32 copysectorcnt;
					U32 offsetsectorinpage;
					if(sectornum <= startsector) // page |--|data|?|
					{
						offsetsectorinpage = startsector-sectornum;
						copysectorcnt = 4-offsetsectorinpage;
						if(startpage == endpage)
						{
							copysectorcnt = endsector+1 -startsector;
						}						
					}
					else//page |data|?|
					{
						offsetsectorinpage = 0;
						copysectorcnt = endsector+1-sectornum;
					}
					if(copysectorcnt>=4)
					{
						for(i=0;i<2048;)
						{
							read_usb_full(&buf[i]);
							i+=64;
						}
						nand_phy_writepage(zeropagenum,buf);
					}
					else
					{
						for(i=0;i<copysectorcnt*512;)
						{
							read_usb_full(&buf[i]);
							i+=64;
						}
						nand_readpage(pagenum,tempbuf);
						memcpy(tempbuf+offsetsectorinpage*512,buf,copysectorcnt*512);
						nand_phy_writepage(zeropagenum,tempbuf);
					}

					
				}
				else
				{
					nand_readpage(pagenum,tempbuf);
					nand_phy_writepage(zeropagenum,tempbuf);
				}
				zeropagenum++;
			}
		}
		
		//restore
		if(nand_eraseblock(blocknum)==FALSE)
		{
			return ;
		}

		for(pagenum=0;pagenum<pageperblock;pagenum++)
		{
			nand_phy_readpage(pagenum,tempbuf);
			nand_writepage(blocknum*pageperblock+pagenum,tempbuf);
		}
	}
}
#endif

static void WriteData(unsigned long baddr,U32 blkcnt)
{
	
#ifdef SDCARD_STORAGE		
	ALIGN4 U8 buf[512];
	int i;
	int k;
	for(k=0;k<blkcnt;k++)
	{
		for(i=0;i<512;)
		{
			read_usb_full(&buf[i]);
			i+=64;
		}
		SDC_Write(buf,(baddr+k)*512,512);
	}
#else
	updatenandsectors(baddr,blkcnt);
#endif	
	
}

static void SendReadData(unsigned long baddr,U32 blkcnt)
{
	ALIGN4 U8 buf[512];
	int i;
	int k;
	for(k=0;k<blkcnt;k++)
	{
#ifdef SDCARD_STORAGE	
		SDC_Read(buf,(baddr+k)*512,512);
#else
		if(nand_readsect(baddr+k,buf)==FALSE)
			debugprintf("nand_readpage(%d) error\r\n",baddr+k);
#endif	
		for(i=0;i<512;)
		{
			write_usb_full(&buf[i]);
			i+=64;
		}
	}
}


void UFI_CMDCHK(void)
{
    unsigned char dir;//direction
    unsigned char cmd;
    unsigned short dataTransferLen;
    union byte2 TransLen;
    union BYTE4 BlkAddr;
    unsigned int bcnt1;


    dir = wrapp->Flag;//direction

    dataTransferLen = wrapp->DTL;
    cmd = bulk_buf[0xf];
    send_csw = 0;

    TransLen.wordsp.bl=bulk_buf[0x17];
    TransLen.wordsp.bh=bulk_buf[0x16];
    BlkAddr.longsp.ll=bulk_buf[0x14];
    BlkAddr.longsp.lh=bulk_buf[0x13];
    BlkAddr.longsp.hl=bulk_buf[0x12];
    BlkAddr.longsp.hh=bulk_buf[0x11];


    if (wrapp->Flag & 0x80) // device to host
    {
//		DEBUGPRINTF("DEVICE TO HOST ( %#x)\r\n",cmd);
        switch (cmd)
        {
        case INQUIRY:               // 0x12
            DEBUGUFI("INQUIRY\r\n");
            write_usb(InquiryData,sizeof(InquiryData));
            break;

        case READ_CAPACITY:         		// 0x25
            DEBUGUFI("READ_CAPACITY\r\n");

            write_usb(ReadCapa,8);
            break;

        case READ_10:               // 0x28
            DEBUGUFI("Read10 : BlkAddr.b4 :0x%x, TransLen.bw:0x%x\r\n",BlkAddr.b4 ,TransLen.bw);
            SendReadData(BlkAddr.b4,TransLen.bw);
            break;
        case READ_FORMAT:       			// 0x23
            DEBUGUFI("READ_FORMAT\r\n");
            write_usb(ReadFromatCapa,12);
            break;
        case MODE_SENSE_06:         		// 0x1A
            DEBUGUFI("MODE_SENSE_06\r\n");
            if (bulk_buf[17]!=0x3f)
            {
                DEBUGUFI(" ==== TEST  ==== \r\n");
                SenseData[12]=0x24;
                send_csw = 1;
                write_usb(ModeParam,0);
            }
            else
            {
                write_usb(ModeParam,12);
            }
            break;
        case REQUEST_SENSE:         		// 0x03
            DEBUGUFI("REQUEST_SENSE\r\n");
            write_usb(SenseData,18);
            break;


        default:
            DEBUGUFI("Unknown UFI Command 0x%x \r\n",cmd);
            send_csw = 1;
            break;
        }

    }
    else // host to device
    {
//		DEBUGPRINTF("HOST TO DEVICE ( %#x)\r\n",cmd);
        switch (cmd)
        {
        case WRITE_10:              // 0x2A
            bcnt1=TransLen.bw;
            DEBUGUFI("\r\nWRITE_10 , addr :0x%x , bcnt1 : 0x%x\r\n",BlkAddr.b4,bcnt1);
            WriteData(BlkAddr.b4,bcnt1);
            break;
        case PREVENT_ALLOW_MEDIUM_REMOVAL: 	// 0x1E
            DEBUGUFI("PREVENT_ALLOW_MEDIUM_REMOVAL\r\n");
            SenseData[12]=0x20;
            send_csw = 1;
            break;

        case VERIFY_10:             		// 0x2F
            DEBUGUFI("VERIFY_10\r\n");
#ifndef SDCARD_STORAGE 	
			nand_flushdata();
#endif		
            break;

        case TEST_UNIT_READY:       		// 0x00
#ifndef SDCARD_STORAGE 				
			nand_flushdata();
#endif		
            break;

        case MODE_SELECT_06:        		// 0x15
            DEBUGUFI("MODE_SELECT_06\r\n");
            break;
        case STOP_START_UNIT:   			// 0x1B
            DEBUGUFI("STOP_START_UNIT\r\n");
            break;

        default:
            DEBUGUFI("Unknown UFI Command 0x%x \r\n",cmd);
            send_csw = 1;
            while (1);
            break;

        }
    }
    sendCSW();
}

void masstorage_proc()
{
    struct CBW wrap1;
    read_usb(bulk_buf,64);
    memcpy(&wrap1,bulk_buf,sizeof(wrap1));
    wrapp=&wrap1;
    UFI_CMDCHK();
}


void InitStorageInfo()
{
	unsigned int pagesize;
	unsigned int fatpagecnt;
	pagesize = 512;//fix, pagesize == blocksize(not nand block)
#ifdef SDCARD_STORAGE	
	SDC_Init();
	fatpagecnt = SDC_GetSectorCount()-1;//sdcard
#else	
	//nand_init(0x1223);
	nand_set_cfg(0x3444);
	nand_init();
	fatpagecnt = nand_get_memsize_kbyte()*2;
#endif	
	ReadCapa[7] =  pagesize & 0xff;
	ReadCapa[6] = (pagesize>>8) & 0xff;
	ReadCapa[5] = (pagesize>>16) & 0xff;
	ReadCapa[4] = (pagesize>>24) & 0xff;
	
	ReadCapa[3] = fatpagecnt & 0xff;
	ReadCapa[2] = (fatpagecnt>>8) & 0xff;
	ReadCapa[1] = (fatpagecnt>>16) & 0xff;
	ReadCapa[0] = (fatpagecnt>>24) & 0xff;

	ModeParam[8] = fatpagecnt & 0xff;
	ModeParam[7] = (fatpagecnt >> 8) & 0xff;
	ModeParam[6] = (fatpagecnt >> 16) & 0xff;
	ModeParam[5] = (fatpagecnt >> 24) & 0xff;
	ModeParam[10] = 0x2;
	
	ReadFromatCapa[7] = (fatpagecnt & 0xff);
	ReadFromatCapa[6] = (fatpagecnt>>8) & 0xff;
	ReadFromatCapa[5] = (fatpagecnt>>16) & 0xff;
	ReadFromatCapa[4] = (fatpagecnt>>24) & 0xff;
	
	ReadFromatCapa[8] = 0x02;
	ReadFromatCapa[10] = 0x2;	
}

void mass_storage_main()
{
	BYTE ep_irq ;
    BYTE usb_irq ;


	InitStorageInfo();
	usb_set_description((BYTE*)Cfg_Descp_Mass,(BYTE*)Dev_Descp_Mass);
	usb_init();
	
	while(1)
	{    
		ep_irq  = *(volatile unsigned char*)__USBEI_REG;
		usb_irq = *(volatile unsigned char*)__USBUI_REG;
		
		if(ep_irq & EP0_IRQ)
		{
			ep0_isr(); //usb.c
		}
		/*
		if (ep_irq & EP2_IRQ)
		{
			debugstring("\r\nEP2_IRQ\r\n");
		}
		*/
		if (ep_irq & EP1_IRQ)
		{
			masstorage_proc();
		}
		/*
		if (ep_irq & EP2_IRQ)
		{
			debugstring("EP2_IRQ\r\n");
		}
		*/
		if(usb_irq & USB_RESET_IRQ)
		{
			usb_reset();
		}
		if ( usb_irq & USB_RESUME_IRQ )
		{
			usb_resume();
		}
		if ( usb_irq & USB_SUSPEND_IRQ )
		{
			usb_suspend();
		}
		
		*(volatile U8*)__USBEI_REG = ep_irq;   // Clear Interrupt 
		*(volatile U8*)__USBUI_REG = usb_irq;  // Clear Interrupt
	}
}
