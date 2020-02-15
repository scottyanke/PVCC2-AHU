/*******************************************************************************
* File Name: DINP.h  
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

#if !defined(CY_PINS_DINP_H) /* Pins DINP_H */
#define CY_PINS_DINP_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "DINP_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 DINP__PORT == 15 && ((DINP__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    DINP_Write(uint8 value);
void    DINP_SetDriveMode(uint8 mode);
uint8   DINP_ReadDataReg(void);
uint8   DINP_Read(void);
void    DINP_SetInterruptMode(uint16 position, uint16 mode);
uint8   DINP_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the DINP_SetDriveMode() function.
     *  @{
     */
        #define DINP_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define DINP_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define DINP_DM_RES_UP          PIN_DM_RES_UP
        #define DINP_DM_RES_DWN         PIN_DM_RES_DWN
        #define DINP_DM_OD_LO           PIN_DM_OD_LO
        #define DINP_DM_OD_HI           PIN_DM_OD_HI
        #define DINP_DM_STRONG          PIN_DM_STRONG
        #define DINP_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define DINP_MASK               DINP__MASK
#define DINP_SHIFT              DINP__SHIFT
#define DINP_WIDTH              4u

/* Interrupt constants */
#if defined(DINP__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in DINP_SetInterruptMode() function.
     *  @{
     */
        #define DINP_INTR_NONE      (uint16)(0x0000u)
        #define DINP_INTR_RISING    (uint16)(0x0001u)
        #define DINP_INTR_FALLING   (uint16)(0x0002u)
        #define DINP_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define DINP_INTR_MASK      (0x01u) 
#endif /* (DINP__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define DINP_PS                     (* (reg8 *) DINP__PS)
/* Data Register */
#define DINP_DR                     (* (reg8 *) DINP__DR)
/* Port Number */
#define DINP_PRT_NUM                (* (reg8 *) DINP__PRT) 
/* Connect to Analog Globals */                                                  
#define DINP_AG                     (* (reg8 *) DINP__AG)                       
/* Analog MUX bux enable */
#define DINP_AMUX                   (* (reg8 *) DINP__AMUX) 
/* Bidirectional Enable */                                                        
#define DINP_BIE                    (* (reg8 *) DINP__BIE)
/* Bit-mask for Aliased Register Access */
#define DINP_BIT_MASK               (* (reg8 *) DINP__BIT_MASK)
/* Bypass Enable */
#define DINP_BYP                    (* (reg8 *) DINP__BYP)
/* Port wide control signals */                                                   
#define DINP_CTL                    (* (reg8 *) DINP__CTL)
/* Drive Modes */
#define DINP_DM0                    (* (reg8 *) DINP__DM0) 
#define DINP_DM1                    (* (reg8 *) DINP__DM1)
#define DINP_DM2                    (* (reg8 *) DINP__DM2) 
/* Input Buffer Disable Override */
#define DINP_INP_DIS                (* (reg8 *) DINP__INP_DIS)
/* LCD Common or Segment Drive */
#define DINP_LCD_COM_SEG            (* (reg8 *) DINP__LCD_COM_SEG)
/* Enable Segment LCD */
#define DINP_LCD_EN                 (* (reg8 *) DINP__LCD_EN)
/* Slew Rate Control */
#define DINP_SLW                    (* (reg8 *) DINP__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define DINP_PRTDSI__CAPS_SEL       (* (reg8 *) DINP__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define DINP_PRTDSI__DBL_SYNC_IN    (* (reg8 *) DINP__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define DINP_PRTDSI__OE_SEL0        (* (reg8 *) DINP__PRTDSI__OE_SEL0) 
#define DINP_PRTDSI__OE_SEL1        (* (reg8 *) DINP__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define DINP_PRTDSI__OUT_SEL0       (* (reg8 *) DINP__PRTDSI__OUT_SEL0) 
#define DINP_PRTDSI__OUT_SEL1       (* (reg8 *) DINP__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define DINP_PRTDSI__SYNC_OUT       (* (reg8 *) DINP__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(DINP__SIO_CFG)
    #define DINP_SIO_HYST_EN        (* (reg8 *) DINP__SIO_HYST_EN)
    #define DINP_SIO_REG_HIFREQ     (* (reg8 *) DINP__SIO_REG_HIFREQ)
    #define DINP_SIO_CFG            (* (reg8 *) DINP__SIO_CFG)
    #define DINP_SIO_DIFF           (* (reg8 *) DINP__SIO_DIFF)
#endif /* (DINP__SIO_CFG) */

/* Interrupt Registers */
#if defined(DINP__INTSTAT)
    #define DINP_INTSTAT            (* (reg8 *) DINP__INTSTAT)
    #define DINP_SNAP               (* (reg8 *) DINP__SNAP)
    
	#define DINP_0_INTTYPE_REG 		(* (reg8 *) DINP__0__INTTYPE)
	#define DINP_1_INTTYPE_REG 		(* (reg8 *) DINP__1__INTTYPE)
	#define DINP_2_INTTYPE_REG 		(* (reg8 *) DINP__2__INTTYPE)
	#define DINP_3_INTTYPE_REG 		(* (reg8 *) DINP__3__INTTYPE)
#endif /* (DINP__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_DINP_H */


/* [] END OF FILE */
