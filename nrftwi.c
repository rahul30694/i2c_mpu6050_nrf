#include "nrftwi.h"
#include <stdbool.h>
#include <stdint.h>


#define MAX_TIMEOUT_LOOPS (20000UL)

static bool twi_master_write(uint8_t * data, uint8_t data_length, bool issue_stop_condition)
{
uint32_t timeout=MAX_TIMEOUT_LOOPS;
 if (data_length==0)
 {
  return false;
  }
  
    NRF_TWI1->TXD           = *data++;
    NRF_TWI1->TASKS_STARTTX = 1;  
    
     while (true)
    {
        while (NRF_TWI1->EVENTS_TXDSENT == 0 && NRF_TWI1->EVENTS_ERROR == 0 && (--timeout))
        {
            // do nothing.
        }

        if (timeout == 0 || NRF_TWI1->EVENTS_ERROR != 0)
        {
            NRF_TWI1->EVENTS_ERROR = 0;
            NRF_TWI1->ENABLE       = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos; 
            NRF_TWI1->POWER        = 0;
            nrf_delay_us(5);
            NRF_TWI1->POWER        = 1;
            NRF_TWI1->ENABLE       = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
             
          (void)twi_master_init();
          return false
        }  
           NRF_TWI1->EVENTS_TXDSENT = 0;
        if (--data_length == 0)
        {
            break;
        }
        
      NRF_TWI1->TXD=*data++;  
   }
   
   if(issue_stop_condition)
   {
   NRF_TWI1->EVENTS_STOPPED= 0;
   NRF_TWI1->TASK_STOP=1;
   
    while(NRF_TWI1->EVENTS_STOPPED == 0) 
    {
    //do nothing
    }
   }
   return true
}
static bool twi_master_read(uint8_t *data,uint8_t data_length,bool issue_stop_condition)
{
uint32_t timeout =MAX_TIMEOUT_LOOPS;
  if (data_length==0)
   {
   return false
   }
   else if(data_length==1)
   { 
   NRF_PPI->CH[0].TEP=(uint32_t)&NRF_TWI1->TASKS_STOP;
   }
   else 
   {
   NRF_PPI->CH[0].TEP=(uint32_t)&NRF_TWI1->TASKS_SUSPEND;
   }
	NRF_PPI->CHENSET=PPI_CHENSET_CH0_Msk;
	NRF_PPI->EVENTS_RXREADY=0;
	NRF_PPI->TASKS_STARTRX=1;
   
   while(true)	
   {
   	while(NRF_TWI1->EVENTS_RXREADY==0 && NRF_TWI1->EVENTS_ERROR == 0 && (--timeout))
        { 
        //do nothing
        }

	NRF_TWI1->EVENTS_RXREADY=0;
	if(timeout==0 || NRF_TWI1->EVENTS_ERROR!=0)
	{ NRF_TWI1->EVENTS_ERROR = 0;
	  NRF_TWI1->ENABLE=TWI_ENABLE_ENABLE_Disabled  << TWI_ENABLE_ENABLE_Pos;
	   NRF_TWI1->POWER        = 0;
            nrf_delay_us(5);
            NRF_TWI1->POWER        = 1;
            NRF_TWI1->ENABLE       = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;

            (void)twi_master_init();

            return false;
        }
       *data++=NRF_TWI1->RXD; 
       
        if (--data_length == 1)
        {
            NRF_PPI->CH[0].TEP = (uint32_t)&NRF_TWI1->TASKS_STOP;
        }
        
       if (data_length == 0)
        {
            break;
        }
        nrf_delay_us(20);
        NRF_TWI1->TASKS_RESUME = 1;
     }
    /** @snippet [TWI HW master read] */

    /* Wait until stop sequence is sent */
    while(NRF_TWI1->EVENTS_STOPPED == 0)
    {
        // Do nothing.
    }
    NRF_TWI1->EVENTS_STOPPED = 0;

    NRF_PPI->CHENCLR = PPI_CHENCLR_CH0_Msk;
    return true;
}
 /*
  @brief Function for detecting stuck slaves (SDA = 0 and SCL = 1) and tries to clear the bus.
 */
