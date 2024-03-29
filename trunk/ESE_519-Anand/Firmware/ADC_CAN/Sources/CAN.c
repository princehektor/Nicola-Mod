#include <hidef.h>
#include <MC9S12C128.h>     /* derivative information */
#include "common.h"
#include "CAN.h"
#pragma LINK_INFO DERIVATIVE "mc9s12c128"

void CANInit(void)
{
    CANCTL1_CANE   = 1;       // Enable CAN module
    CANCTL0_INITRQ = 1;       // Request initialization mode
    while(!CANCTL1_INITAK) ;  // Wait until initialization mode is entered

    CANCTL1 = 0x80;           // Enable CAN, OSCCLK as CAN clock
    CANBTR0 = 0xFF;           // SJW = 4, Prescaler = 64
    CANBTR1 = 0x44;           // Tseg1 = 4, Tseg2 = 4

    CANIDAC  = 0x10;          // 16-bit filter mode
    CANIDMR0 = 0xFF;          // All bits "don't care" (accept all messages)
    CANIDMR1 = 0xFF;
    CANIDMR2 = 0xFF;
    CANIDMR3 = 0xFF;
    CANIDMR4 = 0xFF;
    CANIDMR5 = 0xFF;
    CANIDMR6 = 0xFF;
    CANIDMR7 = 0xFF;

    //CANCTL1_LOOPB = 1; // Enable loopback (ONLY FOR TESTING)

    CANCTL0_INITRQ = 0; // Exit initialization mode
    while(CANCTL1_INITAK) ;
}

INT8 CANTx(UINT16 identifier, UINT8 *data, UINT8 length)
{
    UINT8  i;
    UINT8 *txPtr;
    UINT8  txBuf;

    if(!CANTFLG)
        return -1;  // All TX buffers full

    CANTBSEL = CANTFLG;

    txBuf = CANTBSEL;

    CANTXIDR0 = (UINT16)(identifier & 0x07F8) >> 3;
    CANTXIDR1 = (UINT16)(identifier & 0x0007) << 5;

    CANTXDLR = length;

    txPtr = &CANTXDSR0;
    for(i = 0; i < length; i++)
    {
        *txPtr = data[i];
        txPtr++;
    }

    CANTFLG = CANTBSEL;

    while( (CANTFLG & txBuf) != txBuf ) ;

    return length;
}

INT8 CANRx(UINT16 *identifier, UINT8 *buffer)
{
    UINT8  i;
    UINT8  length;
    UINT8 *rxPtr;

    if(!CANRFLG_RXF) // If data received
        return -1;

    *identifier = (CANRXIDR0 << 3) + (CANRXIDR1 >> 5);

    length = CANRXDLR & 0x0F;

    rxPtr = &CANRXDSR0;
    for(i = 0; i < length; i++)
    {
        buffer[i] = *rxPtr;
        rxPtr++;
    }

    CANRFLG_RXF = 1; // Reset the flag
    return length;
}
