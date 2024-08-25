
## Remote Control Commands:

Remote-controlling the pupil-tracker computer that is running PupilEXT from the display computer that is running a PsychoPy or Matlab script presenting stimuli.

The easiest is to use our sample codes (see folders under "Misc/PupilEXT Experiment Integration"), though if you would like to understand the background of their workings or you encounter difficultied, you can continue reading below for further details.

---
---

### Purpose and method of operation

PupilEXT is suggested to run separately from the experiment computer (called "display computer" from now on) on another computer that is free from any other resource-needy task during operation (called "host computer" from now on). This way, on the one hand, PupilEXT can utilize its own hardware resources unbothered by the experiment program. On the other hand, PupilEXT will not slow down the display computer that is expected to present any kind of stimuli with high temporal precision.

To this end, PupilEXT is equipped with two interfaces, which can be "listened to" for remote control commands, allowing the display computer to communicate with PupilEXT and instrument it via UDP messages or using a COM/serial port. 

Remote control commands are the same for both means of communication. Any whitespace or irregular characters will be stripped from the ends of the message before interpretation. Please avoid using special characters in names of file output paths and names. Parameters of commands (as of PupilEXT v0.1.2) do not need to be put between quotation marks or parentheses.

Even extraordinary cases can be handled e.g. when PupilEXT is instructed by a UDP command to start listening to a COM port for other commands, or stop listening to them.

---

### Sending commands using UDP

User Datagram Protocol (abbreviated UDP) is a transport layer network communication protocol. It is widespread among measurement devices i.e. eye-tracker setups for sending textual commands from the display computer to control the host computer e.g. to start or stop recording or to "send trigger signals" to increment trial numbering. It is realized through network adapters of the communicating computers, thus the commands can travel through an ethernet cable or even WiFi, from computer to computer directly, within a local network (LAN) or even over the internet. In experimental setups however, typically the former are used. 

A port needs to be opened on the receiver side which is "listened to" for our textual commands to control PupilEXT. Sender computer needs to specify the target IP address and port to send to (e.g. "192.168.40.1" and "6900" respectively). On the host computer, PupilEXT listens to any message received on the specified port, but importantly, these messages containing the commands are further filtered using an IP address that you specify. On the host computer running PupilEXT, there are several options for this filter:
- 1, It can be limited to one certain IP address. To achieve this, the IP address in PupilEXT should be set to equal the address that was previously set on the other computer (the display computer). E.g. "192.168.40.2"
- 2, If the IP address for listening is set to "0.0.0.0", any message will be received that was sent from any computer connected to the host (also including the host computer itself). This is the best option if nothing else is working, or you are debugging.
- 3, If the IP is "127.0.0.1" (which IP is conventionally called "localhost" elsewhere) then only those commands will be received that were sent from the host computer itself. This option is rather for special use cases or debugging, when the host and display computer are deliberately the same.

Commands sent using UDP can only be received if there is PupilEXT listening on the receiver side. To achieve this, you probably need to disable firewall, or add PupilEXT as an exception that can communicate through UDP protocol anytime, and/or run PupilEXT with administrator privileges. 

If you directly connect two computers with an "ethernet cable/UTP cable/LAN cable", you should make sure that:
- the network adapters (that correspond to the RJ45 connectors involved on each computer) are set up to use specific, manually set IP addresses, and not to try to figure them out automatically (as typically the latter is the case when you are just connecting to the internet via a router), and that 
- the display computer and host computer are given different IP addresses. 

After you ensured that connection is possible between the computers, you should be able to start listening for the commands in PupilEXT at a specific UDP port number and get them successfully when sent from the display computer to the said port number. 
- If you find that you cannot start listening for remote commands in PupilEXT, the problem should be with a firewall or other limitation of communication rights for PupilEXT. 
- If you can start listening, but the commands do not arrive to PupilEXT, it is possible that 
    - you specified a different port to listen to than the one you use for sending the commands, or 
    - the network adapters were not correctly set up on both computers, or that 
    - the display computer does not succeed in sending the message for various reasons (firewall, no admin rights of PupilEXT, etc).

