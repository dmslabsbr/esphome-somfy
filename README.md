# esphome-somfy
Esphome configuration for Somfy Blind

This project still has bugs and is not optimized for use. So, you may have a little trouble using it.

__Materials:__
* 1 - [Wemos D1](https://s.click.aliexpress.com/e/_d8jADk8);
* 1 - 220 ohms resistor;
* 1 - Led, any color;
* 1 - [FS1000A 433,42 Mhz](https://s.click.aliexpress.com/e/_dZrWjOC) (*);
* 1 - [Breadboard;](http://rover.ebay.com/rover/1/711-53200-19255-0/1?ff3=4&pub=5575522659&toolid=10001&campid=5338569169&customid=somfy&mpre=https%3A%2F%2Fwww.ebay.com%2Fitm%2FMini-Solderless-Breadboard-White-Material-400-Points-Available-DIY%2F223770689013%3Fhash%3Ditem3419c5bdf5%3Ag%3AUesAAOSwSkpd72u8)

(*) You need to buy a FS1000A 433,92 Mhz and change the crystal for a [433,42 Mhz](http://rover.ebay.com/rover/1/711-53200-19255-0/1?ff3=4&pub=5575522659&toolid=10001&campid=5338569169&customid=somfy&mpre=https%3A%2F%2Fwww.ebay.com%2Fitm%2F5PCS-433-42M-433-42MHz-R433-F433-SAW-Resonator-Crystals-TO-39-NEW%2F232574365405%3FssPageName%3DSTRK%253AMEBIDX%253AIT%26_trksid%3Dp2057872.m2749.l2649).

__Software:__
* Running Home Assistant [https://www.home-assistant.io/]
* ESPhome component [https://esphome.io/]

![scheme](/esquema.png)


**1st Prototype**
![1st Prototype](/20200402_111304.jpg)


__Blind control interface__

![Blind control interface](/Blind%20control.png)

__Install__:

1. Copy the files from this repository to the `/config/esphome/` directory.
* RFsomfy.h
* SomfyRts.cpp
* SomfyRts.h
* platformio.ini

2. Create a new ESPHome device and use `esp_somfy.yaml` from this repository.

3. Customize `RFsomfy.h` file, as you need.

````
#define STATUS_LED_PIN D1
#define REMOTE_TX_PIN D0
#define REMOTE_FIRST_ADDR 0x121311   // <- Change remote name and remote code here!
#define REMOTE_COUNT 3   // <- Number of somfy blinds.
````

4. Customize your ESPHome device, changing `esp_somfy.yaml` as you need.

`````
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_pass
  use_address: 192.168.50.12
  manual_ip:
    static_ip: 192.168.50.12
    gateway: 192.168.50.1
    subnet: 255.255.255.0
`````
Change here according to the amount of blinds you have.
```
cover:
- platform: custom
  lambda: |-
    std::vector<Cover *> covers;
    auto rfSomfy0 = new RFsomfy(0);
    rfSomfy0->set_code(1);
    App.register_component(rfSomfy0);
    auto rfSomfy1 = new RFsomfy(1);
    rfSomfy1->set_code(1);  // Set initial rolling code. After it works, remove this line.
    App.register_component(rfSomfy1);
    auto rfSomfy2 = new RFsomfy(2);
    App.register_component(rfSomfy2);
    covers.push_back(rfSomfy0);
    covers.push_back(rfSomfy1);
    covers.push_back(rfSomfy2);
    return {covers};

  covers:
    - name: "Blind 2"
      device_class: shade
      id: somfy0
    - name: "Veneziana casal"
      device_class: shutter
      id: somfy1
    - name: "Veneziana roxo"
      device_class: shutter
      id: somfy2
```

**ATTENTION**

You do not need to use this line `rfSomfy0->set_code(1);` , only if you need to manually set the first code.
