
#include "cantus.h"
#define CMD_FIRMWARE_VER        0x77
#define CMD_NAND_WRITE          0x11
#define CMD_NAND_READ           0x12
#define CMD_NOR_WRITE           0x21
#define CMD_NOR_READ            0x22
#define CMD_NOR_READ            0x22
#define CMD_READ_NOR_ID         0x23
#define CMD_READ_NAND_ID        0x25
#define CMD_RUN_AT        		0x26

#define CMD_SPI_WRITE           0x41
#define CMD_SPI_READ            0x42

#define CMD_SET_RTC_TIME        0x50
#define CMD_MASK_SPI_CH         0x08

#define STATUS_GOOD     0x00
#define STATUS_BAD      0x01
#define STATUS_SIZE_OVERFLOW    0x02
#define STATUS_BAD_ID   0x03
#define STATUS_ERASING  0x04
#define STATUS_ADDR_INVALID 0x05
#define STATUS_STOP         0x06

#define CMD_TIMEOUT 1000000

#define DOWNLOAD_SIZE    2048
#define FIRMWARE_VERSION    "CANTUS DEV TOOLS F/W v1.0"

int WaitUSBBulkData(int timeout);

unsigned char ALIGN4 Tmp_Buf[DOWNLOAD_SIZE];
static unsigned int __nor_sectorsize = 0;
U8 usb_buf[64];

U8 Cfg_Descp[] = {
    0x09,		        // bLength: Size of descriptor
    0x02,				// bDescriptorType: Configuration	
    0x24,0x00,		// wTotalLength: Cfg+Ifc+Class+Ep = 46 bytes
    0x01,				// bNumInterfaces: 1 interface
    0x01,				// bConfigurationValue: 1
    0x00,				// iConfiguration: none
    0x40,				// bmAttributes: Self powered 
    0x32,				// MaxPower: 100mA
//----------------------------------------------------------------------
// INTERFACE DESCRIPTOR
//----------------------------------------------------------------------
    0x09,				// bLength: Size of descriptor
    0x04,				// bDescriptorType: Interface
    0x00,				// bInterfaceNumber: #1
    0x00,				// bAlternateSetting: #0
    0x02,				// bNumEndpoints: 4
    0x00,				// bInterfaceClass: HID-class
    0x00,				// bInterfaceSubClass: none
    0x00,				// bInterfaceProtocol: none
    0x00,				// iInterface: none
//----------------------------------------------------------------------
// ENDPOINT 1 DESCRIPTOR  (BULK OUT)
//----------------------------------------------------------------------
    0x09,				// bLength: Size of descriptor
    0x05,				// bDescriptorType: Endpoint
    0x01,				// bEndpointAddress: OUT, EP1
    0x02,				// bmAttributes: Bulk
    EP1_LEN,0x00,		// wMaxPacketSize: 
    0x01,	            // bInterval: 1ms  , ignored for Bulk
    0x00,0x00,			// 사용 안 되는 dummy 이다. WriteEP0등에서 처리 하기 쉽도록 더미를 두었다. 

//----------------------------------------------------------------------
// ENDPOINT 2 DESCRIPTOR  (BULK IN)
//----------------------------------------------------------------------
    0x09,				// bLength: Size of descriptor
    0x05,				// bDescriptorType: Endpoint
    0x82,				// bEndpointAddress: IN, EP2
    0x02,				// bmAttributes: Bulk
    EP2_LEN,0x00,		// wMaxPacketSize: 
    0x01,	            // bInterval: 1ms, ignored for Bulk 
    0x00,0x00
};
U8 Dev_Descp[] = {
    DEV_LEN,	        // bLength: Size of descriptor
    0x01,				// bDescriptorType: Device	
    0x00,0x01,			// bcdUSB: USB 1.1
    0x00,				// bDeviceClass: none
    0x00,				// bDeviceSubClass: none
    0x00,				// bDeviceProtocol: none
    EP0_LEN,			// bMaxPacketSize0: 16 bytes
    0xDC,0x0A,			// idVendor: 0x0ADC 
    0x18,0x00,  // idProduct: 
    0x01,0x00,			// bcdDevice: device release
    0x00,				// iManufacturer: 
    0x00,				// iProduct: 
    0x00,				// iSerialNumber: none
    0x01				// bnumconfigurations: 1
};

static int run_set_rtc()
{
	int hour, day, week, min, mon, sec, year;

	sec = usb_buf[1];
	min = usb_buf[2];
	hour = usb_buf[3];
	week = usb_buf[4];
	day = usb_buf[5];
	mon = usb_buf[6];
	year = usb_buf[7]
        | (usb_buf[8] << 8)
        | (usb_buf[9] << 16)
        | (usb_buf[10] << 24);

    week -= 1;
    year -= 2000;

	*R_RTCSEC = sec;
	*R_RTCMIN = min;
	*R_RTCHOUR = hour;
	*R_RTCDAY = day;
	*R_RTCWEEK = week;
	*R_RTCMON = mon;
	*R_RTCYEAR = year;
	return 0;
}
	
