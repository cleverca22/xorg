from distutils.core import setup,Extension

module1 = Extension('xorg',libraries=['X11'],sources=['xorg.c'])

setup(name='Xorg',version='1.0',description='test package to send xorg key events',ext_modules=[module1])
