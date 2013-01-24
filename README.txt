Motion Capture Viewer
CM20219 - Fundamentals of Computer Graphics

The project consists of parsing and rendereing motion-capture data in 3D using OpenGL and C.


Running the solution
--------------------

From a command line run: mocaptest <asf file> <amc file>

The executable has been tested on Windows XP only.  Other OS are not officially supported.

Troubleshooting
----------------

1) GLUT32.DLL not found
Copy the GLUT32.DLL in the archive into your C:\WINDOWS directory.

2) Incorrect use of program
If you just glimpse a black box when running mocaptest.exe, and perceive no other effect, then this is because you are double-clicking the EXE file rather than running it from the command line as intended.

Suppose you unzip the archive into c:\mocap.  You need to run cmd.exe from the start menu (Start->run->cmd.exe).
Then type 'cd c:\mocap' without the quotes before executing the command with parameters e.g. mocaptest walk.asf walk.amc