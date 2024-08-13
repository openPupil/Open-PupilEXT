## Executable Arguments

Providing arguments for PupilEXT on startup to let it automatically set runtime variables or perform actions that would be repeatedly done using the graphical interface.

---
---

### Purpose

Even though PupilEXT is an application with a Graphical User Interface (GUI), arguments can be supplied to it, that will be processed upon startup. Some of these arguments perform actions like connecting to a camera, others set runtime variables like the image recording output path.

Using startup arguments can ease everyday use of PupilEXT by automatically executing certain tasks that would otherwise need the same repeated interactions with the GUI by the lab assistant whenever the program needs to be opened. Also, arguments make it convenient to auto-start PupilEXT with supplied arguments upon boot-up of the operating system. This way, powering up the host computer can be configured to directly open and start tracking on the camera.

### Important notes on usage

First all provided arguments are sorted to fit in a predefined order, which then can be properly processed argumentwise. The order is the following:
```
-openSingleCamera
-openStereoCamera
-openSingleWebcam
-setPDAlgorithm
-setPDUsingROI
-setPDComputeOutlineConf
-startTracking
-setDelimiter
-setImageOutputPath
-setImageOutputFormat
-startImageRecording
-setDataOutputPath
-startDataRecording
-connectStreamUDP
-connectStreamCOM
-startStreaming
-connectRemoteUDP
-connectRemoteCOM
```

If an argument instructs the program to do the same again, or perform an impossible action (e.g. to connect to a webcam after it has already connected to a single Basler camera), the conflicting argument(s) will be disregarded, and hopped over.

Arguments are expected to be passed using a single dash character before the name of the argument. Arguments often need additional textual data, which is expected to follow the name of the argument, separated from it with a space character, and wrapped around double quotation marks. An example command line call to start PupilEXT on Windows would be:

```
PupilEXT.exe -openSingleCamera "Basler a2A1920-160umBAS (12345678)" -startTracking -setImageOutputFormat "tiff"
```

If there are no quotation marks around a needed value for an argument, e.g. the camera name text, then PupilEXT will still try to interpret the given text as the needed value, e.g. camera name, but wherever a whitespace character comes, argument parser will think that the next characters belong to the next argument already, and e.g. a camera simply called "Basler" will be erroneously searched for.


---

### List of Executable Arguments


`-openSingleCamera "<camera>"` - Open a single camera. Friendly name of the device should be provided. A friendly name means the name that is displayed to you in Pylon Viewer of your Basler camera, e.g. "Basler a2A1920-160umBAS (12345678)" where the number in parentheses is the serial number of your camera device.

`-openStereoCamera "<camera>" "<camera>"` - Open a stereo camera. Friendly names of the devices should be provided.

`-openSingleWebcam "<camera>"` - Open a single OpenCV camera device ("webcam"). Device ID should be provided, which is an integer, enumerating devices starting from 0.

`-setPDAlgorithm "<algorithm>"` - Set pupil detection algorithm. Accepted algorithms: `else` or `excuse` or `pure` or `purest` or `starburst` or `swirski2d`.

`-setPDUsingROI "<state>"` - Use ROI Area Selection. Either `true` or `false`.

`-setPDComputeOutlineConf "<value>"` - Compute Additional Outline Confidence. Either "true" or "false".

`-startTracking` - Start pupil tracking. A camera needs to be opened beforehand.

`-setDelimiter "<delimiter>"` - Set global delimiter character. Accepted characters are a comma, a semicolon or tab character.

`-setImageOutputPath "<path>"` - Set image recording output path. Make sure you do not use special characters as they may be restricted for use by the operating system in folder names.

`-setImageOutputFormat "<format>"` - Set image recording output format. Format name should be provided: `tiff` or `png` or `bmp` or `jpeg` or `webp` or `pgm`. Cannot be altered while an image recording is going on.

`-startImageRecording` - Start image recording. Image recording path and name should be set beforehand.

`-setDataOutputPath "<path and name>"` - Set data ("csv") recording path and name. Make sure you do not use special characters as they may be restricted for use by the operating system in folder/file names. Cannot be altered while a data ("csv") recording is going on.

`-startDataRecording` - Start data ("csv") recording. Data recording path and name should be set beforehand.

`-connectStreamUDP "<IP>;<port>"` - Establish connection for pupil data streaming target using a UDP port.

`-connectStreamCOM "<port>;<baud>"` - Establish connection for pupil data streaming target using a COM/serial port.

`-startStreaming` - Start data streaming. Streaming target should be available and its port is opened for listening.

`-connectRemoteUDP "<IP>;<port>"` - Establish connection (start listening) for remote control using a UDP port.

`-connectRemoteCOM "<port>;<baud>"` - Establish connection (start listening) for remote control using a COM/serial port.

