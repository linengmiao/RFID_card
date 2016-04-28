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

const char firmwareCommand[1]={0x02};
const char  SAMConfigure[4]={0x14,0x01,0x14,0x01};
const char  readCardCmds[3]={0x4A, 0x01, 0x00};
const char  readBlockCmds[4]={0x40, 0x01, 0x30, 0x04}; // om echt een eeprom blok me data op vd kaart te lezen;

const char  publicKey[6]={0xff,0xff,0xff,0xff,0xff,0xff}; //voor authenticatie
char  authenticateCmds[14]= {0x40, 0x01, 0x60, 0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xe3,0xff,0x20,0xd0}; //eerste 0x00 is welk blok dat je wilt checken

int uartData=0;
//char cardData[4]={'a','l','e','x'};
extern char cardData[4]={0};
extern char newUserData[100]={0};
volatile int gotIp=1;

void UARTStdioIntHandler(void)
{
	 unsigned long ulStatus;
	// static char data[100]={0};
	 static char i=0;
	 int j;

	 RIT128x96x4Enable(1000000);
	 UARTgets(newUserData,100);

	 //UARTprintf("interrupt occured");
	 i=0;
	/* for(i=0;i<100;i++)
	 {newUserData[i]=0;}*/
	 uartData = 1;
}

/*********************************************************************************************************************************************************************************/

err_t connectedToserver(void *arg,  struct tcp_pcb *tpcb, err_t err)
{

	//tcp_write(tpcb, "GET /ontvangenData.php?name=alex HTTP/1.1\r\n\Host:192.168.0.20\r\nconnection: keep-alive\r\n\r\n", 90, 1);
	//tcp_write(tpcb, "GET /nieuweGebruiker.php?naam=alexandre2&voornaam=dao&geboorteDatum=1994-03-06&ingeschrevenTot=2004-02-17 HTTP/1.1\r\n\Host:192.168.0.20\r\nconnection: keep-alive\r\n\r\n", 180, 1);

	UARTprintf("\connected to server.\n");

	if (uartData == 1){ //deze variabele wordt geset op het moment van een interrupt, om users toe te voegen
		//UARTprintf("gecon met de server");
	sendDataToServer(tpcb);

	}
	else{ //als een kaart werd gedetecteerd,  om users te zoeken
	getUser(tpcb);
	}
	uartData = 0;
	tcp_recv(tpcb,ontvangCvanS);

	tcp_close(tpcb);
	return 0;
}

/*********************************************************************************************************************************************************************************/

void sendDataToServer(struct tcp_pcb *tpcb) // bouwt de URL op om die te zenden
{
	char getRequestLineBase[300] = "GET /nieuweGebruiker.php?naam=";
	char addFlags[60]=" HTTP/1.1\r\n\Host:192.168.0.20\r\n\r\n";

	//tcp_write(tpcb, "GET /ontvangenData.php?name=alex HTTP/1.1\r\n\Host:192.168.0.20\r\nconnection: keep-alive\r\n\r\n", 90, 1);
	//http://127.0.0.1/nieuweGebruiker.php?naam=alexandre&voornaam=dao&geboorteDatum=1994-03-06&ingeschrevenTot=2004-02-17

	struct usersData mijnStruct;
	scrapeRxBuffer(&mijnStruct); //return struct met daarin alle data die in de MySQL server kan geplaatst worden

	strcat(getRequestLineBase, mijnStruct.naam);
	strcat(getRequestLineBase,"&voornaam=");
	strcat(getRequestLineBase,mijnStruct.voorNaam);
	strcat(getRequestLineBase,"&geboorteDatum=");
	strcat(getRequestLineBase,mijnStruct.geboorteDatum);
	strcat(getRequestLineBase,"&ingeschrevenTot=");
	strcat(getRequestLineBase,mijnStruct.ingenschrevenTot);
	strcat(getRequestLineBase,addFlags);

	tcp_write(tpcb, getRequestLineBase, 180, 1);
	UARTprintf("New user successfully added.\n");
	uartData = 0;

}

/*********************************************************************************************************************************************************************************/
/*
 * timestamp, voornaam, achternaam, geboortedatum, ingeschreven tot
 * elk van deze elementen worden gescheiden door een tab en teruggegeven onder de vorm van een array van strings
 * elke string wordt vervolgens nr de server gestuurd
 */
