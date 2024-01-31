#!/bin/bash
#
# Script running the TileDigitsContByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --digits
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --digits
diff -ur TileDigitDumps-0 TileDigitDumps-4
