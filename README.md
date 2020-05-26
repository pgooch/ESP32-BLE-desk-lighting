# ESP32-BLE-desk-lighting
This repository contains the Arduino IDE project, the electron/HTML controller, and the printable half case STL file for my ESP32 based bluetooth controlled desk lighting. 

## Why?
The previous owner of mt house decided to install halogen based lighting in the bedroom that became my office. This is nice when you drop something and want to light the entire room up, but normally it's to much light and more importantly durning the summer too much heat. 

In additional to that when I build my new keyboard I decided to forego the backlight mostly because I didn't like the way it looked on the keycaps I'm using, general laziness is also a factor, I've taken apart and built the same keyboard a few times at this point and I wasn't excited about the prospect over another 280 solder joints.

## Materials
I used the following items for this setup.
* Approximately 1 meter of WS2812 compatible LEDs for the back of monitor lighting.
    * I suggest side shooting LEDs, I used these: https://www.aliexpress.com/item/32922687661.html
* About another meter of generic WS2812 strip for the under desk
    * I had some kicking around, inevitably from AlieExpress
* A power supply capable of power the LEDs your using, I used a  5v 15amp one with a barrel jack.
* An ESP32 based development board with USB serial driver.
* Some 3 core wire, as well as various other bits of wire for power.
* A soldering double 1/2 breadboard, I used one from ElectroCookie
* Wire and connectors as needed, I used JSX connectors but you could solder it all together directly.
* A 3d printer to print a little case, totally optional but it makes mounting it easier.

## Wiring
Wiring is simple. I took a Female Barrel plus and connected it to the +/- rails, then I connected my LED strips to pins 12, 13, and 25 for the left monitor, right monitor and under desk lights respectively.

You'll also need to make some wires to run between the strips and the control unit, I used 3 core wire for MaxBrite and make adapters from the PCB plus to the LED strip plugs.

## The case
The case was made in fusion360 and can be printed in ant material, I used a nifty teal PLA.

## ToDo
* The color picker seems to not update the other points when you set the value via JS, thats probably more picker than implementation from what I've read but I didn't get to deep into it.
* Sometimes things will stop working, I think this is more an effect of constant fiddling while developing and not a real problem in normal use, but as I use it id it keeps it up I'll dive back into the ESP32 code and see what might be up.
* Building the electron app fails to work. It appears to build properly but the final app does not function (no index loaded, incorrect icon, who knows what else). No error occur during build and I might look into this but I think that I'll be updating it infrequently enough that I can just access it via the browser (I set up a vhost to the src directory)
* A better readme and some pictures might be nice, but as I'm writing this I'm still waiting on components for final assembly. Also a video of the build process, but that feels more like a V2 thing.

## Future features
* I'm been thinking of getting a motorized sit/stand desk conversion, controlling height with this system would be pretty cool and should be doable from what I've read on the subject.
* A passive IR sensor could potentially turn the LEDs on/off when you sit down or leave from your desk. I don't currently have a suitable one but an order has been placed.
* Space was left on the PCB for a third monitor, however I can't run 3 4k60 displays on my machine without an external GPU and I don't really feel like taking on that cost for another screen I don't really need.