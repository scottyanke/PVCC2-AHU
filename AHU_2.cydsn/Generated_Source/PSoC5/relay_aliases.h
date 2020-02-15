/*******************************************************************************
* File Name: relay.h  
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

#if !defined(CY_PINS_relay_ALIASES_H) /* Pins relay_ALIASES_H */
#define CY_PINS_relay_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define relay_0			(relay__0__PC)
#define relay_0_INTR	((uint16)((uint16)0x0001u << relay__0__SHIFT))

#define relay_INTR_ALL	 ((uint16)(relay_0_INTR))

#endif /* End Pins relay_ALIASES_H */


/* [] END OF FILE */
