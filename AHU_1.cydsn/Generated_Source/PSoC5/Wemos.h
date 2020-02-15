/*******************************************************************************
* File Name: Wemos.h
* Version 2.50
*
* Description:
*  Contains the function prototypes and constants available to the UART
*  user module.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(CY_UART_Wemos_H)
#define CY_UART_Wemos_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
* Conditional Compilation Parameters
***************************************/

#define Wemos_RX_ENABLED                     (1u)
#define Wemos_TX_ENABLED                     (1u)
#define Wemos_HD_ENABLED                     (0u)
#define Wemos_RX_INTERRUPT_ENABLED           (0u)
#define Wemos_TX_INTERRUPT_ENABLED           (0u)
#define Wemos_INTERNAL_CLOCK_USED            (1u)
#define Wemos_RXHW_ADDRESS_ENABLED           (0u)
#define Wemos_OVER_SAMPLE_COUNT              (8u)
#define Wemos_PARITY_TYPE                    (0u)
#define Wemos_PARITY_TYPE_SW                 (0u)
#define Wemos_BREAK_DETECT                   (0u)
#define Wemos_BREAK_BITS_TX                  (13u)
#define Wemos_BREAK_BITS_RX                  (13u)
#define Wemos_TXCLKGEN_DP                    (1u)
#define Wemos_USE23POLLING                   (1u)
#define Wemos_FLOW_CONTROL                   (0u)
#define Wemos_CLK_FREQ                       (0u)
#define Wemos_TX_BUFFER_SIZE                 (4u)
#define Wemos_RX_BUFFER_SIZE                 (4u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_50 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#if defined(Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG)
    #define Wemos_CONTROL_REG_REMOVED            (0u)
#else
    #define Wemos_CONTROL_REG_REMOVED            (1u)
#endif /* End Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Structure Definition
***************************************/

/* Sleep Mode API Support */
typedef struct Wemos_backupStruct_
{
    uint8 enableState;

    #if(Wemos_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End Wemos_CONTROL_REG_REMOVED */

} Wemos_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void Wemos_Start(void) ;
void Wemos_Stop(void) ;
uint8 Wemos_ReadControlRegister(void) ;
void Wemos_WriteControlRegister(uint8 control) ;

void Wemos_Init(void) ;
void Wemos_Enable(void) ;
void Wemos_SaveConfig(void) ;
void Wemos_RestoreConfig(void) ;
void Wemos_Sleep(void) ;
void Wemos_Wakeup(void) ;

/* Only if RX is enabled */
#if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )

    #if (Wemos_RX_INTERRUPT_ENABLED)
        #define Wemos_EnableRxInt()  CyIntEnable (Wemos_RX_VECT_NUM)
        #define Wemos_DisableRxInt() CyIntDisable(Wemos_RX_VECT_NUM)
        CY_ISR_PROTO(Wemos_RXISR);
    #endif /* Wemos_RX_INTERRUPT_ENABLED */

    void Wemos_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void Wemos_SetRxAddress1(uint8 address) ;
    void Wemos_SetRxAddress2(uint8 address) ;

    void  Wemos_SetRxInterruptMode(uint8 intSrc) ;
    uint8 Wemos_ReadRxData(void) ;
    uint8 Wemos_ReadRxStatus(void) ;
    uint8 Wemos_GetChar(void) ;
    uint16 Wemos_GetByte(void) ;
    uint8 Wemos_GetRxBufferSize(void)
                                                            ;
    void Wemos_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define Wemos_GetRxInterruptSource   Wemos_ReadRxStatus

#endif /* End (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */

/* Only if TX is enabled */
#if(Wemos_TX_ENABLED || Wemos_HD_ENABLED)

    #if(Wemos_TX_INTERRUPT_ENABLED)
        #define Wemos_EnableTxInt()  CyIntEnable (Wemos_TX_VECT_NUM)
        #define Wemos_DisableTxInt() CyIntDisable(Wemos_TX_VECT_NUM)
        #define Wemos_SetPendingTxInt() CyIntSetPending(Wemos_TX_VECT_NUM)
        #define Wemos_ClearPendingTxInt() CyIntClearPending(Wemos_TX_VECT_NUM)
        CY_ISR_PROTO(Wemos_TXISR);
    #endif /* Wemos_TX_INTERRUPT_ENABLED */

    void Wemos_SetTxInterruptMode(uint8 intSrc) ;
    void Wemos_WriteTxData(uint8 txDataByte) ;
    uint8 Wemos_ReadTxStatus(void) ;
    void Wemos_PutChar(uint8 txDataByte) ;
    void Wemos_PutString(const char8 string[]) ;
    void Wemos_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void Wemos_PutCRLF(uint8 txDataByte) ;
    void Wemos_ClearTxBuffer(void) ;
    void Wemos_SetTxAddressMode(uint8 addressMode) ;
    void Wemos_SendBreak(uint8 retMode) ;
    uint8 Wemos_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define Wemos_PutStringConst         Wemos_PutString
    #define Wemos_PutArrayConst          Wemos_PutArray
    #define Wemos_GetTxInterruptSource   Wemos_ReadTxStatus