int read_nor_id(U8 *ids)
{
    int ret = 0;
//	PRINTLINE;
    norflash_readid (ids);
 //   PRINTLINE;
    //uput_str_hex(U_DBUG, " NOR ID[1]: 0x", ids[1], 2, "\n");

    if (ids[1] == 0x4F)
	{
		// CANTUS-512 NOR-Flash EN29LV040
		// 64K x 8
        __nor_sectorsize = 64*1024;
	}
    else if (ids[1] == 0x6E)
	{
		// CANTUS-128 NOR-Flash EN29LV010
		// 16K x 8
        __nor_sectorsize = 16*1024;
	}
	else if (ids[1] == 0xd5)
	{
		// CANTUS-128A NOR-Flash EN39LV010
		// 4K x 32
        __nor_sectorsize = 4*1024;
	}
    else
        ret = -1;
    return ret;
}

int run_nor_write(void)
{
    int ret = 0;
    int i;
    unsigned int addr, size;
    unsigned int rcnt = 0;

    addr = ((usb_buf[2])|(usb_buf[3]<<8)|(usb_buf[4]<<16)|(usb_buf[5]<<24));
    size = ((usb_buf[6])|(usb_buf[7]<<8)|(usb_buf[8]<<16)|(usb_buf[9]<<24));

    if ((addr + size) > norflash_get_size()) {
        debugstring("Size overflow.\n");
        usb_buf[0] = STATUS_SIZE_OVERFLOW;
        write_usb(usb_buf, 2);
        ret = -1;
        goto __exit;
    }

    if (addr < norflash_get_sector_size()) {
        debugstring("Invalid ADDR\n");
        usb_buf[0] = STATUS_ADDR_INVALID;
        write_usb(usb_buf, 5);
        ret = -1;
        goto __exit;
    }

    ret = norflash_erase(addr,
            (size + (__nor_sectorsize - 1)) & ~(__nor_sectorsize-1));

    usb_buf[0] = STATUS_GOOD;
    write_usb(usb_buf, 5);

    while(rcnt < size) {
        for (i = 0; i < DOWNLOAD_SIZE; ) 
		{
			ret = WaitUSBBulkData(CMD_TIMEOUT);
            if(ret == -1)
			{
                    usb_buf[0] = STATUS_BAD;
                    write_usb(usb_buf, 5);
                    goto __exit;
            }

            i += read_usb_full(&Tmp_Buf[i]);

            if (rcnt + i >= size) {
                break;
            }
        }

        if (Tmp_Buf[0] == '_' && Tmp_Buf[1] == 'S') {
            if (strcmp(&Tmp_Buf[2], "123$$STOP$$ABC") == 0) {
                usb_buf[0] = STATUS_STOP;
                debugstring("DOWNLOAD STOP!\n");
                write_usb(usb_buf, 2);
                break;
            }

        }

        if (i != DOWNLOAD_SIZE) {
            memset(&Tmp_Buf[i], 0xff, DOWNLOAD_SIZE - i);
        }

        ret = norflash_write (Tmp_Buf, addr, DOWNLOAD_SIZE);

        if (ret != 0) {
            usb_buf[0] = STATUS_BAD;
            usb_buf[1] = addr & 0xff;
            usb_buf[2] = (addr & 0xff00) >> 8;
            usb_buf[3] = (addr & 0xff0000) >> 16;
            usb_buf[4] = (addr & 0xff000000) >> 24;
            write_usb(usb_buf, 5);
            break;
        }
        else {
            usb_buf[0] = STATUS_GOOD;
        }

        write_usb(usb_buf, 5);
        addr += DOWNLOAD_SIZE;

        rcnt += i;
    }
__exit:
    return ret;
}

