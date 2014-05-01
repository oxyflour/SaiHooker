This is a helper html application(HTA) for PaintTool SAI on Windows tablets

##Features
###Unwanted finger touches or palm contacts blocked
This app injects a WH\_GETMESSAGE hook(HookDll.dll) into the SAI process,
thus it's able to check and block messages produced from touch.
By default these touch messages are blocked only when a digitizer pen is detected on the screen.

###Better touch gestures for SAI
Default Windows touch gestures are disabled,
because they are not compatible with SAI
(for example, Windows produces CTRL+MOUSE\_WHEEL messages when you're zooming with your fingers,
but SAI uses PageUp/PageDown to zoom instead).
To make better use of the touch screen,
Single-finger Pan and Two-finger Zoom/Rotate is implemented in this app.
This behaviour is defined in the OnTouchGesture event in the hta file.
Also, multi-finger tap behaviours are defined there
(single finger double tap: pick color,
double finger tap: popup menu,
three finger tap:
toggle fullscreen,
four finger tap: display desktop).

###Extra toolbar window
The HTA (index.hta) is actually a simple html file.
The window have been brought to topmost,
so you can use it as an extra toolbox.
There are now four checkboxes in the window currently.

You can modify the HTA file to satisfy your needs
(make it looking better, add more buttons, etc.),
or even create new ones.

###Right mouse button Actions and Pen Gesture
The right mouse button (RMB) is hooked.
Press RMB and release it in 500ms without moving the cursor sends CTRL-Z to SAI,
which is useful if your pen has at least one button and
you assign RMB to that button (this is the default setting on most tablets).
This action is also defined in the index.hta via OnMouseGesture event.

Hold on RMB and move your pen to stroke mouse gestures.
The actions of the pen movement is connected to the 'popup' xml in index.hta via the 'gst' attribute
(u=up, U=up-right, r=right, R=right-down, d=down, D=down-left, l=left, L=left-top).

What's more, some more complex are defined with the RMB and LMB.
For example, hold on RMB and move cursor up to stroke an 'up' gesture,
then perform a LMB click before releasing the RMB.
Keep holding one of the LMB or RMB and move the cursor up/down to Zoom in/out, left/right to rotate.


##How to use
###Install/Uninstall
Download everything in the latest\_build folder and run install.bat/uninstall.bat as administrator.
Check if there is any error message.

###Run
Double click index.hta to run this app. It will look for SAI process and hook to it automatically.


###License
MIT