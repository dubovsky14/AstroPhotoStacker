The framework is designed for stacking of astro-photos.
The code is under heavy development, so the API is not yet very user-friendly yet and the documentation might be obsolete a little bit.

For now, the code is able to stack photos using either arithmetic average or median.
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
sudo apt-get update -y || true &&  DEBIAN_FRONTEND=noninteractive sudo apt-get install -y tzdata libx11-dev && sudo apt-get install -y --no-install-recommends libopencv-dev
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

```stacker_type``` -> stacking algorithm to be used. The list of available algorithms can be found in ```headers/StackerFactory.h``` header file. Default is ```kappa_sigma_clipping```
