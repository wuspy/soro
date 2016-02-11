# Sooner Rover
 
![img](http://www.okepscor.org/sites/default/files/ou%20logo%20for%20resources%20page.jpg)

##Table of Contents

- **soro**  
The main shared library that will exist on both the rover and mission control. All shared code between the two should go here. Includes the communication functionality.

- **soroui**  
Shared library used by mission control programs to help style their interfaces. Requires libsoro.

- **ArmTestGui**  
Used for testing the arm (no internet). Requires libsoro, libsoroui, libglfw.

- **ArmMissionControl**  
The mission control interface for the arm controller (internet based). Requires libsoro, libsoroui, libglfw.   

- **ArmNetworkInterface**  
The rover-side program for receiving arm commands. Requires libsoro.

###Kevin, if you actually want to work on this stuff:

- Download QtCreator IDE, no other IDE will work (easily).
- Download the entire repo. The projects are somewhat interdependent.
- Build GLFW. You will get link errors on the projects that require it, so look at the *.pro files and add/correct the include and lib paths for unix.
- Try out ArmTestGui first to make sure everything works.
- Ask me for help when you give up trying to figuring out the Channel class.