#endif /* End Wemos_TX_ENABLED || Wemos_HD_ENABLED */

#if(Wemos_HD_ENABLED)
    void Wemos_LoadRxConfig(void) ;
    void Wemos_LoadTxConfig(void) ;
#endif /* End Wemos_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Wemos) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    Wemos_CyBtldrCommStart(void) CYSMALL ;
    void    Wemos_CyBtldrCommStop(void) CYSMALL ;
    void    Wemos_CyBtldrCommReset(void) CYSMALL ;
    cystatus Wemos_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus Wemos_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Wemos)
        #define CyBtldrCommStart    Wemos_CyBtldrCommStart
        #define CyBtldrCommStop     Wemos_CyBtldrCommStop
        #define CyBtldrCommReset    Wemos_CyBtldrCommReset
        #define CyBtldrCommWrite    Wemos_CyBtldrCommWrite
        #define CyBtldrCommRead     Wemos_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Wemos) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define Wemos_BYTE2BYTE_TIME_OUT (25u)
    #define Wemos_PACKET_EOP         (0x17u) /* End of packet defined by bootloader */
    #define Wemos_WAIT_EOP_DELAY     (5u)    /* Additional 5ms to wait for End of packet */
    #define Wemos_BL_CHK_DELAY_MS    (1u)    /* Time Out quantity equal 1mS */

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define Wemos_SET_SPACE      (0x00u)
#define Wemos_SET_MARK       (0x01u)

/* Status Register definitions */
#if( (Wemos_TX_ENABLED) || (Wemos_HD_ENABLED) )
    #if(Wemos_TX_INTERRUPT_ENABLED)
        #define Wemos_TX_VECT_NUM            (uint8)Wemos_TXInternalInterrupt__INTC_NUMBER
        #define Wemos_TX_PRIOR_NUM           (uint8)Wemos_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Wemos_TX_INTERRUPT_ENABLED */

    #define Wemos_TX_STS_COMPLETE_SHIFT          (0x00u)
    #define Wemos_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
    #define Wemos_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #if(Wemos_TX_ENABLED)
        #define Wemos_TX_STS_FIFO_FULL_SHIFT     (0x02u)
    #else /* (Wemos_HD_ENABLED) */
        #define Wemos_TX_STS_FIFO_FULL_SHIFT     (0x05u)  /* Needs MD=0 */
    #endif /* (Wemos_TX_ENABLED) */

    #define Wemos_TX_STS_COMPLETE            (uint8)(0x01u << Wemos_TX_STS_COMPLETE_SHIFT)
    #define Wemos_TX_STS_FIFO_EMPTY          (uint8)(0x01u << Wemos_TX_STS_FIFO_EMPTY_SHIFT)
    #define Wemos_TX_STS_FIFO_FULL           (uint8)(0x01u << Wemos_TX_STS_FIFO_FULL_SHIFT)
    #define Wemos_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << Wemos_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (Wemos_TX_ENABLED) || (Wemos_HD_ENABLED)*/

