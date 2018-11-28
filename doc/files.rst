File Formats
============

breastCrop requires as input a compressed breast phantom file and corresponding .mhd header.  The output is a cropped phantom file and corresponding header.
In the following filename descriptions *nnnnnnnn* represents the seed number specified as a command line option to breastCrop.

Input Files
-----------

The breastCrop input files are created using breastCompress.  breastCrop expects the following files to be present in the input directory specified on the command line:

pc\_\ *nnnnnnnn*\ .raw.gz
   The raw compressed phantom volume stored as 8-bit unsigned integers in a gzip archive.  There is no file header.

pc\_\ *nnnnnnnn*\ .mhd
   A metaImage header file containing information about the phantom data stored in the file pc\_\ *nnnnnnnn*\ .raw.gz

Output Files
------------

Final output file have formats indentical to the input files with suffix *_crop* to designate they represent a cropped phantom

pc\_\ *nnnnnnnn*\ _crop.raw.gz
   The raw croppeded phantom volume stored as 8-bit unsigned integers in a gzip archive.

pc\_\ *nnnnnnnn*\ _crop.mhd
   A metaImage header file containing information about the phantom data stored in the file pc\_\ *nnnnnnnn*\ _crop.raw.gz

.. note:: The input phantom will likely have an associated TDLU location file pc\_\ *nnnnnnnn*\ .loc.  The locations provided are physical coordinates, not voxel indicies,
	  therefore no translation of the .loc file is required for use with the cropped version of the phantom.
