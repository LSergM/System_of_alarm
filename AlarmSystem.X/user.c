/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    /* TODO Initialize User Ports/Peripherals/Project here */

    /* Setup analog functionality and port direction */

    /* Initialize peripherals */
    OPTION_REG = 0;
    
    T1CONbits.TMR1GE  = 0;
    T1CONbits.T1CKPS  = 1;    
    T1CONbits.T1OSCEN = 0;
    T1CONbits.nT1SYNC = 0;        
    T1CONbits.TMR1CS  = 0;
    T1CONbits.TMR1ON  = 1;
    
    //  Pull-up is On
    WPUbits.WPU0 = 0;
    WPUbits.WPU1 = 0;
    WPUbits.WPU2 = 1;
    WPUbits.WPU4 = 1;
    WPUbits.WPU5 = 1;                
    
    TRISIObits.TRISIO0 = 0;
    TRISIObits.TRISIO1 = 0;
    TRISIObits.TRISIO2 = 1;
    TRISIObits.TRISIO4 = 1;
    TRISIObits.TRISIO5 = 1;
    
    VRCON = 0;
    CMCONbits.CM = 7;
    /* Enable interrupts */
}

