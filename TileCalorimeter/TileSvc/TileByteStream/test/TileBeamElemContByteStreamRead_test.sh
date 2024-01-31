#!/bin/bash
#
# Script running the TileBeamElemContByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --beam-elements
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --beam-elements
diff -ur TileBeamElemDumps-0 TileBeamElemDumps-4
