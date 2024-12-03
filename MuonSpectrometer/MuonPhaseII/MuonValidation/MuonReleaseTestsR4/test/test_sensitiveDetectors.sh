#!/bin/sh
#
# art-description: test muon sensitive detectors (Run 4)
#
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: run_MuonGeoModelTestR4_testSensitiveDetectors.log
# art-output: out_MuonGeoModelTestR4_testGeoModel.pool.root


# specify python test script 
package="MuonGeoModelTestR4"
file="testSensitiveDetectors"

# set number of events to process
nevents="1000"

# run in specified directory
mkdir $file; cd $file

# run python test script
log_file="run_${package}_${file}.log"
out_file="out_${package}_${file}.pool.root"

python -m $package.$file --threads 8 --nEvents $nevents --outRootFile ${out_file} > $log_file 2>&1

# save return code and write to art-results output 
rc1=${PIPESTATUS[0]}
echo "art-result: $rc1 $file"

# continue if the run succeded
if [[ $rc1 -eq 0 ]]
then


  # compare the output to the reference file on cvmfs
  ref_out_path="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/"
  art.py compare ref --file=$out_file $PWD $ref_out_path

  # save return code and write to art-results output 
  rc2=${PIPESTATUS[0]}
  echo "art-result: $rc2 $file: output reference comparison"

fi

cd ../

