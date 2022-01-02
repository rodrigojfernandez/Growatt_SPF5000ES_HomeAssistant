# Growatt_SPF5000ES_HomeAssistant

Project to monitor and configure Growatt SPF5000ES with Home Assistant. Based on  Csongor Varga's awesome project https://github.com/nygma2004/growatt2mqtt

Modbus register in my inverted differs from Csongor but I found Growatt OffGrid SPF5000 Modbus RS485 RTU Protocol doc that saved me.

https://docplayer.net/199800433-Growatt-offgrid-spf5000-modbus-rs485-rtu-protocol.html

According to SPF5000ES User Guide RS485 pins to be used are 1 and 2 but I find out that pins are 8 (RS485B) nad 7 (RS485A).  Csongor made a great work with documentation and code comments, take a look at it if you have doubts.
