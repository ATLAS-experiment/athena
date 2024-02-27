#!/bin/bash
#
# Script running the TileMuRcvContByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --murcv
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --murcv
diff -ur TileMuRcvDumps-0 TileMuRcvDumps-4
