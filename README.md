2024-07-31
		SMAS Code Repository
		--------------------

The CSU Snowflake Measurement and Analysis System (SMAS).

GitHub:  https://github.com/hthant/smas_new

Files:
------
 v  README.md			this file
 v  flycapture_codebase/	?? some  .sln.lnk  files ??

    flycapture_inc@		symlink to FlyCapture2 include files
				    (local symlink for portability)

 v  old_windows/		old SMASSystem.cpp code for Windows
 v  onecam/			Single Camera test program, initial Linux port

 v = in Git

Dependencies:
-------------
FlyCapture2
    /usr/include/flycapture/	include files
    /usr/lib/flycapture/	lib files ??

    This is the API we are using.  It no doubt has other libraries
    that it depends on (i.e. for linking our code).


Meeting Notes:
--------------
08/01/2024 - things to do:
- makefile, and linking to include files
- review the code
- try to see if the executable runs correctly on linux
total 20K
drwxrwxr-x 2 snow snow 4.0K Aug  1 15:40 flycapture_codebase
lrwxrwxrwx 1 snow snow   23 Aug  1 15:46 flycapture_inc -> /usr/include/flycapture
lrwxrwxrwx 1 snow snow    8 Aug  1 16:58 flycapture_lib -> /usr/lib
drwxrwxr-x 2 snow snow 4.0K Aug  1 16:17 MultipleCameraEx
drwxrwxr-x 2 snow snow 4.0K Aug  1 15:40 old_windows
drwxrwxr-x 5 snow snow 4.0K Aug  1 16:59 onecam
-rw-rw-r-- 1 snow snow  898 Aug  1 15:40 README.md
