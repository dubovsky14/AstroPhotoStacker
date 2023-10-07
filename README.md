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

```
./bin/produce_alignment_file <reference_file_address> <directory_with_raw_files> <output_alignment_file (optional)>
```

Where ```<reference_file_address``` is address of your reference file,
```<directory_with_raw_files>``` is directory with your raw files - this directory can contain only raw files or some text files, nothing else.
The last argument, ```<output_alignment_file (optional)>``` is optional, it is address of the created text file with the alignment information,
if the address is not specified, it will create ```alignment.txt``` file in directory with raw files.

After this step, you know how to move and rotate all the files so that they will be aligned with the reference photo. You can run stacking step now:

```
./bin/AstroPhotoStacker <path_to_alignment_file>  <output file address> <flat file address>
```



