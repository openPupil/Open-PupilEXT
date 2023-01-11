<div align="center">
  <img src="Misc/img/PupilEXT-Logo.png" width="70%" height="25%">
</div><br/>
<div align="center">
  <a href="https://www.frontiersin.org/articles/10.3389/fnins.2021.676220/abstract" alt="Publication">
        <img src="https://img.shields.io/badge/Frontiers%20in%20Neuroscience-Published-green"/></a>
  <a href="https://doi.org/10.3389/fnins.2021.676220" alt="DOI">
        <img src="https://img.shields.io/badge/DOI-10.3389%2Ffnins.2021.676220-blue"/></a>
 <a href="https://github.com/openPupil/Open-PupilEXT/blob/main/PupilEXT/LICENSE" alt="GNU-GPLv3">
        <img src="https://img.shields.io/badge/License%20PupilEXT-GNU%20GPLv3-blue"/></a>
 <a href="https://github.com/openPupil/Open-PupilEXT" alt="Version">
        <img src="https://img.shields.io/badge/Version-0.1.1%20Beta-green"/></a>
 <a href="https://github.com/openPupil/Open-PupilEXT" alt="Contribution">
        <img src="https://img.shields.io/badge/Contributions-Welcome-green"/></a>
</div>

<p align="center">
    <img src="Misc/img/StartupAnim.gif" align="center" width="55%" height="20%">&nbsp;
  <img src="Misc/img/Calibration.gif" align="center" width="40%" height="15%">
  </p>
 

# PupilEXT: flexible open-source platform for high resolution pupillometry in vision research

This repository provides the official implementation of a free pupillometry cross-platform (MacOS, Windows, Linux) system called PupilEXT proposed in the article *"PupilEXT: flexible open-source platform for high resolution pupillometry in vision research"* authored by Babak Zandi, Moritz Lode, Alexander Herzog, Georgios Sakas and Tran Quoc Khanh from the Technical University of Darmstadt.<br/>

PupilEXT can record eye images using a stereo camera system or a single camera to measure the pupil diameter in real-time. Additionally, it is possible to analyse externally-recorded images without connected cameras through the PupilEXT interface.<br/>

