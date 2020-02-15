/*******************************************************************************
* File Name: DIP.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_DIP_ALIASES_H) /* Pins DIP_ALIASES_H */
#define CY_PINS_DIP_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define DIP_0			(DIP__0__PC)
#define DIP_0_INTR	((uint16)((uint16)0x0001u << DIP__0__SHIFT))

#define DIP_1			(DIP__1__PC)
#define DIP_1_INTR	((uint16)((uint16)0x0001u << DIP__1__SHIFT))

#define DIP_2			(DIP__2__PC)
#define DIP_2_INTR	((uint16)((uint16)0x0001u << DIP__2__SHIFT))

#define DIP_3			(DIP__3__PC)
#define DIP_3_INTR	((uint16)((uint16)0x0001u << DIP__3__SHIFT))

#define DIP_INTR_ALL	 ((uint16)(DIP_0_INTR| DIP_1_INTR| DIP_2_INTR| DIP_3_INTR))

#endif /* End Pins DIP_ALIASES_H */


/* [] END OF FILE */
