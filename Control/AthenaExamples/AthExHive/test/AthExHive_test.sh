#!/bin/bash
#
# Script running the AthExHiveConfig test Python module for CTest.
#

# Return the correct code:
set -e

# Run the test:
python -m AthExHive.AthExHiveConfig --threads=4 | grep HiveAlgF | grep test | awk '{print $6,$7,$8,$9,$10,$11,$12,$13}' | sort -n
