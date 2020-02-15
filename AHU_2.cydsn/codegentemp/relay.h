/*******************************************************************************
* File Name: relay.h  
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

#if !defined(CY_PINS_relay_H) /* Pins relay_H */
#define CY_PINS_relay_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "relay_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 relay__PORT == 15 && ((relay__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    relay_Write(uint8 value);
void    relay_SetDriveMode(uint8 mode);
uint8   relay_ReadDataReg(void);
uint8   relay_Read(void);
void    relay_SetInterruptMode(uint16 position, uint16 mode);
uint8   relay_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the relay_SetDriveMode() function.
     *  @{
     */
        #define relay_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define relay_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define relay_DM_RES_UP          PIN_DM_RES_UP
        #define relay_DM_RES_DWN         PIN_DM_RES_DWN
        #define relay_DM_OD_LO           PIN_DM_OD_LO
        #define relay_DM_OD_HI           PIN_DM_OD_HI
        #define relay_DM_STRONG          PIN_DM_STRONG
        #define relay_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define relay_MASK               relay__MASK
#define relay_SHIFT              relay__SHIFT
#define relay_WIDTH              1u

/* Interrupt constants */
#if defined(relay__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in relay_SetInterruptMode() function.
     *  @{
     */
        #define relay_INTR_NONE      (uint16)(0x0000u)
        #define relay_INTR_RISING    (uint16)(0x0001u)
        #define relay_INTR_FALLING   (uint16)(0x0002u)
        #define relay_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define relay_INTR_MASK      (0x01u) 
#endif /* (relay__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define relay_PS                     (* (reg8 *) relay__PS)
/* Data Register */
#define relay_DR                     (* (reg8 *) relay__DR)
/* Port Number */
#define relay_PRT_NUM                (* (reg8 *) relay__PRT) 
/* Connect to Analog Globals */                                                  
#define relay_AG                     (* (reg8 *) relay__AG)                       
/* Analog MUX bux enable */
#define relay_AMUX                   (* (reg8 *) relay__AMUX) 
/* Bidirectional Enable */                                                        
#define relay_BIE                    (* (reg8 *) relay__BIE)
/* Bit-mask for Aliased Register Access */
#define relay_BIT_MASK               (* (reg8 *) relay__BIT_MASK)
/* Bypass Enable */
#define relay_BYP                    (* (reg8 *) relay__BYP)
/* Port wide control signals */                                                   
#define relay_CTL                    (* (reg8 *) relay__CTL)
/* Drive Modes */
#define relay_DM0                    (* (reg8 *) relay__DM0) 
#define relay_DM1                    (* (reg8 *) relay__DM1)
#define relay_DM2                    (* (reg8 *) relay__DM2) 
/* Input Buffer Disable Override */
#define relay_INP_DIS                (* (reg8 *) relay__INP_DIS)
/* LCD Common or Segment Drive */
#define relay_LCD_COM_SEG            (* (reg8 *) relay__LCD_COM_SEG)
/* Enable Segment LCD */
#define relay_LCD_EN                 (* (reg8 *) relay__LCD_EN)
/* Slew Rate Control */
#define relay_SLW                    (* (reg8 *) relay__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define relay_PRTDSI__CAPS_SEL       (* (reg8 *) relay__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define relay_PRTDSI__DBL_SYNC_IN    (* (reg8 *) relay__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define relay_PRTDSI__OE_SEL0        (* (reg8 *) relay__PRTDSI__OE_SEL0) 
#define relay_PRTDSI__OE_SEL1        (* (reg8 *) relay__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define relay_PRTDSI__OUT_SEL0       (* (reg8 *) relay__PRTDSI__OUT_SEL0) 
#define relay_PRTDSI__OUT_SEL1       (* (reg8 *) relay__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define relay_PRTDSI__SYNC_OUT       (* (reg8 *) relay__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(relay__SIO_CFG)
    #define relay_SIO_HYST_EN        (* (reg8 *) relay__SIO_HYST_EN)
    #define relay_SIO_REG_HIFREQ     (* (reg8 *) relay__SIO_REG_HIFREQ)
    #define relay_SIO_CFG            (* (reg8 *) relay__SIO_CFG)
    #define relay_SIO_DIFF           (* (reg8 *) relay__SIO_DIFF)
#endif /* (relay__SIO_CFG) */

/* Interrupt Registers */
#if defined(relay__INTSTAT)
    #define relay_INTSTAT            (* (reg8 *) relay__INTSTAT)
    #define relay_SNAP               (* (reg8 *) relay__SNAP)
    
	#define relay_0_INTTYPE_REG 		(* (reg8 *) relay__0__INTTYPE)
#endif /* (relay__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_relay_H */


/* [] END OF FILE */
