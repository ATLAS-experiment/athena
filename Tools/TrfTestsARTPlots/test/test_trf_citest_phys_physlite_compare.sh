#!/bin/bash
#
# art-description: RunWorkflowTests Derivation PHYSLITE, diff-root with previous nightly
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8

# Work around for environ['USER'] call in Tools/WorkflowTestRunner/python/ScriptUtils.py
export USER=pilot

# Determine current and previous nightly
currentrel=$AtlasBuildStamp
releasebasedir=`echo $ATLAS_RELEASE_BASE | sed "s/${AtlasBuildStamp}//g"` 
previousrel=`ls -r $releasebasedir | sort | grep 20 | tail -2 | head -1`

# Set some defaults
filename="DAOD_PHYSLITE.myOutput.pool.root"
diff_root_mode="--branches-of-interest"
diff_root_list="AnalysisElectronsAuxDyn(.*) AnalysisJetsAuxDyn(.*) AnalysisLargeRJetsAuxDyn(.*) AnalysisMuonsAuxDyn(.*) AnalysisPhotonsAuxDyn(.*) AnalysisTauJetsAuxDyn(.*)"
max_events=100

# Run2 data ##########
source $AtlasSetup/scripts/asetup.sh Athena,main,$previousrel
RunWorkflowTests_Run2.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} prev_data_PHYSLITE_Run2"
mv run_data_PHYSLITE_Run2 prev_run_data_PHYSLITE_Run2

source $AtlasSetup/scripts/asetup.sh Athena,main,$currentrel
RunWorkflowTests_Run2.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} curr_data_PHYSLITE_Run2"
mv run_data_PHYSLITE_Run2 curr_run_data_PHYSLITE_Run2

reference_file="prev_run_data_PHYSLITE_Run2/"$filename
validation_file="curr_run_data_PHYSLITE_Run2/"$filename
acmd.py diff-root $reference_file $validation_file --order-trees --nan-equal --exact-branches --mode semi-detailed --error-mode resilient $diff_root_mode $diff_root_list --entries $max_events
rc=$?
echo "art-result: ${rc} diff-root data_PHYSLITE_Run2"

# Run2 MC ##########
source $AtlasSetup/scripts/asetup.sh Athena,main,$previousrel
RunWorkflowTests_Run2.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} prev_mc_PHYSLITE_Run2"
mv run_mc_PHYSLITE_Run2 prev_run_mc_PHYSLITE_Run2

source $AtlasSetup/scripts/asetup.sh Athena,main,$currentrel
RunWorkflowTests_Run2.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} curr_mc_PHYSLITE_Run2"
mv run_mc_PHYSLITE_Run2 curr_run_mc_PHYSLITE_Run2

reference_file="prev_run_mc_PHYSLITE_Run2/"$filename
validation_file="prev_run_mc_PHYSLITE_Run2/"$filename
acmd.py diff-root $reference_file $validation_file --order-trees --nan-equal --exact-branches --mode semi-detailed --error-mode resilient $diff_root_mode $diff_root_list --entries $max_events
rc=$?
echo "art-result: ${rc} diff-root mc_PHYSLITE_Run2"

# Run3 data ##########
source $AtlasSetup/scripts/asetup.sh Athena,main,$previousrel
RunWorkflowTests_Run3.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} prev_data_PHYSLITE_Run3"
mv run_data_PHYSLITE_Run3 prev_run_data_PHYSLITE_Run3

source $AtlasSetup/scripts/asetup.sh Athena,main,$currentrel
RunWorkflowTests_Run3.py --CI -d -w Derivation --tag data_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} curr_data_PHYSLITE_Run3"
mv run_data_PHYSLITE_Run3 curr_run_data_PHYSLITE_Run3

reference_file="prev_run_data_PHYSLITE_Run3/"$filename
validation_file="curr_run_data_PHYSLITE_Run3/"$filename
acmd.py diff-root $reference_file $validation_file --order-trees --nan-equal --exact-branches --mode semi-detailed --error-mode resilient $diff_root_mode $diff_root_list --entries $max_events
rc=$?
echo "art-result: ${rc} diff-root data_PHYSLITE_Run3"

# Run3 MC ##########
source $AtlasSetup/scripts/asetup.sh Athena,main,$previousrel
RunWorkflowTests_Run3.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} prev_mc_PHYSLITE_Run3"
mv run_mc_PHYSLITE_Run3 prev_run_mc_PHYSLITE_Run3

source $AtlasSetup/scripts/asetup.sh Athena,main,$currentrel
RunWorkflowTests_Run3.py --CI -d -w Derivation --tag mc_PHYSLITE --threads 4 --no-output-checks
rc=$?
echo "art-result: ${rc} curr_mc_PHYSLITE_Run3"
mv run_mc_PHYSLITE_Run3 curr_run_mc_PHYSLITE_Run3

reference_file="prev_run_mc_PHYSLITE_Run3/"$filename
validation_file="prev_run_mc_PHYSLITE_Run3/"$filename
acmd.py diff-root $reference_file $validation_file --order-trees --nan-equal --exact-branches --mode semi-detailed --error-mode resilient $diff_root_mode $diff_root_list --entries $max_events
rc=$?
echo "art-result: ${rc} diff-root mc_PHYSLITE_Run3"
