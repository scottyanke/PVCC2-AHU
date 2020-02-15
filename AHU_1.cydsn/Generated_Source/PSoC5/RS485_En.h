/*******************************************************************************
* File Name: RS485_En.h  
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

#if !defined(CY_PINS_RS485_En_H) /* Pins RS485_En_H */
#define CY_PINS_RS485_En_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "RS485_En_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 RS485_En__PORT == 15 && ((RS485_En__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    RS485_En_Write(uint8 value);
void    RS485_En_SetDriveMode(uint8 mode);
uint8   RS485_En_ReadDataReg(void);
uint8   RS485_En_Read(void);
void    RS485_En_SetInterruptMode(uint16 position, uint16 mode);
uint8   RS485_En_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the RS485_En_SetDriveMode() function.
     *  @{
     */
        #define RS485_En_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define RS485_En_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define RS485_En_DM_RES_UP          PIN_DM_RES_UP
        #define RS485_En_DM_RES_DWN         PIN_DM_RES_DWN
        #define RS485_En_DM_OD_LO           PIN_DM_OD_LO
        #define RS485_En_DM_OD_HI           PIN_DM_OD_HI
        #define RS485_En_DM_STRONG          PIN_DM_STRONG
        #define RS485_En_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define RS485_En_MASK               RS485_En__MASK
#define RS485_En_SHIFT              RS485_En__SHIFT
#define RS485_En_WIDTH              1u

/* Interrupt constants */
#if defined(RS485_En__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in RS485_En_SetInterruptMode() function.
     *  @{
     */
        #define RS485_En_INTR_NONE      (uint16)(0x0000u)
        #define RS485_En_INTR_RISING    (uint16)(0x0001u)
        #define RS485_En_INTR_FALLING   (uint16)(0x0002u)
        #define RS485_En_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define RS485_En_INTR_MASK      (0x01u) 
#endif /* (RS485_En__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define RS485_En_PS                     (* (reg8 *) RS485_En__PS)
/* Data Register */
#define RS485_En_DR                     (* (reg8 *) RS485_En__DR)
/* Port Number */
#define RS485_En_PRT_NUM                (* (reg8 *) RS485_En__PRT) 
/* Connect to Analog Globals */                                                  
#define RS485_En_AG                     (* (reg8 *) RS485_En__AG)                       
/* Analog MUX bux enable */
#define RS485_En_AMUX                   (* (reg8 *) RS485_En__AMUX) 
/* Bidirectional Enable */                                                        
#define RS485_En_BIE                    (* (reg8 *) RS485_En__BIE)
/* Bit-mask for Aliased Register Access */
#define RS485_En_BIT_MASK               (* (reg8 *) RS485_En__BIT_MASK)
/* Bypass Enable */
#define RS485_En_BYP                    (* (reg8 *) RS485_En__BYP)
/* Port wide control signals */                                                   
#define RS485_En_CTL                    (* (reg8 *) RS485_En__CTL)
/* Drive Modes */
#define RS485_En_DM0                    (* (reg8 *) RS485_En__DM0) 
#define RS485_En_DM1                    (* (reg8 *) RS485_En__DM1)
#define RS485_En_DM2                    (* (reg8 *) RS485_En__DM2) 
/* Input Buffer Disable Override */
#define RS485_En_INP_DIS                (* (reg8 *) RS485_En__INP_DIS)
/* LCD Common or Segment Drive */
#define RS485_En_LCD_COM_SEG            (* (reg8 *) RS485_En__LCD_COM_SEG)
/* Enable Segment LCD */
#define RS485_En_LCD_EN                 (* (reg8 *) RS485_En__LCD_EN)
/* Slew Rate Control */
#define RS485_En_SLW                    (* (reg8 *) RS485_En__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define RS485_En_PRTDSI__CAPS_SEL       (* (reg8 *) RS485_En__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define RS485_En_PRTDSI__DBL_SYNC_IN    (* (reg8 *) RS485_En__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define RS485_En_PRTDSI__OE_SEL0        (* (reg8 *) RS485_En__PRTDSI__OE_SEL0) 
#define RS485_En_PRTDSI__OE_SEL1        (* (reg8 *) RS485_En__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define RS485_En_PRTDSI__OUT_SEL0       (* (reg8 *) RS485_En__PRTDSI__OUT_SEL0) 
#define RS485_En_PRTDSI__OUT_SEL1       (* (reg8 *) RS485_En__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define RS485_En_PRTDSI__SYNC_OUT       (* (reg8 *) RS485_En__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(RS485_En__SIO_CFG)
    #define RS485_En_SIO_HYST_EN        (* (reg8 *) RS485_En__SIO_HYST_EN)
    #define RS485_En_SIO_REG_HIFREQ     (* (reg8 *) RS485_En__SIO_REG_HIFREQ)
    #define RS485_En_SIO_CFG            (* (reg8 *) RS485_En__SIO_CFG)
    #define RS485_En_SIO_DIFF           (* (reg8 *) RS485_En__SIO_DIFF)
#endif /* (RS485_En__SIO_CFG) */

/* Interrupt Registers */
#if defined(RS485_En__INTSTAT)
    #define RS485_En_INTSTAT            (* (reg8 *) RS485_En__INTSTAT)
    #define RS485_En_SNAP               (* (reg8 *) RS485_En__SNAP)
    
	#define RS485_En_0_INTTYPE_REG 		(* (reg8 *) RS485_En__0__INTTYPE)
#endif /* (RS485_En__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_RS485_En_H */


/* [] END OF FILE */
