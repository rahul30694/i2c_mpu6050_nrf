

#include <stdbool.h>
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "app_util_platform.h"

/* TWI PINS */
#define TWI_SCL_PIN     24
#define TWI_SDA_PIN     25

/* MPU-6050 I2C-ADDRESS */
#define MPU_6050_I2C_ADDR   0x68

/* MPU-6050 Register Map */
#define WHO_AM_I            0x75
#define ACCEL_XOUT_H        0x3B
#define TEMP_OUT_H          0x41    
#define GYRO_XOUT_H         0x43         
#define PWR_MGMT_1          0x6B

/* TWI instance. */
static const nrf_drv_twi_t m_twi_mpu_6050 = NRF_DRV_TWI_INSTANCE(0);

/**
 * @brief Function for initializing the TWI peripheral.
 */
 
void twi_init (void)
{
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t twi_mpu_6050_config = {
        .scl            = TWI_SCL_PIN,
        .sda            = TWI_SDA_PIN,
        .frequency      = NRF_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };
    
    err_code = nrf_drv_twi_init(&m_twi_mpu_6050, &twi_mpu_6050_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&m_twi_mpu_6050);
}

/**
 * @brief Function for writing to registers on the MPU-6050.
 */

ret_code_t mpu_6050_I2C_register_write(uint8_t reg_addr, uint8_t * p_tx_data, uint8_t bytes)
{
    ret_code_t ret_code;
    
    uint8_t tx_data[bytes+1];
    
    tx_data[0] = reg_addr;
    
    for(uint8_t i = 0 ; i<bytes ; i++) 
    {
        tx_data[i+1] = p_tx_data[i];
    }   
   
    ret_code = nrf_drv_twi_tx(&m_twi_mpu_6050, MPU_6050_I2C_ADDR, tx_data, sizeof(tx_data), false);
    
    return ret_code;
}

/**
 * @brief Function for reading from registers on the MPU-6050.
 */

 ret_code_t mpu_6050_I2C_register_read( uint8_t reg_addr,  uint8_t * p_rx_data, uint32_t bytes)
{   
    ret_code_t ret_code;
    
    ret_code = nrf_drv_twi_tx(&m_twi_mpu_6050,MPU_6050_I2C_ADDR, &reg_addr, 1, false);
    
    if(ret_code != NRF_SUCCESS)
    {
        return ret_code;
    }
    
    ret_code = nrf_drv_twi_rx(&m_twi_mpu_6050, MPU_6050_I2C_ADDR, p_rx_data, bytes,false);
    
    return ret_code;
}

void mpu_6050_init (void)
{
    ret_code_t err_code;
    
    uint8_t tx_data = 0;
    
    // Write zero to the PWR_MGMT_1 register to wake up the MPU-6050
    err_code = mpu_6050_I2C_register_write(PWR_MGMT_1, &tx_data, 1);
    
    APP_ERROR_CHECK(err_code);
}

void mpu_6050_get_device_id (uint8_t * p_dev_id)
{
    ret_code_t err_code;
    
    uint8_t rx_data;
    
    // Read the I2C Address of the MPU-6050 from the WHO_AM_I register
    err_code = mpu_6050_I2C_register_read(WHO_AM_I,&rx_data,1);
    
    APP_ERROR_CHECK(err_code);
    
    *p_dev_id = rx_data;
}

void mpu_6050_read_acc(int16_t * p_x_val, int16_t * p_y_val, int16_t * p_z_val)
{
    ret_code_t err_code;
    
    // Raw accelerometer measurements buffer
    uint8_t acc_data[6];
    
    // Read the six accelerometer data registers starting from ACCEL_XOUT_H
    err_code = mpu_6050_I2C_register_read(ACCEL_XOUT_H,acc_data,sizeof(acc_data));
    
    APP_ERROR_CHECK(err_code);
    
    /*  Combine the two 8-bit data registers to a 16-bit value
        for each axis by left shifting ACCEL_xOUT_H eight times 
        and OR it with ACCEL_xOUT_L. */
    *p_x_val = (acc_data[0]<<8)|acc_data[1];
    *p_y_val = (acc_data[2]<<8)|acc_data[3];
    *p_z_val = (acc_data[4]<<8)|acc_data[5];
}

void mpu_6050_read_gyro(int16_t * p_x_gyro, int16_t * p_y_gyro, int16_t * p_z_gyro)
{
    ret_code_t err_code;
    
    uint8_t gyro_data[6];
    
    //Read the 6 gyroscope data registers starting from GYRO_XOUT_H
    err_code = mpu_6050_I2C_register_read(GYRO_XOUT_H, gyro_data, sizeof(gyro_data));
    
    APP_ERROR_CHECK(err_code);
    
    /*  Combine the two 8-bit data registers to a 16-bit value
        for each axis by left shifting GYRO_xOUT_H eight times 
        and OR it with GYRO_xOUT_L. */
    *p_x_gyro = (gyro_data[0]<<8)|gyro_data[1];
    *p_y_gyro = (gyro_data[2]<<8)|gyro_data[3];
    *p_z_gyro = (gyro_data[4]<<8)|gyro_data[5];
}

void mpu_6050_read_temp(double * p_temp)
{
    ret_code_t err_code;
    
     // Signed 16-bit integer to store the measurements 
    int16_t temp;
    
    // Raw temperature measurements buffer
    uint8_t temp_data[2];
    
    //Read the two temperature data registers starting from TEMP_OUT_H
    err_code = mpu_6050_I2C_register_read(GYRO_XOUT_H, temp_data, sizeof(temp_data));
    
    APP_ERROR_CHECK(err_code);
    
      
    /*  Combine the two 8-bit data registers to a 16-bit value
        for each axis by left shifting TEMP_OUT_H eight times 
        and OR it with TEMP_OUT_L.  */
    temp = (temp_data[0]<<8)|temp_data[1];
    
    /*  Convert raw measurement to degrees C using formula from TEMP_OUT_H register description 
        in MPU-6050 Register Map and Descriptions document  */
    *p_temp = (temp/340.0)+ 36.53;
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{  
    // Initialize the TWI peripheral
    twi_init();
    
    //Initialize MPU-6050
    mpu_6050_init();
     
    uint8_t device_id;
    
    // Read Device ID
    mpu_6050_get_device_id(&device_id);
     
    int16_t x_val;
    int16_t y_val;
    int16_t z_val;
    
    // Read Accelerometer Data
    mpu_6050_read_acc(&x_val, &y_val, &z_val);
    
    int16_t x_gyro;
    int16_t y_gyro;
    int16_t z_gyro;
    
    // Read Gyroscope Data
    mpu_6050_read_gyro(&x_gyro, &y_gyro, &z_gyro);
    
    double temp_c;
    
    //Read Temperature Data
    mpu_6050_read_temp(&temp_c);

    while (true)
    {
        // Do nothing.
    }
}
/** @} */