We aimed to provide a professional open-source pupillometry measurement platform, making an easy and inexpensive entry into pupil research for interdisciplinary research groups possible. PupilEXT integrates high-resolution industrial cameras from Basler. The pupil detection itself can be performed with one of the state-of-the-art open-source algorithms, Starburst [[1\]](#1), Swirski2D [[2\]](#2), ExCuSe [[3\]](#3), ElSe [[4\]](#4), PuRe [[5\]](#5), and PuReST [[6\]](#6).<br/>

-----

<div align="center">
<a style="font-weight:bold" href="https://www.frontiersin.org/articles/10.3389/fnins.2021.676220/abstract">[Paper]</a>
<a style="font-weight:bold" href="https://www.frontiersin.org/articles/10.3389/fnins.2021.676220/abstract">[Supplementary materials]</a>
</div>

-----

**Features of PupilEXT**

- [x]  High resolution pupillometry
- [x]  Real-time pupillometry
- [x]  Professional graphical user interface programmed with QT/C++ 
- [x]  Supporting stereo camera setups
- [x]  Supporting single camera setups
- [x]  Supporting Basler branded cameras (USB3). Tested with Basler acA2040-120um and acA1300-200um.
- [x]  Offline pupillometry with externally recorded images 
- [x]  Cross-platform software: MacOS, Windows, Linux
- [x]  Intgrates six leading open-source pupil detection algorithms: Starburst [[1\]](#1), Swirski2D [[2\]](#2), ExCuSe [[3\]](#3), ElSe [[4\]](#4), PuRe [[5\]](#5), and PuReST [[6\]](#6).

For initial testing, we recommend using the provided demo datasets. See section 2.4 in this repository. Pay attention to the image playback speed under "preference" (see video tutorial) if you are on a laptop with low computational power.

**Questions?**

If you have new ideas, questions or want to discuss some extended topics, please use our discussion forum:
https://github.com/openPupil/Open-PupilEXT/discussions

**!! NEW 04/2022 !!: A Python version of PupilEXT is now available.**

Note that the Python package of PupilEXT is currently in the experimental stage.

Link to the GitHub repo of PyPupilEXT: [https://github.com/openPupil/PyPupilEXT](https://github.com/openPupil/PyPupilEXT)

**!! NEW 12/2021 !!: Watch the video tutorials of PupilEXT that walk trough the interface**

**Tutorial 1/3:** Stereo camera recording<br/>
https://vimeo.com/564556788

**Tutorial 2/3:** Single camera recording<br/>
https://vimeo.com/564776313

**Tutorial 3/3:** External image analysis<br/>
https://vimeo.com/564778587


<p align="center">
  <img src="Misc/img/PupilEXT-Introduction.gif">
</p>

## 1. Overview
PupilEXT is a real-time pupillometry software whose graphical user interface is mainly programmed using C++ (QT. 5.15) and can be used to integrate high-resolution industrial cameras for online measurements. This Repository provides a guide on how to install, set up and use the system for pupillometry. Before using the PupilEXT framework, we recommend reading the work *"PupilEXT: flexible open-source platform for high resolution pupillometry in vision research"*.

### 1.1 Pupillometry setup configuations
With PupilEXT, you can detect the pupil diameter with three different setup configurations (see Figure). In the stereo camera configuration, the pupil diameter can be recorded directly in millimetres through an internal triangulation procedure. In the simplest case, only one camera must be connected for measuring the pupil diameter in pixels. The conversion from pixels to mm can then manually be performed using a circular reference object. If no camera is available, PupilEXT offers the possibility to load externally recorded images for offline analysis. The externally acquired images can be treated as a live feed, unlocking the full functionality of PupilEXT.

<p align="center">
  <img src="Misc/img/PupilEXT_Measuremtn_Setups.png">
</p>

### 1.2 Hardware and software requirements
Depending on which setup configuration is used, additional hardware components must be purchased. Below is a list of components with which our demo system was built. In general, the USB3 Basler branded camera should be compatible with PupilEXT, as we use Pylon. For offline analysis with externally captured images, none of the listed hardware components is necessary. In any case, the [Pylon Camera Software Suite](https://www.baslerweb.com/de/produkte/software/basler-pylon-camera-software-suite/) from Basler must be installed on your system, as the drivers are needed to start PupilEXT, even if no camera(s) is connected. Check out our publication if you need additional information on how to build the hardware system.

**Components: Stereo camera configuration**

- 2 x Basler acA2040-120um (USB3 cameras)
- 2 x KOWA LM50JC3M2 (2/3" C-Mount lenses)
- 2 x Schneider IF 092 SH 27.0 (Filters)
- 2 x H06S Power I/O Cable
- 1 x STM32 Nucleo F767ZI or similar (Hardware trigger)
- 1 x IR-illumination (Recommended)

**Components: Single camera configuration**

- 1 x Basler acA2040-120um (USB3 cameras)
- 1 x KOWA LM50JC3M2 (2/3" C-Mount lense)
- 1 x Schneider IF 092 SH 27.0 (Red & NIR Color Filter)
- *1 x H06S Power I/O Cable (Optional)*
- *1 x STM32 Nucleo F767ZI or similar (Optional)*
- 1 x IR-illumination (Recommended)

We also tested the system with a Basler acA1300-200um camera model. To build a cheap pupillometry system, you can also have a look to the Basler dart series cameras, which cost approximatley 120 Euro. Note that we did not test PupilEXT with a dart series camera from Basler, but it should work as we use the standard Pylon library from Basler to communicate with the USB3 cameras.

Please use our discussion forum to share your ideas, setup and experience with PupilEXT with other community members:

https://github.com/openPupil/Open-PupilEXT/discussions

## 2. Getting started: The easy way
### 2.1. Installation on MacOS (macOS 10.15 or later)
The direct installation requires that you have macOS 10.15 or later installed on your machine. If this is not the case, you need to build PupilEXT from source, as the provided pre-build binaries in this section will only work on macOS 10.15 or later.

**Step 1: Download and install the Pylon Camera Softwware**

Download the Pylon Camera Software Suite (*.dmg) from the Basler Website:

https://www.baslerweb.com/de/vertrieb-support/downloads/downloads-software/

We tested PupilEXT with **Pylon 6.1.2.** (on Windows only **Pylon 6.0.1.** works, see the Win instructions below). If the Pylon installation does not start because of a security warning from Apple: Open the system preferences from macOS, click on "security & privacy" and press "Open Anyway" under the "General" tab. During the Pylon installation, ensure that a complete installation is carried out with the C++ binaries (Important!). For this, you need to choose the "custom" profile and activate all checkboxes under "Pylon runtime" and "SDKs". The Pylon library is necessary for PupilEXT to control and communicate with Basler branded cameras. The installation is also needed if no camera is connected, as the Basler drivers are part of the software.

Please test your Baslers camera(s) with the installed Pylon Viewer from Basler (see [Basler-Documentation: Pylon Viewer](https://docs.baslerweb.com/overview-of-the-pylon-viewer)) before driving the camera(s) with PupilEXT.

**Step 2: Download and run PupilEXT**

We have built and deployed PupilEXT for macOS 10.15 or later. You only need to download PupilEXT and open it. If you run into security warnings from Apple, you need to open the system preferences, click on "security & privacy" and then press "Open Anyway" under the "General" tab.

Download PupilEXT Version 0.1.1 Beta (MAC) from the release page of the project's GitHub repository:

https://github.com/openPupil/Open-PupilEXT/releases/tag/v0.1.1-beta

After you have downloaded the software, you can open the dmg file and run PupilEXT (see animation). We tested the program on a fresh installed Macbook Pro (Intel) with macOS Big Sur (Version 11.2.3) and a Mac Mini (Intel) with macOS Catalina (Version 10.15.7). If you run into issues, you need to build PupilEXT from source.

<p align="center">
  <img src="Misc/img/PupilEXT_Start_Mac.gif">
</p>

**Step 3: Testing with the demo datasets**

You can load the provided demo dataset (see section 2.4) into PupilEXT for initial testing and enjoying the features during offline analysis. We recommend watching the video tutorials (see section 2.5) of PupilEXT before using the provided eye images. 

**Additional Information**

It should be noted that the PupilEXT software is not designed for the dark-mode of macOS, as the icons in the quick-toolbar are colored with black lines. Therefore, the icons will not be visible when running PupilEXT in dark mode. We recommend to use the software only in light-mode of macOS.

### 2.2. Installation on Windows 10 (64 bit)
**Step 1: Download and install the Pylon Camera Softwware**

Download the Pylon Camera Software Suite (*.exe) from the Basler Website:

https://www.baslerweb.com/de/vertrieb-support/downloads/downloads-software/

We tested PupilEXT with **Pylon 6.0.1.** on several fresh installed Windows 10 (64 bit) machines. Only the **Pylon 6.0.1.** version works with PupilEXT, as Windows is a diva. During the Pylon installation, ensure that a complete installation is carried out with the C++ binaries (Important!). For this, you need to choose the "custom" profile and activate all checkboxes under "Pylon runtime" and "SDKs". The Pylon library is necessary for PupilEXT to control and communicate with Basler branded cameras. The installation is also needed when no camera is connected, as the Basler drivers are part of the software.

Please test your camera with the installed Pylon Viewer from Basler (see [Basler-Documentation: Pylon Viewer](https://docs.baslerweb.com/overview-of-the-pylon-viewer)) before driving the cameras with PupilEXT.

**Step 2: Download and run PupilEXT**

For Windows, we have already built the software so that PupilEXT can simply be downloaded and will start without any further dependencies. Unpack the downloaded zip file and open PupilEXT.exe. It should run, as we tested it on several systems. However, if you run into unknown issues, you need to build PupilEXT from source (see instructions below).

Download PupilEXT Version 0.1.1 Beta (Windows) from the release page of the project's GitHub repository:

https://github.com/openPupil/Open-PupilEXT/releases/tag/v0.1.1-beta

**Step 3: Testing with the demo datasets**

You can load the provided demo dataset (see section 2.4) into PupilEXT for initial testing and enjoying the features during offline analysis. We recommend watching the video tutorials (see section 2.5) of PupilEXT before using the provided eye images.

### 2.3. Setup the microcontroller for the stereo camera configuration
A microcontroller is necessary if a real-time online pupil measurement should be carried out with a stereo camera system. The microcontroller has the task of generating a so-called electrical hardware-trigger, which is used to trigger the image acquisition on the camera (see [Basler-Documentation: Triggering](https://docs.baslerweb.com/triggered-image-acquisition)). The electrical hardware-trigger consists of a timed voltage signal of 3.3 V, which is applied to one of the camera's GPIO-Pins. If a sequence of images need to be acquired with a stable FPS, the hardware-trigger is a PWM-signal with a fixed frequency (see [Basler-Documentation: Trigger-Types](https://docs.baslerweb.com/trigger-selector) and [Basler-Documentation: GPIO Lines](https://docs.baslerweb.com/general-purpose-io-(gpio)-lines)). The advantage of acquiring images through a hardware-trigger signal is that the electrical signal can be connected parallel to both cameras, leading to a highly synchronized image recording. Synchronous image acquisition from both cameras is essential in a stereo camera system; otherwise, the conversion from pixels to mm is not reliable. The PupilEXT software uses a hardware-trigger from an external microcontroller (STM32 Nucleo) to acquire images from the stereo camera system. If you only want to use a single camera for pupil measurement, section 2.3 is not relevant for you.

In this way, the image capture process is as follows: The PupilEXT software sends a command (Protocol) to the microcontroller via USB (uart), aiming to start a logical hardware-trigger signal, which is transmitted from the microcontroller to the camera(s). The captured images are passed directly from the camera to PupilEXT via USB3. In continuous shooting, PupilEXT sends a command to let the microcontroller generate a continuous PWM signal, so there is no need for a single command for each trigger signal. This procedure is standard for all professional stereo camera systems. Hardware trigger signals can also be generated from the camera itself, but there is a risk that due to internal delays, the images will not be captured synchronously. Therefore, adding an external signal generator increases the stability of the system.

Since a standardised protocol between the PupilEXT software and the microcontroller is required, we have uploaded the corresponding embedded code in this repository in [``Misc/Microcontroller/STMNucleo_F767ZI``](Misc/Microcontroller/STMNucleo_F767ZI) and [``Misc/Microcontroller/STMNucleo_L432KC``](Misc/Microcontroller/STMNucleo_L432KC). The embedded projects were created using PlatformIO in Visual Studio Code. You can use either an STM32 Nucleo F767ZI or STM32 Nucleo L432KC microcontroller to flash our provided code to the microcontroller. Alternatively, you can use your own microcontroller, but you must pay attention to the communication protocol (see our publication).

We used the [Mbed 5 framework](https://os.mbed.com) for programming the STM32 Nucleo microcontroller. To simplify the flashing process for you, we highly recommend flashing the microcontroller using [Mbed Studio](https://os.mbed.com/studio/), as this is more user-friendly. Below is a step-by-step guide on how to flash the microcontroller with Mbed Studio.

**Step 1: Installation of Mbed Studio**

Download and install Mbed Studio. A free account is required before you can use it. 

https://os.mbed.com/studio/

**Step 2: Flash the microcontroller**

Create a new project inside Mbed Studio and select your connected microcontroller from the list. In the following, we assume that you are using an STM32 Nucleo L432KC. Be sure to select Mbed OS 5 as a template from the list. You can select an empty programme as template. In the fresh created main.cpp file, paste the below-listed code and press the play button in Mbed Studio (Build profile: Release) to flash the microcontroller. If you are using an STM32 Nucleo F767ZI, use another code from the file [Misc/Microcontroller/STMNucleo_F767ZI/src/main.cpp](Misc/Microcontroller/STMNucleo_F767ZI/src/main.cpp), as the pin assignment is different.

```c++
#include "RawSerial.h"
#include "mbed.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_LENGTH 15
#define BAUDRATE 115200

#define TX_PIN USBTX
#define RX_PIN USBRX

#define LED_PIN_1 LED1
#define Trigger_Pin PB_0

RawSerial pc(TX_PIN, RX_PIN);
volatile char rx_buf[BUFF_LENGTH];
Timer timer_Stopper;

DigitalOut LED_Green(LED_PIN_1);
DigitalOut Trigger(Trigger_Pin);
Ticker myTick;

// Global Values
char M_or_T; // T = Manually Trigger Mode
uint32_t Manually_Values[2];
uint32_t Automatic_Values[1];

volatile int flag_1 = 0;
volatile int flag_2 = 0;
int LED_ticker = 0;
int Threshold_Ticker = 0;
int STOP = 0;

void onTick() {
  LED_Green = !LED_Green;
  Trigger = !Trigger;
  LED_ticker++;
  if (LED_ticker >= 2 * Threshold_Ticker) {
    myTick.detach();
    LED_Green = 0;
    Trigger = 0;
  }
  if (STOP == 1) {
    myTick.detach();
    LED_Green = 0;
    Trigger = 0;
  }
}

void serialCb() {

  char *pch;
  pch = strtok((char *)rx_buf, "X");

  M_or_T = *pch;

  if (M_or_T == 'T') {
    pch = strtok(NULL, "X");
    Manually_Values[0] = atoi(pch);

    pch = strtok(NULL, "X");
    Manually_Values[1] = atoi(pch);
  }

  switch (M_or_T) {

  case 'T':
    Threshold_Ticker = Manually_Values[0];
    STOP = 0;
    LED_ticker = 0;
    myTick.attach_us(&onTick, Manually_Values[1]);
    break;

  case 'S':
    STOP = 1;
    break;

  default:
    break;
  }
}

void callback() {
  if (pc.getc() == '<') {
    pc.putc('<');
    for (int i = 0; i < 20; i++) {

      rx_buf[i] = pc.getc();

      if (rx_buf[i] != '>') {
        pc.putc(rx_buf[i]);
      }

      if (rx_buf[i] == '>') {
        pc.putc('>');
        pc.putc('\n');
        flag_1 = 1;
        break;
      }
    }
  }
}

int main() {
  LED_Green = 0;
  LED_ticker = 0;
  Trigger = 0;
  pc.baud(115200);
  pc.attach(&callback, Serial::RxIrq);
  pc.printf("Program started! \n");

  while (1) {
    if (flag_1 == 1) {
      serialCb();
      flag_1 = 0;
    }
  }
}
// STM32 Nucleo L432KC flashing code
// The provided code for the STM32 Nucleo is licensed under the MIT
// See the lincense file in this repostory:
// Microcontroller/STMNucleo_L432KC/LICENSE
```

**Step 3: Wire the components together:**

Plug the two purchased I/O cables (see section 1.2: Hardware requirements) into the I/O connector of camera 1 and 2. At the open end of the I/O cable are the respective 6 I/O lines of the camera. PIN 1, corresponding to Line 3 of the I/O cable can be used to apply a logical hardware trigger signal to the Basler acA2040-120um camera. Please read the documentation and warnings of your camera regarding the I/O PINs, otherwise you may damage your camera if the wrong voltage values are used (see [Basler-Manual: GPIO Lines](https://docs.baslerweb.com/general-purpose-io-(gpio)-lines)). Pay attention to the correct PIN numbers of the I/O connector and check the camera's documentation to see which I/O PIN is the correct one for your case: https://docs.baslerweb.com/aca2040-120um

Connect the respective I/O line of camera 1 and camera 2 in parallel with the trigger PIN on the microcontroller. If you have used our embedded code to flash the microcontroller, it is PIN ``PB_0`` on the STM32 Nucleo L432KC or PIN ``PC_6`` on the STM32 Nucleo F767ZI. You can check where the PIN is located on the board by checking the Mbed webpage:

Pinout sketch STM32 Nucleo L432KC: https://os.mbed.com/platforms/ST-Nucleo-L432KC/

Pinout sketch STM32 Nucleo F767ZI: https://os.mbed.com/platforms/ST-Nucleo-F767ZI/

We recommend verifying that the microcontroller has been successfully flashed. To do this, you can enter the following command <TX10X1000000> (Protocol: <T X Count_of_Trigger X Time_Trigger_on_us>) in a serial monitor (Baudrate: 115200). For this, either the integrated serial monitor of Mbed Studio can be used or another one like HWMonitor or [CoolTerm](https://freeware.the-meiers.org). This command should start a flashing green LED (10 times) on the microcontroller, with a turn-on time of 1000000 microseconds in each cycle. Since the hardware-trigger PIN of the microcontroller is "connected" to the LED, it signifies that the PC-to-microcontroller communication and the signal generation are working. Additionally, the trigger signal can be measured using an oscilloscope. 

Once the microcontroller's correct operation has been checked, the cameras can be connected to the microcontroller and the PC via USB. Simultaneously, the hardware trigger PIN of the microcontroller should be connected in parallel with GPIO PINs of camera 1 and camera 2. Do not forget to connect the GND line of the camera to the GND PIN of the microcontroller. The figure in [``Misc/img/PupilEXT_Measuremtn_Setups.png``](Misc/img/PupilEXT_Measuremtn_Setups.png) illustrates the setup of a stereo camera configuration. After the connection has been made correctly, the PupilEXT software can be started to begin measuring the pupil diameter through the stereo camera system.

### 2.4. Demo dataset for offline analysis
We provide three different recorded datasets containing eye images that can be analysed using PupilEXT. This should give users the possibility to work directly with the software PupilEXT. The first two data sets were recorded using a single camera. Data set number 3 was recorded with a stereo camera system and is used in the video tutorial of PupilEXT (see section 2.5). The images were acquired directly with PupilEXT, as we offer the feature to record images for later offline measurement of the pupil diameter without connected cameras. The data set is suitable for playing around with PupilEXT and getting a first impression.

We stored the demo data in the TUdatalib repository: https://tudatalib.ulb.tu-darmstadt.de/handle/tudatalib/2929

| Description | Preview | Download-Link | File-size |
| :---         |     :---:      |          :---- |         :---: |
| Recorded with a single camera under natural light with a filter in front of the lens. Dataset is without a calibration file. | <img src="Misc/img/Demo_Data_1_Single_Camera.gif" width="200"/> | [Download dataset 1](https://tudatalib.ulb.tu-darmstadt.de/bitstream/handle/tudatalib/2929/Single_camera_recording_1.zip?sequence=1&isAllowed=y) | 1.2 Gb |
| Recorded with a single camera under natural light without a filter in front of the lens. Dataset is without a calibration file | <img src="Misc/img/Demo_Data_2_Single_Camera.gif" width="200"/> | [Download dataset 2](https://tudatalib.ulb.tu-darmstadt.de/bitstream/handle/tudatalib/2929/Single_camera_recording_2.zip?sequence=2&isAllowed=y) | 600 Mb |
| Recorded using a stereo camera system with IR-illumination and a filter in front of the lens. Calibration files are available and loaded to PupilEXT (see instructions below). | <img src="Misc/img/Demo_Data_3_Stereo_Camera.gif" width="200"/> | [Part 1](https://tudatalib.ulb.tu-darmstadt.de/bitstream/handle/tudatalib/2929/Stereo_camera_recording_Part_1.zip?sequence=3&isAllowed=y)<br>[Part 2](https://tudatalib.ulb.tu-darmstadt.de/bitstream/handle/tudatalib/2929/Stereo_camera_recording_Part_2_CAM0.zip?sequence=4&isAllowed=y)<br>[Part 3](https://tudatalib.ulb.tu-darmstadt.de/bitstream/handle/tudatalib/2929/Stereo_camera_recording_Part_3_CAM1.zip?sequence=5&isAllowed=y) | ~10 Gb |

**Instructions on how to use the stereo camera demo data:**
The data set is divided into three parts. Please download all three files first. Part 1 (``Stereo_camera_recording_Part_1``) is the main folder containing the two subfolders ``Calibration_Files`` and ``Stereo_Images``. The ``Stereo_Images`` subfolder is currently empty and needs to be filled with the demo images of ``cam 0`` and ``cam 1``. For this, download Part 2 (``Stereo_camera_recording_Part_2_CAM0``) and Part 3 (``Stereo_camera_recording_Part_3_CAM1``). Unzip the two folders and rename the folder ``Stereo_camera_recording_Part_2_CAM0`` to ``0`` and also rename folder ``Stereo_camera_recording_Part_3_CAM1`` to ``1``. Move the renamed folder ``0`` and ``1`` into the subfolder ``Stereo_Images`` of the main folder ``Stereo_camera_recording_Part_1`` (Part 1). In PupilEXT, you only need to choose the subfolder Stereo_Imagesas image source. The folder ``0`` and ``1`` will be automatically assigned to ``cam 0`` and ``cam 1``. The individual calibration files are in the subfolder ``Calibration_Files``. You only need to choose the stereo calibration file and not the single calibration files for each camera (see video tutorials).



**Note on externally acquired images**

When loading images to PupilEXT for offline analysis, take care of proper file names. For example, if you label your files with sequenced numbers such as 1.png, 2.png, 3.png, 4.png, 5.png, 6.png, 7.png, 8.png, 9.png, 10.png, 11.png, you will run into issues, as the order of the images will be changed to 1.png, 10.png, 11.png, 2.png, ...

It is better to label the images with 001.png, with the time stamp in milliseconds or similar. If you recorded the images with PupilEXT, everything is labelled correctly as we use the timestamp in milliseconds as filenames. The provided demo datasets are recorded with PupilEXT.

### 2.5 Notes about the pupil detection acuracy

The pupil detection can be determined in PupilEXT with one of the six open-source pupil detection methods Starburst [[1\]](#1), Swirski2D [[2\]](#2), ExCuSe [[3\]](#3), ElSe [[4\]](#4), PuRe [[5\]](#5), and PuReST [[6\]](#6). The respective algorithm can be selected directly within the pupillometry interface. A pupil measurement's accuracy depends on the applied detection algorithm if the image quality and composition are ideal. Each pupil detection algorithm has a certain number of parameters (constants) that the user must set.  For instance, at least the minimum or maximum possible pupil diameter must be adjusted, which can depend on the image's resolution. In PupilEXT, we offer the possibility to adjust the parameters of an applied detection algorithm, leading to more freedom in increasing the measurement accuracy. The original publications' standard parameters are usually not ideal for all measurement settings and should be adjusted if the detection rate is not good enough.

The accuracy can also be increased by selecting a specific region of interest (ROI) in the image. In this way, the algorithm no longer needs to perform pupil detection over the entire image. A smaller ROI size also reduces the calculation time, allowing a higher framerate in real-time measurements. We provide different preset settings for the algorithms` parameters which are dependent on the used ROI size. However, we highly recommend using the PuRe or PuReST algorithm, as it needs only three parameters. Furthermore, these two pupil detection algorithms are considered both the fastest and accurate in the literature and are part of the amazing open-source eye-tracking software EyeRecToo [[7\]](#7). According to our preliminary investigations, the Swirski algorithm can provide better results, as it does not downscale the image before a pupil detection. The downside is that it needs a higher calculation time and has a higher number of parameters. There is the risk that the parameters do not match the image composition, leading to poor pupil detection accuracy.

### 2.5. How to use PupilEXT ?
Three videos are provided to illustrate the handling and the features of the *PupilEXT* software. The first video introduces the *PupilEXT* software in stereo vision mode. The second video shows how the pupillometry can be carried out by using a single camera. The third video shows how pupillometry can be carried out with existing eye images without connected cameras.

**Video Tutorial 1 – Stereo camera setup:** https://vimeo.com/564556788

**Video Tutorial 2 – Single camera setup:** https://vimeo.com/564776313

**Video Tutorial 3 – Offline analysis:** https://vimeo.com/564778587

## 3. Build PupilEXT from source: The advanced way

If you would like to contribute to this project, extend PupilEXT with custom functions, or the provided binaries do not work on your machine, building PupilEXT on your machine is necessary. The annoying part of compiling C++ projects is the integration of third-party libraries into a project. For this, you have three options: (i) use a system package manager like brew to download and build third-party libraries; (ii) download the libraries without a package manager and build it; (iii) integrating the libraries directly into the project. 

If you want to stick with option (ii) you are a professional C++ developer and do not need this tutorial. Option (i) is preferable if you have several C++ projects on your machine and want to share the libraries between the projects, saving disc space (see section 3.1 and section 3.2). However, we highly recommend option (iii), which has the advatange that the integration of thirdparty libraries is fully automated(see section 3.0). You should have intermediate knowledge of C++ programming and CMake before you follow this tutorial, as we provide no warranty. 

PupilEXT need the following libraries: Boost 1.75-0_2, Ceres 2.0.0, Eigen 3.3.9, OpenCV 4.5.1_3, QT 5.15.0, [spii](https://github.com/PetterS/spii), tbb, pylon 6.

**Important Note 1:** When using Boost@1.76 or higher, you will run into compiling issues, as boost::math::sign will not work, which is used by one of the pupil detection methods.

**Important Note 2:** If you want to follow option (i) or (ii)  you need to remove the 'vcpkg.json' file and the '.vscode' folder. 

### 3.0 Build PupilEXT with vcpkg manifest (recommended)

The PupilEXT project contains a vcpkg.json file in which all the required C++ libraries are defined. To download and build the libraries, we use the package management software vcpkg (https://vcpkg.io/en/index.html). We have placed the vcpkg GitHub repository as a submodule under [``3rdparty/vcpkg``](3rdparty/vcpkg), meaning that the required libraries will be downloaded automatically to the PupilEXT project folder, regardless of your system. This has the advantage that the PupilEXT folder can be easily deleted when the C++ libraries are no longer needed. However, as the libraries are downloaded and built, care must be taken to ensure that at least 6 GB are available on the disc for the PupilEXT folder (on windows ~13 GB). However, the QT library and the Pylon drivers for the cameras will not be managed via vcpkg, so they have to be downloaded and installed manually (see step 1 and 2).

**Step 1: Download and install the latest Pylon Camera Softwware**

Download the Pylon Camera Software Suite (*.dmg-File) from the Basler Website:
https://www.baslerweb.com/de/vertrieb-support/downloads/downloads-software/

Important: You need to modify the [`Open-PupilEXT/cmake/FindPylon.cmake`](https://github.com/openPupil/Open-PupilEXT/blob/main/cmake/FindPylon.cmake) file according to the Pylon version before you start to build the project. Please read the latest post in this discussion for more information: https://github.com/openPupil/Open-PupilEXT/discussions/21

**Step 2: Download and install QT**

Download the QT from the website. Please install QT 5.15:
https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5

QT 6 or higher is currently not supported.

**Step 3: Clone this repository**

Type into your terminal the following command

```shell
git clone --recurse-submodules https://github.com/openPupil/Open-PupilEXT.git
```

The ``--recurse-submodules`` option is important, as vcpkg is a submodule. Without this option, the 3rdparty folder will not contain vcpkg packet manager. 

Open the ``CMakeLists.tx`` file in the PupilEXT root and change the following line, according to your installed QT and user name

```Cmake
set(QT_VERSION 5.15.2) # Change the version according to your installation
set(USER_NAME_MAC papillon) # Only necessary for macOS users, skip if you are on WIN
```

**Step 4: Decide which Editor to use**

Depending on which editor is used, the procedure is slightly different. We have created a summary of how to proceed if you want to use [Visual Studio Code](https://code.visualstudio.com), [CLion](https://www.jetbrains.com/clion/promo/?source=google&medium=cpc&campaign=11964013627&gclid=Cj0KCQjw-NaJBhDsARIsAAja6dPFpHIXmFeKhVv0qheHTki-WmG5DepDCzR0iWuYQqAXEM1eEAELpy4aApLsEALw_wcB) or [QT Creator](https://www.qt.io/product/development-tools) to start PupilEXT (see the below option 1 to 4). 

**Option 1: Build with Visual Studio Code**

First, the following extensions must be downloaded in the editor for the procedure to work:

(1) C++ extension: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

(2) Cmake extension: https://marketplace.visualstudio.com/items?itemName=twxs.cmake

(3) Cmake Tools extension: https://code.visualstudio.com/docs/cpp/cmake-linux

Then open the ``.vscode/settings.json`` file in the cloned PupilEXT folder with a text editor. The entry ``"VCPKG_TARGET_TRIPLET": "x64-osx"`` must be changed to ``"VCPKG_TARGET_TRIPLET": "x64-windows-static-md"`` if you have a Windows operating system. If you are on a Mac, leave the entry as it is. Open the project folder with Visual Studio Code and build the project. If you press build for the first time, it may take a little longer, as the required libraries are downloaded and will build automatically. Make sure that you have enough disc space on your machine.

**Option 2: Build with CLion**

Open the project folder with CLion. Then some settings have to be made. For this, go to ``CLion > Preferences > Build, Execution, Deployment`` in the toolbar. Choose Debug and type in the Cmake options the following command:

```shell
-DVCPKG_TARGET_TRIPLET=x64-osx -DCMAKE_TOOLCHAIN_FILE=3rdparty/vcpkg/scripts/buildsystems/vcpkg.cmake
```
Note that `x64-osx` must be changed to `x64-windows-static-md`, if you are on a Windows machine. Next, choose for the build directory, the following folder: ``build``. This is very important, as the CMakeLists.txt is adjusted to find the libraries in a folder called ``build``. By default, CLion will build the project in a folder called ``cmake-build-debug``, which will not work with the current CMakeLists.txt. Press okay and build the project. The C++ libraries will be downloaded and build automatically for you. The first run will take some time. Make sure that you have enough disc space. 

**Option 3: Build with QT Creator**

QTCreator also supports CMake, which means that PupilEXT can also be opened there. As with CLion, only the following options need to be added: 

```shell
-DVCPKG_TARGET_TRIPLET=x64-osx -DCMAKE_TOOLCHAIN_FILE=3rdparty/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Note that `x64-osx` must be changed to `x64-windows-static-md`, if you are on a Windows machine. Next, choose for the build directory, the following folder: ``build``.

**Option 4: Build from terminal**

Open your terminal and type the following commands if you are on a Mac:

```shell
cd Open-PupilEXT

mkdir build

cd build

cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-osx -DCMAKE_TOOLCHAIN_FILE=3rdparty/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build . --config Release

./src/PupilEXT
```

If you are on Windows 10  use these commands (not tested, but should be similar):

```shell
cd Open-PupilEXT

mkdir build

cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -DCMAKE_TOOLCHAIN_FILE=3rdparty/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build . --config Release
```
Then, the executable will be available here ``/Open-PupilEXT-main/build/src/PupilEXT``.

Note, the option ``-DCMAKE_TOOLCHAIN_FILE=3rdparty/vcpkg/scripts/buildsystems/vcpkg.cmak`` makes sure to use the submodule as package manager and download the C++ libraries defined in the vcpkg.json file. The ``-DVCPKG_TARGET_TRIPLET=x64-os`` option is necessary to let vcpkg know which triplet your need. On Windows you need to change the tripplet to ``-DVCPKG_TARGET_TRIPLET=x64-windows-static-md``.

### 3.1 How to build from source on MacOS

**Step 1: Download and install the Pylon Camera Softwware**

Download the Pylon Camera Software Suite (*.dmg-File) from the Basler Website:
https://www.baslerweb.com/de/vertrieb-support/downloads/downloads-software/

**Step 2: Download and install QT**

Download the QT from the website. Please install QT 5.15:
https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5

**Step 3: Install the package manager homebrew**

Homebrew is a package manager for macOS with which C++ libraries can be installed relatively easily via the terminal. Instructions on how to install Homebrew can be found here: https://brew.sh

To install homebrew , you can enter the following command in the macOS terminal.

```shell
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**Step 4: C++ Libraries**

Now install the necessary C++ libraries for PupilEXT through homebrew. The Ceres library will be installed by OpenCV.

```shell
brew install cmake
brew install boost
brew install Eigen
brew install glog
brew install tbb
brew install opencv
```

**Step 5: Download and build the spii library**

An additional library called spii must be downloaded and built manually. This library is not included in Homebrew. First, go to the following GitHub repository and download it:

https://github.com/PetterS/spii

Open the downloaded folder ``spii-master`` with [CLion](https://www.jetbrains.com/clion/). By default, CLion is set to debug mode. Therefore, the release option must be added. For this, go to CLion > Preferences > Build, Execution, Deployment in the toolbar. Then press the + symbol under CMake, which automatically adds the release mode. You can then switch to release in the top right-hand corner of the CLion toolbar. You can now build the project. Simply go to Build > Build Project.

**Step 6: Add spii to PupilEXT**

The source files of PupilEXT are located here in Github under the PupilEXT folder. Download it from GitHub and create a new folder called ``PupilEXT/external`` in the main PupilEXT folder. In ``external/`` add another new folder called ``spii/``. 

In the folder ``PupilEXT/external/spii`` you need to copy the spii build created in step 5. First go to ``spii-master/cmake-build-release`` and copy the folder ``spii-master/cmake-build-release/lib`` to ``spii-master``. Then copy all files inside the ``spii-master/`` folder into ``PupilEXT/external/spii``.

**Step 7: Build PupilEXT from source**

Now, open the PupilEXT project with CLion. In the file ``PupilEXT/CMakeLists.txt`` the paths must be updated so that the installed C++ libraries from step 4 can be correctly recognised. The complete content in ``PupilEXT/CMakeLists.txt`` can be removed and replaced by the following code snippet:

```cmake
cmake_minimum_required(VERSION 3.15)
project(PupilEXT)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/Users/papillon/Qt/5.15.2/clang_64")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/Cellar/glog/0.4.0")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/include/gflags")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/Library/Frameworks/pylon.framework")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/Cellar/tbb/2020_U3_1")

set(SPII_INSTALL_DIR "/Users/papillon/Desktop/PupilEXT/external/spii")
set(spii_INCLUDE_DIRS ${SPII_INSTALL_DIR}/include)
set(GLOG_INCLUDE_DIR "/usr/local/Cellar/glog/0.4.0/include")
set(EIGEN_INCLUDE_DIR "/usr/local/Cellar/eigen/3.3.9/include/eigen3/Eigen")
set(EIGEN3_INCLUDE_DIR "/usr/local/include/eigen3")
SET("OpenCV_DIR" "/usr/local/Cellar/opencv/4.5.1_3/lib/cmake/opencv4")
SET("TBB_DIR" "/usr/local/Cellar/tbb/2020_U3_1/lib/cmake/TBB")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/share/ceres-solver")
else()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/share/ceres-solver")
endif()

set(PYLON_HOME "/Library/Frameworks/pylon.framework")
set(PYLON_INCLUDE_DIR "/Library/Frameworks/pylon.framework/Headers")

find_package(Qt5 COMPONENTS Widgets Concurrent SerialPort Charts Svg PrintSupport REQUIRED)
find_package(OpenCV REQUIRED PATHS "/usr/local/Cellar/opencv/4.5.1_3")
find_package(Boost 1.72 REQUIRED)
find_package(TBB REQUIRED PATHS "${PROJECT_SOURCE_DIR}/external/tbb")
find_package(Eigen3 REQUIRED )
find_package(Ceres REQUIRED)
find_package(Pylon REQUIRED)
find_package(OpenGL REQUIRED)

set(TBB_LIBRARY_DEBUG "/usr/local/include/tbb")
set(TBB_LIBRARY_RELEASE "/usr/local/include/tbb")

find_library (spii_LIBRARY_RELEASE
        spii
        PATHS ${SPII_INSTALL_DIR}/lib/)
find_library (meschach_LIBRARY_RELEASE
        meschach
        PATHS ${SPII_INSTALL_DIR}/lib/)
if (spii_LIBRARY_RELEASE AND meschach_LIBRARY_RELEASE)
    set(spii_LIBRARIES ${spii_LIBRARY_RELEASE} ${meschach_LIBRARY_RELEASE})
else()
    set(spii_LIBRARIES "")
endif()

include_directories(${Qt5Core_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIR}
        ${spii_INCLUDE_DIRS}
        ${EIGEN_INCLUDE_DIR}
        ${CERES_INCLUDE_DIRS}
        ${PYLON_INCLUDE_DIR}
        "singleeyefitter")

add_subdirectory (src)
add_subdirectory (singleeyefitter)

message(STATUS "")
message(STATUS "spii_LIBRARIES:\"${spii_LIBRARIES}\"")
message(STATUS "--- Include directories ---" )
message(STATUS " QT5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}" )
message(STATUS " OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}" )
message(STATUS " Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}" )
message(STATUS " TBB_INCLUDE_DIRS: ${TBB_INCLUDE_DIR}" )
message(STATUS " spii_INCLUDE_DIRS: ${spii_INCLUDE_DIRS}" )
message(STATUS " EIGEN_INCLUDE_DIR: ${EIGEN_INCLUDE_DIR}" )
message(STATUS " CERES_INCLUDE_DIRS: ${CERES_INCLUDE_DIRS}" )
message(STATUS " PYLON_INCLUDE_DIRS: ${PYLON_INCLUDE_DIR}" )
message(STATUS "---------------------------" )
message(STATUS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")
```

Make sure that all folder paths in the ``CMakelist.txt`` are adapted to your system. For instance, the path ``/Users/papillonmac/Qt/5.15.0/clang_64`` must be changed with the name of your user name and your specific QT version installed on your PC.

----

The described changes to the Pylon library in this section can be skipped, as we included in the updated CMakelists.txt the following code for macOS:

```cmake
set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/Library/Frameworks/pylon.framework/Headers")
    set(PYLON_INCLUDE_DIR ${PYLON_INCLUDE_DIR} "/Library/Frameworks/pylon.framework/Headers/GenICam")
```

**Old description (not needed):**

The last step is to change something in the Pylon library; otherwise, the library will not be found properly. For this, open the following folder on your computer: /Library/Frameworks/pylon.framework/Versions/A/Headers/GenICam. All files in this folder must be copied to /Library/Frameworks/pylon.framework/Versions/A/Headers. 

---

Now, you should be able to start PupilEXT properly in CLion. Please remember to use the **release** option as in step 5; otherwise, PupilEXT will run in debug mode and will be significantly slower. 

### 3.2 How to build from source on Windows 10

**Step 1: Install Visual Studio 2019**

C++ MFC must be installed during the installation. Additionally, the English language package must be included. Visual Studio can be downloaded from the following page:

https://visualstudio.microsoft.com/downloads/

**Step 2: Download and install the Pylon Camera Software**

Download the Pylon Camera Software Suite (*.dmg) from the Basler Website:
https://www.baslerweb.com/de/vertrieb-support/downloads/downloads-software/

**Step 3: Download and install QT**

Download the QT from the website. Please install QT 5.15. You need to add ``msvc2017_64`` during the installation:
https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5

**Step 4: Install the C++ package manager**

There is a C++ package manager from Microsoft for Windows, making it relatively easy to download libraries. For this, install the program ``vcpkg``. The instructions on how to install the package manager can be found on the following page:

https://docs.microsoft.com/de-de/cpp/build/install-vcpkg?view=msvc-160&tabs=windows

To install vcpkg type in your console the following commands:

```shell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

**Step 5: Install the C++ Librarys**

Go to your console and use the following commands to install the C++ libraries:

```shell
vcpkg install boost:x64-windows
vcpkg install Eigen:x64-windows
vcpkg install ceres:x64-windows
vcpkg install glog:x64-windows
vcpkg install opencv:x64-windows
vcpkg install tbb:x64-windows
```

**Step 6: Download and build spii**

The spii library is not included in vcpkg. You need to build it manually. First, go to the following page and download the spii library from GitHub:

https://github.com/PetterS/spii

Then [CLion](https://www.jetbrains.com/clion/) can be used to open and build the spii library. Make sure that you use the 64 bit architecture. Then the folder ``spii-master/cmake-build-release/lib`` can be copied into the main folder ``spii-master/``. Lastly, copy the files inside ``spii-master/`` to ``PupilEXT/external/spii``.

**Step 7: Prepare the CMakelists file**

You can add the following code snippet to the file ``PupilEXT/CMakeLists.txt``  to let PupilEXT recognize vcpkg libraries. Make sure to adjust the appropriate paths and version numbers.

```cmake
cmake_minimum_required(VERSION 3.15)
project(PupilEXT)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Qt/Qt5.13.1/5.10.0/msvc2017_64")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/vcpkg/installed/x64-windows")
set(GLOG_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include")
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/vcpkg/installed/x64-windows/debug")
else()
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/vcpkg/installed/x64-windows/include")
endif()

set(TBB_INCLUDE_DIR "C:/vcpkg/installed/x64-windows/include/tbb")
set(SPII_INSTALL_DIR "${PROJECT_SOURCE_DIR}/external/spii")
set(spii_INCLUDE_DIRS ${SPII_INSTALL_DIR}/include)
set(PYLON_HOME "C:/Program Files/Basler/pylon 6/Development/")
find_package(Qt5 COMPONENTS Widgets SerialPort Charts Svg PrintSupport REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost 1.72 REQUIRED)
find_package(TBB REQUIRED PATHS "${PROJECT_SOURCE_DIR}/external/tbb")
find_package(Eigen3 REQUIRED )
find_package(Ceres CONFIG REQUIRED)
find_package(Pylon REQUIRED)
set(TBB_LIBRARY_DEBUG "${PROJECT_SOURCE_DIR}/external/tbb/lib/intel64")
set(TBB_LIBRARY_RELEASE "${PROJECT_SOURCE_DIR}/external/tbb/lib/intel64")

find_library (spii_LIBRARY_RELEASE
        spii
        PATHS ${SPII_INSTALL_DIR}/lib/)
find_library (meschach_LIBRARY_RELEASE
        meschach
        PATHS ${SPII_INSTALL_DIR}/lib/)
if (spii_LIBRARY_RELEASE AND meschach_LIBRARY_RELEASE)
    set(spii_LIBRARIES ${spii_LIBRARY_RELEASE} ${meschach_LIBRARY_RELEASE})
else()
    set(spii_LIBRARIES "")
endif()

include_directories(${Qt5Core_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIR}
        ${spii_INCLUDE_DIRS}
        ${EIGEN_INCLUDE_DIR}
        ${CERES_INCLUDE_DIRS}
        ${PYLON_INCLUDE_DIR}
        "singleeyefitter")

add_subdirectory (src)
add_subdirectory (singleeyefitter)

message(STATUS "")
message(STATUS "spii_LIBRARIES:\"${spii_LIBRARIES}\"")
message(STATUS "--- Include directories ---" )
message(STATUS " QT5Core_INCLUDE_DIRS: ${Qt5Core_INCLUDE_DIRS}" )
message(STATUS " OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}" )
message(STATUS " Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}" )
message(STATUS " TBB_INCLUDE_DIRS: ${TBB_INCLUDE_DIR}" )
message(STATUS " spii_INCLUDE_DIRS: ${spii_INCLUDE_DIRS}" )
message(STATUS " EIGEN_INCLUDE_DIR: ${EIGEN_INCLUDE_DIR}" )
message(STATUS " CERES_INCLUDE_DIRS: ${CERES_INCLUDE_DIRS}" )
message(STATUS " Pylon_INCLUDE_DIRS: ${PYLON_INCLUDE_DIR}" )
message(STATUS "---------------------------" )
message(STATUS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")
```

**Step 8: Environment variables**

Add the bin folder of vcpkg and QT to the environment variables of Windows; otherwise, the libraries will not be found.

**Step 9: Build PupilEXT with CLion**

Before the programme can be build, some settings must be made in CLion:

1) File > Setting > Build, Execution, Deoployment > CMAKE under CMAKE options add the following parameter: --config Debug -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

2) Then press the plus sign to add the release configuration. Add the following parameters to the release options:: --config Release -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

3) The build architecture must be changed to amd64 under Toolchain, as the vcpkg libraries were also installed with 64 bit.

Next, the bin folders must be added to the path of CLION, as these are independent of the Windows environment variables. Under Run > Edit Configuration the paths to VCPKG and QT can be added:

```shell
C:\Qt\Qt5.13.1\5.10.0\msvc2017_64\bin
C:\vcpkg\installed\x64-windows\bin
```

In order to start the programme, the SPII DLL must be added to ``PupilEXT\cmake-build-debug\src``. If everything has been done correctly, you can now press Start and the software should open without any issues. 

## 4. Misc

**Camera emulation**
For debugging porpuses, the Pylon SDK supports emulating camera devices that are then displayed as physical cameras in the PupilExt software. To activate the camera emulation, the system environment variable "PYLON_CAMEMU" needs to be set. The number of available emulator devices can be controlled by exporting the PYLON_CAMEMU environment variable. For example, ``export PYLON_CAMEMU=2``.

## 5. Known issues
- **MacOS:** Quick-toolbar icons not visible in dark-mode. ***Solution:*** *Open PupilEXT in light-mode of MacOS.* ***Comment:*** *Need to add positive-constrast icons and make it dependent on the light-mode or dark-mode state.* ***Priority:*** *Low*
  
- **MacOS:** PupilEXT crashed when choosing ExCuSe a pupil detection ***Solution:*** *Unknown, as this issue only happens on Mac.* ***Comment:*** *Please do not use the ExCuSe algorithm if you are on a Mac* ***Priority:*** *Low*

- **All Platforms:** When using two cameras (stereo camera mode), the label of the camera #1 jumps back to default after pressing connect. No functional impact, as only the GUI is affected.

- **All Platforms:** When using the hardware trigger, the maximum trigger duration is limited to 35 minutes. To solve this issue, another data type must be selected, as currently the duration is provided in seconds.

-  **All Platforms:** When choosing a high fps (e.g. 120 Hz) in stereo mode, the timestamp seems to count backwards. Probably this is related to the way the frames are fetched from the camera's buffer, as the timestamp should be accurate. A current workarround is to re-order the data during pre-processing.

- **All Platforms:** In stereo mode, it is possible that some timestamps occur twice, which can be traced back to rounding issues.

- **Windows:** If the user name of the working system has a space, saving the log files does not work. A current workaround is to save the logfile directly on the hard disk like 'c:/' or similar.

## 6. Citation
Please consider to cite our work if you find this repository or our results useful for your research:

B. Zandi, M. Lode, A. Herzog, G. Sakas, and T. Q. Khanh, “PupilEXT: Flexible Open-Source Platform for High-Resolution Pupillometry in Vision Research,” Front. Neurosci., vol. 15, Jun. 2021, doi: 10.3389/fnins.2021.676220.

```bib
@Article{10.3389/fnins.2021.676220,
AUTHOR = {Zandi, Babak and Lode, Moritz and Herzog, Alexander and Sakas, Georgios and Khanh, Tran Quoc},
TITLE = {PupilEXT: Flexible Open-Source Platform for High-Resolution Pupillometry in Vision Research},
JOURNAL = {Frontiers in Neuroscience},
VOLUME={15},      
PAGES={603},     
YEAR={2021}, 
URL={https://www.frontiersin.org/article/10.3389/fnins.2021.676220},
DOI={10.3389/fnins.2021.676220},    
ISSN={1662-453X}}
```

## 7. References
<a id="1">**[1]**</a> Dongheng Li and Derrick J. Parkhurst. Starburst: A robust algorithm for video-based eye tracking. IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR), September 2005.

<a id="2">**[2]**</a> Lech Swirski, Andreas Bulling, and Neil Dodgson. Robust real-time pupil tracking in highly off-axis images. In Proceedings - 2012 ACM Symposium on Eye Tracking Research and Applications (ETRA), pages 173–176, 2012.

<a id="3">**[3]**</a> Wolfgang Fuhl, Thomas Kübler, Katrin Sippel, Wolfgang Rosenstiel, and Enkelejda Kasneci. Excuse: robust pupil detection in real-world scenarios. In International Conference on Computer Analysis of Images and Patterns, pages 39–51. Springer, 2015.

<a id="4">**[4]**</a> Wolfgang Fuhl, Thiago C. Santini, Thomas Kübler, and Enkelejda Kasneci. ElSe: Ellipse selection for robust pupil detection in real-world environments. In Proceedings - 2016 ACM Symposium on Eye Tracking Research and Applications (ETRA), volume 14, pages 123–130, 2016.

<a id="5">**[5]**</a> Thiago Santini, Wolfgang Fuhl, and Enkelejda Kasneci. PuRe: Robust pupil detection for real-time pervasive eye tracking. Computer Vision and Image Understanding, 170:40–50, 2018.

<a id="6">**[6]**</a> Thiago Santini, Wolfgang Fuhl, and Enkelejda Kasneci. PuReST: Robust pupil tracking for real-time pervasive eye tracking. In Proceedings - 2018 ACM Symposium on Eye Tracking Research and Applications (ETRA). ACM, 2018.

<a id="7">**[7]**</a> Thiago Santini, Wolfgang Fuhl, David Geisler and Enkelejda Kasneci. EyeRecToo: Open-source Software for Real-time Pervasive Head-mounted Eye Tracking. VISIGRAPP 2017.

## 8. Open source projects inside PupilEXT
PupilEXT integrates several open source libraries. This document provides a list of the used libraries. The respective licenses of the libraries are provided as *.txt file in in the subfolder `3rdparty/PupilEXT_Third_Party_Licenses`.

### List of Pupil Detection Libraries

<a id="EyeRecToo" href="https://github.com/tcsantini/EyeRecToo"><b>EyeRecToo</b></a> is an open-source eye tracking software for head-mounted eye tracker and integrates the most advanced state-of-the-art open-source pupil detection algorithms.  We used the implementation of the EyeRecToo’s pupil class and the integrated detection methods for PupilEXT. (License: Copyright (c) 2018, Thiago Santini / University of Tübingen). **License:** For academic and non-commercial use only ([Link License](https://github.com/tcsantini/EyeRecToo/blob/master/LICENSE) | [Project Page](https://github.com/tcsantini/EyeRecToo)).

<a id="PuRe" href="https://www.sciencedirect.com/science/article/abs/pii/S1077314218300146?via%3Dihub"><b>PuRe</b></a> Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, PuRe: Robust pupil detection for real-time pervasive eye tracking. *Computer Vision and Image Understanding*. **2018**, ISSN 1077-3142. https://doi.org/10.1016/j.cviu.2018.02.002. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/PuRe.h)).

<a id="PuReST" href="https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h"><b>PuReST</b></a> Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci. PuReST: Robust pupil tracking for real-time pervasive eye tracking. *Symposium on Eye Tracking Research and Applications (ETRA)*. **2018**. https://doi.org/10.1145/3204493.3204578. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h)).

<a id="ElSe" href="https://dl.acm.org/doi/10.1145/3204493.3204578"><b>ElSe</b></a> Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci. ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments. *ETRA 2016 : Eye Tracking Research and Application.* **2016.** Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ElSe.h)).

<a id="ExCuSe" href="https://link.springer.com/chapter/10.1007/978-3-319-23192-1_4"><b>ExCuSe</b></a> Wolfgang Fuhl, Thomas Kübler, Katrin Simpel, Wolfgang Rosenstiel, Enkelejda Kasneci. ExCuSe: Robust Pupil Detection in Real-World Scenarios. *CAIP 2015 : Computer Analysis of Images and Patterns*. **2015**. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ExCuSe.h)).

<a id="Starburst" href="https://ieeexplore.ieee.org/document/1565386"><b>Starburst</b></a> Dongheng Li, Winfield, D., Parkhurst, D. J. Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches. in *2005 IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR’05) - Workshops* vol. 3 79–79 (IEEE, 2005). https://doi.org/10.1109/CVPR.2005.531. Based on the [cvEyeTracker](https://github.com/thirtysixthspan/cvEyeTracker) Version 1.2.5 implementation. **License:** GNU General Public License ([Link](https://github.com/thirtysixthspan/cvEyeTracker/blob/master/ransac_ellipse.cpp))

<a id="Swirski2D" href="https://dl.acm.org/doi/10.1145/2168556.2168585"><b>Swirski2D</b></a>  Lech Swirski, Andreas Bulling, Neil Dodgson. Robust real-time pupil tracking in highly off-axis images. *ETRA 2012: Proceedings of the Symposium on Eye Tracking Research and Applications*. **2012**. https://doi.org/10.1145/2168556.2168585.  **License:** MIT License, Copyright (c) 2014 Lech Swirski ([Link](https://github.com/LeszekSwirski/pupiltracker/blob/master/LICENSE.md))

<a id="Swirski3D" href="https://www.cl.cam.ac.uk/research/rainbow/projects/eyemodelfit/"><b>Swirski2D</b></a> Lech Swirski, Neil Dodgson. A fully-automatic, temporal approach to single camera, glint-free 3D eye model fitting. *Proceedings of ECEM 2013*. **2013**.  **License:** MIT License, Copyright (c) 2014 Lech Swirski ([Link](https://github.com/LeszekSwirski/singleeyefitter/blob/master/LICENSE.md))

### List of Software Libraries

<a id="QT" href="https://www.qt.io/"><b>QT</b></a>  is an open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms such as Linux, Windows, macOS, Android or embedded systems. (License: GPL 3.0)

<a id="QCustomPlot" href="https://www.qcustomplot.com/"><b>QCustomPlot</b></a> is a Qt C++ widget for plotting and data visualization. It has no further dependencies and is well documented. (License: GPL 3.0)

<a id="OpenCV" href="https://opencv.org/"><b>OpenCV</b></a> is a highly optimized computer vision library with focus on real-time applications. In this repository it is used for image manipulation and plotting of ellipse pupil detections. (License: Apache 2 / BSD)

<a id="Glog" href="https://github.com/google/glog"><b>Glog</b></a> is a library for logging. ([License](https://github.com/google/glog/blob/master/COPYING))

<a id="Boost" href="https://www.boost.org"><b>Boost</b></a> is a set of various C++ libraries for processing tasks. ([License](https://www.boost.org/users/license.html))

<a id="Ceres-Solver" href="http://ceres-solver.org"><b>Ceres-Solver</b></a> is a optimisation library. ([License](https://github.com/ceres-solver/ceres-solver/blob/master/LICENSE))

<a id="Eigen" href="https://eigen.tuxfamily.org/index.php?title=Main_Page#License"><b>Eigen</b></a> is a library for linear algebra. ([License](https://eigen.tuxfamily.org/index.php?title=Main_Page#License))

<a id="Spii" href="https://github.com/PetterS/spii"><b>Spii</b></a> is a library for optimisation. ([License](https://github.com/PetterS/spii/blob/master/LICENSE))

<a id="Tbb" href="https://github.com/oneapi-src/oneTBB"><b>Tbb</b></a> is for parallel programming. ([License](https://github.com/oneapi-src/oneTBB/blob/master/LICENSE.txt))

<a id="Breeze-Icons" href="https://github.com/KDE/breeze-icons"><b>Breeze Icons</b></a> is a set of icons. ([License](https://github.com/KDE/breeze-icons/blob/master/icons/LICENSE))

<a id="Gflags" href="https://github.com/gflags/gflags"><b>Gflags</b></a> is a library for comandline processing. ([License](https://github.com/gflags/gflags/blob/master/COPYING.txt))

## 9. Acknowledgment
We thank the German Research Foundation (DFG) by funding the research (grant number: 450636577).

This project was made possible by the outstanding previous published open-source projects in the field of pupil detection and eye-tracking. Therefore, we would like to thank the authors of the ground-breaking algorithms PuRe, PuReST, ElSe, ExCuSe, Starburst and Swirski, who made their methods available to the public. Namely, we have to thank Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci, Katrin Sippel, Wolfgang Rosenstiel, Dongheng Li, D. Winfield, D. Parkhurst, Lech Swirski, Andreas Bulling and Neil Dodgson for their open-source contributions which are part of PupilEXT. Additionally, we would like to thank the outstanding developers of the software EyeRecToo, whose open-source eye-tracking software inspired us for this work. We used the implementation of the EyeRecToo’s pupil class and the integrated detection methods for PupilEXT.

We appreciate the contributions of Paul Myland, who supported us as a co-supervisor in a bachelor thesis, which topically worked on one part of this project. We highly welcome the contribution of Mohammad Zidan for the mechanical construction of the stereo camera system and the NIR illumination, which was done during his bachelor thesis, supervised by Babak Zandi. Finally, we would like to thank Felix Wirth and Thomas Lautenschläger who joined us as student assistants in the initial phase of the project.

## 10. License

The work's content (Paper) is licensed under a [Creative Commons Attribution 4.0 International License.](http://creativecommons.org/licenses/by/4.0/)

The embedded code for the STM32 Nucleo is licensed under the [MIT](https://opensource.org/licenses/mit-license.php) license.

The software PupilEXT is licensed under [GNU General Public License v.3.0.](https://github.com/openPupil/Open-PupilEXT/blob/main/Misc/LICENSE), Copyright (c) 2021 Technical University of Darmstadt. The pupil detection functionalities of PupilEXT are for academic and **non-commercial** use only. Please note that third-party libraries used in PupilEXT may be distributed under other open-source licenses. Please read the above section 8: Open source projects inside PupilEXT.

This program is distributed in the hope that it will be useful, but without any warranty, without even the implied warranty of fitness for a particular purpose.

