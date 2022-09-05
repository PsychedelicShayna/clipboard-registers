# Clipboard Registers
This is a Qt console application for Windows that grants you 46 additional buffers you can use to store clipboard data into. The data is accessible via a console window that overlays your foreground window when you double tap right control (toggles show/hide). You can use a-z, 1-9, and kp_1-kp_9 as registers. Data can be loaded from/to said registers via inputting shortcuts into the console overlay. I ripped this idea from Emacs.

As it currently stands, this program is quite buggy, and at some point in the future it is going to be re-written with a proper GUI. Is is technically usable, but isn't the most pleasant to use, and some of the functionality, such as copying files and or image data, does not work as intended. 

## Command Line Arguments
`clipboard-registers.exe --help`
```
--fullscreen-overlay (-f)      |    Overlay the console on the entire window, not the center.
--borderless (-b)              |    Disable the console window's titlebar.
--console-width (-cw)          |    Sets the center relative width of the console window, if fullscreen is disabled. Default: 500
--console-height (-ch)         |    Sets the center relative height of the console window, if fullscreen is disabled. Default: 200
--opacity (-o) [0-255]         |    Set the console opacity, valid range is 0 - 255.
--help (-h)                    |    This help message.
```

## Screenshot
![](screenshots/screenshot1.png?raw=true)
