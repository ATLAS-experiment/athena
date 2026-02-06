#!/usr/bin/env bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Check if NVIDIA GPU (and driver) is available. Returns 0 on success.
#
# Can be used as a PRE_EXEC_SCRIPT in a unit test to ensure it only runs if
# a GPU is available, e.g.:
#
# atlas_add_test( MyGPUTest
#                 PRE_EXEC_SCRIPT "nvidia-check.sh || exit 2"
#                 ...
#                 PROPERTIES SKIP_RETURN_CODE 2 )
#

{
    command -v nvidia-smi && nvidia-smi -L
} &> /dev/null
