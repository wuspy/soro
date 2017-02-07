## Copyright 2016 The University of Oklahoma.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

TEMPLATE = subdirs

SUBDIRS =\
    libsoro \
    audio_streamer \
    video_streamer \
    rover \
    rover2 \
    libsoromc \
    mission_control \
    research_control \
    research_rover \
    libsorogst \
    SoroTests

libsorogst.depends = libsoro
libsoromc.depends = libsoro libsorogst
video_streamer.depends = libsoro libsorogst
audio_streamer.depends = libsoro libsorogst
rover.depends = libsoro
rover2.depends = libsoro
research_rover.depends = libsoro libsorogst
mission_control.depends = libsoro libsoromc libsorogst
research_control.depends = libsoro libsoromc libsorogst
SoroTests.depends = libsoro libsoromc libsorogst