To test sending and receiving UDP messages, you can try a free program, e.g. PacketSender (https://packetsender.com/).

Communication using UDP usually provides ample temporal precision, meaning that the sent command should arrive within 4-5 milliseconds.

Listening to localhost is also possible, for prototyping or other use cases involving the host computer as a display computer at the same time.

---

### Sending commands using COM/serial

(Serial) Communication Ports (COM ports) represent a lower level of communication, which can also be used to transmit textual commands between computers. This way, two computers can be connected using a serial "RS-232" cable. A port is seen on the computers that does not necessarily employ the same numbering (e.g. you see a COM port called "COM1" on one computer, and "COM4" on another). In order to connect one computer to another via serial, the physical connection must be ensured first, as there is a handshake between computers when the connection is established. Note that the COM ports are not only detectable when the cable is plugged in, but always when a COM port exists on the computer, so it can happen that you still see the ports, though the connection cannot be made due to a broken cable. 

COM ports provide excellent temporal precision, as the messages sent can surely arrive within one millisecond.

In case your computer does not have a COM port by default, you can use any "USB to COM port adapter" which will create an emulated COM port on your computer. These adapters will work with PupilEXT, as they are also seen by the program as a regular built-in COM ports. However, USB-emulated COM ports might exhibit worse temporal fidelity than their built-in version, as their speed also depends on the USB adapter of the computer, which acts as a bottleneck.

---

### List of Remote Control Commands:

Important: The "smaller than" and "less than" signs (i.e. "<>") below are for designating the borders of placeholders, and should not be included in the actual remote control commands sent to PupilEXT.

`T` - Increment trial numbering.

`M <message>` - Send textual message, to be saved along with the image recording as timestamped annotation, just like trial numbering increment triggers.

`A1 <camera>` - Open a single camera. Friendly name of the device should be provided. A friendly name means the name that is displayed to you in Pylon Viewer of your Basler camera, e.g. `Basler a2A1920-160umBAS (12345678)` where the number in parentheses is the serial number of your camera device.

`A2 <camera>;<camera>` - Open a stereo camera. Friendly names of the devices should be provided, separated by either `,` or `|` or `;` characters.

`AW <deviceID>` - Open a single OpenCV camera device ("webcam"). Device ID should be provided, which is an integer, enumerating devices starting from `0`.

`AT` - Start pupil tracking. A camera needs to be opened beforehand.

`AX` - Stop pupil tracking.

`AR` - Start data ("csv") recording. Data recording path and name should be set beforehand.


`AS` - Stop data ("csv") recording. 

`AV` - Start data streaming. Streaming target should be available and its port is opened for listening.

`AC` - Stop data streaming.

`AM` - Start image recording. Image recording path and name should be set beforehand.

`AA` - Stop image recording.

`AD` - Disconnect from camera.

`AF` - Force reset the trial counter. (Recommended for development purposes only.)

`GP <path and name>` - Set image recording output path. Make sure you do not use special characters as they may be restricted for use by the operating system in folder names. Cannot be altered while an image recording is going on.

`GL <path and name>` - Set data ("csv") recording path and name. Make sure you do not use special characters as they may be restricted for use by the operating system in folder/file names. Cannot be altered while a data ("csv") recording is going on.

`GC <delimiter>` - Set global delimiter character. Accepted characters are a comma, a semicolon or tab character. Cannot be altered while a data ("csv") recording is going on.

`GI <format>` - Set image recording output format. Format name should be provided: `tiff` or `png` or `bmp` or `jpeg` or `webp` or `pgm`. Cannot be altered while an image recording is going on.

`PA <algorithm>` - Set pupil detection algorithm. Accepted algorithms: `else` or `excuse` or `pure` or `purest` or `starburst` or `swirski2d`.

`PR <state>` - Use ROI Area Selection. Either `true` or `false`.

`PO <state>` - Compute Additional Outline Confidence. Either `true` or `false`.

`CRC udp;<IP>;<port>` - Establish connection (start listening) for remote control using a UDP port.

`CRC com;<port>;<baud>` - Establish connection (start listening) for remote control using a COM/serial port.

`CRD <interface>` - Disconnect (stop listening) for remote control. Interface type should be provided: `udp` or `com`.

`CSC udp;<IP>;<port>` - Establish connection for pupil data streaming target using a UDP port.

`CSC com;<port>;<baud>` - Establish connection for pupil data streaming target using a COM/serial port.

`CSD <interface>` - Disconnect streaming target. Interface type should be provided: `udp` or `com`.

`CMC udp;<IP>;<port>` - Establish connection to the microcontroller unit using a UDP port.

`CMC com;<port>;<baud>` - Establish connection to the microcontroller unit using a COM/serial port.

`CMD` - Disconnect microcontroller unit.

`IT <mode>` - Set Image Acquisition triggering mode. Valid inputs are: `H` for hardware-based and: `S` for software-based image acquisition triggering

`IHQ` - Start hardware triggering

`IHY` - Stop hardware triggering

`IHL <value>` - Set hardware triggering line source, valid values are: `1`, `2`, `3`, `4`

`IHR <value>` - Set hardware triggering runtime length in minutes, any positive floating point number is accepted. Value `0` can be used to set infinite triggering

`IHT <value>` - Set hardware triggering framerate, any >=1 positive integer number is accepted

`ISC <value>` - Enable framerate limiting for software triggering. Either `true` or `false`

`IST <value>` - Set software triggering framerate limit, any >=1 positive integer number is accepted

`IE <value>` - Set exposure in microseconds, any positive floating point number is accepted

`IG <value>` - Set gain, any floating point number is accepted, minimum `0.0`
