#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>

#include<unistd.h>
#include<fcntl.h>

#include<X11/Xlib.h>
#include <X11/Xatom.h>
#include<X11/cursorfont.h>

static Atom atom_wm_state = None;

/*
 * Check if window has given property
 */
static Bool
Window_Has_Property(Display * dpy, Window win, Atom atom)
{
    Atom type_ret;
    int format_ret;
    unsigned char *prop_ret;
    unsigned long bytes_after, num_ret;

    type_ret = None;
    prop_ret = NULL;
    XGetWindowProperty(dpy, win, atom, 0, 0, False, AnyPropertyType,
                       &type_ret, &format_ret, &num_ret,
                       &bytes_after, &prop_ret);
    if (prop_ret)
        XFree(prop_ret);

    return (type_ret != None) ? True : False;
}

/*
 * Check if window is viewable
 */
static Bool
Window_Is_Viewable(Display * dpy, Window win)
{
    Bool ok;
    XWindowAttributes xwa;

    XGetWindowAttributes(dpy, win, &xwa);

    ok = (xwa.class == InputOutput) && (xwa.map_state == IsViewable);

    return ok;
}

/*
 * Find a window that has WM_STATE set in the window tree below win.
 * Unmapped/unviewable windows are not considered valid matches.
 * Children are searched in top-down stacking order.
 * The first matching window is returned, None if no match is found.
 */
static Window
Find_Client_In_Children(Display * dpy, Window win)
{
    Window root, parent;
    Window *children;
    unsigned int n_children;
    int i;

    if (!XQueryTree(dpy, win, &root, &parent, &children, &n_children))
        return None;
    if (!children)
        return None;

    /* Check each child for WM_STATE and other validity */
    win = None;
    for (i = (int) n_children - 1; i >= 0; i--) {
        if (!Window_Is_Viewable(dpy, children[i])) {
            children[i] = None; /* Don't bother descending into this one */
            continue;
        }
        if (!Window_Has_Property(dpy, children[i], atom_wm_state))
            continue;

        /* Got one */
        win = children[i];
        goto done;
    }

    /* No children matched, now descend into each child */
    for (i = (int) n_children - 1; i >= 0; i--) {
        if (children[i] == None)
            continue;
        win = Find_Client_In_Children(dpy, children[i]);
        if (win != None)
            break;
    }

  done:
    XFree(children);

    return win;
}

/*
 * Find virtual roots (_NET_VIRTUAL_ROOTS)
 */
static unsigned long *
Find_Roots(Display * dpy, Window root, unsigned int *num)
{
    Atom type_ret;
    int format_ret;
    unsigned char *prop_ret;
    unsigned long bytes_after, num_ret;
    Atom atom;

    *num = 0;
    atom = XInternAtom(dpy, "_NET_VIRTUAL_ROOTS", False);
    if (!atom)
        return NULL;

    type_ret = None;
    prop_ret = NULL;
    if (XGetWindowProperty(dpy, root, atom, 0, 0x7fffffff, False,
                           XA_WINDOW, &type_ret, &format_ret, &num_ret,
                           &bytes_after, &prop_ret) != Success)
        return NULL;

    if (prop_ret && type_ret == XA_WINDOW && format_ret == 32) {
        *num = num_ret;
        return ((unsigned long *) prop_ret);
    }
    if (prop_ret)
        XFree(prop_ret);

    return NULL;
}

/*
 * Find child window at pointer location
 */
static Window
Find_Child_At_Pointer(Display * dpy, Window win)
{
    Window root_return, child_return;
    int dummyi;
    unsigned int dummyu;

    XQueryPointer(dpy, win, &root_return, &child_return,
                  &dummyi, &dummyi, &dummyi, &dummyi, &dummyu);

    return child_return;
}

/*
 * Find client window at pointer location
 *
 * root   is the root window.
 * subwin is the subwindow reported by a ButtonPress event on root.
 *
 * If the WM uses virtual roots subwin may be a virtual root.
 * If so, we descend the window stack at the pointer location and assume the
 * child is the client or one of its WM frame windows.
 * This will of course work only if the virtual roots are children of the real
 * root.
 */
