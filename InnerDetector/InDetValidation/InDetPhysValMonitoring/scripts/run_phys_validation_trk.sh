#!/bin/bash

setupATLAS ()
{ 
    if [ -d /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase ]; then
        export ALRB_localConfigDir="/etc/hepix/sh/GROUP/zp/alrb";
        export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase;
        source $ATLAS_LOCAL_ROOT_BASE/user/atlasLocalSetup.sh;
        return $?;
    else
        \echo "Error: cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase is unavailable" 1>&2;
        return 64;
    fi
}

while getopts j:t:d:r:s:h flag

do
        case "${flag}" in
                j) export JIRA=$OPTARG
		   echo "JIRA ticket is $JIRA";;
		t) export Task=$OPTARG
		   echo "$Task";;
		d) export DSID=$OPTARG
		   echo "Dataset ID $DSID";;
		r) export reference_sample=$OPTARG
		   echo "Reference sample $reference_sample";;
		s) export test_sample=$OPTARG
		   echo "Test sample $test_sample";;
                h) # Display Help
                   echo "This script compares reference and test sample for tracking validation."
                   echo
                   echo "sh run_phys_validation_trk.sh [-j|t|d|r|s|h]"
                   echo "options:"
		   echo "j     JIRA ticket (example ATLPHYSVAL-XXXX)"
		   echo "t     Task (example TaskX)"
		   echo "d     Dataset ID (example XXXXXX)"
		   echo "r     Reference Sample (example validX...)"
		   echo "s     Test Sample (example validX...)"
                   echo "h     Print this Help"
		   echo
		   echo "sh run_phys_validation_trk.sh -j ATLPHYSVAL-1210 -t TaskE -d 601229 -r valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.NTUP_PHYSVAL.e8514_s4481_s4469_r16369_p6745_p6746_p6747_tid43890298_00 -s valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.NTUP_PHYSVAL.e8514_s4481_s4469_r16425_p6745_p6746_p6747_tid43890394_00"
                   echo ;;
                *) echo "Invalid option: -$flag." && help ;;
        esac
done

echo "setupATLAS"
setupATLAS
#asetup Athena,main,latest
asetup Athena,24.0.15
voms-proxy-init -voms atlas
lsetup rucio

export tag_ref=Ref
export tag_test=Test

cd /tmp/$USER/

rucio list-dataset-replicas $reference_sample
rucio list-dataset-replicas $test_sample

rucio list-files $reference_sample
rucio list-files $test_sample

rucio download $reference_sample
rucio download $test_sample

export ValDir=/afs/cern.ch/atlas/groups/validation/Tracking/$JIRA/
export ValDirTask=/afs/cern.ch/atlas/groups/validation/Tracking/$JIRA/$Task
export ValDirTask2=/afs/cern.ch/atlas/groups/validation/Tracking/$JIRA/$Task/$DSID

if [ ! -d "$ValDir" ]; then
    mkdir $ValDir
fi

if [ ! -d "$ValDirTask" ]; then
    mkdir $ValDirTask
fi

if [ ! -d "$ValDirTask2" ]; then
    mkdir $ValDirTask2
fi

cd $ValDirTask2
echo $PWD

hadd -f ref.root /tmp/$USER/$reference_sample/*root*
hadd -f test.root /tmp/$USER/$test_sample/*root*

postProcessIDPVMHistos ref.root
postProcessIDPVMHistos test.root

physval_make_web_display.py --reffile "$tag_ref:ref.root" --outdir=$PWD --title $tag_test test.root --startpath=SquirrelPlots --ratio --ratiorange 0.02 --ratio2D

cd ../../../

echo "Please check files and upload them to the database: python3 CreateDB.py --folder_path '$ValDirTask2' --JIRA '$JIRA' --Task '$Task' --DSID '$DSID' --Domain 'Tracking' --Status 'Yellow'"

echo "After that the plots are: https://atlas-physval.web.cern.ch/tasks.php?jira=$JIRA&domain=Tracking"