void scrapeRxBuffer(struct usersData *ptrMijnStruct)
{
	int i=0;
	int j=0;
	int k=0;
	char UARTBuffer[100]={0};
	char mijnString[100]={0};


	//UARTprintf("typ de informatie: ");
	strcpy(ptrMijnStruct ->voorNaam ,"");
	strcpy(ptrMijnStruct ->naam ,"");
	strcpy(ptrMijnStruct ->geboorteDatum ,"");
	strcpy(ptrMijnStruct ->ingenschrevenTot ,"");
	strcpy(ptrMijnStruct ->foto ,"");
	strcpy(ptrMijnStruct ->timeStamp ,"");
/*
	RIT128x96x4Init(1000000);
	RIT128x96x4StringDraw("stuur uw data", 12, 0, 15);

	while(UARTgets(UARTBuffer,100)==0);
	RIT128x96x4Init(1000000);
	RIT128x96x4StringDraw("data ontvangen", 12, 0, 15);
*/
	while(newUserData[i] !='%') //einde vd data wordt in qt aangeduid met een procent om eventuele rommel te vermijden
	{
		 mijnString[k] = newUserData[i];
		 if (j!=0)
		 {k++;}
		 if (newUserData[i]=='|')
		 {
			 if (j==0)
			{
				 mijnString[strlen(mijnString)-1] = 0;
				 strcpy(ptrMijnStruct -> voorNaam ,mijnString);
				 strcpy(mijnString,"");
			}
			 else if(j==1)
			 {
				 mijnString[strlen(mijnString)-1] = 0;
				 strcpy(ptrMijnStruct ->naam ,mijnString);
				 strcpy(mijnString,"");
			 }
			 else if(j==2)
			 {
				 mijnString[strlen(mijnString)-1] = 0;
				 strcpy(ptrMijnStruct ->geboorteDatum ,mijnString);
				 strcpy(mijnString,"");
			 }
			 else if(j==3)
			 {
				 mijnString[strlen(mijnString)-1] = 0;
				 strcpy(ptrMijnStruct ->ingenschrevenTot ,mijnString);
				 strcpy(mijnString,"");
			 }
			 else if(j==4)
			 {
				 mijnString[strlen(mijnString)-1] = 0;
				 strcpy(ptrMijnStruct ->foto ,mijnString);
				 strcpy(mijnString,"");
			 }
			 else if(j==5)
			 {
				 strcpy(ptrMijnStruct ->timeStamp ,mijnString);
				 strcpy(mijnString,"");
			 }
			 j++;
			 k=0;
		 }
			i++;
			if(j==0){
			k++;}

	}
	UARTprintf("user info:\n");
	UARTprintf("voornaam: %s\n",ptrMijnStruct ->voorNaam);
	UARTprintf("naam: %s\n",ptrMijnStruct ->naam);
	UARTprintf("geboortedatum: %s\n",ptrMijnStruct ->geboorteDatum);
	UARTprintf("ingeschreven tot: %s\n",ptrMijnStruct ->ingenschrevenTot);
	UARTprintf("foto: %s\n",ptrMijnStruct ->foto);
	UARTprintf("timestamp: %s\n",ptrMijnStruct ->timeStamp);
}

err_t ontvangCvanS(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	char *ptr=NULL;
	int found=0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);

	tcp_recved(tpcb, 1);
	ptr =  (char *)p->payload;
	//UARTprintf(ptr);

	/* data scrapen op zoek naar nuttige info, hy gaat elke struct af*/
	found = herScrapeHTML(ptr);

/* In deze tweede struct zit de data*/
/*	p =  (char *)p->next;
	ptr =  (char *)p->payload;
	UARTprintf(ptr);
*/

	return 0;
}

/*********************************************************************************************************************************************************************************/

int herScrapeHTML(char *ptr)
{
	if (strstr(ptr,"gevonden")!=NULL)   //!!!! is CASESENSITIVE
	{
		UARTprintf("user found\n");
	}
	else {
		UARTprintf("user not found\n");
	}

}

/*********************************************************************************************************************************************************************************/

