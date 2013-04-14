#include <python2.6/Python.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

Display *display;
Window window;
Window *targetWindow;

static PyObject *xorg_init(PyObject *self, PyObject *args) {
	//const char *disp;
	
	//if (!PyArg_ParseTuple(args, "s", &disp)) return NULL;
	display = XOpenDisplay(0);
	if (display == NULL) return NULL; // FIXME, add exception
	window = XDefaultRootWindow(display);
	if (!window) return NULL; // FIXME
	targetWindow = NULL;
	return Py_BuildValue("i", 0);
}
Window *find_window(Display *display, Window parentWindow, const char *findme) {
	int i;
	Window root;
	Window parent;
	Window *children = 0;
	unsigned int childrenCount = 0;
	XQueryTree(display,parentWindow,&root,&parent,&children,&childrenCount);
	
	char *window_name = 0;
	Window *window = 0;
	for (i=0; i<childrenCount; i++) {
		if (XFetchName(display,children[i],&window_name) == 1) {
			if (strcasestr(window_name,findme) != NULL) {
				XFree(window_name);
				return &children[i];
			}
			XFree(window_name);
		}
		
		window = find_window(display,children[i],findme);
		if (window != 0) {
			XFree(children);
			return window;
		}
	}
	if (children != 0) XFree(children);
	return 0;
}
static PyObject *xorg_setTarget(PyObject *self, PyObject *args) {
	const char *findme;
	if (!PyArg_ParseTuple(args,"s",&findme)) return NULL;
	targetWindow = find_window(display,window,findme);
	if (targetWindow == NULL) {
		printf("error\n");
		return NULL; // FIXME
	}
	return Py_BuildValue("i",0);
}
XKeyEvent create_key_event(Display *display, Window window, Window root, int press, int keycode, int modifiers) {
	XKeyEvent event;
	event.display = display;
	event.window = window;
	event.root = root;
	event.subwindow = None;
	event.time = CurrentTime;
	event.x = 1;
	event.y = 1;
	event.x_root = 1;
	event.y_root = 1;
	event.same_screen = True;
	event.keycode= keycode;
	event.state = modifiers;
	event.send_event = 1;
	event.serial = 0;
	if (press) event.type = KeyPress;
	else event.type = KeyRelease;
	return event;
}
static PyObject *xorg_sendKey(PyObject *self,PyObject *args) {
	unsigned int keycode;// = XKeysymToKeycode(display,XK_A);
	XKeyEvent event;
	int down;
	const char *keystring;
	
	if (!PyArg_ParseTuple(args,"is",&down,&keystring)) return NULL;
	KeySym keysym = XStringToKeysym(keystring);
	if (keysym == NoSymbol) {
		printf("keycode '%s' not found\n",keystring);
		return NULL; // FIXME, add exception
	}
	keycode = XKeysymToKeycode(display,keysym);
	if (keycode == NoSymbol) {
		printf("keycode '%s' not found\n",keystring);
		return NULL; // FIXME
	}
	event = create_key_event(display,*targetWindow,window,down,keycode,0);
	if (down == 1) {
		XSendEvent(event.display,event.window,True,KeyPressMask,(XEvent*)&event);
	} else {
		XSendEvent(event.display,event.window,True,KeyReleaseMask,(XEvent*)&event);
	}
	XFlush(display);
	return Py_BuildValue("i",0);
}
static PyMethodDef XorgMethods[] = {
	{"init",xorg_init,METH_VARARGS,"opens the xorg display"},
	{"setTarget",xorg_setTarget,METH_VARARGS,"sets target window"},
	{"sendKey",xorg_sendKey,METH_VARARGS,"send a key"},
	{NULL,NULL,0,NULL}
};
PyMODINIT_FUNC initxorg() {
	(void) Py_InitModule("xorg", XorgMethods);
}
