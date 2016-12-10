# Sooner Rover
 
![img](http://www.okepscor.org/sites/default/files/ou%20logo%20for%20resources%20page.jpg)

This is the repository for the 2015-16 Sooner Rover team.

This codebase is still under active development as it is being used for a teleoperation research project. [Here is the commit](https://github.com/doublejinitials/soro/tree/1b0f66f3ee376ff07cd3b8e10c90ad4633ec3a63) where you can see the state of the code as it was for the 2016 NASA/NIA Robo-Ops competition.

## Dependencies

### Qt 5.7+

Qt is the main development platform for our code, with the exception of mbed programs. The minimum version is 5.7 because the research control project is based on QtQuick Controls 2.

### Gstreamer 1.X

Gstreamer functions as our media stack, and is used to encode and decode audio and video.

### Qt5Gstreamer

This is a small library which helps bind Gstreamer's API's to a Qt application.

### SDL

SDL (Simple DirectMedia Layer) is a development toolkit designed primarily for making OpenGL games, but we use it to read from joysticks and gamepads.

### Point Grey Flycapture SDK

*This library is currently unused and is not required, but may return in the future if we switch back to using our Point Grey cameras.*

This is required to interface with Point Grey cameras.

## Compiling

This software will only compile and run on linux.

No current linux distribution, with the possible exception of Arch/Manjaro with KDE, ships the Qt5WebEngine module which is a dependency of this software. You should have full a Qt development environment set up, and should deploy Qt libraries with this software.

On Debian/Ubuntu/Mint, all the dependencies can be installed with

    apt install libsdl2-dev gstreamer1.0-* libqt5gstreamer-dev

On Arch/Manjaro

    pacman -S sdl2 gstreamer gst-libav gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly qt-gstreamer

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
