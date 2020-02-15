/*******************************************************************************
* File Name: DIP.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_DIP_H) /* Pins DIP_H */
#define CY_PINS_DIP_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "DIP_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 DIP__PORT == 15 && ((DIP__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    DIP_Write(uint8 value);
void    DIP_SetDriveMode(uint8 mode);
uint8   DIP_ReadDataReg(void);
uint8   DIP_Read(void);
void    DIP_SetInterruptMode(uint16 position, uint16 mode);
uint8   DIP_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the DIP_SetDriveMode() function.
     *  @{
     */
        #define DIP_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define DIP_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define DIP_DM_RES_UP          PIN_DM_RES_UP
        #define DIP_DM_RES_DWN         PIN_DM_RES_DWN
        #define DIP_DM_OD_LO           PIN_DM_OD_LO
        #define DIP_DM_OD_HI           PIN_DM_OD_HI
        #define DIP_DM_STRONG          PIN_DM_STRONG
        #define DIP_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define DIP_MASK               DIP__MASK
#define DIP_SHIFT              DIP__SHIFT
#define DIP_WIDTH              4u

/* Interrupt constants */
#if defined(DIP__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in DIP_SetInterruptMode() function.
     *  @{
     */
        #define DIP_INTR_NONE      (uint16)(0x0000u)
        #define DIP_INTR_RISING    (uint16)(0x0001u)
        #define DIP_INTR_FALLING   (uint16)(0x0002u)
        #define DIP_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define DIP_INTR_MASK      (0x01u) 
#endif /* (DIP__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define DIP_PS                     (* (reg8 *) DIP__PS)
/* Data Register */
#define DIP_DR                     (* (reg8 *) DIP__DR)
/* Port Number */
#define DIP_PRT_NUM                (* (reg8 *) DIP__PRT) 
/* Connect to Analog Globals */                                                  
#define DIP_AG                     (* (reg8 *) DIP__AG)                       
/* Analog MUX bux enable */
#define DIP_AMUX                   (* (reg8 *) DIP__AMUX) 
/* Bidirectional Enable */                                                        
#define DIP_BIE                    (* (reg8 *) DIP__BIE)
/* Bit-mask for Aliased Register Access */
#define DIP_BIT_MASK               (* (reg8 *) DIP__BIT_MASK)
/* Bypass Enable */
#define DIP_BYP                    (* (reg8 *) DIP__BYP)
/* Port wide control signals */                                                   
#define DIP_CTL                    (* (reg8 *) DIP__CTL)
/* Drive Modes */
#define DIP_DM0                    (* (reg8 *) DIP__DM0) 
#define DIP_DM1                    (* (reg8 *) DIP__DM1)
#define DIP_DM2                    (* (reg8 *) DIP__DM2) 
/* Input Buffer Disable Override */
#define DIP_INP_DIS                (* (reg8 *) DIP__INP_DIS)
/* LCD Common or Segment Drive */
#define DIP_LCD_COM_SEG            (* (reg8 *) DIP__LCD_COM_SEG)
/* Enable Segment LCD */
#define DIP_LCD_EN                 (* (reg8 *) DIP__LCD_EN)
/* Slew Rate Control */
#define DIP_SLW                    (* (reg8 *) DIP__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define DIP_PRTDSI__CAPS_SEL       (* (reg8 *) DIP__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define DIP_PRTDSI__DBL_SYNC_IN    (* (reg8 *) DIP__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define DIP_PRTDSI__OE_SEL0        (* (reg8 *) DIP__PRTDSI__OE_SEL0) 
#define DIP_PRTDSI__OE_SEL1        (* (reg8 *) DIP__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define DIP_PRTDSI__OUT_SEL0       (* (reg8 *) DIP__PRTDSI__OUT_SEL0) 
#define DIP_PRTDSI__OUT_SEL1       (* (reg8 *) DIP__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define DIP_PRTDSI__SYNC_OUT       (* (reg8 *) DIP__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(DIP__SIO_CFG)
    #define DIP_SIO_HYST_EN        (* (reg8 *) DIP__SIO_HYST_EN)
    #define DIP_SIO_REG_HIFREQ     (* (reg8 *) DIP__SIO_REG_HIFREQ)
    #define DIP_SIO_CFG            (* (reg8 *) DIP__SIO_CFG)
    #define DIP_SIO_DIFF           (* (reg8 *) DIP__SIO_DIFF)
#endif /* (DIP__SIO_CFG) */

/* Interrupt Registers */
#if defined(DIP__INTSTAT)
    #define DIP_INTSTAT            (* (reg8 *) DIP__INTSTAT)
    #define DIP_SNAP               (* (reg8 *) DIP__SNAP)
    
	#define DIP_0_INTTYPE_REG 		(* (reg8 *) DIP__0__INTTYPE)
	#define DIP_1_INTTYPE_REG 		(* (reg8 *) DIP__1__INTTYPE)
	#define DIP_2_INTTYPE_REG 		(* (reg8 *) DIP__2__INTTYPE)
	#define DIP_3_INTTYPE_REG 		(* (reg8 *) DIP__3__INTTYPE)
#endif /* (DIP__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_DIP_H */


/* [] END OF FILE */
