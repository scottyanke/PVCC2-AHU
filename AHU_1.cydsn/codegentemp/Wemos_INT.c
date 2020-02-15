/*******************************************************************************
* File Name: WemosINT.c
* Version 2.50
*
* Description:
*  This file provides all Interrupt Service functionality of the UART component
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Wemos.h"
#include "cyapicallbacks.h"


/***************************************
* Custom Declarations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED))
    /*******************************************************************************
    * Function Name: Wemos_RXISR
    ********************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine for RX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_rxBuffer - RAM buffer pointer for save received data.
    *  Wemos_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  Wemos_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  Wemos_rxBufferOverflow - software overflow flag. Set to one
    *     when Wemos_rxBufferWrite index overtakes
    *     Wemos_rxBufferRead index.
    *  Wemos_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when Wemos_rxBufferWrite is equal to
    *    Wemos_rxBufferRead
    *  Wemos_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  Wemos_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(Wemos_RXISR)
    {
        uint8 readData;
        uint8 readStatus;
        uint8 increment_pointer = 0u;

    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Wemos_RXISR_ENTRY_CALLBACK
        Wemos_RXISR_EntryCallback();
    #endif /* Wemos_RXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Wemos_RXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        do
        {
            /* Read receiver status register */
            readStatus = Wemos_RXSTATUS_REG;
            /* Copy the same status to readData variable for backward compatibility support 
            *  of the user code in Wemos_RXISR_ERROR` section. 
            */
            readData = readStatus;

            if((readStatus & (Wemos_RX_STS_BREAK | 
                            Wemos_RX_STS_PAR_ERROR |
                            Wemos_RX_STS_STOP_ERROR | 
                            Wemos_RX_STS_OVERRUN)) != 0u)
            {
                /* ERROR handling. */
                Wemos_errorStatus |= readStatus & ( Wemos_RX_STS_BREAK | 
                                                            Wemos_RX_STS_PAR_ERROR | 
                                                            Wemos_RX_STS_STOP_ERROR | 
                                                            Wemos_RX_STS_OVERRUN);
                /* `#START Wemos_RXISR_ERROR` */

                /* `#END` */
                
            #ifdef Wemos_RXISR_ERROR_CALLBACK
                Wemos_RXISR_ERROR_Callback();
            #endif /* Wemos_RXISR_ERROR_CALLBACK */
            }
            
            if((readStatus & Wemos_RX_STS_FIFO_NOTEMPTY) != 0u)
            {
                /* Read data from the RX data register */
                readData = Wemos_RXDATA_REG;
            #if (Wemos_RXHW_ADDRESS_ENABLED)
                if(Wemos_rxAddressMode == (uint8)Wemos__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readStatus & Wemos_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readStatus & Wemos_RX_STS_ADDR_MATCH) != 0u)
                        {
                            Wemos_rxAddressDetected = 1u;
                        }
                        else
                        {
                            Wemos_rxAddressDetected = 0u;
                        }
                    }
                    if(Wemos_rxAddressDetected != 0u)
                    {   /* Store only addressed data */
                        Wemos_rxBuffer[Wemos_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* Without software addressing */
                {
                    Wemos_rxBuffer[Wemos_rxBufferWrite] = readData;
                    increment_pointer = 1u;
                }
            #else  /* Without addressing */
                Wemos_rxBuffer[Wemos_rxBufferWrite] = readData;
                increment_pointer = 1u;
            #endif /* (Wemos_RXHW_ADDRESS_ENABLED) */

                /* Do not increment buffer pointer when skip not addressed data */
                if(increment_pointer != 0u)
                {
                    if(Wemos_rxBufferLoopDetect != 0u)
                    {   /* Set Software Buffer status Overflow */
                        Wemos_rxBufferOverflow = 1u;
                    }
                    /* Set next pointer. */
                    Wemos_rxBufferWrite++;

                    /* Check pointer for a loop condition */
                    if(Wemos_rxBufferWrite >= Wemos_RX_BUFFER_SIZE)
                    {
                        Wemos_rxBufferWrite = 0u;
                    }

                    /* Detect pre-overload condition and set flag */
                    if(Wemos_rxBufferWrite == Wemos_rxBufferRead)
                    {
                        Wemos_rxBufferLoopDetect = 1u;
                        /* When Hardware Flow Control selected */
                        #if (Wemos_FLOW_CONTROL != 0u)
                            /* Disable RX interrupt mask, it is enabled when user read data from the buffer using APIs */
                            Wemos_RXSTATUS_MASK_REG  &= (uint8)~Wemos_RX_STS_FIFO_NOTEMPTY;
                            CyIntClearPending(Wemos_RX_VECT_NUM);
                            break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                        #endif /* (Wemos_FLOW_CONTROL != 0u) */
                    }
                }
            }
        }while((readStatus & Wemos_RX_STS_FIFO_NOTEMPTY) != 0u);

        /* User code required at end of ISR (Optional) */
        /* `#START Wemos_RXISR_END` */

        /* `#END` */

    #ifdef Wemos_RXISR_EXIT_CALLBACK
        Wemos_RXISR_ExitCallback();
    #endif /* Wemos_RXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
    }
    
#endif /* (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED)) */


#if (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED)
    /*******************************************************************************
    * Function Name: Wemos_TXISR
    ********************************************************************************
    *
    * Summary:
    * Interrupt Service Routine for the TX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Wemos_txBuffer - RAM buffer pointer for transmit data from.
    *  Wemos_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmitted byte.
    *  Wemos_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(Wemos_TXISR)
    {
    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Wemos_TXISR_ENTRY_CALLBACK
        Wemos_TXISR_EntryCallback();
    #endif /* Wemos_TXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Wemos_TXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        while((Wemos_txBufferRead != Wemos_txBufferWrite) &&
             ((Wemos_TXSTATUS_REG & Wemos_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer wrap around */
            if(Wemos_txBufferRead >= Wemos_TX_BUFFER_SIZE)
            {
                Wemos_txBufferRead = 0u;
            }

            Wemos_TXDATA_REG = Wemos_txBuffer[Wemos_txBufferRead];

            /* Set next pointer */
            Wemos_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START Wemos_TXISR_END` */

        /* `#END` */

    #ifdef Wemos_TXISR_EXIT_CALLBACK
        Wemos_TXISR_ExitCallback();
    #endif /* Wemos_TXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
   }
#endif /* (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED) */


/* [] END OF FILE */