void getUser(struct tcp_pcb *tpcb)
{
	int i=0;
	int j=0;
	int k=0;
	char UARTBuffer[100]={0};
	char mijnString[100]={0};
	char user[4]={0};

	//http://127.0.0.1/searchUser.php?naam=alex
	//tcp_write(tpcb, "GET /ontvangenData.php?name=alex HTTP/1.1\r\n\Host:192.168.0.20\r\nconnection: keep-alive\r\n\r\n", 90, 1);

	char getRequestLineBase[100] = "GET /searchUser.php?naam=";
	char addFlags[80]=" HTTP/1.1\r\n\Host:192.168.0.20\r\n\r\n";

	for(i=0;i<4;i++)
	{user[i]=cardData[i];} // om een onbekende reden is er afval na de naam in de var cardData

	strcat(getRequestLineBase, user);
	strcat(getRequestLineBase,addFlags);
	//UARTprintf(getRequestLineBase);
	//UARTprintf("net voor de write");
	//tcp_write(tpcb, "GET /searchUser.php?naam=alex HTTP/1.1\r\n\Host:192.168.0.20\r\nconnection: keep-alive\r\n\r\n", 180, 1);
	tcp_write(tpcb, getRequestLineBase, 100, 1);
	//UARTprintf("User being searched.\n");

}
/*********************************************************************************************************************************************************************************/

/******************************************************************************************************************************************************/

void resetModule()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);

	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,1);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,0);
	SysCtlDelay(6000);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0,1);
}

/*********************************************************************************************************************************************************************************/


void checkIRQ()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);//irq
	GPIOPadConfigSet(GPIO_PORTB_BASE,GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

	while(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_0) == 1)
	{//UARTprintf("irq pin is hoog\n");
		if(uartData==1)
		{ //UARTprintf("UARTdata is plots 1");
			break;}
	}
}

/*********************************************************************************************************************************************************************************/

void sendCommand(char *cmd, int length)
{
	int i;
	int checksum=0x00+0x00+0xff+0xd4;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); //i2c
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_3);
	I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, (0x48>>1), false);

	I2CMasterDataPut(I2C0_MASTER_BASE, 0x00); //preamble
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	I2CMasterDataPut(I2C0_MASTER_BASE, 0x00);//preamble
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));


	I2CMasterDataPut(I2C0_MASTER_BASE, 0xff);//startcode
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	I2CMasterDataPut(I2C0_MASTER_BASE, length); //command length
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	I2CMasterDataPut(I2C0_MASTER_BASE, (~length)+1); //~length+1 fe
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	I2CMasterDataPut(I2C0_MASTER_BASE, 0xd4); //host to module
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	for (i=0; i<length-1;i++)
	{
		I2CMasterDataPut(I2C0_MASTER_BASE, cmd[i]); //0x02
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
		while(I2CMasterBusy(I2C0_MASTER_BASE));
		checksum+=cmd[i];
	}

	I2CMasterDataPut(I2C0_MASTER_BASE, ~checksum);//checksum=0x2a
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	I2CMasterDataPut(I2C0_MASTER_BASE, 0x00); //postamble
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));
}

/*********************************************************************************************************************************************************************************/

void readACKframe()
{
	unsigned long ackBuff[15]={0};
	int i=1;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, (0x48>>1), true);

	ackBuff[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	while (ackBuff[1]==0){
			ackBuff[1]=I2CMasterDataGet(I2C0_MASTER_BASE);
			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(I2C0_MASTER_BASE));
		//	UARTprintf("ik wacht\n");
	}
	for(i=2;i<8;i++){
		ackBuff[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		while(I2CMasterBusy(I2C0_MASTER_BASE));
	}

	ackBuff[8]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	/*for (i=0;i<9;i++)
	{
		UARTprintf("ack[%d]:%u    ",i,ackBuff[i]);  // wat ik krijg: 0-1-0-0-255-0-255-0-2
	}*/
	//UARTprintf("---------------------------------------------------------------\n");
	//UARTprintf("\n\n");

}

/*********************************************************************************************************************************************************************************/

