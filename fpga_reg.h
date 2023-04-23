#ifndef fpga_cfg_HEADER_H
#define fpga_cfg_HEADER_H

#define REGISTER_BASE_AD7656	    0x43c00000	/* AD7656 */
#define REGISTER_BASE_FIFO_READ_0	0x43C10000	/* FIFO_READ_0 */
#define REGISTER_BASE_FIFO_READ_1	0x43C20000	/* FIFO_READ_1 */
#define REGISTER_BASE_FIFO_READ_2	0x43C30000	/* FIFO_READ_2 */
#define REGISTER_BASE_FIFO_READ_3	0x43C40000	/* FIFO_READ_3 */
#define REGISTER_BASE_FIFO_READ_4	0x43C50000	/* FIFO_READ_4 */    

typedef struct{
    unsigned int dummy_0;            
    unsigned int reg_control;       //1    
    unsigned int reg_reset;         //2  
    unsigned int empty;             //3
    unsigned int full;           	//4	
    unsigned int fifo_data;         //5    
    unsigned int dummy_6;           
    unsigned int dummy_7;           
    unsigned int dummy_8;           
    unsigned int ver;               //9
    unsigned int dummy_10;           
    unsigned int dummy_11;           
    unsigned int dummy_12;           
    unsigned int dummy_13;
    unsigned int dummy_14;           
    unsigned int dummy_15;           
    unsigned int dummy_16;           
    unsigned int dummy_17; 
    unsigned int dummy_18;           
    unsigned int dummy_19;           
    unsigned int dummy_20;                     			
} REGISTER_CONFIG_CS0;

REGISTER_CONFIG_CS0 *fifo_read_0;
REGISTER_CONFIG_CS0 *fifo_read_1;
REGISTER_CONFIG_CS0 *fifo_read_2;
REGISTER_CONFIG_CS0 *fifo_read_3;
REGISTER_CONFIG_CS0 *fifo_read_4;

typedef struct{
    unsigned int reg_reset;            
    unsigned int reg_count_reset;       //1    
    unsigned int sensing_count;         //2  
    unsigned int sens_interval;             //3
    unsigned int total_count;           	//4	
    unsigned int auto_onoff;         //5    
    unsigned int dummy_6;           
    unsigned int dummy_7;           
    unsigned int dummy_8;           
    unsigned int dummy_9;               //9
    unsigned int ver;           
    unsigned int manual_trigger_count;           
    unsigned int auto_trigger_count;           
    unsigned int end_count;
    unsigned int z_sum;           
    unsigned int xy_sum;           
    unsigned int dummy_16;           
    unsigned int dummy_17; 
    unsigned int dummy_18;           
    unsigned int dummy_19;           
    unsigned int dummy_20;                     			
} REGISTER_CONFIG_AD7656;

REGISTER_CONFIG_AD7656 *ad7656;

#endif