Window
Find_Client(Display * dpy, Window root, Window subwin)
{
    unsigned long *roots;
    unsigned int i, n_roots;
    Window win;

    /* Check if subwin is a virtual root */
    roots = Find_Roots(dpy, root, &n_roots);
    for (i = 0; i < n_roots; i++) {
        if (subwin != roots[i])
            continue;
        win = Find_Child_At_Pointer(dpy, subwin);
        if (win == None)
            return subwin;      /* No child - Return virtual root. */
        subwin = win;
        break;
    }
    if (roots)
        XFree(roots);

    if (atom_wm_state == None) {
        atom_wm_state = XInternAtom(dpy, "WM_STATE", False);
        if (!atom_wm_state)
            return subwin;
    }

    /* Check if subwin has WM_STATE */
    if (Window_Has_Property(dpy, subwin, atom_wm_state))
        return subwin;

    /* Attempt to find a client window in subwin's children */
    win = Find_Client_In_Children(dpy, subwin);
    if (win != None)
        return win;             /* Found a client */

    /* Did not find a client */
    return subwin;
}

/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

Window undecorate(Window w)
{
	Display*display;
	display = XOpenDisplay(NULL);
	w = Find_Client(display,DefaultRootWindow(display),w);
	XCloseDisplay(display);	

	return w;
}

Window*snapshot(unsigned int*count)
{
	Display*display;
	Window root;
	Window parent;
	Window*children;

	display = XOpenDisplay(NULL);
	if(display==NULL)
		return NULL;

	int err;
	err = XQueryTree(display,DefaultRootWindow(display),&root,&parent,&children,count); 
	if(err==0)
		children=NULL;

	XCloseDisplay(display);

	return children;
}

int help(void)
{
#if 0
	printf("\n\nxlaunch -- launches an x11 app, and prints the app's X-Window-ID\n"
	       "examples:\n"
	       " $xlaunch xterm #launches xterm, and prints xterm's X-Window-ID\n"
	       " $xlaunch xcalc #launches xcalc, and prints xcalc's X-Window-ID\n"
	       " $xlaunch xclock #launches xclock, and prints xclock's X-Window-ID\n"
	       "\n");
#else
	printf("\n"
	       "                               xlaunch\n"
	       "                               *******\n"
	       "launches an x11 app and prints the app's X-Window-ID to stdout.\n"
	       "\n"
	       "to launch firefox and get its X-Window-ID, type:\n"
	       " $ xlaunch firefox\n"
	       "\n"
	       "to launch xterm and get its X-Window-ID, type:\n"
	       " $ xlaunch xterm\n"
	       "\n"
	       );
#endif
	exit(0);
}

int main(int argc, char*args[])
{
	if(argc<2)
		return help();
	else if(argc==2 && (!strcmp(args[1],"-h") || !strcmp(args[1],"--help")))
		return help();

	Window*list0;
	unsigned int count0;
	list0 = snapshot(&count0);

	char**cmd;
	cmd = args+1;

	pid_t pid;
	pid = fork();
	if(pid==-1)
		goto fail;
	if(pid==0)
	{
		dup2(open("/dev/null",O_RDONLY),0);
		dup2(open("/dev/null",O_WRONLY),1);
		dup2(open("/dev/null",O_WRONLY),2);

		execvp(*cmd,cmd);
		perror("xlaunch: fatal: app failed to launch");
		exit(127);
	}

	int t;
	int i;
	int j;
	Window*list1;
	unsigned int count1;

	fprintf(stderr,"xlaunching %s, please wait...",*cmd); fflush(stderr);
	for(t=0;t<30;++t)
	{
		usleep(500*1000);

		list1 = snapshot(&count1);
		if(list1==NULL)
			continue;

		for(j=0;j<count1;++j)
			for(i=0;i<count0;++i)
				if(list1[j]==list0[i]) 
					list1[j]=-1;

		for(j=0;j<count1;++j)
			if(list1[j]!=-1)
			{ 
				/* we may have have caught the window in an inconsistent state */
				if(undecorate(list1[j]) == list1[j])
					continue;
				fprintf(stderr,"ok\n"); fflush(stderr);

				goto found;
			}

		fprintf(stderr,"."); fflush(stderr);

		XFree(list1);
	}
	fprintf(stderr,"fail\n");
	fprintf(stderr,"xlaunch: error: could not detect %s's X window\n",args[1]);
	return 0;
found:
	printf("%lu\n", undecorate(list1[j]));

	return 0; 
fail:
	perror("xlaunch: fatal");
	exit(EXIT_FAILURE);
}

