# ESP32-BLE-desk-lighting
This repository contains the Arduino IDE project, the electron/HTML controller, and the printable half case STL file for my ESP32 based bluetooth controlled desk lighting. 

## Why?
The previous owner of my house decided to install halogen based lighting in the bedroom that became my office. This is nice when you drop something and want to light the entire room up, but normally it's to much light and more importantly durning the summer too much heat. 

In additional to that when I build my new keyboard I decided to forego the backlight mostly because I didn't like the way it looked on the keycaps I'm using, general laziness is also a factor, I've taken apart and built the same keyboard a few times at this point and I wasn't excited about the prospect over another 280 solder joints.

## Materials
I used the following items for this setup.
* Approximately 1 meter of WS2812 compatible LEDs for the back of monitor lighting.
    * I suggest side shooting LEDs, I used these: https://www.aliexpress.com/item/32922687661.html
* About another meter of generic WS2812 strip for the under desk
    * I had some kicking around, inevitably also from AliExpress
* A few 470ohm resistors installed on the data line per the Adafruit NeoPixels Uberguide.
* A power supply capable of power the LEDs your using and a way to get that power to the board, I used a 5v 15amp one that came with a female barrel jack.
* An ESP32 based development board with USB serial driver (or a way to program a bare one).
* Some 3 core wire, as well as various other bits of wire for power.
* A solderable double 1/2 breadboard, I used one from ElectroCookie via Amazon
* Wire and connectors as needed, I used JSX connectors but you could solder it all together directly.
* A 3d printer to print a little case, totally optional but it makes mounting it easier.

## Wiring
Wiring is simple. I took a Female Barrel plus and connected it to the +/- rails, then I connected my LED strips to pins 12, 13, and 25 for the left monitor, right monitor and under desk lights respectively.

You'll also need to make some wires to run between the strips and the control unit, I used 3 core wire for MaxBrite and make adapters from the PCB plus to the LED strip plugs.

## The case
The case was made in fusion360 and can be printed in any material, I used a nifty teal PLA. It's a pretty basic print and setting can be more or less whatever prints best for you. There are screw holes in it, however you can fill them with glue and screw into that if they don't work after printing (their tiny 2mm screws so it's hard to get them to print threads cleanly).

## Building the control app
After running `npm install` you should be able to build the app for your current platform by navigating to the app directory and running `electron-packager .`. This is the primary way I use the app, to occasionally change color or brightness. You can also point a vhost to the src directory and it should work as a hosted webpage, however the page must be hosted, not at `file://` and it will need to be a secure domain or the browser will need to be told to ignore that. Overall it was kinda a fiddly process so I just use the app (because macOS update reset vhosts every update now).

## ToDo
* The color picker seems to not update the other points when you set the value via JS, thats probably more picker than implementation from what I've read but I didn't get to deep into it.
* I'm not entirely sure how it handles various power cycling possibilities, however it seems to be able to recover well enough for my use case.
* The electron app does not build with an icon, I'm not sure why it all appears to be properly configured, from reading it seems to be a not to uncommon problem.
* A better readme and some pictures might be nice, but as I'm writing this I'm still waiting on components for final assembly. Also a video of the build process, but that feels more like a V2 thing.

## Future features
* I'm been thinking of getting a motorized sit/stand desk conversion, controlling height with this system would be pretty cool and should be doable from what I've read on the subject.
* A passive IR sensor could potentially turn the LEDs on/off when you sit down or leave from your desk. I don't currently have a suitable one but an order has been placed.
* Space was left on the PCB for a third monitor, however I can't run 3 4k60 displays on my machine without an external GPU and I don't really feel like taking on that cost for another screen I don't really need.