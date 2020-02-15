/*******************************************************************************
* File Name: Tx_En.h  
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

#if !defined(CY_PINS_Tx_En_ALIASES_H) /* Pins Tx_En_ALIASES_H */
#define CY_PINS_Tx_En_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define Tx_En_0			(Tx_En__0__PC)
#define Tx_En_0_INTR	((uint16)((uint16)0x0001u << Tx_En__0__SHIFT))

#define Tx_En_INTR_ALL	 ((uint16)(Tx_En_0_INTR))

#endif /* End Pins Tx_En_ALIASES_H */


/* [] END OF FILE */
