#!/bin/sh
#
# art-description: test muon geometry model (Run 4)
#
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: run_MuonGeoModelTestR4_testGeoModel.log
# art-output: out_MuonGeoModelTestR4_testGeoModel.root


# specify python test script 
package="MuonGeoModelTestR4"
file="testGeoModel"

# specify chambers to exclude in the test
excludedChambers="none"

# run in specified directory
mkdir $file; cd $file

# run python test script
log_file="run_${package}_${file}.log"
out_file="out_${package}_${file}.root"
python -m $package.$file --outRootFile $out_file --excludedChambers $excludedChambers > $log_file 2>&1

# save return code and write to art-results output 
rc1=${PIPESTATUS[0]}
echo "art-result: $rc1 $file"

cd ../

