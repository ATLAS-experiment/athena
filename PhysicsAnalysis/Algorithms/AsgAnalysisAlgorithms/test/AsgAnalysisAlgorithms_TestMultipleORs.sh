#!/bin/sh
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

# Return the correct code:
set -e

# Run the test

CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC --text-config AsgAnalysisAlgorithms/test_multiple_ORs.yaml -e 150 -o output_multipleORs.root

testOutputContent.py --input_file output_multipleORs.root --tree_name analysis --branches el_select_passesOR_postJvt_NOSYS el_select_passesOR_preJvt_NOSYS jet_select_passesOR_postJvt_NOSYS jet_select_passesOR_preJvt_NOSYS mu_select_passesOR_postJvt_NOSYS mu_select_passesOR_preJvt_NOSYS ph_select_passesOR_postJvt_NOSYS ph_select_passesOR_preJvt_NOSYS tau_select_passesOR_postJvt_NOSYS tau_select_passesOR_preJvt_NOSYS
