This tool converts pixel cluster dE/dx scale factor files to SqliteDB files. It requires a PixelGeometry.dat file from RunPrintSiDetElements.py. When using this tool, make sure that the geometry tags are set to the appropriate value. (Currently, they default to Run 4.)

- First run pix_waferID_to_hashID_maker.py 

This script converts the geometry.dat file output obtained from the run of InnerDetector/InDetExample/InDetDetDescrExample/tools/RunPrintSiDetElements.py into a JSON file which maps Pixel wafer hashes to Pixel wafer IDs.

- Second run storePixelClusdEdxSF.py 

This creates a SqliteDB file based on the flattened ROOT files with Pixel cluster dE/dx scale factors. Currently it assumes the IOV for a scale factor is valid until the next run. For example, if the ROOT file contains 451543 and 451557, it will set the SFs from 451543 to be valid up until the start of run 451557. It currently assumes the runs in the ROOT file are in increasing order.