#if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )
    #if(Wemos_RX_INTERRUPT_ENABLED)
        #define Wemos_RX_VECT_NUM            (uint8)Wemos_RXInternalInterrupt__INTC_NUMBER
        #define Wemos_RX_PRIOR_NUM           (uint8)Wemos_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Wemos_RX_INTERRUPT_ENABLED */
    #define Wemos_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define Wemos_RX_STS_BREAK_SHIFT             (0x01u)
    #define Wemos_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define Wemos_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define Wemos_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define Wemos_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define Wemos_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define Wemos_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define Wemos_RX_STS_MRKSPC           (uint8)(0x01u << Wemos_RX_STS_MRKSPC_SHIFT)
    #define Wemos_RX_STS_BREAK            (uint8)(0x01u << Wemos_RX_STS_BREAK_SHIFT)
    #define Wemos_RX_STS_PAR_ERROR        (uint8)(0x01u << Wemos_RX_STS_PAR_ERROR_SHIFT)
    #define Wemos_RX_STS_STOP_ERROR       (uint8)(0x01u << Wemos_RX_STS_STOP_ERROR_SHIFT)
    #define Wemos_RX_STS_OVERRUN          (uint8)(0x01u << Wemos_RX_STS_OVERRUN_SHIFT)
    #define Wemos_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << Wemos_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define Wemos_RX_STS_ADDR_MATCH       (uint8)(0x01u << Wemos_RX_STS_ADDR_MATCH_SHIFT)
    #define Wemos_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << Wemos_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define Wemos_RX_HW_MASK                     (0x7Fu)
#endif /* End (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */

/* Control Register definitions */
#define Wemos_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define Wemos_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define Wemos_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define Wemos_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define Wemos_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define Wemos_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define Wemos_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define Wemos_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define Wemos_CTRL_HD_SEND               (uint8)(0x01u << Wemos_CTRL_HD_SEND_SHIFT)
#define Wemos_CTRL_HD_SEND_BREAK         (uint8)(0x01u << Wemos_CTRL_HD_SEND_BREAK_SHIFT)
#define Wemos_CTRL_MARK                  (uint8)(0x01u << Wemos_CTRL_MARK_SHIFT)
#define Wemos_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << Wemos_CTRL_PARITY_TYPE0_SHIFT)
#define Wemos_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << Wemos_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define Wemos_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define Wemos_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define Wemos_SEND_BREAK                         (0x00u)
#define Wemos_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define Wemos_REINIT                             (0x02u)
#define Wemos_SEND_WAIT_REINIT                   (0x03u)

#define Wemos_OVER_SAMPLE_8                      (8u)
#define Wemos_OVER_SAMPLE_16                     (16u)

#define Wemos_BIT_CENTER                         (Wemos_OVER_SAMPLE_COUNT - 2u)

#define Wemos_FIFO_LENGTH                        (4u)
#define Wemos_NUMBER_OF_START_BIT                (1u)
#define Wemos_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation */
#define Wemos_TXBITCTR_BREAKBITS8X   ((Wemos_BREAK_BITS_TX * Wemos_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation */
#define Wemos_TXBITCTR_BREAKBITS ((Wemos_BREAK_BITS_TX * Wemos_OVER_SAMPLE_COUNT) - 1u)

#define Wemos_HALF_BIT_COUNT   \
                            (((Wemos_OVER_SAMPLE_COUNT / 2u) + (Wemos_USE23POLLING * 1u)) - 2u)
#if (Wemos_OVER_SAMPLE_COUNT == Wemos_OVER_SAMPLE_8)
    #define Wemos_HD_TXBITCTR_INIT   (((Wemos_BREAK_BITS_TX + \
                            Wemos_NUMBER_OF_START_BIT) * Wemos_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define Wemos_RXBITCTR_INIT  ((((Wemos_BREAK_BITS_RX + Wemos_NUMBER_OF_START_BIT) \
                            * Wemos_OVER_SAMPLE_COUNT) + Wemos_HALF_BIT_COUNT) - 1u)

#else /* Wemos_OVER_SAMPLE_COUNT == Wemos_OVER_SAMPLE_16 */
    #define Wemos_HD_TXBITCTR_INIT   ((8u * Wemos_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount = 16 */
    #define Wemos_RXBITCTR_INIT      (((7u * Wemos_OVER_SAMPLE_COUNT) - 1u) + \
                                                      Wemos_HALF_BIT_COUNT)
#endif /* End Wemos_OVER_SAMPLE_COUNT */

#define Wemos_HD_RXBITCTR_INIT                   Wemos_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 Wemos_initVar;
#if (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED)
    extern volatile uint8 Wemos_txBuffer[Wemos_TX_BUFFER_SIZE];
    extern volatile uint8 Wemos_txBufferRead;
    extern uint8 Wemos_txBufferWrite;
