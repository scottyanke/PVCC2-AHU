/*******************************************************************************
* File Name: Rx_Timer_Int.h
* Version 1.70
*
*  Description:
*   Provides the function definitions for the Interrupt Controller.
*
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_ISR_Rx_Timer_Int_H)
#define CY_ISR_Rx_Timer_Int_H


#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void Rx_Timer_Int_Start(void);
void Rx_Timer_Int_StartEx(cyisraddress address);
void Rx_Timer_Int_Stop(void);

CY_ISR_PROTO(Rx_Timer_Int_Interrupt);

void Rx_Timer_Int_SetVector(cyisraddress address);
cyisraddress Rx_Timer_Int_GetVector(void);

void Rx_Timer_Int_SetPriority(uint8 priority);
uint8 Rx_Timer_Int_GetPriority(void);

void Rx_Timer_Int_Enable(void);
uint8 Rx_Timer_Int_GetState(void);
void Rx_Timer_Int_Disable(void);

void Rx_Timer_Int_SetPending(void);
void Rx_Timer_Int_ClearPending(void);


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the Rx_Timer_Int ISR. */
#define Rx_Timer_Int_INTC_VECTOR            ((reg32 *) Rx_Timer_Int__INTC_VECT)

/* Address of the Rx_Timer_Int ISR priority. */
#define Rx_Timer_Int_INTC_PRIOR             ((reg8 *) Rx_Timer_Int__INTC_PRIOR_REG)

/* Priority of the Rx_Timer_Int interrupt. */
#define Rx_Timer_Int_INTC_PRIOR_NUMBER      Rx_Timer_Int__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable Rx_Timer_Int interrupt. */
#define Rx_Timer_Int_INTC_SET_EN            ((reg32 *) Rx_Timer_Int__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the Rx_Timer_Int interrupt. */
#define Rx_Timer_Int_INTC_CLR_EN            ((reg32 *) Rx_Timer_Int__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the Rx_Timer_Int interrupt state to pending. */
#define Rx_Timer_Int_INTC_SET_PD            ((reg32 *) Rx_Timer_Int__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the Rx_Timer_Int interrupt. */
#define Rx_Timer_Int_INTC_CLR_PD            ((reg32 *) Rx_Timer_Int__INTC_CLR_PD_REG)


#endif /* CY_ISR_Rx_Timer_Int_H */


/* [] END OF FILE */
