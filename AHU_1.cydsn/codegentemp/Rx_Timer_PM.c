/*******************************************************************************
* File Name: Rx_Timer_PM.c
* Version 2.80
*
*  Description:
*     This file provides the power management source code to API for the
*     Timer.
*
*   Note:
*     None
*
*******************************************************************************
* Copyright 2008-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "Rx_Timer.h"

static Rx_Timer_backupStruct Rx_Timer_backup;


/*******************************************************************************
* Function Name: Rx_Timer_SaveConfig
********************************************************************************
*
* Summary:
*     Save the current user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Rx_Timer_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void Rx_Timer_SaveConfig(void) 
{
    #if (!Rx_Timer_UsingFixedFunction)
        Rx_Timer_backup.TimerUdb = Rx_Timer_ReadCounter();
        Rx_Timer_backup.InterruptMaskValue = Rx_Timer_STATUS_MASK;
        #if (Rx_Timer_UsingHWCaptureCounter)
            Rx_Timer_backup.TimerCaptureCounter = Rx_Timer_ReadCaptureCount();
        #endif /* Back Up capture counter register  */

        #if(!Rx_Timer_UDB_CONTROL_REG_REMOVED)
            Rx_Timer_backup.TimerControlRegister = Rx_Timer_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: Rx_Timer_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Rx_Timer_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void Rx_Timer_RestoreConfig(void) 
{   
    #if (!Rx_Timer_UsingFixedFunction)

        Rx_Timer_WriteCounter(Rx_Timer_backup.TimerUdb);
        Rx_Timer_STATUS_MASK =Rx_Timer_backup.InterruptMaskValue;
        #if (Rx_Timer_UsingHWCaptureCounter)
            Rx_Timer_SetCaptureCount(Rx_Timer_backup.TimerCaptureCounter);
        #endif /* Restore Capture counter register*/

        #if(!Rx_Timer_UDB_CONTROL_REG_REMOVED)
            Rx_Timer_WriteControlRegister(Rx_Timer_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: Rx_Timer_Sleep
********************************************************************************
*
* Summary:
*     Stop and Save the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Rx_Timer_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void Rx_Timer_Sleep(void) 
{
    #if(!Rx_Timer_UDB_CONTROL_REG_REMOVED)
        /* Save Counter's enable state */
        if(Rx_Timer_CTRL_ENABLE == (Rx_Timer_CONTROL & Rx_Timer_CTRL_ENABLE))
        {
            /* Timer is enabled */
            Rx_Timer_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            Rx_Timer_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    Rx_Timer_Stop();
    Rx_Timer_SaveConfig();
}


/*******************************************************************************
* Function Name: Rx_Timer_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Rx_Timer_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void Rx_Timer_Wakeup(void) 
{
    Rx_Timer_RestoreConfig();
    #if(!Rx_Timer_UDB_CONTROL_REG_REMOVED)
        if(Rx_Timer_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                Rx_Timer_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
