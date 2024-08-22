
## Build environment troubleshooting notes (FAQ):

For building and development using Visual Studio Code

---
---
### Any operating system:

---
---

PROBLEM: 

Miscellaneous

SOLUTION:
- If in VS Code: Run VS Code with administrator rights.
- Check Open-PupilEXT/CMakeLists.txt file contents (not Open-PupilEXT/src/CMakeLists.txt). 
- If in VS Code: Check .vscode/settings.json. If there is only a settings.json.default, as you just freshly cloned the repository to your machine, then you firstly need to make a copy of that file and rename it to settings.json, and tailor that to your system and CMake path as you can see that in comments.
- Check whether Basler Pylon installation directory (headers folder of Development folder, and x64 and Win32 folders od Runtime folder) are added to System Path variable.
- Ensure that you have the right Qt version installed. (Has to be in unison with the version defined in Open-PupilEXT/CMakeLists.txt file. If the version mentioned here does not accord to the one you have on your computer, do not modify the version in the CMakeLists.txt, but install the right version. Upgrading Qt version of the project is a delicate matter, and needs thorough refactoring and testing.)

---
PROBLEM:

Vcpkg does not build a dependency for whatever reason.

SOLUTION:
- Check that it is not occuring due to a misconfiguration in your settings.json (If in VS Code) or your Qt or Pylon installation, Path variables, etc.
- Note that VCPKG is a "submodule" in our git repo, so in case you cloned the Open-PupilEXT project with the recurse-submodules option (which we recommend) then a specific version of VCPKG (which is anchored to a certain commit of the VCPKG repo) will get downloaded, and all the dependencies that it wants to build during the configure step of the PupilEXT build process, will accord to the timepoint that this VCPKG commit designates. If we forget to update this submodule reference to a newer commit for a long time, it can happen that VCPKG and dependencies known by it will get outdated, so that they can not be downloaded anymore. To solve this, you can either refresh the reference to the submodule and commit that so people who will eventually clone PupilEXT will not have to face this issue again, or you can locally solve this problem on your very machine, by using:
"git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.bat
vcpkg integrate install"
- Wipe VCPKG output by deleting Open-PupilEXT/build/vcpkg_installed directory (can be huge, ~6 GB) and perform VCPKG install by opening a command window

---
PROBLEM (If in VS Code):

"CMake Error at CMakeLists.txt:103 (find_package):
Could not find a configuration file for package "Whatever" that is
compatible with requested version ""."

SOLUTION:
- Select the right kit regarding CPU architecture.

---
PROBLEM:

I added a new header or cpp file to the codebase, but it does not get compiled.

SOLUTION:
- Has to be told to CMake to get them compiled, and also for IntelliSense to see them, by addig the new file names to: Open-PupilEXT/src/CMakeLists.txt

---
PROBLEM (If in VS Code):

Project builds but does not run.

SOLUTION:
- Check whether you built with the proper kit selected / so that you target the machine you are using. (Likely the one with x86_amd64 will work.)

---
PROBLEM:

