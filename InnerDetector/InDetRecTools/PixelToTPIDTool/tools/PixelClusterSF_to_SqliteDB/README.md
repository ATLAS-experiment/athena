This tool converts pixel cluster dE/dx scale factor files to SqliteDB files. It utilizes a JSON file pixel_wafer_id_hash_map.json to appropriately assign scale factors to each pixel module.

storePixelClusdEdxSF.py 

This creates a SqliteDB file based on the flattened ROOT files with Pixel cluster dE/dx scale factors. Currently it assumes the IOV for a scale factor is valid until the next run. For example, if the ROOT file contains 451543 and 451557, it will set the SFs from 451543 to be valid up until the start of run 451557. It currently assumes the runs in the ROOT file are in increasing order.

Important variables to change folder name, schema, and tags, with default values:
- L14 FOLDER_DB_NAME = "/PIXEL/test"
- L125 local_db_file = "TEST.db"
- L126 local_db_name = "CONDBR2"
- L127 tag = "PixelTest"

To change the input scale factor file:
- L133 pixelClusSF_input_file_path = "sf_input_files/data23_sf_flat.root"

pix_waferID_to_hashID_maker.py 

The pix_waferID_to_hashID_maker.py script converts the geometry.dat file output obtained from the run of InnerDetector/InDetExample/InDetDetDescrExample/tools/RunPrintSiDetElements.py into a JSON file which maps Pixel wafer hashes to Pixel wafer IDs.

It utilizes a PixelGeometry.dat file from RunPrintSiDetElements.py to create a map between Pixel wafer IDs and hashes. If using RunPrintSiDetElements.py, make sure that the geometry tags are set to the appropriate value. (Currently, its default is Run 4.)
