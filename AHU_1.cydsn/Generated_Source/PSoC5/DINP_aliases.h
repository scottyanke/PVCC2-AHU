/*******************************************************************************
* File Name: DINP.h  
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

#if !defined(CY_PINS_DINP_ALIASES_H) /* Pins DINP_ALIASES_H */
#define CY_PINS_DINP_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define DINP_0			(DINP__0__PC)
#define DINP_0_INTR	((uint16)((uint16)0x0001u << DINP__0__SHIFT))

#define DINP_1			(DINP__1__PC)
#define DINP_1_INTR	((uint16)((uint16)0x0001u << DINP__1__SHIFT))

#define DINP_2			(DINP__2__PC)
#define DINP_2_INTR	((uint16)((uint16)0x0001u << DINP__2__SHIFT))

#define DINP_3			(DINP__3__PC)
#define DINP_3_INTR	((uint16)((uint16)0x0001u << DINP__3__SHIFT))

#define DINP_INTR_ALL	 ((uint16)(DINP_0_INTR| DINP_1_INTR| DINP_2_INTR| DINP_3_INTR))

#endif /* End Pins DINP_ALIASES_H */


/* [] END OF FILE */
