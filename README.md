# ESP8266-Alexa-to-Somfy
Modified Philips Hue Alexa device code, adds 3rd control function via logic for full voice control of handheld Somfy 3-button motorized drapery controllers.

Somfy Add-On Version

*****UPDATE*****
After I changed my device name from "draperies" to "drapes," I can now tell Alexa to "Open the drapes" or "Close the drapes," and they respond, even though Alexa thinks this is an on-off device. I discovered that last night, was surprised. I was also surprised when I said, "Alexa, turn on the bedroom," where the draperies are located in addition to three Philips Hue bulbs linked to a Philips 1st-gen hub. She turned on the lights all right, but also opened the draperies. 

This sketch is modified from the irdevkit.ino code published by kakopappa. Kakopappa did the heavy lifting here, not me; kudos to kakopappa. It takes pseudo-Philips hard, ON/OFF control and adapts it to create 1/2-second button pushes on any of three buttons on an RTS Somfy 3-button handheld motorized window controller. Connection instructions are included in the sketch comments. Change line 21 if you want a different device name (default is “drapes"), but please beware that Alexa may not respond to OPEN or CLOSE, only to turn on or turn off if you change this name. Lastly, set in your SSID and password and you are good to go. Soldering required, or a good bed-of-nails test jig for your Somfy remote. Hardware details: I am running a NodeMCU v1.0 board. Requires ESP8266WiFi.h, ESP8266Webserver.h, WiFiUdp.h and <functional>, whatever that last is. Just reading the include-file listing here. 
A Boolean variable myState is added representing the drapes state when in the custom-set Somfy MY (intermediate) position and to accommodate the 3rd button along with some if/else/OR logic in the turnOffRelay() and turnOnRelay() functions. Once initialized by a “Alexa, open the drapes” command followed by “Alexa, close the drapes” command, the custom-opening-distance “MY” button is “voice-pressed” by ordering draperies to close when they’re already closed or ordering draperies to open when they’re already open.
 There is no no state recognition within Alexa for being in the MY partially open state, of course.
  
Here’s how it "cheats in" or achieves 3 button pushes from a 2-state Alexa device:
1)	Drapes are closed. Say, “Alexa, Open the drapes.”
Drapes go to the full-open position
2)	Drapes are open. Say, “Alexa, Close the drapes.”
Drapes go to the closed position
3)	Drapes are closed. Say, “Alexa, Close the drapes.”
Drapes go to the pre-programmed MY position.
4)	Drapes are open. Say, “Alexa, OPen the drapes.”
Drapes go to the pre-programmed MY position, as in 3.
In other words, tell it to go where it isn’t, it goes there. Tell it to go where it already is, it goes to the intermediate position. A little crude? Yes, but a $40 Somfy remote (or $20 aftermarket 3rd-party handheld) is a lot cheaper than a full-blown Somfy MyLink Ethernet bridge unit ($250 on a good day). Plus, hacked beats ready-made any day:)
Using this to control Somfy motorized window treatments requires a Somfy 3-button handheld controller dedicated to the project. Planned improvements include:
1)	ASK/OOK pulse coding tricks so that the ESP8266 code directly controls an off-the-shelf 433.42-MHz ASK/OOK low-power RF transmitter, as opposed to wiring into an off-the-shelf Somfy handheld. Not sure how to do that yet, but maybe there are some published tricks for this out there, haven’t looked hard yet.
2)	Add an initialization step, so that the controller automatically performs an open/close step when powered up or rebooted so that states in the ESP8266 and in Alexa and its app are in agreement upon boot up, no user-initialization open-close step required.
3)	Add watchdog code or an external MCU (SOT-23 PIC10F200 or some such) to perform this function so that the ESP8266 never crashes; as is, it will crash about once a week, haven’t tried to debug that.
Philips Note: If you have a Philips Hue Internet bridge unit on the same network as this ESP8266 device, power off the Philips Hub/Bridge unit while you are performing device discovery. Otherwise, Alexa will never recognize your ESP8266 device. Once the ESP8266 is recognized, you can power up the Philips Hub, and both it and your new, added ESP8266 device will be recognized and working fine forever.
Motor Note: Somfy motorized products have a self-preservation feature to ensure that motors live a long, uncooked life. When you first get this new toy working and you ask Alexa to open the drapes, close the drapes, close the drapes, open the drapes… eventually your drapes or roller shades will stop responding - for a little while, perhaps a minute. No worry. Give the motor a little time to cool down, and you'll be back to the races.. Such rapidfire open/close/open/close commands never happen in real life, so don’t fix it. because it’s not broken.
There are probably other uses for this two-states to three-button-presses conversion. Please report any adaptations you find useful.

Any feedback appreciated. Please let me know how this works for you.

Stuckey

