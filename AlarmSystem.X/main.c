/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/
#define IN_TURN_ON      (GPIObits.GP5 == 0) 
#define IN_TURN_OFF     (GPIObits.GP4 == 0)
#define IN_SENSOR       (GPIObits.GP2 == 0)
#define OUT_SIREN_OFF   GPIObits.GP1 = 0
#define OUT_SIREN_ON    GPIObits.GP1 = 1
#define OUT_SUPPLAY_OFF GPIObits.GP0 = 0
#define OUT_SUPPLAY_ON  GPIObits.GP0 = 1

#define TIMER_40_SEC    255
#define TIMER_250_MSEC  1
#define TIMER_30_SEC    191
/* i.e. uint8_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

//__CONFIG _INTRC_OSC_NOCLKOUT & _MCLRE_OFF & CP_OFF & _CPD_OFF & _WDT_OFF & _BODEN_ON & _PWRTE_ON;
// CONFIG
#pragma config FOSC = INTRCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF       // GP3/MCLR pin function select (GP3/MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

void main(void)
{
    enum {
        WITHOUT_PROTECT         = 0,
        STARTING_FIST_SIREN     = 1,
        STARTING_SECOND_SIREN   = 2,
        STARTING                = 3,
        AT_THE_GUARD            = 4,
        STOPPING_FIST_SIREN     = 5,
        STOPPING_SECOND_SIREN   = 6,
        STOPPING                = 7,
    };
    
    uint8_t system_state = STARTING_FIST_SIREN;
    uint8_t timer = 0;
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();
    
    while(1)
    {
        if(PIR1bits.TMR1IF)
        {
            PIR1bits.TMR1IF = 0;
            if (timer)  timer--;
        }
        
        switch(system_state)
        {
            case WITHOUT_PROTECT:   
                OUT_SIREN_OFF;
                if(IN_TURN_ON)
                {                                    
                    timer = TIMER_250_MSEC;
                    system_state = STARTING_FIST_SIREN;
                }
                break;
            case STARTING_FIST_SIREN:                
                OUT_SIREN_ON;
                if (0 == timer)
                {  
                    timer = TIMER_40_SEC;                    
                    system_state = STARTING_SECOND_SIREN;
                }                 
                break;
            case STARTING_SECOND_SIREN:
                OUT_SIREN_OFF;
                OUT_SUPPLAY_ON;
                if (0 == timer)
                {  
                    timer = TIMER_250_MSEC;                    
                    system_state = STARTING;
                }                 
                break;                
            case STARTING:
                OUT_SIREN_ON;
                if (0 == timer)
                {
                    system_state = AT_THE_GUARD;
                }
                break;                
            case AT_THE_GUARD:
                if (timer) OUT_SIREN_ON;
                else       OUT_SIREN_OFF; 
                if(IN_SENSOR && !timer)
                {
                   timer = TIMER_30_SEC;
                }
                if(IN_TURN_OFF)
                {   
                    timer = TIMER_250_MSEC;
                    system_state = STOPPING_FIST_SIREN;
                }                 
                break;
            case STOPPING_FIST_SIREN:
                OUT_SIREN_ON;
                OUT_SUPPLAY_OFF;
                if (0 == timer)
                {                    
                    timer = TIMER_250_MSEC;                    
                    system_state = STOPPING_SECOND_SIREN;
                }
                break;
            case STOPPING_SECOND_SIREN:
                OUT_SIREN_OFF;
                if (0 == timer)
                {
                    timer = TIMER_250_MSEC;                    
                    system_state = STOPPING;
                }
                break; 
            case STOPPING:
                OUT_SIREN_ON;
                if (0 == timer)
                {                    
                    system_state = WITHOUT_PROTECT;
                }
                break;                
            default:
                //system_state = STARTING_FIST_SIREN;
                break;
        }                        
    }
}

