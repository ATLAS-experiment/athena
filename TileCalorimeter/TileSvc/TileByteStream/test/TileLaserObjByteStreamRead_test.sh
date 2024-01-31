#!/bin/bash
#
# Script running the TileLaserObjByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --laser-object
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --laser-object
diff -ur TileLaserDumps-0 TileLaserDumps-4
