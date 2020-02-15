/*******************************************************************************
* File Name: Wemos_PM.c
* Version 2.50
*
* Description:
*  This file provides Sleep/WakeUp APIs functionality.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Wemos.h"


/***************************************
* Local data allocation
***************************************/

static Wemos_BACKUP_STRUCT  Wemos_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: Wemos_SaveConfig
********************************************************************************
*
* Summary:
*  This function saves the component nonretention control register.
*  Does not save the FIFO which is a set of nonretention registers.
*  This function is called by the Wemos_Sleep() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Wemos_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Wemos_SaveConfig(void)
{
    #if(Wemos_CONTROL_REG_REMOVED == 0u)
        Wemos_backup.cr = Wemos_CONTROL_REG;
    #endif /* End Wemos_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Wemos_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the nonretention control register except FIFO.
*  Does not restore the FIFO which is a set of nonretention registers.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Wemos_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
* Notes:
*  If this function is called without calling Wemos_SaveConfig() 
*  first, the data loaded may be incorrect.
*
*******************************************************************************/
void Wemos_RestoreConfig(void)
{
    #if(Wemos_CONTROL_REG_REMOVED == 0u)
        Wemos_CONTROL_REG = Wemos_backup.cr;
    #endif /* End Wemos_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Wemos_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred API to prepare the component for sleep. 
*  The Wemos_Sleep() API saves the current component state. Then it
*  calls the Wemos_Stop() function and calls 
*  Wemos_SaveConfig() to save the hardware configuration.
*  Call the Wemos_Sleep() function before calling the CyPmSleep() 
*  or the CyPmHibernate() function. 
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Wemos_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Wemos_Sleep(void)
{
    #if(Wemos_RX_ENABLED || Wemos_HD_ENABLED)
        if((Wemos_RXSTATUS_ACTL_REG  & Wemos_INT_ENABLE) != 0u)
        {
            Wemos_backup.enableState = 1u;
        }
        else
        {
            Wemos_backup.enableState = 0u;
        }
    #else
        if((Wemos_TXSTATUS_ACTL_REG  & Wemos_INT_ENABLE) !=0u)
        {
            Wemos_backup.enableState = 1u;
        }
        else
        {
            Wemos_backup.enableState = 0u;
        }
    #endif /* End Wemos_RX_ENABLED || Wemos_HD_ENABLED*/

    Wemos_Stop();
    Wemos_SaveConfig();
}


/*******************************************************************************
* Function Name: Wemos_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred API to restore the component to the state when 
*  Wemos_Sleep() was called. The Wemos_Wakeup() function
*  calls the Wemos_RestoreConfig() function to restore the 
*  configuration. If the component was enabled before the 
*  Wemos_Sleep() function was called, the Wemos_Wakeup()
*  function will also re-enable the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Wemos_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Wemos_Wakeup(void)
{
    Wemos_RestoreConfig();
    #if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )
        Wemos_ClearRxBuffer();
    #endif /* End (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */
    #if(Wemos_TX_ENABLED || Wemos_HD_ENABLED)
        Wemos_ClearTxBuffer();
    #endif /* End Wemos_TX_ENABLED || Wemos_HD_ENABLED */

    if(Wemos_backup.enableState != 0u)
    {
        Wemos_Enable();
    }
}


/* [] END OF FILE */
