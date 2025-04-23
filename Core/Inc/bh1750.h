/*
 * bh1750.h
 *
 *  Created on: Jan 20, 2025
 *      Author: JakubLeb
 */

#ifndef INC_BH1750_H_
#define INC_BH1750_H_

#define BH1750_ADDRESS 0x23 << 1
#define BH1750_CMD_POWER_ON      0x01
#define BH1750_CMD_RESET         0x07
#define BH1750_CMD_CONT_H_MODE   0x10

void BH1750_Init(void);
void BH1750_StartMeasurement(void);
float BH1750_ReadLux(void);


#endif /* INC_BH1750_H_ */
