/*
 * as608.h
 *
 *  Created on: 2026年10月18日
 *      Author: Voyager
 */

#ifndef DRIVER_AS608_H_
#define DRIVER_AS608_H_

#include "mydefine.h"

void as608_init(void);
void PS_GetImage(void);
void PS_GenChar(u8 buffer);
void PS_RegModel(void);
void PS_StoreChar(u16 page);
void PS_Search(void);
void PS_Empty(void);
void PS_ReadIndexTable(u8 page);
void as608_task(void);



#endif /* DRIVER_AS608_H_ */
