#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#!/bin/bash

#This tool runs Athena CA reconstruction workflows, via python, from ESD/AOD
#The workflows are defined in the test_names array, and the python scripts are to be found in Reconstruction/eflowRec/python/
#The tool runs each workflow twice: once in a "Default" environment and once in a "Local" environment (with build setup). 
#It then compares the outputs using acmd.py diff-root
#The results of the comparisons are summarized at the end of the script.
#If you have a local work folder called myReleaseWorkDir the expected directory structure inside that is:
#build
#athena
#This script should be run from the myReleaseWorkDir directory. The script will create subdirectories for each test run and store log files there.

# ==========================================================
# Configuration
# ==========================================================
# Add your test names here
test_names=("PFRunESDtoAOD_EOverP_mc21_13p6TeV" 
            "PFRunESDtoAOD_TopoTowers_mc21_14TeV" 
            "PFRunESDtoAOD_WithJetsTausMET_CPData_mc21_13p6TeV" 
            "PFRunESDtoAOD_WithJetsTausMET_TruthCheating_mc21_13p6TeV" 
            "PFRunESDtoAOD_WithJetsTausMET_mc21_13p6TeV" 
            "PFRunESDtoAOD_WithJetsTausMET_mc21_14TeV")

# Path to the build setup script
BUILD_SETUP="build/x86_64-el9-gcc14-opt/setup.sh"

# Array to store results for the summary
declare -a results

# ==========================================================
# Main Execution Loop
# ==========================================================

for testName in "${test_names[@]}"; do
    echo "----------------------------------------------------"
    echo "Starting test pair for: $testName"
    echo "----------------------------------------------------"

    # 1. Run the "Default" environment
    echo "[1/2] Running Default environment..."
    (
        export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
        source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh --quiet
        asetup Athena,main,latest
        
        mkdir -p "run_${testName}_Default"
        cd "run_${testName}_Default" || exit
        
        python "../athena/Reconstruction/eflowRec/python/$testName.py" >& test.log 
    )
    echo "Default environment run completed for $testName.py"

    # 2. Run the "Local" environment (with build setup)
    echo "[2/2] Running Local environment..."
    (
        export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
        source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh --quiet
        asetup Athena,main,latest
        
        if [ -f "$BUILD_SETUP" ]; then
            source "$BUILD_SETUP"
        else
            echo "Warning: Build setup not found at $BUILD_SETUP"
        fi
        
        mkdir -p "run_${testName}_Local"
        cd "run_${testName}_Local" || exit
        
        python "../athena/Reconstruction/eflowRec/python/$testName.py" >& test.log 
    )
    echo "Local environment run completed for $testName.py"

    # 3. Run the Diff Comparison
    echo "Comparing outputs for $testName.py..."
    
    file_default="run_${testName}_Default/output_AOD.root"
    file_local="run_${testName}_Local/output_AOD.root"
    
    # Check if files exist before diffing
    if [[ -f "$file_default" && -f "$file_local" ]]; then
        # Run acmd.py and capture output
        # Note: Your prompt compared default vs default; I have updated this to compare Local vs Default.
        diff_output=$(acmd.py diff-root --order-trees --error-mode resilient --nan-equal --entries=100 "$file_default" "$file_local" 2>&1 | tee "diff_${testName}.log")
        
        # Check for "error" (case insensitive)
        if echo "$diff_output" | grep -q "ERROR"; then
            echo "****************************************************"
            echo "$testName Failed:"
            echo "$diff_output" | grep  "ERROR"
            echo "Check 'diff_${testName}.log' for full details."
            echo "****************************************************"
            results+=("$testName: FAIL: Differences in output found")
        else
            echo "$testName passed: No errors found in root diff."
            results+=("$testName: PASS")
        fi
    else
        echo "Error: One or both output files are missing for $testName.py"
        results+=("$testName: FAIL (Files Missing)")
    fi

done

# ==========================================================
# Final Summary Report
# ==========================================================
echo ""
echo "===================================================="
echo "                TEST RUN SUMMARY                    "
echo "===================================================="
for entry in "${results[@]}"; do
    echo "$entry"
done
echo "===================================================="
echo "Full diff logs are available in: $LOG_DIR"

