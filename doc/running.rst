Running the Software
====================

Command line options
--------------------

breastCrop is run from the command line and requires that several command line options be specified::

  > breastCrop -s <SEED> -g <GAP> -x <XVOX> -y <YVOX> -z <ZVOX> -d [DIRECTORY] -h

required command line arguments are

-s SEED             the seed number of the compressed phantom you wish to crop
-g GAP              size of air gap to maintain around phantom (mm)
-x XVOX             crop size in x direction (number of voxels)
-y YVOX             crop size in y direction (number of voxels)
-z ZVOX             crop size in z direction (number of voxels)

optional command line arguments are

-d DIRECTORY        the directory containing the uncompressed breast phantom.  The output compressed breast phantom will be placed in this directory as well.  The default value is '.' representing the current working directory.
-h                  print a help message

Operating Modes
---------------

breastCrop has two operating modes, automated determination of crop dimensions and proscribed crop dimensions.  Automated crop dimensions are triggered by specifying 0 for
the crop sizes, e.g. the command,::

  > breastCrop -s 123456789 -g 1.0 -x 0 -y 0 -z 0

will crop the compressed phantom pc_123456789.raw.gz in the current working directory to the extent of the compression surfaces, with a 1 mm air gap around the edges
of the phantom.  The command::

  > breastCrop -s 123456789 -g 1.0 -x 500 -y 600 -z 600

will crop the same phantom to a voxel size of 500x600x600 with the region cropped as close as possible to the region that would have been selected with automatically selected
dimensions.  The main purpose for this mode is to eleminiate small variations in crop size across multiple phantom realizations to simplify batch processing with MC-GPU.




