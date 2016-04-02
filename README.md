#xlaunch 
runs a GUI app, prints the X window-id of the app to stdout, then exits.  useful for getting a handle on a specific instance of an app in order to embed it into another window.

#build

to build and install xlaunch, type:

 $ make xlaunch 
 
 $ sudo make install

(this will install xlaunch to /usr/local/bin)

try

 $ make help

and

 $ make options

for information on reconfiguring the build process via environment variables.


#examples

 $ xlaunch firefox

 $ xlaunch gedit *.c

 $ xlaunch xdg-open /path/to/an/unusual/file


to disable the 'xlaunch is waiting...' animation, try:

 $ xlaunch ____ 2>/dev/null 


to disable the stdout and stderr of the GUI app, but not the output of xlaunch, try:

 $ xlaunch sh -c "____ *.c > /dev/null 2>&1"  


#caveats 
xlaunch just waits for the next new window to be created.  try to avoid starting several programs simultaneously while using xlaunch, incase it mistakenly outputs the ID of the wrong window.

apps can take a long time to appear.  Freedesktop.org Startup ID Specification suggests allowing 15 seconds for an app to show itself.  xlaunch waits 15 seconds and reports failure if no new windows were found, but the app might still pop-up later.

apps with splash screens, or more than one initial window (like gimp), will trip up xlaunch.


#license 
xlaunch is stitched together from the internals of xprop, so inherits its license. 


#future work
have xlaunch ignore new override_redirect windows (which will typically be splash screens)

have xlaunch follow the Freedesktop.org Startup ID Specification, so the window-manager can change the mouse cursor to 'busy' until the app has launched.

