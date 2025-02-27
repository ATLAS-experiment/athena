#!/bin/sh
# Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
if ! (type nvidia-smi); then # Pass if NVidia driver not available
    echo "SKIPPING TEST -- NVidia driver not found"
    exit 2
fi
if ! (nvidia-smi -q -i 0); then # Pass if no GPU available
    echo "SKIPPING TEST -- No GPU found"
    exit 2
fi
athena.py --threads=4 AthExCUDA/LinearTransformAsyncExampleConfig.py