#endif /* (Wemos_TX_INTERRUPT_ENABLED && Wemos_TX_ENABLED) */
#if (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED))
    extern uint8 Wemos_errorStatus;
    extern volatile uint8 Wemos_rxBuffer[Wemos_RX_BUFFER_SIZE];
    extern volatile uint8 Wemos_rxBufferRead;
    extern volatile uint8 Wemos_rxBufferWrite;
    extern volatile uint8 Wemos_rxBufferLoopDetect;
    extern volatile uint8 Wemos_rxBufferOverflow;
    #if (Wemos_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 Wemos_rxAddressMode;
        extern volatile uint8 Wemos_rxAddressDetected;
    #endif /* (Wemos_RXHW_ADDRESS_ENABLED) */
#endif /* (Wemos_RX_INTERRUPT_ENABLED && (Wemos_RX_ENABLED || Wemos_HD_ENABLED)) */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define Wemos__B_UART__AM_SW_BYTE_BYTE 1
#define Wemos__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define Wemos__B_UART__AM_HW_BYTE_BY_BYTE 3
#define Wemos__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define Wemos__B_UART__AM_NONE 0

#define Wemos__B_UART__NONE_REVB 0
#define Wemos__B_UART__EVEN_REVB 1
#define Wemos__B_UART__ODD_REVB 2
#define Wemos__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define Wemos_NUMBER_OF_DATA_BITS    ((8u > 8u) ? 8u : 8u)
#define Wemos_NUMBER_OF_STOP_BITS    (1u)

#if (Wemos_RXHW_ADDRESS_ENABLED)
    #define Wemos_RX_ADDRESS_MODE    (0u)
    #define Wemos_RX_HW_ADDRESS1     (0u)
    #define Wemos_RX_HW_ADDRESS2     (0u)
#endif /* (Wemos_RXHW_ADDRESS_ENABLED) */

#define Wemos_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << Wemos_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << Wemos_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << Wemos_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << Wemos_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << Wemos_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << Wemos_RX_STS_BREAK_SHIFT) \
                                        | (0 << Wemos_RX_STS_OVERRUN_SHIFT))