static bool twi_master_clear_bus(void)
{
  uint32_t twi_state;
  bool bus_clear;
  uint32_t clk_pin_config;
  uint32_t data_pin_config;
  
  // Save and disable TWI hardware so software can take control over the pins.
    twi_state        = NRF_TWI1->ENABLE;
    NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
  
 clk_pin_config = \
        NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER];
     NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER] =      \
        (GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1    << GPIO_PIN_CNF_DRIVE_Pos)   \
      | (GPIO_PIN_CNF_PULL_Pullup   << GPIO_PIN_CNF_PULL_Pos)    \
      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)   \
      | (GPIO_PIN_CNF_DIR_Output    << GPIO_PIN_CNF_DIR_Pos);

    data_pin_config = \
        NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_DATA_PIN_NUMBER];
    NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_DATA_PIN_NUMBER] =       \
        (GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1    << GPIO_PIN_CNF_DRIVE_Pos)   \
      | (GPIO_PIN_CNF_PULL_Pullup   << GPIO_PIN_CNF_PULL_Pos)    \
      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)   \
      | (GPIO_PIN_CNF_DIR_Output    << GPIO_PIN_CNF_DIR_Pos);

    TWI_SDA_HIGH();
    TWI_SCL_HIGH();
    TWI_DELAY();
   
   if ((TWI_SDA_READ()==1) && (TWI_SCL_READ()==1)
   {
   bus_clear=true;
   }
   else
    {
        uint_fast8_t i;
        bus_clear = false;
          //clock maxm 18 pulse scenario
    for (i=18; i--;)
        {
            TWI_SCL_LOW();
            TWI_DELAY();
            TWI_SCL_HIGH();
            TWI_DELAY();

            if (TWI_SDA_READ() == 1)
            {
                bus_clear = true;
                break;
            }
        }
    }      
 
NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER] = clk_pin_config;
NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER] = data_pin_config;
   
    NRF_TWI1->ENABLE = twi_state;

    return bus_clear;
}
bool twi_master_init(void)
{
   
    NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER] =     \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[TWI_MASTER_CONFIG_DATA_PIN_NUMBER] =      \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    NRF_TWI1->EVENTS_RXDREADY = 0;
    NRF_TWI1->EVENTS_TXDSENT  = 0;
    NRF_TWI1->PSELSCL         = TWI_MASTER_CONFIG_CLOCK_PIN_NUMBER;
    NRF_TWI1->PSELSDA         = TWI_MASTER_CONFIG_DATA_PIN_NUMBER;
    NRF_TWI1->FREQUENCY       = TWI_FREQUENCY_FREQUENCY_K100 << TWI_FREQUENCY_FREQUENCY_Pos;
    NRF_PPI->CH[0].EEP        = (uint32_t)&NRF_TWI1->EVENTS_BB;
    NRF_PPI->CH[0].TEP        = (uint32_t)&NRF_TWI1->TASKS_SUSPEND;
    NRF_PPI->CHENCLR          = PPI_CHENCLR_CH0_Msk;
    NRF_TWI1->ENABLE          = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;

    return twi_master_clear_bus();
}


//  Function for transfer by twi_master.
 
bool twi_master_transfer(uint8_t address,uint8_t *data,uint8_t data_length,bool issue_stop_condition)
{
	bool transfer_succeeded=false;
    if(data_length > 0 && twi_master_clear_bus())
    {
	NRF_TWI1->ADDRESS=(address >> 1);
	if((address & TWI_READ_BIT))
	{
	 transfer_succeeded=twi_master_read(data,data_length,issue_stop_condition);
	}
        else
        {
        transfer_succeeded=twi_master_write(data,data_length,issue_stop_condition);
        }    
   }	
   return transfer_succeeded;
 }  
                                          