static int run_nor_read(void)
{
    int ret = 0;
    unsigned int i;
    unsigned int addr, size;
    unsigned int wsize;
    //__u8 verify;
    unsigned int usb_delay = 0;

    addr = ((usb_buf[2]) | (usb_buf[3]<<8) | (usb_buf[4]<<16) | (usb_buf[5]<<24));
    size = ((usb_buf[6]) | (usb_buf[7]<<8) | (usb_buf[8]<<16) | (usb_buf[9]<<24));
    //verify = usb_buf[10];


    if ((addr + size) > norflash_get_size()) {
        debugstring( "Size overflow.\n");

        size = size - ((addr+size)-norflash_get_size());
    }

    usb_buf[0] = STATUS_GOOD;
    write_usb(usb_buf, 1);

    while (size) {
        usb_delay = 0;
		ret = WaitUSBBulkData(CMD_TIMEOUT);
		if(ret == -1)
                goto __exit;
        
        usb_buf[0] = 0;
        read_usb_full(&usb_buf[0]);

        if ((usb_buf[0] == 'S') || (usb_buf[0] != 'C')) {
            debugstring( "UPLOAD STOP!\n");
            break;
        }

        /* READ NOR */
        for (i = 0; i < DOWNLOAD_SIZE; i++) {
            Tmp_Buf[i] = *(volatile U8*)(addr+i);
        }

        for (i = 0; i < DOWNLOAD_SIZE; ) {
            if (size >= EP1_LEN)
                wsize = EP1_LEN;
            else
                wsize = size;

            write_usb(&Tmp_Buf[i], wsize);

            i += EP1_LEN;
            size -= wsize;
        }

        addr += DOWNLOAD_SIZE;
    }

__exit:

    //uput_str_hex(U_DBUG, "usb_delay: 0x", usb_delay, 8, "\n");
    //uput_str_hex(U_DBUG, "NAND UPLOAD END: 0x", ret, 2, "\n");

    return ret;
}
void usb_cmd_proc()
{
    int ret;
    int down_complete = 0;
    static char firsttime=1;
    U8 cmd;
	U32 startaddr ;
	void (*startfp)();
    
    if (firsttime)
    {
		read_usb_full(usb_buf);
		cmd = usb_buf[0];
        if (cmd == CMD_FIRMWARE_VER)
        {
            memset(usb_buf, 0x55, EP1_LEN);
            strncpy(usb_buf, FIRMWARE_VERSION, EP1_LEN);
            usb_buf[EP1_LEN-1] = 0;
            //uput_str_hex(U_DBUG, "Version: ", usb_buf, "\n");
            write_usb(usb_buf, EP1_LEN);
            firsttime = 0;
        }
        return;
    }
    while(1)
    {
		read_usb_full(usb_buf);
		cmd = usb_buf[0];
        switch (cmd)
        {

        case CMD_NOR_WRITE:
            run_nor_write();
            down_complete = 1;
            break;

        case CMD_NOR_READ:
            run_nor_read();
            down_complete = 1;
            break;

        case CMD_READ_NOR_ID:
            ret = read_nor_id(usb_buf);
            if (ret != 0)
            {
                down_complete = 1;
                usb_buf[0] = STATUS_BAD;
                usb_buf[1] = STATUS_BAD;
                write_usb(usb_buf, 2);
            }
            else
            {
                write_usb(usb_buf, 2);
            }

            down_complete = 0;
            break;
        case CMD_SET_RTC_TIME:
            run_set_rtc();
            usb_buf[0] = STATUS_GOOD;
            write_usb(usb_buf, 1);
            down_complete = 1;
            break;
		case CMD_RUN_AT:
			startaddr = usb_buf[4]+(usb_buf[5]<<8)+(usb_buf[6]<<16)+(usb_buf[7]<<24);
			startfp = (void (*)())startaddr;
			dcache_invalidate_way();
			startfp();
        default:
            debugprintf("Unknown CMD: 0x%x\r\n", usb_buf[0]);
            usb_buf[0] = STATUS_BAD;
            write_usb(usb_buf, 5);
            down_complete = 1;
            break;
        }
        if (down_complete)
            break;
    }
}

int WaitUSBBulkData(int timeout)
{
	BYTE ep_irq ;
    BYTE usb_irq ;
	int i=0;
	int re=-1;
	while(1)
	{    
		ep_irq  = *(volatile U8*)(__USBEI_REG);
		usb_irq = *(volatile U8*)(__USBUI_REG);
		
		
		if(ep_irq & 1)//EP0_IRQ
		{
			ep0_isr();
		}
		if (ep_irq & (1<<1))//EP1_IRQ
		{
			re = 1;
			break;
		}

		if(usb_irq & USB_RESET_IRQ)
		{
			usb_reset();
		}
		else if ( usb_irq & USB_RESUME_IRQ )
		{
			usb_resume();
		}
		else if ( usb_irq & USB_SUSPEND_IRQ )
		{
			usb_suspend();
		}
		if(timeout != -1)
			i++;
		if(i>timeout)
			break;	
		*(volatile U8*)__USBEI_REG = ep_irq;   // Clear Interrupt 
		*(volatile U8*)__USBUI_REG = usb_irq;  // Clear Interrupt

	}
	*(volatile U8*)__USBEI_REG = ep_irq;   // Clear Interrupt 
	*(volatile U8*)__USBUI_REG = usb_irq;  // Clear Interrupt

	return re;
}

void download_main()
{
    //USB On, not USB ctrl
    *R_P1oDIR = (1<<5);//PIO1.5 output
    *R_P1oHIGH = (1<<5);//PIO1.5 high , on
    delayms(100);
	usb_set_description(Cfg_Descp,Dev_Descp);
    usb_init();
	while(1)
	{
		while(WaitUSBBulkData(0xfffffff)==-1);
		usb_cmd_proc();
	}
}
