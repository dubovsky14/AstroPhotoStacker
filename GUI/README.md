**How to install dependencies and compile Graphical User Interface**

The GUI is based on ```wxwidgets``` library, so in order to use the framework you will have to install it first.

Installing wxwidgets
----------------------

Installing wxwidgets is a bit tricky, but maybe this set of commands could work. If not, see the next chapter.

Firstly go to a folder where you want to download and compile wxwidgets, then do:


```
sudo apt install libgtk-3-dev
wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.7/wxWidgets-3.2.7.tar.bz2
tar -xvzf wxWidgets-3.2.7.tar.bz2
cd wxWidgets-3.2.7/
mkdir gtk-build
cd gtk-build
../configure
make -j3
sudo make install
sudo ldconfig
```

If the instruction above did not work, follow the guide in one of the links bellow.

https://wiki.wxwidgets.org/Compiling_and_getting_started

https://docs.codelite.org/wxWidgets/repo320/#ubuntu-and-debian



Compiling the GUI itself:
-------------------------

The GUI depends, of course, also on the rest of this package, so if you have not installed its dependencies and compiled it yet, please do so before compiling the GUI part.

In order to compile the GUI part go to ```GUI``` folder and then:

```
mkdir bin

cd bin

cmake ../.

make # alternatively "make -j" to compile on all CPUs
```

This will create an executable ```AstroPhotoStackerGUI```, you can then excute it (```./AstroPhotoStackerGUI```).

**How to use GUI**

Using the ```File``` menu (top left) you can add light frames, dark frames or flat frames. Many different types of light frames are supported: DSLR/mirrorless raw files, FIT file from dedicated astronomical camera, RGB image in various formats (TIF, JPG, etc.), and raw/non-raw video frames. Video input is not allowed for calibration frames (only still images).

Firstly you have to add the files and check them (you can check them one-by-one or use "Check all" button). Then you have to align them (calculate how to shift and rotate them in order to be aligned). Use "Align" button. When your files are aligned, you can stack them. Optionally, if you want to remove hot pixels, run the hot pixel identification before the stacking. When running ot pixel identification, be sure that you checked only the raw files, this step does not work with any other type of files.