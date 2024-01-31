#!/bin/bash
#
# Script running the TileL2ContByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --level2
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --level2
diff -ur TileL2Dumps-0 TileL2Dumps-4
