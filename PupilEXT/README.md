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