void readResponse() //FIRMWARE VERSIE enzo
{
	unsigned long response[12];
	int iResponse=0;
	int i;
	int chipNumber;
	int firmwareVersion;
	int k=0;

	checkIRQ();

	response[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

		for(i=1;i<12;i++)
		{
			response[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(I2C0_MASTER_BASE));
			//UARTprintf("ik wacht\n");

			if(i==7){
			iResponse =response[7];
			iResponse<<=8;}

			if(i==8){
			iResponse |=response[8];
			iResponse<<=8;}

			if(i==9){
			iResponse |=response[9];
			iResponse<<=8;}

			if(i==10){
			iResponse |=response[10];
			iResponse<<=8;}


		}

	//UARTprintf("---------------------------------------------------------------\n");

	response[12]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	chipNumber=((iResponse>>24)&0xff);
	firmwareVersion = ((iResponse>>16)&0xff);

		UARTprintf("chipnumber: %d\n", chipNumber);
		UARTprintf("firmwareversion: %d\n", firmwareVersion);


}

/*********************************************************************************************************************************************************************************/

void readSAMResponse()
{
	unsigned long response[10]={0};
	int i,k,j;


	checkIRQ();

	response[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));


	for(i=1;i<9;i++)
	{
		response[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		while(I2CMasterBusy(I2C0_MASTER_BASE));
	}

	response[10]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	/*UARTprintf("response\n");
		for(i=0;i<10;i++)
		{
			UARTprintf("%d: %x\n", i,response[i]);

			for (j=0;j<10000;j++)
			{k+=5;}
		}
*/
}

/*********************************************************************************************************************************************************************************/

void authentificeerKaart(char * uidValue)
{
	int i,k;
	int returnVal;

	//char uidValue[4]={0};

	sendCommand(readCardCmds,4);
	checkIRQ();
	readACKframe();
	UARTprintf("waiting for card...\n");
	checkIRQ(); //zodra er een kaart aanwezig zal zijn zal hy voorby deze functie geraken
	if(uartData==1)
		{
		//UARTprintf("\nexit authentificeerkaart...");
		return;}

	returnVal = readCardType(uidValue);
	if(returnVal==1){
		//UARTprintf("4byte UID gedetecteerd\n");

		/*for(i=0;i<4;i++)
		{
			UARTprintf("byte[%d]: %x\n",i, uidValue[i]);
		}*/

		//UARTprintf("\n\n");

		return uidValue;
	}
	else
	{
		while(1){
				UARTprintf("vette error\n");
				for(i=0;i<1000;i++)
				{k+=5;}
			}
	}

}

/*********************************************************************************************************************************************************************************/

int readCardType(char * uidValue)
{
	unsigned long cardData[20]={0};
	int i,k,j=0;
	int flagBit=0;
	unsigned long uid[10]={0};

	cardData[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

	UARTprintf("card being read");
	for(i=1;i<19;i++)
	{
		cardData[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		while(I2CMasterBusy(I2C0_MASTER_BASE));


		if (cardData[12]==4) // kan ook cardData[14] zijn... niet duidelijk
		{flagBit = 1;}
		uid[0]=cardData[i];

		if(i>14 && i<= 18)
		{

			uidValue[j] =cardData[i];
			j++;

		}

	}

	cardData[19]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

/*	while(1){
	UARTprintf("data:");
	for(i=0;i<20;i++)
	{
		UARTprintf("%x ",cardData[i]);
	}
	UARTprintf("\n");
	}*/
//4-ff-d0-0


if (flagBit == 1)
{return 1;}
else {
	return 0;
}

}

/*********************************************************************************************************************************************************************************/

void authentificeerDataBlock(char *key, char blokNummer, char *cardId)
{
	int i=0;
	int j,k;
	authenticateCmds[3]=blokNummer;
	authenticateCmds[10]=cardId[0];
	authenticateCmds[11]=cardId[1];
	authenticateCmds[12]=cardId[2];
	authenticateCmds[13]=cardId[3];

/*	while(1)
	{
		for (i=0;i<14;i++)
		{
			UARTprintf("%x ",authenticateCmds[i]);
		}
		UARTprintf("\n");
		for(k=0;k<1000;k++)
		{k+=1;}
	}
*/
	sendCommand(authenticateCmds,15);
	checkIRQ();
	readACKframe();
	readResponseKaart(0);

}

/*********************************************************************************************************************************************************************************/

void readResponseKaart(int functie) //datakaar
{
	unsigned long response[20]={0};
	int i;
	int k=0;

	checkIRQ();

	response[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

		for(i=1;i<19;i++)
		{
			response[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(I2C0_MASTER_BASE));

		}

	//UARTprintf("---------------------------------------------------------------\n");

	response[19]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

//byte t = 0xd5, 8: 0x41, 9:0x00 anders klopt er iets ni

	/*	for (i=0;i<20;i++)
		{
			UARTprintf("blokdata[%d]: %u\n",i ,response[i]);
			for(k=0;k<1000;k++)
			{k+=1;}
		}*/


}

/*********************************************************************************************************************************************************************************/

void readBlock(char blokNummer, char *cardData)
{
	int functie=1;
	//readBlockCmds[3] = blokNummer; // effe achterwege gelate
	sendCommand(readBlockCmds,5);
	checkIRQ();
	readACKframe();
	//readResponseKaart(functie);
	getDataBlock(cardData);
}

/*********************************************************************************************************************************************************************************/

void getDataBlock(char *cardData) // de data wordt van de kaart gelezen en bijgehouden onder de vorm van een string. eerste char vd string zit in cardData[0]
{

	char response[28]={0};
	int i;
	int k=0;

	checkIRQ();

	response[0]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
	while(I2CMasterBusy(I2C0_MASTER_BASE));
		for(i=1;i<27;i++)
		{
			response[i]=I2CMasterDataGet(I2C0_MASTER_BASE);
			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(I2C0_MASTER_BASE));
		}
	//UARTprintf("---------------------------------------------------------------\n");

	response[27]=I2CMasterDataGet(I2C0_MASTER_BASE);
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	while(I2CMasterBusy(I2C0_MASTER_BASE));

//byte t = 0xd5, 8: 0x41, 9:0x00 anders klopt er iets ni
	cardData[0] = response[10];
	cardData[1]	= response[11];
	cardData[2] = response[12];
	cardData[3] = response[13];


}

/*********************************************************************************************************************************************************************************/

void
DisplayIPAddress(unsigned long ipaddr, unsigned long ulCol,
                 unsigned long ulRow)
{
    char pucBuf[16];
    unsigned char *pucTemp = (unsigned char *)&ipaddr;

    // Convert the IP Address into a string.
    usprintf(pucBuf, "%d.%d.%d.%d", pucTemp[0], pucTemp[1], pucTemp[2],
             pucTemp[3]);

    RIT128x96x4StringDraw(pucBuf, ulCol, ulRow, 15);
}

/*********************************************************************************************************************************************************************************/
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void SysTick_Handler(void)
{
    // Call the lwIP timer handler.
    lwIPTimer(SYSTICKMS);
}

/*********************************************************************************************************************************************************************************/

void Ethernet_IRQHandler(void)
{
	lwIPEthernetIntHandler();
}

/*********************************************************************************************************************************************************************************/
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
int pushButtonLeftActive(){
	return GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_2) ? 0 : 1;
}

/*********************************************************************************************************************************************************************************/

int pushButtonRightActive(){
	return GPIOPinRead(GPIO_PORTE_BASE,GPIO_PIN_3) ? 0 : 1;
}

/*********************************************************************************************************************************************************************************/

/*********************************************************************************************************************************************************************************/
// Required by lwIP library to support any host-related timer functions.
//*****************************************************************************
void
lwIPHostTimerHandler(void)
{
    static unsigned long ulLastIPAddress = 0;
    unsigned long ulIPAddress;

    ulIPAddress = lwIPLocalIPAddrGet();

    UARTprintf("in time handler");
    //
    // If IP Address has not yet been assigned, update the display accordingly
    //
    if(ulIPAddress == 0)
    {
        RIT128x96x4Enable(1000000);
        RIT128x96x4StringDraw("wachtend op Ip adres", 0, 24, 15);
        RIT128x96x4Disable();
        UARTprintf("wachtend op ip\n");
    }

    //
    // Check if IP address has changed, and display if it has.
    //
    else if(ulLastIPAddress != ulIPAddress)
    {
     /*   ulLastIPAddress = ulIPAddress;
        RIT128x96x4Enable(1000000);
        RIT128x96x4StringDraw("                       ", 0, 16, 15);
        RIT128x96x4StringDraw("                       ", 0, 24, 15);
        RIT128x96x4StringDraw("IP:   ", 0, 16, 15);
        RIT128x96x4StringDraw("MASK: ", 0, 24, 15);
        RIT128x96x4StringDraw("GW:   ", 0, 32, 15);
        DisplayIPAddress(ulIPAddress, 36, 16);
        ulIPAddress = lwIPLocalNetMaskGet();
        DisplayIPAddress(ulIPAddress, 36, 24);
        ulIPAddress = lwIPLocalGWAddrGet();
        DisplayIPAddress(ulIPAddress, 36, 32);
        RIT128x96x4Disable();*/
		gotIp = 1;
		UARTprintf("ip ontvangen\n");
    }
    UARTprintf("ik ben ni blijve hange\n");

}

/*********************************************************************************************************************************************************************************/
