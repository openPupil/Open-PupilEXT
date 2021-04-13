# Repository

The main Readme with a full description about this project is available here: https://github.com/openPupil/Open-PupilEXT

# Pupil detection

Currently the software integrates multiple state of the art open-source pupil detection algorithm available from recent publications. PupilEXT contains the pupil detection algorithms ElSe [[1\]](#1), ExCuSe [[2\]](#2), PuRe [[3\]](#3), PuReST [[4\]](#4), Starburst [[5\]](#5), and Swirski2D [[6\]](#6).

# Camera emulation

For debugging porpuses, the Pylon SDK supports emulating camera devices that are then displayed as physical cameras in the PupilExt software. To activate the camera emulation, the system environment variable "PYLON_CAMEMU" needs to be set. The number of available emulator devices can be controlled by exporting the PYLON_CAMEMU environment variable. For example, ``export PYLON_CAMEMU=2``.

# References

<a id="1">[1]</a>  Dongheng Li and Derrick J. Parkhurst. Starburst: A robust algorithm for video-based eye tracking. IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR), September 2005.

<a id="2">[2]</a> Lech Swirski, Andreas Bulling, and Neil Dodgson. Robust real-time pupil tracking in highly off-axis images. In Proceedings - 2012 ACM Symposium on Eye Tracking Research and Applications (ETRA), pages 173–176, 2012.

<a id="3">[3]</a> Wolfgang Fuhl, Thomas Kübler, Katrin Sippel, Wolfgang Rosenstiel, and Enkelejda Kasneci. Excuse: robust pupil detection in real-world scenarios. In International Conference on Computer Analysis of Images and Patterns, pages 39–51. Springer, 2015.

<a id="4">[4]</a> Wolfgang Fuhl, Thiago C. Santini, Thomas Kübler, and Enkelejda Kasneci. ElSe: Ellipse selection for robust pupil detection in real-world environments. In Proceedings - 2016 ACM Symposium on Eye Tracking Research and Applications (ETRA), volume 14, pages 123–130, 2016.

<a id="5">[5]</a> Thiago Santini, Wolfgang Fuhl, and Enkelejda Kasneci. PuRe: Robust pupil detection for real-time pervasive eye tracking. Computer Vision and Image Understanding, 170:40–50, 2018.

<a id="6">[6]</a> Thiago Santini, Wolfgang Fuhl, and Enkelejda Kasneci. PuReST: Robust pupil tracking for real-time pervasive eye tracking. In Proceedings - 2018 ACM Symposium on Eye Tracking Research and Applications (ETRA). ACM, 2018.

<a id="7">[7]</a> Thiago Santini, Wolfgang Fuhl, David Geisler and Enkelejda Kasneci. EyeRecToo: Open-source Software for Real-time Pervasive Head-mounted Eye Tracking. VISIGRAPP 2017.

# License

PupilEXT is available under the GPL License (see [LICENSE](LICENSE)). Note that third-party libraries used in this project may be distributed under other open source licenses.

[**QT**](https://www.qt.io/) is an open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms such as Linux, Windows, macOS, Android or embedded systems. (License: GPL2.0/3.0 & LGPL 3.0)

[**OpenCV**](https://opencv.org/) is a highly optimized computer vision library with focus on real-time applications. In this repository it is used for image manipulation and plotting of ellipse pupil detections. (License: Apache 2 / BSD)

[**Basler Pylon**](https://www.baslerweb.com/en/products/software/basler-pylon-camera-software-suite/pylon-open-source-projects/) is a software package consisting of an easy-to-use SDK as well as drivers and tools that let you operate any Basler camera on a PC with a Windows, Linux, or macOS operating system. (License: Not specified)

[**QCustomPlot**](https://www.qcustomplot.com/) is a Qt C++ widget for plotting and data visualization. It has no further dependencies and is well documented. (License: GPL 3.0)

[**EyeRecToo**](https://github.com/tcsantini/EyeRecToo) is an open-source eye tracking software for head-mounted eye tracker and integrates the most advanced state-of-the-art open-source pupil detection algorithms.  We used the implementation of the EyeRecToo’s pupil class and the integrated detection methods for PupilEXT. (License: Copyright (c) 2018, Thiago Santini / University of Tübingen). **License:** For academic and non-commercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/LICENSE)).

**[PuRe:](https://www.sciencedirect.com/science/article/abs/pii/S1077314218300146?via%3Dihub)** Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, PuRe: Robust pupil detection for real-time pervasive eye tracking. *Computer Vision and Image Understanding*. **2018**, ISSN 1077-3142. https://doi.org/10.1016/j.cviu.2018.02.002. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/PuRe.h)).

**[PuReST:](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h)** Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci. PuReST: Robust pupil tracking for real-time pervasive eye tracking. *Symposium on Eye Tracking Research and Applications (ETRA)*. **2018**. https://doi.org/10.1145/3204493.3204578. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h)).

**[ElSe:](https://dl.acm.org/doi/10.1145/3204493.3204578)** Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci. ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments. *ETRA 2016 : Eye Tracking Research and Application.* **2016.** Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ElSe.h)).

**[ExCuSe:](https://link.springer.com/chapter/10.1007/978-3-319-23192-1_4)** Wolfgang Fuhl, Thomas Kübler, Katrin Simpel, Wolfgang Rosenstiel, Enkelejda Kasneci. ExCuSe: Robust Pupil Detection in Real-World Scenarios. *CAIP 2015 : Computer Analysis of Images and Patterns*. **2015**. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ExCuSe.h)).

**[Starbust:](https://ieeexplore.ieee.org/document/1565386)** Dongheng Li, Winfield, D., Parkhurst, D. J. Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches. in *2005 IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR’05) - Workshops* vol. 3 79–79 (IEEE, 2005). https://doi.org/10.1109/CVPR.2005.531. Based on the [cvEyeTracker](https://github.com/thirtysixthspan/cvEyeTracker) Version 1.2.5 implementation. **License:** GNU General Public License ([Link](https://github.com/thirtysixthspan/cvEyeTracker/blob/master/ransac_ellipse.cpp))

**[Swirski2D:](https://dl.acm.org/doi/10.1145/2168556.2168585)** Lech Swirski, Andreas Bulling, Neil Dodgson. Robust real-time pupil tracking in highly off-axis images. *ETRA 2012: Proceedings of the Symposium on Eye Tracking Research and Applications*. **2012**. https://doi.org/10.1145/2168556.2168585.  **License:** MIT License, Copyright (c) 2014 Lech Swirski ([Link](https://github.com/LeszekSwirski/pupiltracker/blob/master/LICENSE.md))