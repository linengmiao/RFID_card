The goal of this project is to read and write RFID cards. The cortex M4-microcontroller (LM3S6965) 
communicates with the RFID reader/writer via I2C. 

1) users are added to the local SQL-database via the Qt GUI
2) The gui sends the users data via UART to the mcu
3) mcu writes data via i2c to the rfid r/w.
4) RFID r/w writes this data on the card

next time the card is placed on the RFID-reader:

5) the mcu reads the data on the card(via i2c) and checks the sql-database(SQL Query + LWIP) whether the user is existant or not

- The data from the GUI application to the mcu is via uart.
- the communication from the mcu to the SQL-database is via LWIP (TCP/IP)

