The framework is designed for stacking of astro-photos.
The code is still under development.  There is also GUI, which is the recommended way on how to use the software, since some options are not supported via command line.

Example images which were stacked by this framework can be found here: https://app.astrobin.com/equipment/explorer/software/13498/michal-dubovsky-astrophotostacker

In order to use the GUI, you firstly need to compile the back-end part (see the instructions below), then you have to compile GUI part (instructions in ```GUI/``` folder).

For now, the code is able to stack photos using arithmetic average, median or kappa-sigma clipping (again using either average or median).
It can apply flat frames, but no other calibration frames are implemented yet.

The framework depends on ```libraw``` (for raw files reading) and ```OpenCV``` (for saving stacked photo as picture file) libraries.

Installing the dependencies (on Linux):
---------------------------------------

Firstly you need some basic C++ development tools

```
sudo apt-get install gcc cmake build-essential git
```

You also need ```libraw```:

```
sudo apt-get install libraw-dev
```

and ```OpenCV```:

```
sudo apt-get update -y || true &&  DEBIAN_FRONTEND=noninteractive sudo apt-get install -y tzdata libx11-dev && sudo apt-get install -y libopencv-dev
```

and ```exiv2```:


```
sudo apt-get install -y libexiv2-dev
```

Now you should have all the dependencies installed.


How to checkout and compile the code:
--------------------------------------

```
git clone git@github.com:dubovsky14/AstroPhotoStacker.git

cd AstroPhotoStacker

mkdir bin

cd bin

cmake ../.

make # optionally use "make -j4" for compiling on 4 CPUs, or "make -j" to compile on all available CPUs

cd ..
```

How to run the code:
--------------------

You need to run two steps. The first one will calculate how to shift and rotate your photos to be aligned with a reference photo.
So firstly look at your photos and choose one of them, that you want to use as a reference (stacked photo will cover this field of view).
Then run the following command:

**Alignment step:**
```
./bin/produce_alignment_file -option1 <option1_value> -option2 <option2_value> ...
```

Where the following options are mandatory:

```reference_file``` -> this is address of the reference photo. The final stacked image will have this field of view - i.e. all other pictures will be aligned to this photo.

```raw_files_dir``` -> directory with the raw files. It can contain only raw files or some text files, nothing else.

Few arguments are optional:

```alignment_file``` -> address of the output text file with the alignments. If not specified, file called ```alignment.txt``` will be created in the raw files directory.

```n_cpu``` -> number of CPUs to run on

**Hot pixel identification (optional)**

When median stacking methods are used, hot pixels are usually not a big issue. But if you need to remove them anyway, you can do it in 2 steps.

In the first step you need to run over raw files and identify the hot pixels in them. Their list will be saved to a text file:

```
./bin/identify_hot_pixels -raw_files_dir <path to the directory with raw files>
```

the following options are optional:

```hot_pixels_file``` : address of the output text file. By default it will be stored in raw file folder and named ```hot_pixels.txt```

```n_cpu``` : number of CPUs to use

After producing the text file with hot pixels, you can use it in the stacking step (described bellow). The value of the hot pixel will be replaced by mean value of its closest same color neighbors.
When you want to use hot pixel text file in the stacking step, you have to use optional argument ```hot_pixels_file```

**Stacking step:**

After producing the alignment file, you can run the stacking itself:

```
./bin/AstroPhotoStacker -option1 <option1_value> -option2 <option2_value> ...
```

Where the following options are mandatory:

```alignment_file``` -> path to the alignment file. The list of raw files to stack will be  read from this file.

```output``` -> address of the output stacked image that is going to be created.

and these arguments are optional:

```flat_frame``` -> address of the flat frame

```memory_limit``` -> limit on used memory in MB

```n_cpu``` -> number of CPUs to run on

```stacker_type``` -> stacking algorithm to be used. The list of available algorithms can be found in ```headers/StackerFactory.h``` header file. Default is ```kappa-sigma clipping```

```hot_pixels_file``` -> text file with hot pixels coordinates (described in ```Hot pixel identification``` part)

GUI
---

There is a graphical user interface, which is still in development, you can find it in ```GUI``` folder, together with instructions how to use it.