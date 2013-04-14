import xorg

# key names must be valid names from the #defines in /usr/include/X11/keysymdef.h

def poke(key):
	xorg.sendKey(1,key); # press
	xorg.sendKey(0,key); # release
xorg.init();
xorg.setTarget("Event");
poke('a');
poke('space');
poke('b');
poke('c');
poke('BackSpace');
print("done");
