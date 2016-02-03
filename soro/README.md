# libsoro

This Qt project contains the code for the libsoro shared library, designed to run on both the rover and mission control equipment.

## Channel

The Channel class provides message based communication between two internet endpoints, each using their own properly configured channel object.



## Logger

The Logger class is a simple event log used for debugging purposes that outputs an HTML file. All classes in libsoro should use it.
