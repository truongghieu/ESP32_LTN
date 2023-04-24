/* 
	DHT11 temperature sensor driver
*/

#ifndef DHT11_H_  
#define DHT11_H_

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

// == function prototypes =======================================

void 	setDHTgpio(int gpio);
void 	errorHandler(int response);
int 	readDHT();
extern float 	getHumidity();
extern float 	getTemperature();
int 	getSignalLevel( int usTimeOut, bool state );

#endif