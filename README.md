This is a helper html application(HTA) for PaintTool SAI on Windows tablets

##Features
###Unwanted finger touches or palm contacts blocked
This app injects a WH\_GETMESSAGE hook(HookDll.dll) into the SAI process, thus it's able to check and block messages produced from touch. By default these touch messages are blocked only when a digitizer pen is detected on the screen.

###Better touch gestures for SAI
Default Windows touch gestures are disabled, because they are not compatible with SAI (for example, Windows produces CTRL+MOUSE\_WHEEL messages when you're zooming, but SAI uses PageUp/PageDown to zoom instead). To make better use of the touch screen, Single-finger Pan and Two-finger Zoom/Rotate is implemented in this app.

###Extra toolbar window
The HTA (index.hta) is actually a simple html file...

You can modify the HTA file to satisfy your needs (make it looking better, add more buttons, etc.), or even create new ones if you like.

###Multi-finger Tap event
By default a two-finger tap popups a context menu defined in the 'popup' xml in index.hta, and a three-finger tap toggles fullscreen for SAI window (F11). See 'OnFingerTap' event in index.hta if you want different actions.

###Ctrl+Alt Actions and Pen Gesture
Shortly press Ctrl+Alt without moving the pen sends CTRL-Z to SAI, which is useful if your pen has at least one button and you assign Ctrl+Alt to that button in the SAI setting panel. This action is also defined in the index.hta by 'OnVirtualKey' event and thus can be easily modified.
Hold on Ctrl+Alt and move your pen to use predefined pen gestures. The actions of the pen movement is connected to the 'popup' xml in index.hta with the 'gst' attribute (u=up, U=up-right, r=right, R=right-down, d=down, D=down-left, l=left, L=left-top).


##How to use
###Install/Uninstall
Download everything in the latest\_build folder and run install.bat/uninstall.bat as administrator. Check if there is any error message.

###Run
Double click index.hta to run this app. It will look for SAI and hook it automatically.


###License
May I leave this blank?
