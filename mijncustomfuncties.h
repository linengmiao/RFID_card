#ifndef __MIJNCUSTOMFUNCTIES_H__
#define __MIJNCUSTOMFUNCTIES_H__

#include "lwip/tcp.h"
/* Defines*/
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0


//RFID
/*commandos*/
extern const char firmwareCommand[1];//={0x02};
extern const char  SAMConfigure[4];//={0x14,0x01,0x14,0x01};
extern const char  readCardCmds[3];//={0x4A, 0x01, 0x00};
extern const char  readBlockCmds[4];//={0x40, 0x01, 0x30, 0x04}; // om echt een eeprom blok me data op vd kaart te lezen;

extern const char  publicKey[6];//={0xff,0xff,0xff,0xff,0xff,0xff}; //voor authenticatie
extern char  authenticateCmds[14];//= {0x40, 0x01, 0x60, 0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xe3,0xff,0x20,0xd0}; //eerste 0x00 is welk blok dat je wilt checken

/*LWIP*/
struct usersData
{
	char naam[20];
	char voorNaam[20];
	char geboorteDatum[20];
	char ingenschrevenTot[20];
	char foto[20];
	char timeStamp[20];

};

extern int uartData;//=0;
//char cardData[4]={'a','l','e','x'};
extern char cardData[4];//={0};
extern char newUserData[100];//={0};
extern volatile int gotIp;
/*****************************************************************************************************************/
err_t connectedToserver(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t zendCnaarS(void *, struct tcp_pcb *,u16_t );
err_t ontvangCvanS(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
void sendDataToServer(struct tcp_pcb *tpcb);
void scrapeRxBuffer(struct usersData *mijnStructPtr);

void getUser(struct tcp_pcb *tpcb);
int herScrapeHTML(char *ontvangenData);

//RFID module
//*setup*/
void resetModule(); //pwm4 = reset
void sendCommand(char *cmd, int length);
void checkIRQ();
void readACKframe();
void readResponse();//FIRMWARE VERSIE enzo
void readSAMResponse();

/*data transfer*/
void readCard();
int readCardType(char *cmd);
void authentificeerKaart();
void authentificeerDataBlock(char *key, char blokNummer, char *cardId);
void readResponseKaart(int functie);
void readBlock(char blokNummer, char *cardData);
void getDataBlock();// de data wordt van de kaart gelezen en bijgehouden onder de vorm van een string. eerste char vd string zit in test

/*qt->sql uart*/
void UARTStdioIntHandler();


/* External Application references.*/
extern void fs_init(void);
	
#endif
