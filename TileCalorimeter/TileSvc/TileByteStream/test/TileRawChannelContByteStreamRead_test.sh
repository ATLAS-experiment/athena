#!/bin/bash
#
# Script running the TileRawChannelContByteStreamRead_test.py test with CTest.
#

# Run the job:
python -m TileByteStream.TileRawDataReadTestConfig --raw-channels
python -m TileByteStream.TileRawDataReadTestConfig --thread=4 --raw-channels
diff -ur TileRawChannelDumps-0 TileRawChannelDumps-4
