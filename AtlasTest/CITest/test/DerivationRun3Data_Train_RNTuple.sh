#!/bin/bash

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Explicitly set maxEvents so that the preExec doesn't get overwritten
RunWorkflowTests_Run3.py --CI -d -w Derivation -e '--maxEvents=500 --preExec="flags.PoolSvc.DefaultContainerType=\"ROOTRNTUPLE\";" --parallelCompression="False"' --tag data_PHYS_PHYSLITE --threads 4 --no-output-checks
