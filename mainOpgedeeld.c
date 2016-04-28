#include <string.h>

#include "lwip/tcp.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_i2c.h"

#include "driverlib/ethernet.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "driverlib/i2c.h"

#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "httpserver_raw/httpd.h"
#include "drivers/rit128x96x4.h"

//nieuwe users qt->sql
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"

#include "C:/Users/Justine/Desktop/alex/fidel SimpleLWIPv2/SimpleLWIP/mijncustomfuncties.h"


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif




//*****************************************************************************************
int main(void)
{
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];
	struct tcp_pcb * connectie;
	struct ip_addr serverIp;
	gotIp = 0;

	//rfid
	char uidValue[4]={0};
	int i=0;
	int k=0;

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);


    // Initialize the UART for debug output.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    //
    // Configure SysTick for a periodic interrupt.
    //
    SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
    SysTickEnable();
    SysTickIntEnable();

    IntMasterEnable();

/*mijn UART*/

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    IntRegister(INT_UART0,UARTStdioIntHandler );
    IntEnable(INT_UART0);
   UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);


    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        //
        RIT128x96x4StringDraw("MAC Address", 0, 16, 15);
        RIT128x96x4StringDraw("Not Programmed!", 0, 24, 15);
        while(1)
        {
        }
    }

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
    pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
    pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    // Initialze the lwIP library, using static IP.
	// 192.168.0.10 255.255.255.0 192.168.0.1
    lwIPInit(pucMACArray, 0xC0A8000A, 0xFFFFFF00, 0xC0A80001, IPADDR_USE_STATIC);
    // Initialze the lwIP library, using DHCP.
//	lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
    // Setup the device locator service.
    LocatorInit();
    LocatorMACAddrSet(pucMACArray);
    LocatorAppTitleSet("EK-LM3S6965 WDA_tcp");


    IntPriorityGroupingSet(4);
    IntPrioritySet(INT_ETH, ETHERNET_INT_PRIORITY);
    IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);


	resetModule();
	sendCommand(SAMConfigure,5);
	checkIRQ();
	readACKframe();
	readSAMResponse();

    while(1)
    {
		if(gotIp){
			UARTprintf("in main lus");

			authentificeerKaart(uidValue);
			if(uartData==0)
			{
				//UARTprintf("ik zit in de main, fout");
				authentificeerDataBlock(authenticateCmds,4,uidValue);
				readBlock('4', cardData);
			}
		//	UARTprintf("ik ga connecte me de server");
			IP4_ADDR(&serverIp, 192,168,0,20);
			connectie=tcp_new();
			tcp_connect(connectie,&serverIp,80,connectedToserver);
			//while(1);

		}
		UARTprintf("gn ip");
    }
    return 0;
}

