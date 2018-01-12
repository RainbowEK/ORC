# ORC: OBS Remote Control (Qt Creator C++) 

This program is a windows client for the OBS Studio "web-socket" plug-in.
Is is written in C++, with Qt Creator (Qt5.9.2 - MinGW 32bit).

**Disclaimer:**
*This is developed for my own personal use, by myself. Others are freely allowed to use the code and/or build upon their own liability and responsibility without any guarantee or support. I claim nothing and herewith disclaim everything.*

**Install and Run:**
Just store the run directory at your own convenient location.
Configuration settings are in the config.json file, including the description.
Start ORC.exe (is a Windows 32bit app) and that's it.

**Build/Compile:**
Open the ORC.pro file with Qt Creator. Build from there. I used kit "Qt5.9.2 - MinGW 32bit"
You can start the application via "ORC_QT". 
However if you make a shortcut to "ORC_QT" and use the shortcut to start the app, than the taskbar icon will not be animated.
Therefor I used a .bat file (in which ORC_QT is started) and converted that to an exe file (used freeware tool from http://www.f2ko.de/en/b2e.php). This is the ORC.exe file in the install folder for example. When you make shortcuts to that file and start the application via the shortcut, the taskbar icon will be animated.

**Screenshots:**
See below... (located in ScreenShot folder)

08-Jan-2018

Fix included per 08-Jan-2018: The ORC.exe file was not executable on other PC's than my own. This had to do with license restriction.

Have Fun!

Regards,
RainbowEK.

![](file:./screenshots/MainWindow.PNG)

![](file:./screenshots/DebugWindow.PNG)