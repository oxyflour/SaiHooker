This is a helper html application(HTA) for PaintTool SAI on Windows tablets

##Features
###Unwanted finger touches or palm contacts blocked
This app injects a WH\_GETMESSAGE hook(HookDll.dll) into the SAI process, thus it's able to check and block messages produced from touch. By default these touch messages are blocked only when a digitizer pen is detected on the screen.

###Better touch gestures for SAI
Default Windows touch gestures are disabled, because they are not compatible with SAI (for example, Windows produces CTRL+MOUSE\_WHEEL messages when you're zooming, but SAI uses PageUp/PageDown to zoom instead). To make better use of the touch screen, Single-finger Pan and Two-finger Zoom/Rotate is implemented in this app.

###Extra toolbar window
The HTA (index.hta) is actually a simple html file...

You can modify the HTA file to satisfy your needs, or even create new ones if you like.


###Extra popup menu
BACKSPACE key is hooked in this app. Click on BACKSPACE sends CTRL-Z(Undo) to SAI, while long press on the key will popup a menu where the digitizer pen hovers. This action is defined in the index.hta with 'OnVirtualKey' Event, and the menu can be customized by modifing the 'popup' xml. It will become useful if your pen has at least one button and you assign BACKSPACE key to the button in the driver setting panel.

###Mouse Gesture
Press BACKSPACE and move your pen to use this feature. The actions of the pen movement is also connected to the 'popup' xml with the 'gst' attribute (u=up, U=up-right, r=right, R=right-down, d=down, D=down-left, l=left, L=left-top).


##How to use
###Install/Uninstall
Download everything in the latest\_build folder and run install.bat/uninstall.bat as administrator. Check if there is any error message.

###Run
Double click index.hta to run this app. It will look for SAI and hook it automatically.


###License
May I leave this blank?
