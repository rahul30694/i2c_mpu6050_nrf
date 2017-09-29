
#ifndef MPU6050_H
#define MPU6050_H


#include <stdbool.h>
#include <stdint.h>



#define ADDRESS_WHO_AM_I (0x75U) //!< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) //!< 

#define MPU6050_ADDRESS         0x69
#define MPU6050_GYRO_OUT        0x43
#define MPU6050_ACC_OUT         0x3B



bool mpu6050_init(uint8_t device_address);


bool mpu6050_register_write(uint8_t register_address, const uint8_t value);


bool mpu6050_register_read(uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes);


bool mpu6050_verify_product_id(void);



void MPU6050_ReadAcc( int16_t *pACC_X , int16_t *pACC_Y , int16_t *pACC_Z );
void MPU6050_ReadGyro(int16_t *pGYRO_X , int16_t *pGYRO_Y , int16_t *pGYRO_Z );




#endif 
