
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_i2c.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"

#include "utils/uartstdio.h"


int main(void)
{
	int i;
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); //i2c
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);


	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

	GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_3);


	I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, (0x48>>1), false);

	while(1)
    {
	/* poging bus vrij te maken
	 	I2CMasterDataPut(I2C0_MASTER_BASE, 0x55);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE))
		{
			i= i +5;
		}
     	I2CMasterDataPut(I2C0_MASTER_BASE, 0x55);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_STOP);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE));
*/

		I2CMasterDataPut(I2C0_MASTER_BASE, 0x00);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE));

		I2CMasterDataPut(I2C0_MASTER_BASE, 0x00);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE))

		I2CMasterDataPut(I2C0_MASTER_BASE, 0x55);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE)); //hier blockt hij, vorige data kon ik zien op mijn scoop

		I2CMasterDataPut(I2C0_MASTER_BASE, 0x00);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
		while(I2CMasterBusBusy(I2C0_MASTER_BASE));

		UARTprintf("test", I2CMasterErr(I2C0_MASTER_BASE));
    }
}
