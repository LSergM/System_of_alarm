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
//#define UN_HERITATE_ON
#ifdef UN_HERITATE_ON
    #define IN_TURN_ON      (inputs & num_in[2]) 
    #define IN_TURN_OFF     (inputs & num_in[1])
    #define IN_SENSOR       (inputs & num_in[0])
#else
    #define IN_TURN_ON      (0 == GPIObits.GP5) 
    #define IN_TURN_OFF     (0 == GPIObits.GP4)
    #define IN_SENSOR       (0 == GPIObits.GP2)
#endif
#define OUT_SIREN_OFF   GPIObits.GP1 = 0
#define OUT_SIREN_ON    GPIObits.GP1 = 1
#define OUT_SUPPLAY_OFF GPIObits.GP0 = 0
#define OUT_SUPPLAY_ON  GPIObits.GP0 = 1

#define TIMER_40_SEC    200
#define TIMER_250_MSEC  1
#define TIMER_1_SEC     4
#define TIMER_30_SEC    300

#define TIME_DELAY      60000U
#define ACTIV_INPUT     50000U

#define NUM_INPUT      3U

/* i.e. uint8_t <variable_name>; */
const uint8_t num_in[NUM_INPUT] = {0x04,0x10,0x20};

uint8_t unharitate(void);
void SetTimer(uint16_t ticks);

uint16_t timer = 0;
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
    uint8_t inputs = 0;    
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
        #ifdef UN_HERITATE_ON
            inputs = unharitate();
        #endif    
        switch(system_state)
        {
            case WITHOUT_PROTECT:   
                OUT_SIREN_OFF;
                if(IN_TURN_ON)
                {                                    
                    SetTimer(TIMER_250_MSEC);
                    system_state = STARTING_FIST_SIREN;
                }
                break;
            case STARTING_FIST_SIREN:                
                OUT_SIREN_ON;
                if (0 == timer)
                {  
                    SetTimer(TIMER_40_SEC);                    
                    system_state = STARTING_SECOND_SIREN;
                }                 
                break;
            case STARTING_SECOND_SIREN:
                OUT_SIREN_OFF;
                OUT_SUPPLAY_ON;
                if (0 == timer)
                {  
                    SetTimer(TIMER_250_MSEC);                    
                    system_state = STARTING;
                }            
                if(IN_TURN_OFF)
                {                  
                    SetTimer(TIMER_250_MSEC);
                    system_state = STOPPING_FIST_SIREN;
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
                   SetTimer(TIMER_30_SEC);
                }
                if(IN_TURN_OFF)
                {   
                    SetTimer(TIMER_250_MSEC);
                    system_state = STOPPING_FIST_SIREN;
                }                 
                break;
            case STOPPING_FIST_SIREN:
                OUT_SIREN_ON;       
                OUT_SUPPLAY_OFF;
                if (0 == timer)
                {                    
                    SetTimer(TIMER_250_MSEC);                    
                    system_state = STOPPING_SECOND_SIREN;
                }
                break;
            case STOPPING_SECOND_SIREN:
                OUT_SIREN_OFF;
                if (0 == timer)
                {
                    SetTimer(TIMER_250_MSEC);                    
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

void SetTimer(uint16_t ticks)
{
    PIR1bits.TMR1IF = 0;
    timer = ticks;
    TMR1  = 0;
}

uint8_t unharitate(void)
{
    static uint16_t cnt[NUM_INPUT+1];
    static uint8_t in_state = 0;
    uint8_t i = 0;
    
    for (i=0; i<NUM_INPUT; i++)
    {
        if( !(GPIO & num_in[i]) ) 
            cnt[i]++;
        else if (cnt[i]) cnt[i]--;
    }
    if(++cnt[3] > TIME_DELAY)
    {
        cnt[3] = 0;
        for (i=0; i<NUM_INPUT; i++)
        {
            if(cnt[i] > ACTIV_INPUT) in_state |= num_in[i];
            else in_state &= ~num_in[i];
            cnt[i] = 0;
        }            
    }        
    return in_state;
}

