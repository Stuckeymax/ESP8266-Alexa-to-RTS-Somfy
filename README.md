# ESP8266-Alexa-to-Somfy
Modified Philips Hue Alexa device code, adds 3rd control function via logic for full voice control of handheld Somfy 3-button motorized drapery controllers.

Somfy Add-On Version
This sketch is modified from the irdevkit.ino code published by kakopappa. Kakopappa did the heavy lifting here, not me; kudos to kakopappa. It takes pseudo-Philips hard, ON/OFF control and adapts it to create 1/2-second button pushes on any of three buttons on an RTS Somfy 3-button handheld motorized window controller. Connection instructions are included in the sketch comments. Change line 21 if you want a different device name (default is “drapes;” set in your SSID and password and you are good to go. Soldering required, or a good bed-of-nails test jig for your Somfy remote. Hardware details: I am running a NodeMCU v1.0 board. Requires ESP8266WiFi.h, ESP8266Webserver.h, WiFiUdp.h and <functional>, whatever that last is. Just reading the include-file listing here. 
A Boolean variable myState is added to accommodate the 3rd button along with some if/else/OR logic in the turnOffRelay() and turnOnRelay() functions. Once initialized by a “turn on drapes” command followed by a “turn off drapes” command, the custom-opening-distance “MY” button is “voice-pressed” by ordering draperies OFF when they’re already off (closed; think light) or ordering draperies ON when they’re already on (open).
You have to turn the draperies ON or OFF, because OPEN or CLOSED is not supported for this faux-Philips device, which is a fake Philips lighting controller listening on port 1900, I think. Alexa will tell you as much. This code includes device-state feedback to Alexa so that when you open the iPhone app, the correct state is always displayed if open or closed (on or off, respectively). No state recognition for being in the MY partially open state, of course.
Here’s how it achieves 3 button pushes from a 2-state Alexa device:
1)	Drapes are closed. Say, “Alexa, turn ON the drapes.”
Drapes go to the full-open position
2)	Drapes are open. Say, “Alexa, turn OFF the drapes.”
Drapes go to the closed position
3)	Drapes are closed. Say, “Alexa, turn OFF the drapes.”
Drapes go to the pre-programmed MY position.
4)	Drapes are open. Say, “Alexa, turn ON the drapes.”
Drapes go to the pre-programmed MY position, as in 3.
In other words, tell it to go where it isn’t, it goes there. Tell it to go where it already is, it goes to the intermediate position. Yes, this is a little clunky, but a $40 Somfy remote is a lot cheaper than a full-blown Somfy MyLink Ethernet bridge unit ($250 on a good day). Plus hacked beats ready-made any day:)
Using this to control Somfy motorized window treatments requires a Somfy 3-button handheld controller dedicated to the project. Planned improvements include:
1)	ASK pulse coding tricks so that the code directly controls an off-the-shelf 433.42-MHz ASK/OOK low-power RF transmitter, as opposed to wiring into an off-the-shelf Somfy handheld. Not sure how to do that yet, but maybe there some published tricks for this out there, haven’t looked much.
2)	Add an initialization step, so that the controller automatically performs an open/close step so that states in the ESP8266 and in Alexa and its app are in agreement upon boot up, no user-initialization required.
3)	Add watchdog code or an external MCU (SOT-23 PIC10F200 or some such) to perform this function so that the thing never crashes; as is, it will crash about once a week, haven’t tried to debug it.
Philips Note: If you have a Philips lighting Internet bridge unit on the same network as this device, power off the Philips Hub/Bridge unit while you are performing device discovery. Otherwise, Alexa will never recognize your ESP8266 device. Power up your ESP8266, and once it’s recognized, you can power up the Philips Hub, and both it and your new, added ESP8266 device will be recognized and working fine forever.
Motor Note: Somfy motorized products have a self-preservation feature to ensure that motors live a long, uncooked life. When you first get this new toy working and you kick back, ordering Alexa to turn on the drapes, turn off the drapes, turn off the drapes, turn on the drapes… eventually your drapes or roller shades or what have you will stop responding. No worry. Give the motor a few minutes to cool down, and everything will start working again like it never took a breather. This open/closed/open/closed business never happens in real life, so don’t fix it; it’s not broken.
There are probably other uses for this two-to-three conversion. Please report any adaptations you find useful.
Enjoy!

Stuckeymax

