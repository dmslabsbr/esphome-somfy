# esphome-somfy
Esphome configuration for Somfy Blind



<a href="https://www.buymeacoffee.com/dmslabs"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a pizza&emoji=🍕&slug=dmslabs&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff"></a>

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=9S3JYKPHR3XQ6)
[![Donate with Bitcoin](https://en.cryptobadges.io/badge/micro/1MAC9RBnPYT9ua1zsgvhwfRoASTBKr4QL8)](https://www.blockchain.com/btc/address/1MAC9RBnPYT9ua1zsgvhwfRoASTBKr4QL8)

<img alt="Lines of code" src="https://img.shields.io/tokei/lines/github/dmslabsbr/esphome-somfy">
<img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/dmslabsbr/esphome-somfy">




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

![scheme](/img/esquema.png)

  I found on the internet some instructions on how to make an antenna for 433.42Mhz transmission.

  It was better than just a thread. In fact, I think that some problems in programming the controls have improved.

![Antenna](/img/Antenna.png)


**1st Prototype**
![1st Prototype](/img/20200402_111304.jpg)



## Install:

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

5. Compile and Upload customized ESPhome to your device.

## How to configure

1. Insert your new entities in your home assistant's dashboard.

      They should have names similar to these:
      * cover.veneziana_roxo
      * cover.blind_roxo
      * cover.blind_2

![Blind Card](/img/Blind%20card.png)

## How to program

1. For programming, place your ESPHome device as close as possible to the blind to be controlled.

   Once programmed, you can place it further away, in a position that is close to all the blinds to be controlled.

2. Choose one of the entities and open it in full mode.


    __Blind control interface__

![Blind control interface](/img/Blind%20control.png)

3. Put your blind in programming mode. If necessary, consult the blind manual or the manufacturer.

4. Slide the bar that controls the tilt position to the value 11.

   a) This causes your ESP device to enter programming mode. As if it were an additional remote control.
  
   b) If the programming without problems, your blind will move immediately.

   c) In case of problems, check your device's log.

## Blind configuration commands

Some commands were created, accessed by tilting the blind to try to facilitate debugging and configuration.

```
// cmd 11 - program mode
// cmd 16 - program mode for grail devices
// cmd 21 - delete rolling code file
// cmd 41 - List files
// cmd 51 - Test filesystem.
// cmd 61 - Format filesystem and test.
// cmd 71 - Show actual rolling code
// cmd 81 - Get all rolling code
// cmd 85 - Write new rolling codes
```