I added a new dependency of a Qt library, but even if I included headers (e.g. #include &lt;QWhatever&gt;), the code does not compile.

SOLUTION:
- Has to be told to CMake to get them compiled by addig reference to the library into Open-PupilEXT/CMakeLists.txt by extending the line beginning with "find_package(Qt5 COMPONENTS ..." AND into Open-PupilEXT/src/CMakeLists.txt by extending the line beginning with "Qt5::Widgets Qt5::Concurrent  ...". (This is because we use CMake here. If we didn't, of yource it would be enough to add "QT += whatever"  to the Qt .pro project file.)
- Also ensure that the library can be added to the project without any license violation.
- (Windows-specific:) The library also likely uses a .dll file from Qt, for release build a QtWhatever.dll and for debug build a QtWhateverd.dll. These also need to be copied to the Open-PupilEXT/build/src/Release and Open-PupilEXT/build/src/Debug from the Qt compiled library collection, found under the folder where you installed Qt.

---
---
### Windows-specific:

By default, in case if you want to build it on an x64 PC using MSVC.

---
---

PROBLEM (If in VS Code):
 
Miscellaneous:

SOLUTION:
- Ensure you have installed VS Code using the "System installer" and not the "User installer".

---
PROBLEM:

Basler Pylon references cannot be found thus compiling or linking fails. E.g.
"LINK : fatal error LNK1104: cannot open file 'GCBase_MD_VC141_v3_1_Basler_pylon.lib'"
or "'pylon/PylonIncludes.h': No such file or directory".

SOLUTION:
- Ensure that the right Pylon version is installed on your computer (which has to be findable by cmake/FindPylon.cmake).
- Ensure that Pylon was installed for "Developer" use.
- Ensure that Pylon installation directory is added to the System Path variable.

---
PROBLEM:

I want to clear the contents of Qt persistent storage (/Application Settings /QSettings) to get a fresh start.

SOLUTION:
Navigate to the .ini file that is indicated at the bottom of the first page of the About dialog in PupilEXT, and delete files in there. They will be created upon next run of PupilEXT.exe

---
PROBLEM:

Vcpkg cannot unpack the automatically downloaded files, because it does not find a 7zip executable anywhere.

SOLUTION:
You can simply manually download a portable 7zip executable and put it in the vcpkg download folder

---
PROBLEM (If in VS Code):

Vcpkg does not find build tools components / halts on searching for msvc.

SOLUTION:
- Ensure that you have downloaded and installed VS Build Tools 2017 or higher, and its components "Build Tools for CMake", "Windows SDK" (pick your version) and "MSVC" (pick a version at least v141) on your computer.
- Ensure that the installer correctly added the build tools location to the System Path variable (may need reboot). If not, add it manually (something like this: C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools).

---
PROBLEM (If in VS Code, on Windows):

There is nothing under "kits" in VS Code, so I cannot select a right kit for me, e.g. "Visual Studio Community 2017 x86_amd64".

SOLUTION:
- Ensure that you have downloaded and installed VS Build Tools 2017 or higher, and its components "Build Tools for CMake", "Windows SDK" (pick your version) and "MSVC" (pick a version at least v141) on your computer.
- Ensure that the installer correctly added the build tools location to the System Path variable (may need reboot). If not, add it manually (something like this: C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools).

---
PROBLEM (Windows-specific):

Program compiles, but does not start, beacause .dll files are missing.

SOLUTION:
- Upon building, no dll files are created. The release version of them can be retreived from the last release of PupilEXT, but for the debug version .dll's (the ones ending with a d in their name) you need to find and copy them from your Qt installation.
The libraries and Qt components that PupilEXT uses use .dll files from Qt, for release build a QtWhatever.dll and for debug build a QtWhateverd.dll. These need to be copied to the Open-PupilEXT/build/src/Release and Open-PupilEXT/build/src/Debug from the Qt compiled library collection, found under the folder where you installed Qt.

---
---

## Linux-specific: 
By default, in case if you want to build it on an x64 PC using G++.

---
---

PROBLEM:

Build does not start because CMake, Ninja, or gcc, g++ are missing, or spurious "cannot find" problems arise.

SOLUTION:
- Ensure that the necessary packages exist on your system:
"sudo apt install cmake ninja-build gcc g++ build-essential curl zip unzip tar nasm mesa-common-dev libglu1-mesa-dev"
- Ensure that cmake path is correctly set in settings.json (to find out e.g. where CMake is located on your machine, just type "which cmake" in the terminal)

---
PROBLEM: 

While building, the IDE does not find the package TIFF

SOLUTION:

Install the needed package by typing this in the terminal:
`sudo apt install libtiff-dev`

---
PROBLEM:

CLion crashes upon opening the project

SOLUTION:

It is very likely that it cannot render the README.md markdown file.
You can just very fast click on the X close button on the README.md file tab in the workspace before it tries to load it, for the first time
and to permanently solve it, disable the markdown plugin in settings, among installed plugins
If your Clion has already memorized that your last opened project was this one, and upon opening the application it tries to open it again, leading to crash, please resolve this problem by clean restarting CLion after disabling auto project reopening feature. A working solution is to:
- 1. Open the file ide.general.xml in ~/.config/JetBrains/CLionXXXX.X/options
- 2. Add `<option name="reopenLastProject" value="false" /> to <component name="GeneralSettings">`
- 3. Save changes
- 4. Start CLion again

(Source of solution to clearing auto project open: https://youtrack.jetbrains.com/issue/CPP-21985/CLion-crashes-when-reopening-last-projects-Linux#focus=Comments-27-4371764.0-0, last accessed: 2024.08.14.)

---
---

## MacOS-specific:

By default, in case if you want to build it on an x64 Mac using Clang.

---
---

PROBLEM:

Build does not start because CMake or Ninja are missing, or spurious "cannot find" problems arise.

SOLUTION:
- Ensure that the necessary packages exist on your system, and download them if not (e.g. using: `brew install ninja nasm pkg-config cmake`)
- Ensure that cmake path is correctly set in settings.json (to find out e.g. where CMake is located on your machine, just type "which cmake" in the terminal)
- Note that there may be a Clang supplied with your XCode already, also one that is installed alongside Qt if you put a checkbox for that in the Qt downloader manager.

---

PROBLEM:

Qt installer is stuck because it cannot fetch package data.

SOLUTION:

Add a new mirror and let it find there, e.g. using: `open -a qtinstall.app --args --mirror https://ftp.fau.de/qtproject/` 



