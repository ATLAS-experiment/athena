#!/bin/sh
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

# Return the correct code:
set -e

# Run the test

CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_single_OR_noEnforce.yaml -e 150 -o output_singleOR_noEnforce.root

testOutputContent.py --input_file output_singleOR_noEnforce.root --tree_name analysis --branches el_select_passesOR_NOSYS jet_select_passesOR_NOSYS mu_select_passesOR_NOSYS ph_select_passesOR_NOSYS tau_select_passesOR_NOSYS

