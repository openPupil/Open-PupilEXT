# PupilEXT Third-Party Licenses<br>

PupilEXT integrates several open source libraries. This document provides a list of the used libraries. The respective licenses of the libraries are provided as *.txt file in in the subfolder `3rdparty/PupilEXT_Third_Party_Licenses`.

### Table of Content

**List of Pupil Detection Libraries**
* [EyeRecToo](#EyeRecToo)
* [PuRe](#PuRe)
* [PuReST](#PuReST)
* [ElSe](#ElSe)
* [ExCuSe](#ExCuSe)
* [Starburst](#Starburst)
* [Swirski2D](#Swirski2D)
* [Swirski3D](#Swirski3D)

**List of Software Libraries**
* [QT](#QT)
* [QCustomPlot](#QCustomPlot)
* [OpenCV](#OpenCV)
* [Glog](#Glog)
* [Boost](#Boost)
* [Ceres-Solver](#Ceres-Solver)
* [Eigen](#Eigen)
* [Spii](#Spii)
* [Tbb](#Tbb)
* [Breeze-Icons](#Breeze-Icons)
* [Gflags](#Gflags)

## List of Pupil Detection Libraries

<a id="EyeRecToo" href="https://github.com/tcsantini/EyeRecToo"><b>EyeRecToo</b></a> is an open-source eye tracking software for head-mounted eye tracker and integrates the most advanced state-of-the-art open-source pupil detection algorithms.  We used the implementation of the EyeRecToo’s pupil class and the integrated detection methods for PupilEXT. (License: Copyright (c) 2018, Thiago Santini / University of Tübingen). **License:** For academic and non-commercial use only ([Link License](https://github.com/tcsantini/EyeRecToo/blob/master/LICENSE) | [Project Page](https://github.com/tcsantini/EyeRecToo)).

<a id="PuRe" href="https://www.sciencedirect.com/science/article/abs/pii/S1077314218300146?via%3Dihub"><b>PuRe</b></a> Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci, PuRe: Robust pupil detection for real-time pervasive eye tracking. *Computer Vision and Image Understanding*. **2018**, ISSN 1077-3142. https://doi.org/10.1016/j.cviu.2018.02.002. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/PuRe.h)).

<a id="PuReST" href="https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h"><b>PuReST</b></a> Thiago Santini, Wolfgang Fuhl, Enkelejda Kasneci. PuReST: Robust pupil tracking for real-time pervasive eye tracking. *Symposium on Eye Tracking Research and Applications (ETRA)*. **2018**. https://doi.org/10.1145/3204493.3204578. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-commercial purposes ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-tracking/PuReST.h)).

<a id="ElSe" href="https://dl.acm.org/doi/10.1145/3204493.3204578"><b>ElSe</b></a> Wolfgang Fuhl, Thiago Santini, Thomas Kübler, Enkelejda Kasneci. ElSe: Ellipse Selection for Robust Pupil Detection in Real-World Environments. *ETRA 2016 : Eye Tracking Research and Application.* **2016.** Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ElSe.h)).

<a id="ExCuSe" href="https://link.springer.com/chapter/10.1007/978-3-319-23192-1_4"><b>ExCuSe</b></a> Wolfgang Fuhl, Thomas Kübler, Katrin Simpel, Wolfgang Rosenstiel, Enkelejda Kasneci. ExCuSe: Robust Pupil Detection in Real-World Scenarios. *CAIP 2015 : Computer Analysis of Images and Patterns*. **2015**. Part of the [EyeRecToo](https://github.com/tcsantini/EyeRecToo) software. Copyright (c) 2018, Thiago Santini, University of Tübingen. **License:** For non-comercial use only ([Link](https://github.com/tcsantini/EyeRecToo/blob/master/EyeRecToo/src/pupil-detection/ExCuSe.h)).

<a id="Starburst" href="https://ieeexplore.ieee.org/document/1565386"><b>Starburst</b></a> Dongheng Li, Winfield, D., Parkhurst, D. J. Starburst: A hybrid algorithm for video-based eye tracking combining feature-based and model-based approaches. in *2005 IEEE Computer Society Conference on Computer Vision and Pattern Recognition (CVPR’05) - Workshops* vol. 3 79–79 (IEEE, 2005). https://doi.org/10.1109/CVPR.2005.531. Based on the [cvEyeTracker](https://github.com/thirtysixthspan/cvEyeTracker) Version 1.2.5 implementation. **License:** GNU General Public License ([Link](https://github.com/thirtysixthspan/cvEyeTracker/blob/master/ransac_ellipse.cpp))

<a id="Swirski2D" href="https://dl.acm.org/doi/10.1145/2168556.2168585"><b>Swirski2D</b></a>  Lech Swirski, Andreas Bulling, Neil Dodgson. Robust real-time pupil tracking in highly off-axis images. *ETRA 2012: Proceedings of the Symposium on Eye Tracking Research and Applications*. **2012**. https://doi.org/10.1145/2168556.2168585.  **License:** MIT License, Copyright (c) 2014 Lech Swirski ([Link](https://github.com/LeszekSwirski/pupiltracker/blob/master/LICENSE.md))

<a id="Swirski3D" href="https://www.cl.cam.ac.uk/research/rainbow/projects/eyemodelfit/"><b>Swirski3D</b></a> Lech Swirski, Neil Dodgson. A fully-automatic, temporal approach to single camera, glint-free 3D eye model fitting. *Proceedings of ECEM 2013*. **2013**.  **License:** MIT License, Copyright (c) 2014 Lech Swirski ([Link](https://github.com/LeszekSwirski/singleeyefitter/blob/master/LICENSE.md))

## List of Software Libraries

<a id="QT" href="https://www.qt.io/"><b>QT</b></a>  is an open-source widget toolkit for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms such as Linux, Windows, macOS, Android or embedded systems. (License: GPL 3.0)

<a id="QCustomPlot" href="https://www.qcustomplot.com/"><b>QCustomPlot</b></a> is a Qt C++ widget for plotting and data visualization. It has no further dependencies and is well documented. (License: GPL 3.0)

<a id="OpenCV" href="https://opencv.org/"><b>OpenCV</b></a> is a highly optimized computer vision library with focus on real-time applications. In this repository it is used for image manipulation and plotting of ellipse pupil detections. (License: Apache 2 / BSD)

<a id="Glog" href="https://github.com/google/glog"><b>Glog</b></a> is a library for logging. ([License](https://github.com/google/glog/blob/master/COPYING))

<a id="Boost" href="https://www.boost.org"><b>Boost</b></a> is a set of various C++ libraries for processing tasks. ([License](https://www.boost.org/users/license.html))

<a id="Ceres-Solver" href="http://ceres-solver.org"><b>Ceres-Solver</b></a> is a optimisation library. ([License](https://github.com/ceres-solver/ceres-solver/blob/master/LICENSE))

<a id="Eigen" href="https://eigen.tuxfamily.org/index.php?title=Main_Page#License"><b>Eigen</b></a> is a library for linear algebra. ([License](https://eigen.tuxfamily.org/index.php?title=Main_Page#License))

<a id="Spii" href="https://github.com/PetterS/spii"><b>Spii</b></a> is a library for optimisation. ([License](https://github.com/PetterS/spii/blob/master/LICENSE))

<a id="Tbb" href="https://github.com/oneapi-src/oneTBB"><b>Tbb</b></a> is for parallel programming. ([License](https://github.com/oneapi-src/oneTBB/blob/master/LICENSE.txt))

<a id="Gflags" href="https://github.com/gflags/gflags"><b>Gflags</b></a> is a library for comandline processing. ([License](https://github.com/gflags/gflags/blob/master/COPYING.txt))

## List of used graphics and icons

<a id="Breeze-Icons" href="https://github.com/KDE/breeze-icons"><b>Breeze Icons</b></a> is a set of icons. ([License](https://github.com/KDE/breeze-icons/blob/master/icons/LICENSE))

The following icon files are modifications of existing icons from the KDE Breeze icon set: 1cam1pup.svg, 1cam2pup.svg, 1cam1pup.svg, 1Mcam1pup.svg, 2cam1pup.svg, 2cam2pup.svg, 2cam2pupNS.svg, media-record-green.svg, camera-video-stereo.svg, crosshairs-gaze-calibration.svg, crosshairs-gaze-validation.svg. These modified files were made by contributor Gábor Bényei, and can be found in the icons folder as resources, licensed in accordance to KDE Breeze license (GNU LESSER GENERAL PUBLIC LICENSE Version 3).

The following icon files are the work of Gábor Bényei, and are declared public domain: computer-connection.svg, equals1b.svg, plus1b.svg, messageEmpty.svg, rs232.svg.

The OpenCV logo (OpenCV_Logo_with_text_svg_version.svg) is the work of Adi Shavit, under BSD license. License text and source: https://commons.wikimedia.org/wiki/File:OpenCV_Logo_with_text_svg_version.svg, last accessed 2024.08.16. 09:36

The Qt logo (Qt_logo_2016.svg) is the work of The Qt Project, and is public domain. License reference and source: https://commons.wikimedia.org/wiki/File:Qt_logo_2016.svg, last accessed 2024.08.16 09:36


