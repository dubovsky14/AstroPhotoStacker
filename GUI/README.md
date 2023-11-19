**How to install dependencies and compile Graphical User Interface**

The GUI is based on ```wxwidgets``` library, so in order to use the framework you will have to install it first. The easiest way is to install it using pip:

```
pip3 install wxwidgets
```

The GUI depends, of course, also on the rest of this package, so if you have not installed its dependencies and compiled it yet, please do so before compiling the GUI part.

In order to compile the GUI part:

```
mkdir bin

cd bin

cmake ../.

make # alternatively "make -j" to compile on all CPUs
```

This will create an executable ```AstroPhotoStackerGUI```

**How to use GUI**

Using the ```File``` menu (top left) you can add light frames and flat frames. Light frames must be raw files from a RGB digital camera (DSLR or mirrorless). Firstly you have to add the files and check them (you can check them one-by-one or use "Check all" button). Then you have to align them (calculate how to shift and rotate them in order to be aligned). Use "Align" button. When your files are aligned, you can stack them. Optionally, if you want to remove hot pixels, run the hot pixel identification before the stacking. When running ot pixel identification, be sure that you checked only the raw files, this step does not work with any other type of files.