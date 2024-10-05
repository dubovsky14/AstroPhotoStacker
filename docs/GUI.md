What is ```AstroPhotoStacker``` and what does it currently support:
--------------------------------------------------------------------

The ```AstroPhotoStacker``` is an application which allows to stack astrophotos, as well as some other types of photos, but it can be useful also for some other types of photography, for example for photographing the lightning.

It supports few stacking algorithms:

**average** - it is the simplest (but also the worst performing) algorithm. The stacked value in a given pixel is just an aritmetic average of all values from all input photos. This way a satelite pass (or airplane) will be clearly visible in the stacked photo. The advantage of this approach is that it is fast, and needs relativelly small amount of memory.

**median** - signifncantly better performing algorithm compared to the average. It calculates median from all photos in each pixels. This way the satelatites and planes have much smaller effect on the stacked photo. Its disadvantage is that it requires quite a lot of memory.

**kappa-sigma mean** - this algorithm has 2 free parameters: kappa and the number of iterations. The algorithm calculates mean value and standard deviation of all values for each pixel. Then it removes all values differing from mean by more than ```kappa``` standard deviations. This process is repeated chosen number of times (iterations). Then the mean is calculated from all values survaving the last iteration.

**kappa-sigma meedian** - the same as ```kappa-sigma mean```, but the mean value at the end is replaced by median.

**cut-off average** - this cuts off the tails from both sides (one can choose a fraction of values to cut-off) and then it calculates the mean value from the remaining values.

**maximum** - for each pixel take the maximum value. This stacking algorithm does not perform well on deep sky objects. But it's useful for stacking star trails and photos of lightnings.


How to use Graphical User Interface (GUI) of ```AstroPhotoStacker```
--------------------------------------------------------------------