#define Wemos_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << Wemos_TX_STS_COMPLETE_SHIFT) \
                                        | (1 << Wemos_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << Wemos_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << Wemos_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Wemos_CONTROL_REG \
                            (* (reg8 *) Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define Wemos_CONTROL_PTR \
                            (  (reg8 *) Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Wemos_TX_ENABLED)
    #define Wemos_TXDATA_REG          (* (reg8 *) Wemos_BUART_sTX_TxShifter_u0__F0_REG)
    #define Wemos_TXDATA_PTR          (  (reg8 *) Wemos_BUART_sTX_TxShifter_u0__F0_REG)
    #define Wemos_TXDATA_AUX_CTL_REG  (* (reg8 *) Wemos_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Wemos_TXDATA_AUX_CTL_PTR  (  (reg8 *) Wemos_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Wemos_TXSTATUS_REG        (* (reg8 *) Wemos_BUART_sTX_TxSts__STATUS_REG)
    #define Wemos_TXSTATUS_PTR        (  (reg8 *) Wemos_BUART_sTX_TxSts__STATUS_REG)
    #define Wemos_TXSTATUS_MASK_REG   (* (reg8 *) Wemos_BUART_sTX_TxSts__MASK_REG)
    #define Wemos_TXSTATUS_MASK_PTR   (  (reg8 *) Wemos_BUART_sTX_TxSts__MASK_REG)
    #define Wemos_TXSTATUS_ACTL_REG   (* (reg8 *) Wemos_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define Wemos_TXSTATUS_ACTL_PTR   (  (reg8 *) Wemos_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(Wemos_TXCLKGEN_DP)
        #define Wemos_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Wemos_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Wemos_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define Wemos_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define Wemos_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Wemos_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Wemos_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Wemos_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Wemos_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define Wemos_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) Wemos_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* Wemos_TXCLKGEN_DP */

#endif /* End Wemos_TX_ENABLED */

#if(Wemos_HD_ENABLED)

    #define Wemos_TXDATA_REG             (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__F1_REG )
    #define Wemos_TXDATA_PTR             (  (reg8 *) Wemos_BUART_sRX_RxShifter_u0__F1_REG )
    #define Wemos_TXDATA_AUX_CTL_REG     (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define Wemos_TXDATA_AUX_CTL_PTR     (  (reg8 *) Wemos_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Wemos_TXSTATUS_REG           (* (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_REG )
    #define Wemos_TXSTATUS_PTR           (  (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_REG )
    #define Wemos_TXSTATUS_MASK_REG      (* (reg8 *) Wemos_BUART_sRX_RxSts__MASK_REG )
    #define Wemos_TXSTATUS_MASK_PTR      (  (reg8 *) Wemos_BUART_sRX_RxSts__MASK_REG )
    #define Wemos_TXSTATUS_ACTL_REG      (* (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Wemos_TXSTATUS_ACTL_PTR      (  (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End Wemos_HD_ENABLED */

#if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )
    #define Wemos_RXDATA_REG             (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__F0_REG )
    #define Wemos_RXDATA_PTR             (  (reg8 *) Wemos_BUART_sRX_RxShifter_u0__F0_REG )
    #define Wemos_RXADDRESS1_REG         (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__D0_REG )
    #define Wemos_RXADDRESS1_PTR         (  (reg8 *) Wemos_BUART_sRX_RxShifter_u0__D0_REG )
    #define Wemos_RXADDRESS2_REG         (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__D1_REG )
    #define Wemos_RXADDRESS2_PTR         (  (reg8 *) Wemos_BUART_sRX_RxShifter_u0__D1_REG )
    #define Wemos_RXDATA_AUX_CTL_REG     (* (reg8 *) Wemos_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Wemos_RXBITCTR_PERIOD_REG    (* (reg8 *) Wemos_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Wemos_RXBITCTR_PERIOD_PTR    (  (reg8 *) Wemos_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Wemos_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Wemos_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Wemos_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Wemos_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Wemos_RXBITCTR_COUNTER_REG   (* (reg8 *) Wemos_BUART_sRX_RxBitCounter__COUNT_REG )
    #define Wemos_RXBITCTR_COUNTER_PTR   (  (reg8 *) Wemos_BUART_sRX_RxBitCounter__COUNT_REG )

    #define Wemos_RXSTATUS_REG           (* (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_REG )
    #define Wemos_RXSTATUS_PTR           (  (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_REG )
    #define Wemos_RXSTATUS_MASK_REG      (* (reg8 *) Wemos_BUART_sRX_RxSts__MASK_REG )
    #define Wemos_RXSTATUS_MASK_PTR      (  (reg8 *) Wemos_BUART_sRX_RxSts__MASK_REG )
    #define Wemos_RXSTATUS_ACTL_REG      (* (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Wemos_RXSTATUS_ACTL_PTR      (  (reg8 *) Wemos_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */

#if(Wemos_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define Wemos_INTCLOCK_CLKEN_REG     (* (reg8 *) Wemos_IntClock__PM_ACT_CFG)
    #define Wemos_INTCLOCK_CLKEN_PTR     (  (reg8 *) Wemos_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define Wemos_INTCLOCK_CLKEN_MASK    Wemos_IntClock__PM_ACT_MSK
#endif /* End Wemos_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(Wemos_TX_ENABLED)
    #define Wemos_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End Wemos_TX_ENABLED */

#if(Wemos_HD_ENABLED)
    #define Wemos_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End Wemos_HD_ENABLED */

#if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )
    #define Wemos_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

/* UART v2_40 obsolete definitions */
#define Wemos_WAIT_1_MS      Wemos_BL_CHK_DELAY_MS   

#define Wemos_TXBUFFERSIZE   Wemos_TX_BUFFER_SIZE
#define Wemos_RXBUFFERSIZE   Wemos_RX_BUFFER_SIZE

#if (Wemos_RXHW_ADDRESS_ENABLED)
    #define Wemos_RXADDRESSMODE  Wemos_RX_ADDRESS_MODE
    #define Wemos_RXHWADDRESS1   Wemos_RX_HW_ADDRESS1
    #define Wemos_RXHWADDRESS2   Wemos_RX_HW_ADDRESS2
    /* Backward compatible define */
    #define Wemos_RXAddressMode  Wemos_RXADDRESSMODE
#endif /* (Wemos_RXHW_ADDRESS_ENABLED) */

/* UART v2_30 obsolete definitions */
#define Wemos_initvar                    Wemos_initVar

#define Wemos_RX_Enabled                 Wemos_RX_ENABLED
#define Wemos_TX_Enabled                 Wemos_TX_ENABLED
#define Wemos_HD_Enabled                 Wemos_HD_ENABLED
#define Wemos_RX_IntInterruptEnabled     Wemos_RX_INTERRUPT_ENABLED
#define Wemos_TX_IntInterruptEnabled     Wemos_TX_INTERRUPT_ENABLED
#define Wemos_InternalClockUsed          Wemos_INTERNAL_CLOCK_USED
#define Wemos_RXHW_Address_Enabled       Wemos_RXHW_ADDRESS_ENABLED
#define Wemos_OverSampleCount            Wemos_OVER_SAMPLE_COUNT
#define Wemos_ParityType                 Wemos_PARITY_TYPE

#if( Wemos_TX_ENABLED && (Wemos_TXBUFFERSIZE > Wemos_FIFO_LENGTH))
    #define Wemos_TXBUFFER               Wemos_txBuffer
    #define Wemos_TXBUFFERREAD           Wemos_txBufferRead
    #define Wemos_TXBUFFERWRITE          Wemos_txBufferWrite
#endif /* End Wemos_TX_ENABLED */
#if( ( Wemos_RX_ENABLED || Wemos_HD_ENABLED ) && \
     (Wemos_RXBUFFERSIZE > Wemos_FIFO_LENGTH) )
    #define Wemos_RXBUFFER               Wemos_rxBuffer
    #define Wemos_RXBUFFERREAD           Wemos_rxBufferRead
    #define Wemos_RXBUFFERWRITE          Wemos_rxBufferWrite
    #define Wemos_RXBUFFERLOOPDETECT     Wemos_rxBufferLoopDetect
    #define Wemos_RXBUFFER_OVERFLOW      Wemos_rxBufferOverflow
#endif /* End Wemos_RX_ENABLED */

#ifdef Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Wemos_CONTROL                Wemos_CONTROL_REG
#endif /* End Wemos_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Wemos_TX_ENABLED)
    #define Wemos_TXDATA                 Wemos_TXDATA_REG
    #define Wemos_TXSTATUS               Wemos_TXSTATUS_REG
    #define Wemos_TXSTATUS_MASK          Wemos_TXSTATUS_MASK_REG
    #define Wemos_TXSTATUS_ACTL          Wemos_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(Wemos_TXCLKGEN_DP)
        #define Wemos_TXBITCLKGEN_CTR        Wemos_TXBITCLKGEN_CTR_REG
        #define Wemos_TXBITCLKTX_COMPLETE    Wemos_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define Wemos_TXBITCTR_PERIOD        Wemos_TXBITCTR_PERIOD_REG
        #define Wemos_TXBITCTR_CONTROL       Wemos_TXBITCTR_CONTROL_REG
        #define Wemos_TXBITCTR_COUNTER       Wemos_TXBITCTR_COUNTER_REG
    #endif /* Wemos_TXCLKGEN_DP */
#endif /* End Wemos_TX_ENABLED */

#if(Wemos_HD_ENABLED)
    #define Wemos_TXDATA                 Wemos_TXDATA_REG
    #define Wemos_TXSTATUS               Wemos_TXSTATUS_REG
    #define Wemos_TXSTATUS_MASK          Wemos_TXSTATUS_MASK_REG
    #define Wemos_TXSTATUS_ACTL          Wemos_TXSTATUS_ACTL_REG
#endif /* End Wemos_HD_ENABLED */

#if( (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) )
    #define Wemos_RXDATA                 Wemos_RXDATA_REG
    #define Wemos_RXADDRESS1             Wemos_RXADDRESS1_REG
    #define Wemos_RXADDRESS2             Wemos_RXADDRESS2_REG
    #define Wemos_RXBITCTR_PERIOD        Wemos_RXBITCTR_PERIOD_REG
    #define Wemos_RXBITCTR_CONTROL       Wemos_RXBITCTR_CONTROL_REG
    #define Wemos_RXBITCTR_COUNTER       Wemos_RXBITCTR_COUNTER_REG
    #define Wemos_RXSTATUS               Wemos_RXSTATUS_REG
    #define Wemos_RXSTATUS_MASK          Wemos_RXSTATUS_MASK_REG
    #define Wemos_RXSTATUS_ACTL          Wemos_RXSTATUS_ACTL_REG
#endif /* End  (Wemos_RX_ENABLED) || (Wemos_HD_ENABLED) */

#if(Wemos_INTERNAL_CLOCK_USED)
    #define Wemos_INTCLOCK_CLKEN         Wemos_INTCLOCK_CLKEN_REG
#endif /* End Wemos_INTERNAL_CLOCK_USED */

#define Wemos_WAIT_FOR_COMLETE_REINIT    Wemos_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_Wemos_H */


/* [] END OF FILE */
