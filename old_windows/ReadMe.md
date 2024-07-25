#❄ Camera Control Code ❄#

This repository contains the C++ code that controls the cameras of the Snowflake Sensing System (SSS). This code, as it says in the name, controls all features related to the cameras. You run this code to have this system begin image capturing. See the docstrings in each included module for more documentation

##Camera Control Code (SMASSystem.cpp)##


This is currently the only module inside of the camera-control 
code for the SSS. The beginning of this module initiziales every 
camera parameter. It then then handles the capturing, retrieving, 
and saving of images from each camera to the hard drives. 

To use, do the following:

- Open Microsoft Visual Studio
- Open the file `SMASSystem.cpp`
- Change destination file path on line 649 "E:\\<foldernamehere>"
- Run the code by clicking the `Local Windows Debugger` button on the top of Visual Studio.
- You will see a command window open and a bunch of command console outputs. These are the cameras being setup.

---

###**To Exit**###

If you do not follow these instructions to exit the program, the system will crash and require a full power cylce.

To exit:

- Type the characters `exit` and then hit enter.
- Then hit abort on the popup window

*NEVER HIT THE STOP BUTTON ON THE TOP OF VISUAL STUDIO*

---

###More Detail on the Actual Code###

In this program, you will find some complex parallel programming 
algorthims. This section will breifly explain what the code is doing. 
Follow comments in the code for more details. 

There is a master updateAtomics thread that will be in a 
continuous while loop throughout the time while the program is running.
For each individual camera thread, we look at the images captured since save. If that number is 2, 
a camera has been "lapped" and then we set droppedFrame to true, and allowSave as false.
If droppedFrame became true, we shift each of the images captured since save back one in order to reset 
the count. If save is set to true, we change the boolean allowSave to be true and 
then we save the photo. 

As mentioned in the previous paragraph, we spawn individual threads that perfrom the 
images captured since save function.Each of these threads has access to their own images 
captured since save and the updateAtomics thread keeps track of all of these to decide when 
to save a photo and when not to. 
