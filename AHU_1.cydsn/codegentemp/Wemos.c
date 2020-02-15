/*******************************************************************************
* File Name: Wemos.c
* Version 2.50
*
* Description:
*  This file provides all API functionality of the UART component
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
#if (Wemos_INTERNAL_CLOCK_USED)
    #include "Wemos_IntClock.h"
#endif /* End Wemos_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 Wemos_initVar = 0u;

#if (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED)
    volatile uint8 Wemos_txBuffer[Wemos_TX_BUFFER_SIZE];
    volatile uint8 Wemos_txBufferRead = 0u;
    uint8 Wemos_txBufferWrite = 0u;
#endif /* (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED) */

#if (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED))
    uint8 Wemos_errorStatus = 0u;
    volatile uint8 Wemos_rxBuffer[Wemos_RX_BUFFER_SIZE];
    volatile uint8 Wemos_rxBufferRead  = 0u;
    volatile uint8 Wemos_rxBufferWrite = 0u;
    volatile uint8 Wemos_rxBufferLoopDetect = 0u;
    volatile uint8 Wemos_rxBufferOverflow   = 0u;
    #if (Wemos_RXHW_ADDRESS_ENABLED)
        volatile uint8 Wemos_rxAddressMode = Wemos_RX_ADDRESS_MODE;
        volatile uint8 Wemos_rxAddressDetected = 0u;
    #endif /* (Wemos_RXHW_ADDRESS_ENABLED) */
#endif /* (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED)) */


/*******************************************************************************
* Function Name: Wemos_Start
********************************************************************************
*
* Summary:
*  This is the preferred method to begin component operation.
*  Wemos_Start() sets the initVar variable, calls the
*  Wemos_Init() function, and then calls the
*  Wemos_Enable() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The Wemos_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time Wemos_Start() is called. This
*  allows for component initialization without re-initialization in all
*  subsequent calls to the Wemos_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Wemos_Start(void) 
{
    /* If not initialized then initialize all required hardware and software */
    if(Wemos_initVar == 0u)
    {
        Wemos_Init();
        Wemos_initVar = 1u;
    }

    Wemos_Enable();
}


