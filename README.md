# Growatt SPF5000ES with Home Assistant

Project to monitor and configure Growatt SPF5000ES with Home Assistant. Based on  Csongor Varga's awesome project https://github.com/nygma2004/growatt2mqtt

Modbus register in my inverted differs from Csongor but I found Growatt OffGrid SPF5000 Modbus RS485 RTU Protocol doc that helped me.

https://docplayer.net/199800433-Growatt-offgrid-spf5000-modbus-rs485-rtu-protocol.html

According to SPF5000ES User Guide RS485 pins to be used are 1 and 2 but I find out that pins are 8 (RS485B) nad 7 (RS485A) are the one used.  Csongor made a great work with documentation and code comments, take a look at it if you have doubts.

In addition you have to add an MQTT sensor to your Home Assistant config

```sh
sensor:
  - platform: mqtt
    name: "display_name"
    state_topic: "topicRoot_used/estado"
    json_attributes_topic: "topicRoot_used/data"
    json_attributes_template: "{{ value_json | tojson }}"
    device_class: energy
```

If you want to change any of the inverter settings defined in the callback funtion, your need to call the mqtt publish service from home assistant with corresponding topic and proper message. Take a look at the callback funtion to see which topics were included for changing the inverter settings.
