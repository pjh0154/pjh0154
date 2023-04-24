#ifndef fpga_cfg_HEADER_H
#define fpga_cfg_HEADER_H

#define REGISTER_BASE_OCP   	        0x43c00000	
#define REGISTER_BASE_UART_PVSING	    0x42C00000	
#define REGISTER_BASE_UART_NVSING	    0x42C10000	
#define REGISTER_BASE_PATTERN_GENERATOR	0x43C10000	
#define REGISTER_BASE_ADS124S08 	    0x43C20000	
#define REGISTER_BASE_ADS124S08_RM	    0x43C30000	   
#define REGISTER_BASE_AX_GPIO_0         0x41200000
#define REGISTER_BASE_PCA9665           0x43c40000
#define REGISTER_BASE_AX_GPIO_ENBLE     0x41220000

/*typedef struct{
    unsigned int dummy_0;            
    unsigned int dummy_1;       //1    
    unsigned int reg_reset;         //2  
    unsigned int empty;             //3
    unsigned int full;           	//4	
    unsigned int fifo_data;         //5    
    unsigned int dummy_6;           
    unsigned int dummy_7;           
    unsigned int dummy_8;           
    unsigned int ver;               //9
    unsigned int dummy_10;                              			
} REGISTER_CONFIG_CS0;*/

typedef struct{
    unsigned int GPIO_DATA;
    unsigned int GPIO_TRI;
    unsigned int GPIO2_DATA;
    unsigned int GPIO2_TRI;
    unsigned int GGIER;     
    unsigned int IP_IER;
    unsigned int IP_ISR;                                                      			
} GPIO_REG;
GPIO_REG *gpio;
GPIO_REG *gpio_enble;

typedef struct{
    unsigned int OCP_STATUS;            
    unsigned int OCP_CONTROL;          
    unsigned int EACH_CHANNEL_OCP_FLAG;         
    unsigned int OCP_EN_MASK;                               			
} OCP;
OCP *ocp;

typedef struct{
    unsigned int PG_STATUS;            
    unsigned int PG_CONTROL;          
    unsigned int PG_OCP_DELAY;         
    unsigned int PG_LP_SEL;                               			
} PATTERN_GENERATOR;
PATTERN_GENERATOR *pattern_generator;

typedef struct{
    unsigned int PCA9665_CONTROL;            
    unsigned int PCA9665_ADDRESS;          
    unsigned int PCA9665_WDATA;         
    unsigned int PCA9665_RDATA;                                  			
} I2C_BRIDGE;
I2C_BRIDGE *i2c_bridge;

typedef struct{
    unsigned int ADC_STATUS;            
    unsigned int ADC_CONTROL;          
    unsigned int ADC_REG_ADDRESS;         
    unsigned int ADC_REG_WRITE;  
    unsigned int ADC_REG_READ;         
    unsigned int ADC_DATA_READ; 
    unsigned int ADC_AUTO_DATA_SEL;         
    unsigned int ADC_AUTO_DATA;
    unsigned int ADC_AUTO_DELAY;                                            			
} ADS124S08_SENSING;
ADS124S08_SENSING *ads124s08_sensing;
ADS124S08_SENSING *ads124s08_rm;

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