/*******************************************************************************
* Function Name: Wemos_Init
********************************************************************************
*
* Summary:
*  Initializes or restores the component according to the customizer Configure
*  dialog settings. It is not necessary to call Wemos_Init() because
*  the Wemos_Start() API calls this function and is the preferred
*  method to begin component operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Wemos_Init(void) 
{
    #if(Wemos_RX_ENABLED || Wemos_HD_ENABLED)

        #if (Wemos_RX_INTERRUPT_ENABLED)
            /* Set RX interrupt vector and priority */
            (void) CyIntSetVector(Wemos_RX_VECT_NUM, &Wemos_RXISR);
            CyIntSetPriority(Wemos_RX_VECT_NUM, Wemos_RX_PRIOR_NUM);
            Wemos_errorStatus = 0u;
        #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        #if (Wemos_RXHW_ADDRESS_ENABLED)
            Wemos_SetRxAddressMode(Wemos_RX_ADDRESS_MODE);
            Wemos_SetRxAddress1(Wemos_RX_HW_ADDRESS1);
            Wemos_SetRxAddress2(Wemos_RX_HW_ADDRESS2);
        #endif /* End Wemos_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        Wemos_RXBITCTR_PERIOD_REG = Wemos_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        Wemos_RXSTATUS_MASK_REG  = Wemos_INIT_RX_INTERRUPTS_MASK;
    #endif /* End Wemos_RX_ENABLED || Wemos_HD_ENABLED*/

    #if(Wemos_TX_ENABLED)
        #if (Wemos_TX_INTERRUPT_ENABLED)
            /* Set TX interrupt vector and priority */
            (void) CyIntSetVector(Wemos_TX_VECT_NUM, &Wemos_TXISR);
            CyIntSetPriority(Wemos_TX_VECT_NUM, Wemos_TX_PRIOR_NUM);
        #endif /* (Wemos_TX_INTERRUPT_ENABLED) */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if (Wemos_TXCLKGEN_DP)
            Wemos_TXBITCLKGEN_CTR_REG = Wemos_BIT_CENTER;
            Wemos_TXBITCLKTX_COMPLETE_REG = ((Wemos_NUMBER_OF_DATA_BITS +
                        Wemos_NUMBER_OF_START_BIT) * Wemos_OVER_SAMPLE_COUNT) - 1u;
        #else
            Wemos_TXBITCTR_PERIOD_REG = ((Wemos_NUMBER_OF_DATA_BITS +
                        Wemos_NUMBER_OF_START_BIT) * Wemos_OVER_SAMPLE_8) - 1u;
        #endif /* End Wemos_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if (Wemos_TX_INTERRUPT_ENABLED)
            Wemos_TXSTATUS_MASK_REG = Wemos_TX_STS_FIFO_EMPTY;
        #else
            Wemos_TXSTATUS_MASK_REG = Wemos_INIT_TX_INTERRUPTS_MASK;
        #endif /*End Wemos_TX_INTERRUPT_ENABLED*/

    #endif /* End Wemos_TX_ENABLED */

    #if(Wemos_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        Wemos_WriteControlRegister( \
            (Wemos_ReadControlRegister() & (uint8)~Wemos_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(Wemos_PARITY_TYPE << Wemos_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End Wemos_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: Wemos_Enable
********************************************************************************
*
* Summary:
*  Activates the hardware and begins component operation. It is not necessary
*  to call Wemos_Enable() because the Wemos_Start() API
*  calls this function, which is the preferred method to begin component
*  operation.

* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Wemos_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void Wemos_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if (Wemos_RX_ENABLED || Wemos_HD_ENABLED)
        /* RX Counter (Count7) Enable */
        Wemos_RXBITCTR_CONTROL_REG |= Wemos_CNTR_ENABLE;

        /* Enable the RX Interrupt */
        Wemos_RXSTATUS_ACTL_REG  |= Wemos_INT_ENABLE;

        #if (Wemos_RX_INTERRUPT_ENABLED)
            Wemos_EnableRxInt();

            #if (Wemos_RXHW_ADDRESS_ENABLED)
                Wemos_rxAddressDetected = 0u;
            #endif /* (Wemos_RXHW_ADDRESS_ENABLED) */
        #endif /* (Wemos_RX_INTERRUPT_ENABLED) */
    #endif /* (Wemos_RX_ENABLED || Wemos_HD_ENABLED) */

    #if(Wemos_TX_ENABLED)
        /* TX Counter (DP/Count7) Enable */
        #if(!Wemos_TXCLKGEN_DP)
            Wemos_TXBITCTR_CONTROL_REG |= Wemos_CNTR_ENABLE;
        #endif /* End Wemos_TXCLKGEN_DP */

        /* Enable the TX Interrupt */
        Wemos_TXSTATUS_ACTL_REG |= Wemos_INT_ENABLE;
        #if (Wemos_TX_INTERRUPT_ENABLED)
            Wemos_ClearPendingTxInt(); /* Clear history of TX_NOT_EMPTY */
            Wemos_EnableTxInt();
        #endif /* (Wemos_TX_INTERRUPT_ENABLED) */
     #endif /* (Wemos_TX_INTERRUPT_ENABLED) */

    #if (Wemos_INTERNAL_CLOCK_USED)
        Wemos_IntClock_Start();  /* Enable the clock */
    #endif /* (Wemos_INTERNAL_CLOCK_USED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Wemos_Stop
********************************************************************************
*
* Summary:
*  Disables the UART operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Wemos_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if (Wemos_RX_ENABLED || Wemos_HD_ENABLED)
        Wemos_RXBITCTR_CONTROL_REG &= (uint8) ~Wemos_CNTR_ENABLE;
    #endif /* (Wemos_RX_ENABLED || Wemos_HD_ENABLED) */

    #if (Wemos_TX_ENABLED)
        #if(!Wemos_TXCLKGEN_DP)
            Wemos_TXBITCTR_CONTROL_REG &= (uint8) ~Wemos_CNTR_ENABLE;
        #endif /* (!Wemos_TXCLKGEN_DP) */
    #endif /* (Wemos_TX_ENABLED) */

    #if (Wemos_INTERNAL_CLOCK_USED)
        Wemos_IntClock_Stop();   /* Disable the clock */
    #endif /* (Wemos_INTERNAL_CLOCK_USED) */

    /* Disable internal interrupt component */
    #if (Wemos_RX_ENABLED || Wemos_HD_ENABLED)
        Wemos_RXSTATUS_ACTL_REG  &= (uint8) ~Wemos_INT_ENABLE;

        #if (Wemos_RX_INTERRUPT_ENABLED)
            Wemos_DisableRxInt();
        #endif /* (Wemos_RX_INTERRUPT_ENABLED) */
    #endif /* (Wemos_RX_ENABLED || Wemos_HD_ENABLED) */

    #if (Wemos_TX_ENABLED)
        Wemos_TXSTATUS_ACTL_REG &= (uint8) ~Wemos_INT_ENABLE;

        #if (Wemos_TX_INTERRUPT_ENABLED)
            Wemos_DisableTxInt();
        #endif /* (Wemos_TX_INTERRUPT_ENABLED) */
    #endif /* (Wemos_TX_ENABLED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Wemos_ReadControlRegister
********************************************************************************
*
* Summary:
*  Returns the current value of the control register.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the control register.
*
*******************************************************************************/
uint8 Wemos_ReadControlRegister(void) 
{
    #if (Wemos_CONTROL_REG_REMOVED)
        return(0u);
    #else
        return(Wemos_CONTROL_REG);
    #endif /* (Wemos_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: Wemos_WriteControlRegister
********************************************************************************
*
* Summary:
*  Writes an 8-bit value into the control register
*
* Parameters:
*  control:  control register value
*
* Return:
*  None.
*
*******************************************************************************/
void  Wemos_WriteControlRegister(uint8 control) 
{
    #if (Wemos_CONTROL_REG_REMOVED)
        if(0u != control)
        {
            /* Suppress compiler warning */
        }
    #else
       Wemos_CONTROL_REG = control;
    #endif /* (Wemos_CONTROL_REG_REMOVED) */
}


#if(Wemos_RX_ENABLED || Wemos_HD_ENABLED)
    /*******************************************************************************
    * Function Name: Wemos_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the RX interrupt sources enabled.
    *
    * Parameters:
    *  IntSrc:  Bit field containing the RX interrupts to enable. Based on the 
    *  bit-field arrangement of the status register. This value must be a 
    *  combination of status register bit-masks shown below:
    *      Wemos_RX_STS_FIFO_NOTEMPTY    Interrupt on byte received.
    *      Wemos_RX_STS_PAR_ERROR        Interrupt on parity error.
    *      Wemos_RX_STS_STOP_ERROR       Interrupt on stop error.
    *      Wemos_RX_STS_BREAK            Interrupt on break.
    *      Wemos_RX_STS_OVERRUN          Interrupt on overrun error.
    *      Wemos_RX_STS_ADDR_MATCH       Interrupt on address match.
    *      Wemos_RX_STS_MRKSPC           Interrupt on address detect.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Wemos_SetRxInterruptMode(uint8 intSrc) 
    {
        Wemos_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: Wemos_ReadRxData
    ********************************************************************************
    *
    * Summary:
    *  Returns the next byte of received data. This function returns data without
    *  checking the status. You must check the status separately.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Received data from RX register
    *
    * Global Variables:
    *  Wemos_rxBuffer - RAM buffer pointer for save received data.
    *  Wemos_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Wemos_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Wemos_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Wemos_ReadRxData(void) 
    {
        uint8 rxData;

    #if (Wemos_RX_INTERRUPT_ENABLED)

        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Wemos_DisableRxInt();

        locRxBufferRead  = Wemos_rxBufferRead;
        locRxBufferWrite = Wemos_rxBufferWrite;

        if( (Wemos_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Wemos_rxBuffer[locRxBufferRead];
            locRxBufferRead++;

            if(locRxBufferRead >= Wemos_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Wemos_rxBufferRead = locRxBufferRead;

            if(Wemos_rxBufferLoopDetect != 0u)
            {
                Wemos_rxBufferLoopDetect = 0u;
                #if ((Wemos_RX_INTERRUPT_ENABLED) && (Wemos_FLOW_CONTROL != 0u))
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Wemos_HD_ENABLED )
                        if((Wemos_CONTROL_REG & Wemos_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only in RX
                            *  configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Wemos_RXSTATUS_MASK_REG  |= Wemos_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Wemos_RXSTATUS_MASK_REG  |= Wemos_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Wemos_HD_ENABLED */
                #endif /* ((Wemos_RX_INTERRUPT_ENABLED) && (Wemos_FLOW_CONTROL != 0u)) */
            }
        }
        else
        {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
            rxData = Wemos_RXDATA_REG;
        }

        Wemos_EnableRxInt();

    #else

        /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
        rxData = Wemos_RXDATA_REG;

    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Wemos_ReadRxStatus
    ********************************************************************************
    *
    * Summary:
    *  Returns the current state of the receiver status register and the software
    *  buffer overflow status.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Current state of the status register.
    *
    * Side Effect:
    *  All status register bits are clear-on-read except
    *  Wemos_RX_STS_FIFO_NOTEMPTY.
    *  Wemos_RX_STS_FIFO_NOTEMPTY clears immediately after RX data
    *  register read.
    *
    * Global Variables:
    *  Wemos_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn't free space in
    *   Wemos_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   Wemos_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 Wemos_ReadRxStatus(void) 
    {
        uint8 status;

        status = Wemos_RXSTATUS_REG & Wemos_RX_HW_MASK;

    #if (Wemos_RX_INTERRUPT_ENABLED)
        if(Wemos_rxBufferOverflow != 0u)
        {
            status |= Wemos_RX_STS_SOFT_BUFF_OVER;
            Wemos_rxBufferOverflow = 0u;
        }
    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        return(status);
    }


    /*******************************************************************************
    * Function Name: Wemos_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Returns the last received byte of data. Wemos_GetChar() is
    *  designed for ASCII characters and returns a uint8 where 1 to 255 are values
    *  for valid characters and 0 indicates an error occurred or no data is present.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Character read from UART RX buffer. ASCII characters from 1 to 255 are valid.
    *  A returned zero signifies an error condition or no data available.
    *
    * Global Variables:
    *  Wemos_rxBuffer - RAM buffer pointer for save received data.
    *  Wemos_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Wemos_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Wemos_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Wemos_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

    #if (Wemos_RX_INTERRUPT_ENABLED)
        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Wemos_DisableRxInt();

        locRxBufferRead  = Wemos_rxBufferRead;
        locRxBufferWrite = Wemos_rxBufferWrite;

        if( (Wemos_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Wemos_rxBuffer[locRxBufferRead];
            locRxBufferRead++;
            if(locRxBufferRead >= Wemos_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Wemos_rxBufferRead = locRxBufferRead;

            if(Wemos_rxBufferLoopDetect != 0u)
            {
                Wemos_rxBufferLoopDetect = 0u;
                #if( (Wemos_RX_INTERRUPT_ENABLED) && (Wemos_FLOW_CONTROL != 0u) )
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Wemos_HD_ENABLED )
                        if((Wemos_CONTROL_REG & Wemos_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only if
                            *  RX configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Wemos_RXSTATUS_MASK_REG |= Wemos_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Wemos_RXSTATUS_MASK_REG |= Wemos_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Wemos_HD_ENABLED */
                #endif /* Wemos_RX_INTERRUPT_ENABLED and Hardware flow control*/
            }

        }
        else
        {   rxStatus = Wemos_RXSTATUS_REG;
            if((rxStatus & Wemos_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO */
                rxData = Wemos_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (Wemos_RX_STS_BREAK | Wemos_RX_STS_PAR_ERROR |
                                Wemos_RX_STS_STOP_ERROR | Wemos_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        }

        Wemos_EnableRxInt();

    #else

        rxStatus =Wemos_RXSTATUS_REG;
        if((rxStatus & Wemos_RX_STS_FIFO_NOTEMPTY) != 0u)
        {
            /* Read received data from FIFO */
            rxData = Wemos_RXDATA_REG;

            /*Check status on error*/
            if((rxStatus & (Wemos_RX_STS_BREAK | Wemos_RX_STS_PAR_ERROR |
                            Wemos_RX_STS_STOP_ERROR | Wemos_RX_STS_OVERRUN)) != 0u)
            {
                rxData = 0u;
            }
        }
    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Wemos_GetByte
    ********************************************************************************
    *
    * Summary:
    *  Reads UART RX buffer immediately, returns received character and error
    *  condition.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  MSB contains status and LSB contains UART RX data. If the MSB is nonzero,
    *  an error has occurred.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint16 Wemos_GetByte(void) 
    {
        
    #if (Wemos_RX_INTERRUPT_ENABLED)
        uint16 locErrorStatus;
        /* Protect variables that could change on interrupt */
        Wemos_DisableRxInt();
        locErrorStatus = (uint16)Wemos_errorStatus;
        Wemos_errorStatus = 0u;
        Wemos_EnableRxInt();
        return ( (uint16)(locErrorStatus << 8u) | Wemos_ReadRxData() );
    #else
        return ( ((uint16)Wemos_ReadRxStatus() << 8u) | Wemos_ReadRxData() );
    #endif /* Wemos_RX_INTERRUPT_ENABLED */
        
    }


    /*******************************************************************************
    * Function Name: Wemos_GetRxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Returns the number of received bytes available in the RX buffer.
    *  * RX software buffer is disabled (RX Buffer Size parameter is equal to 4): 
    *    returns 0 for empty RX FIFO or 1 for not empty RX FIFO.
    *  * RX software buffer is enabled: returns the number of bytes available in 
    *    the RX software buffer. Bytes available in the RX FIFO do not take to 
    *    account.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  uint8: Number of bytes in the RX buffer. 
    *    Return value type depends on RX Buffer Size parameter.
    *
    * Global Variables:
    *  Wemos_rxBufferWrite - used to calculate left bytes.
    *  Wemos_rxBufferRead - used to calculate left bytes.
    *  Wemos_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 Wemos_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Wemos_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt */
        Wemos_DisableRxInt();

        if(Wemos_rxBufferRead == Wemos_rxBufferWrite)
        {
            if(Wemos_rxBufferLoopDetect != 0u)
            {
                size = Wemos_RX_BUFFER_SIZE;
            }
            else
            {
                size = 0u;
            }
        }
        else if(Wemos_rxBufferRead < Wemos_rxBufferWrite)
        {
            size = (Wemos_rxBufferWrite - Wemos_rxBufferRead);
        }
        else
        {
            size = (Wemos_RX_BUFFER_SIZE - Wemos_rxBufferRead) + Wemos_rxBufferWrite;
        }

        Wemos_EnableRxInt();

    #else

        /* We can only know if there is data in the fifo. */
        size = ((Wemos_RXSTATUS_REG & Wemos_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        return(size);
    }


    /*******************************************************************************
    * Function Name: Wemos_ClearRxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears the receiver memory buffer and hardware RX FIFO of all received data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_rxBufferWrite - cleared to zero.
    *  Wemos_rxBufferRead - cleared to zero.
    *  Wemos_rxBufferLoopDetect - cleared to zero.
    *  Wemos_rxBufferOverflow - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may
    *  have remained in the RAM.
    *
    * Side Effects:
    *  Any received data not read from the RAM or FIFO buffer will be lost.
    *
    *******************************************************************************/
    void Wemos_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Clear the HW FIFO */
        enableInterrupts = CyEnterCriticalSection();
        Wemos_RXDATA_AUX_CTL_REG |= (uint8)  Wemos_RX_FIFO_CLR;
        Wemos_RXDATA_AUX_CTL_REG &= (uint8) ~Wemos_RX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Wemos_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Wemos_DisableRxInt();

        Wemos_rxBufferRead = 0u;
        Wemos_rxBufferWrite = 0u;
        Wemos_rxBufferLoopDetect = 0u;
        Wemos_rxBufferOverflow = 0u;

        Wemos_EnableRxInt();

    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

    }


    /*******************************************************************************
    * Function Name: Wemos_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the software controlled Addressing mode used by the RX portion of the
    *  UART.
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  Wemos__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  Wemos__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  Wemos__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  Wemos__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  Wemos__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  Wemos_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void Wemos_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(Wemos_RXHW_ADDRESS_ENABLED)
            #if(Wemos_CONTROL_REG_REMOVED)
                if(0u != addressMode)
                {
                    /* Suppress compiler warning */
                }
            #else /* Wemos_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = Wemos_CONTROL_REG & (uint8)~Wemos_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << Wemos_CTRL_RXADDR_MODE0_SHIFT);
                Wemos_CONTROL_REG = tmpCtrl;

                #if(Wemos_RX_INTERRUPT_ENABLED && \
                   (Wemos_RXBUFFERSIZE > Wemos_FIFO_LENGTH) )
                    Wemos_rxAddressMode = addressMode;
                    Wemos_rxAddressDetected = 0u;
                #endif /* End Wemos_RXBUFFERSIZE > Wemos_FIFO_LENGTH*/
            #endif /* End Wemos_CONTROL_REG_REMOVED */
        #else /* Wemos_RXHW_ADDRESS_ENABLED */
            if(0u != addressMode)
            {
                /* Suppress compiler warning */
            }
        #endif /* End Wemos_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Wemos_SetRxAddress1
    ********************************************************************************
    *
    * Summary:
    *  Sets the first of two hardware-detectable receiver addresses.
    *
    * Parameters:
    *  address: Address #1 for hardware address detection.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void Wemos_SetRxAddress1(uint8 address) 
    {
        Wemos_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: Wemos_SetRxAddress2
    ********************************************************************************
    *
    * Summary:
    *  Sets the second of two hardware-detectable receiver addresses.
    *
    * Parameters:
    *  address: Address #2 for hardware address detection.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void Wemos_SetRxAddress2(uint8 address) 
    {
        Wemos_RXADDRESS2_REG = address;
    }

#endif  /* Wemos_RX_ENABLED || Wemos_HD_ENABLED*/


#if( (Wemos_TX_ENABLED) || (Wemos_HD_ENABLED) )
    /*******************************************************************************
    * Function Name: Wemos_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the TX interrupt sources to be enabled, but does not enable the
    *  interrupt.
    *
    * Parameters:
    *  intSrc: Bit field containing the TX interrupt sources to enable
    *   Wemos_TX_STS_COMPLETE        Interrupt on TX byte complete
    *   Wemos_TX_STS_FIFO_EMPTY      Interrupt when TX FIFO is empty
    *   Wemos_TX_STS_FIFO_FULL       Interrupt when TX FIFO is full
    *   Wemos_TX_STS_FIFO_NOT_FULL   Interrupt when TX FIFO is not full
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Wemos_SetTxInterruptMode(uint8 intSrc) 
    {
        Wemos_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: Wemos_WriteTxData
    ********************************************************************************
    *
    * Summary:
    *  Places a byte of data into the transmit buffer to be sent when the bus is
    *  available without checking the TX status register. You must check status
    *  separately.
    *
    * Parameters:
    *  txDataByte: data byte
    *
    * Return:
    * None.
    *
    * Global Variables:
    *  Wemos_txBuffer - RAM buffer pointer for save data for transmission
    *  Wemos_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  Wemos_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  Wemos_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Wemos_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(Wemos_initVar != 0u)
        {
        #if (Wemos_TX_INTERRUPT_ENABLED)

            /* Protect variables that could change on interrupt. */
            Wemos_DisableTxInt();

            if( (Wemos_txBufferRead == Wemos_txBufferWrite) &&
                ((Wemos_TXSTATUS_REG & Wemos_TX_STS_FIFO_FULL) == 0u) )
            {
                /* Add directly to the FIFO. */
                Wemos_TXDATA_REG = txDataByte;
            }
            else
            {
                if(Wemos_txBufferWrite >= Wemos_TX_BUFFER_SIZE)
                {
                    Wemos_txBufferWrite = 0u;
                }

                Wemos_txBuffer[Wemos_txBufferWrite] = txDataByte;

                /* Add to the software buffer. */
                Wemos_txBufferWrite++;
            }

            Wemos_EnableTxInt();

        #else

            /* Add directly to the FIFO. */
            Wemos_TXDATA_REG = txDataByte;

        #endif /*(Wemos_TX_INTERRUPT_ENABLED) */
        }
    }


    /*******************************************************************************
    * Function Name: Wemos_ReadTxStatus
    ********************************************************************************
    *
    * Summary:
    *  Reads the status register for the TX portion of the UART.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Contents of the status register
    *
    * Theory:
    *  This function reads the TX status register, which is cleared on read.
    *  It is up to the user to handle all bits in this return value accordingly,
    *  even if the bit was not enabled as an interrupt source the event happened
    *  and must be handled accordingly.
    *
    *******************************************************************************/
    uint8 Wemos_ReadTxStatus(void) 
    {
        return(Wemos_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: Wemos_PutChar
    ********************************************************************************
    *
    * Summary:
    *  Puts a byte of data into the transmit buffer to be sent when the bus is
    *  available. This is a blocking API that waits until the TX buffer has room to
    *  hold the data.
    *
    * Parameters:
    *  txDataByte: Byte containing the data to transmit
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_txBuffer - RAM buffer pointer for save data for transmission
    *  Wemos_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  Wemos_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  Wemos_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void Wemos_PutChar(uint8 txDataByte) 
    {
    #if (Wemos_TX_INTERRUPT_ENABLED)
        /* The temporary output pointer is used since it takes two instructions
        *  to increment with a wrap, and we can't risk doing that with the real
        *  pointer and getting an interrupt in between instructions.
        */
        uint8 locTxBufferWrite;
        uint8 locTxBufferRead;

        do
        { /* Block if software buffer is full, so we don't overwrite. */

        #if ((Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Disable TX interrupt to protect variables from modification */
            Wemos_DisableTxInt();
        #endif /* (Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3) */

            locTxBufferWrite = Wemos_txBufferWrite;
            locTxBufferRead  = Wemos_txBufferRead;

        #if ((Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Enable interrupt to continue transmission */
            Wemos_EnableTxInt();
        #endif /* (Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3) */
        }
        while( (locTxBufferWrite < locTxBufferRead) ? (locTxBufferWrite == (locTxBufferRead - 1u)) :
                                ((locTxBufferWrite - locTxBufferRead) ==
                                (uint8)(Wemos_TX_BUFFER_SIZE - 1u)) );

        if( (locTxBufferRead == locTxBufferWrite) &&
            ((Wemos_TXSTATUS_REG & Wemos_TX_STS_FIFO_FULL) == 0u) )
        {
            /* Add directly to the FIFO */
            Wemos_TXDATA_REG = txDataByte;
        }
        else
        {
            if(locTxBufferWrite >= Wemos_TX_BUFFER_SIZE)
            {
                locTxBufferWrite = 0u;
            }
            /* Add to the software buffer. */
            Wemos_txBuffer[locTxBufferWrite] = txDataByte;
            locTxBufferWrite++;

            /* Finally, update the real output pointer */
        #if ((Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3))
            Wemos_DisableTxInt();
        #endif /* (Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3) */

            Wemos_txBufferWrite = locTxBufferWrite;

        #if ((Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3))
            Wemos_EnableTxInt();
        #endif /* (Wemos_TX_BUFFER_SIZE > Wemos_MAX_BYTE_VALUE) && (CY_PSOC3) */

            if(0u != (Wemos_TXSTATUS_REG & Wemos_TX_STS_FIFO_EMPTY))
            {
                /* Trigger TX interrupt to send software buffer */
                Wemos_SetPendingTxInt();
            }
        }

    #else

        while((Wemos_TXSTATUS_REG & Wemos_TX_STS_FIFO_FULL) != 0u)
        {
            /* Wait for room in the FIFO */
        }

        /* Add directly to the FIFO */
        Wemos_TXDATA_REG = txDataByte;

    #endif /* Wemos_TX_INTERRUPT_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Wemos_PutString
    ********************************************************************************
    *
    * Summary:
    *  Sends a NULL terminated string to the TX buffer for transmission.
    *
    * Parameters:
    *  string[]: Pointer to the null terminated string array residing in RAM or ROM
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  If there is not enough memory in the TX buffer for the entire string, this
    *  function blocks until the last character of the string is loaded into the
    *  TX buffer.
    *
    *******************************************************************************/
    void Wemos_PutString(const char8 string[]) 
    {
        uint16 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Wemos_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent */
            while(string[bufIndex] != (char8) 0)
            {
                Wemos_PutChar((uint8)string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Wemos_PutArray
    ********************************************************************************
    *
    * Summary:
    *  Places N bytes of data from a memory array into the TX buffer for
    *  transmission.
    *
    * Parameters:
    *  string[]: Address of the memory array residing in RAM or ROM.
    *  byteCount: Number of bytes to be transmitted. The type depends on TX Buffer
    *             Size parameter.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  If there is not enough memory in the TX buffer for the entire string, this
    *  function blocks until the last character of the string is loaded into the
    *  TX buffer.
    *
    *******************************************************************************/
    void Wemos_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Wemos_initVar != 0u)
        {
            while(bufIndex < byteCount)
            {
                Wemos_PutChar(string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Wemos_PutCRLF
    ********************************************************************************
    *
    * Summary:
    *  Writes a byte of data followed by a carriage return (0x0D) and line feed
    *  (0x0A) to the transmit buffer.
    *
    * Parameters:
    *  txDataByte: Data byte to transmit before the carriage return and line feed.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Wemos_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function */
        if(Wemos_initVar != 0u)
        {
            Wemos_PutChar(txDataByte);
            Wemos_PutChar(0x0Du);
            Wemos_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: Wemos_GetTxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Returns the number of bytes in the TX buffer which are waiting to be 
    *  transmitted.
    *  * TX software buffer is disabled (TX Buffer Size parameter is equal to 4): 
    *    returns 0 for empty TX FIFO, 1 for not full TX FIFO or 4 for full TX FIFO.
    *  * TX software buffer is enabled: returns the number of bytes in the TX 
    *    software buffer which are waiting to be transmitted. Bytes available in the
    *    TX FIFO do not count.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Number of bytes used in the TX buffer. Return value type depends on the TX 
    *  Buffer Size parameter.
    *
    * Global Variables:
    *  Wemos_txBufferWrite - used to calculate left space.
    *  Wemos_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 Wemos_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Wemos_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Wemos_DisableTxInt();

        if(Wemos_txBufferRead == Wemos_txBufferWrite)
        {
            size = 0u;
        }
        else if(Wemos_txBufferRead < Wemos_txBufferWrite)
        {
            size = (Wemos_txBufferWrite - Wemos_txBufferRead);
        }
        else
        {
            size = (Wemos_TX_BUFFER_SIZE - Wemos_txBufferRead) +
                    Wemos_txBufferWrite;
        }

        Wemos_EnableTxInt();

    #else

        size = Wemos_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & Wemos_TX_STS_FIFO_FULL) != 0u)
        {
            size = Wemos_FIFO_LENGTH;
        }
        else if((size & Wemos_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* (Wemos_TX_INTERRUPT_ENABLED) */

    return(size);
    }


    /*******************************************************************************
    * Function Name: Wemos_ClearTxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears all data from the TX buffer and hardware TX FIFO.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_txBufferWrite - cleared to zero.
    *  Wemos_txBufferRead - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may have
    *  remained in the RAM.
    *
    * Side Effects:
    *  Data waiting in the transmit buffer is not sent; a byte that is currently
    *  transmitting finishes transmitting.
    *
    *******************************************************************************/
    void Wemos_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Clear the HW FIFO */
        Wemos_TXDATA_AUX_CTL_REG |= (uint8)  Wemos_TX_FIFO_CLR;
        Wemos_TXDATA_AUX_CTL_REG &= (uint8) ~Wemos_TX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Wemos_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Wemos_DisableTxInt();

        Wemos_txBufferRead = 0u;
        Wemos_txBufferWrite = 0u;

        /* Enable Tx interrupt. */
        Wemos_EnableTxInt();

    #endif /* (Wemos_TX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Wemos_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Transmits a break signal on the bus.
    *
    * Parameters:
    *  uint8 retMode:  Send Break return mode. See the following table for options.
    *   Wemos_SEND_BREAK - Initialize registers for break, send the Break
    *       signal and return immediately.
    *   Wemos_WAIT_FOR_COMPLETE_REINIT - Wait until break transmission is
    *       complete, reinitialize registers to normal transmission mode then return
    *   Wemos_REINIT - Reinitialize registers to normal transmission mode
    *       then return.
    *   Wemos_SEND_WAIT_REINIT - Performs both options: 
    *      Wemos_SEND_BREAK and Wemos_WAIT_FOR_COMPLETE_REINIT.
    *      This option is recommended for most cases.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_initVar - checked to identify that the component has been
    *     initialized.
    *  txPeriod - static variable, used for keeping TX period configuration.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  SendBreak function initializes registers to send 13-bit break signal. It is
    *  important to return the registers configuration to normal for continue 8-bit
    *  operation.
    *  There are 3 variants for this API usage:
    *  1) SendBreak(3) - function will send the Break signal and take care on the
    *     configuration returning. Function will block CPU until transmission
    *     complete.
    *  2) User may want to use blocking time if UART configured to the low speed
    *     operation
    *     Example for this case:
    *     SendBreak(0);     - initialize Break signal transmission
    *         Add your code here to use CPU time
    *     SendBreak(1);     - complete Break operation
    *  3) Same to 2) but user may want to initialize and use the interrupt to
    *     complete break operation.
    *     Example for this case:
    *     Initialize TX interrupt with "TX - On TX Complete" parameter
    *     SendBreak(0);     - initialize Break signal transmission
    *         Add your code here to use CPU time
    *     When interrupt appear with Wemos_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *  The Wemos_SendBreak() function initializes registers to send a
    *  break signal.
    *  Break signal length depends on the break signal bits configuration.
    *  The register configuration should be reinitialized before normal 8-bit
    *  communication can continue.
    *
    *******************************************************************************/
    void Wemos_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(Wemos_initVar != 0u)
        {
            /* Set the Counter to 13-bits and transmit a 00 byte */
            /* When that is done then reset the counter value back */
            uint8 tmpStat;

        #if(Wemos_HD_ENABLED) /* Half Duplex mode*/

            if( (retMode == Wemos_SEND_BREAK) ||
                (retMode == Wemos_SEND_WAIT_REINIT ) )
            {
                /* CTRL_HD_SEND_BREAK - sends break bits in HD mode */
                Wemos_WriteControlRegister(Wemos_ReadControlRegister() |
                                                      Wemos_CTRL_HD_SEND_BREAK);
                /* Send zeros */
                Wemos_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Wemos_TXSTATUS_REG;
                }
                while((tmpStat & Wemos_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Wemos_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Wemos_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Wemos_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Wemos_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Wemos_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Wemos_REINIT) ||
                (retMode == Wemos_SEND_WAIT_REINIT) )
            {
                Wemos_WriteControlRegister(Wemos_ReadControlRegister() &
                                              (uint8)~Wemos_CTRL_HD_SEND_BREAK);
            }

        #else /* Wemos_HD_ENABLED Full Duplex mode */

            static uint8 txPeriod;

            if( (retMode == Wemos_SEND_BREAK) ||
                (retMode == Wemos_SEND_WAIT_REINIT) )
            {
                /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode */
                #if( (Wemos_PARITY_TYPE != Wemos__B_UART__NONE_REVB) || \
                                    (Wemos_PARITY_TYPE_SW != 0u) )
                    Wemos_WriteControlRegister(Wemos_ReadControlRegister() |
                                                          Wemos_CTRL_HD_SEND_BREAK);
                #endif /* End Wemos_PARITY_TYPE != Wemos__B_UART__NONE_REVB  */

                #if(Wemos_TXCLKGEN_DP)
                    txPeriod = Wemos_TXBITCLKTX_COMPLETE_REG;
                    Wemos_TXBITCLKTX_COMPLETE_REG = Wemos_TXBITCTR_BREAKBITS;
                #else
                    txPeriod = Wemos_TXBITCTR_PERIOD_REG;
                    Wemos_TXBITCTR_PERIOD_REG = Wemos_TXBITCTR_BREAKBITS8X;
                #endif /* End Wemos_TXCLKGEN_DP */

                /* Send zeros */
                Wemos_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Wemos_TXSTATUS_REG;
                }
                while((tmpStat & Wemos_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Wemos_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Wemos_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Wemos_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Wemos_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Wemos_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Wemos_REINIT) ||
                (retMode == Wemos_SEND_WAIT_REINIT) )
            {

            #if(Wemos_TXCLKGEN_DP)
                Wemos_TXBITCLKTX_COMPLETE_REG = txPeriod;
            #else
                Wemos_TXBITCTR_PERIOD_REG = txPeriod;
            #endif /* End Wemos_TXCLKGEN_DP */

            #if( (Wemos_PARITY_TYPE != Wemos__B_UART__NONE_REVB) || \
                 (Wemos_PARITY_TYPE_SW != 0u) )
                Wemos_WriteControlRegister(Wemos_ReadControlRegister() &
                                                      (uint8) ~Wemos_CTRL_HD_SEND_BREAK);
            #endif /* End Wemos_PARITY_TYPE != NONE */
            }
        #endif    /* End Wemos_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: Wemos_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the transmitter to signal the next bytes is address or data.
    *
    * Parameters:
    *  addressMode: 
    *       Wemos_SET_SPACE - Configure the transmitter to send the next
    *                                    byte as a data.
    *       Wemos_SET_MARK  - Configure the transmitter to send the next
    *                                    byte as an address.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  This function sets and clears Wemos_CTRL_MARK bit in the Control
    *  register.
    *
    *******************************************************************************/
    void Wemos_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable */
        if(addressMode != 0u)
        {
        #if( Wemos_CONTROL_REG_REMOVED == 0u )
            Wemos_WriteControlRegister(Wemos_ReadControlRegister() |
                                                  Wemos_CTRL_MARK);
        #endif /* End Wemos_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
        #if( Wemos_CONTROL_REG_REMOVED == 0u )
            Wemos_WriteControlRegister(Wemos_ReadControlRegister() &
                                                  (uint8) ~Wemos_CTRL_MARK);
        #endif /* End Wemos_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndWemos_TX_ENABLED */

#if(Wemos_HD_ENABLED)


    /*******************************************************************************
    * Function Name: Wemos_LoadRxConfig
    ********************************************************************************
    *
    * Summary:
    *  Loads the receiver configuration in half duplex mode. After calling this
    *  function, the UART is ready to receive data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  Valid only in half duplex mode. You must make sure that the previous
    *  transaction is complete and it is safe to unload the transmitter
    *  configuration.
    *
    *******************************************************************************/
    void Wemos_LoadRxConfig(void) 
    {
        Wemos_WriteControlRegister(Wemos_ReadControlRegister() &
                                                (uint8)~Wemos_CTRL_HD_SEND);
        Wemos_RXBITCTR_PERIOD_REG = Wemos_HD_RXBITCTR_INIT;

    #if (Wemos_RX_INTERRUPT_ENABLED)
        /* Enable RX interrupt after set RX configuration */
        Wemos_SetRxInterruptMode(Wemos_INIT_RX_INTERRUPTS_MASK);
    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Wemos_LoadTxConfig
    ********************************************************************************
    *
    * Summary:
    *  Loads the transmitter configuration in half duplex mode. After calling this
    *  function, the UART is ready to transmit data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  Valid only in half duplex mode. You must make sure that the previous
    *  transaction is complete and it is safe to unload the receiver configuration.
    *
    *******************************************************************************/
    void Wemos_LoadTxConfig(void) 
    {
    #if (Wemos_RX_INTERRUPT_ENABLED)
        /* Disable RX interrupts before set TX configuration */
        Wemos_SetRxInterruptMode(0u);
    #endif /* (Wemos_RX_INTERRUPT_ENABLED) */

        Wemos_WriteControlRegister(Wemos_ReadControlRegister() | Wemos_CTRL_HD_SEND);
        Wemos_RXBITCTR_PERIOD_REG = Wemos_HD_TXBITCTR_INIT;
    }

#endif  /* Wemos_HD_ENABLED */


/* [] END OF FILE */
