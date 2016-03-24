# Mission Control

This program has the following dependencies

* Qt 5.6 Framework
* libVLC
* VLC-Qt
* GLFW3
* Roboto font family

On Ubuntu, these can be installed easily

    add-apt-repository ppa:ntadej/tano
    apt-get update
    apt-get install vlc libvlc-dev libglfw3-dev fonts-roboto libvlc-qt-*

Ubuntu comes with Qt 5.X installed, however it is unlikely that this project will run on this pre-installed environment. This is because Qt very recently integrated the Blink web engine (which this project depends on) into its SDK to replace WebKit and no distro currently ships these newer libraries (16.04 might). Replacing a distro's Qt framework is serious bananas, and still I'm working to figure out the best way to do this.

Of course, running this using the Qt 5.6 SDK works fine, but installing the 5.6 SDK does not replace a disto's Qt framework. It just installs it to another directory and points the linker there at runtime.
