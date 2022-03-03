# linux-macro-recorder
Linux keyboard and mouse macro recorder

What this is:
This is a quick hack or proof of concept to record and reply keyboard and mouse input in Linux. Tested in Ubuntu 20.04, automatic initialization of the devices might not work on other distributions as I just wrote quickly something that worked for me. The whole program is just 255 lines of code and should be relatively easy to understand for anyone who want to improve it. 

How is this different from other macro recorders:
- It requires root access, because it interacts direct with /dev/input/event* files that are readable by root only.
- It monitors both mouse and keyboard. Many macro recorders focus on keyboard only.
- Because it accesses the Linux input devices directly, it is really fast, much faster than normal macro recorders, especially written in Python that have several layers before they get access to the mouse and keyboard. You can even automate actions in 3rd person shooting games. But because it is just a quick hack, it is not perfect so trying to execute some very detailed movements can get timings wrong. The speed was the main motivation for writing this. There are many better macro-tools for those who don't need speed.

Possible improvements ideas that should all be quite easy to implement:
- Adding saving/loading from file
- Storing multiple macros at the same time
- Configurable keys and max size
- Adjusting repeat speed
- Repeating in a loop

Installation (it is just a single file, just compile it):
gcc recorder.c -o recorder

Starting ( /dev/input/event* files are readable for root only):
sudo ./recorder

Usage:
- Press F8 to start recording, F4 to end it.
- After recording, press F7 to repeat recorded events. Press any key from keybaord to abort execution.
- Press F9 to exit

Warning/Disclaimer:
- As I said before, this is just a quick hack. It is not properly tested, code reviewed or anything. As it is executed as a root, it has potential for causing direct harm or giving access to unauthorized people. 
