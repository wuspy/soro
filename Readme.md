# Sooner Rover
 
![img](http://www.okepscor.org/sites/default/files/ou%20logo%20for%20resources%20page.jpg)

This is the repository for the 2015-16 Sooner Rover team.

## Dependencies

### Qt 5.4+

Qt is the main development platform for our code, with the exception of mbed programs. Our minimum version is 5.4, because the embedded Google Maps requires the increased performance of the Blink web engine which was introduced in this version. However, Qt 5.7 is the target version and this should be used if possible.

### Gstreamer 1.0

Gstreamer functions as our media stack, and is used to encode and decode audio and video.

### Qt5Gstreamer

This is a small library which helps bind Gstreamer's API's to a Qt application.

### SDL

SDL (Simple DirectMedia Layer) is a development toolkit designed primarily for making OpenGL games, but we use it to read from joysticks and gamepads.

### Point Grey Flycapture SDK

*This library is currently unused and is not required, but may return in the future if we switch back to using our Point Grey cameras.*

This is required to interface with Point Grey cameras.

## Quick Start

### Ubuntu 16.04

*This version of Ubuntu ships with Qt 5.5, which can be used to develop this software, HOWEVER, the Qt WebEngine module is not included or easily obtainable. Therefore, a separate Qt environment is required for development.*

Download  and install Qt and the QtCreator IDE from [here](https://www.qt.io/download-open-source/#section-2).

Install the following dependencies:

    apt install -y build-essential git gstreamer1.0-* libqt5gstreamer-* libsdl2-*

Clone this repository. Since this is a private repository, you will be prompted to enter your username and password.

    git clone https://github.com/doublejinitials/soro

Open QtCreator, then open the project you wish to work on or compile from the location you cloned this repository to.

### OSX

I have no idea. Installing Qt will be the same process, but the other dependencies I'm not sure. Also, some code in this repository is Linux specific and will need to be written for OSX (ex. using video4linux as a gstreamer source, enumerating /dev/video* to search for webcams). All of this code should be wrapped in #ifdef \_\_LINUX\_\_ tags for easy searching.

### Windows

Again, no idea. See above.

## License

Copyright 2016 The University of Oklahoma

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
