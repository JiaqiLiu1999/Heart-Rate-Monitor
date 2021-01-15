# Heart-Rate-Monitor

## Contribution
This project is based measuring the heart rate using MAX30102 the Heart-Rate Oximeter Pulse sensor, OLED display and FRDMKL03 ARM board. The implementation is achieved using the Warp firmware[1]. Four new files related to the sensors are created which are devMAX30102.c, devMAX30102.h, devSSD1331.c and devSSD1331.h. Moreover, four files are modified, which are warp-kl03-ksdk1.1-boot.c, warp.h,CMakeLists.txt and build.sh.

## Implementation
devMAX30102.c, devMAX30102.h are created to enable the Heart-Rate Pulse sensor to communicate with FRDMKL03 via I2C. MAX30102 works by sending light using LED and detecting the intensity of the reflected light using a photodetector. The change of light intensity relates to the change of blood vessel volume as the different volumes of blood vessels have different abilities in absorbing light. The raw data is read from the FIFO register, and the control of the read pointer should be aware of as the read pointer does not automatically increase the I2C. The raw data is then processed by the heart rate algorithm to provide a value of beats per minute, and four values are used to calculate the average result of bpm. Spo2 can also be calculated using MAX30102. However, the accuracy is affected by the register overflow according to the MAX30102 datasheet as Arm MO/M3 is used in this project. Therefore, Spo2 is not considered in this project.

The average result will be printed on the OLED display. devSSD1331.c and devSSD1331.h contain the code for printing a string consists of both char and number. When there is no value, it will ask the user to place a finger on the sensor. 

For warp-kl03-ksdk1.1-boot.c, changes are made from line 2561 to 2753. In addition, some functions of warp-kl03-ksdk1.1-boot.c are comment out as the sensors should be triggered once the power is on instead of triggering them by pressing 'z'. However, those functions are useful to check specific contents of registers while designing the project.

## Reference 
[1] https://github.com/physical-computation/Warp-firmware
