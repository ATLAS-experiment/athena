#!/bin/sh
#
# art-description: test muon geometry model
#
# art-type: local
# art-include: main/Athena
# art-output: run_MuonGeoModelTest_testGeoModel.log
# art-output: out_MuonGeoModelTest_testGeoModel.root


# specify python test script 
package="MuonGeoModelTest"
file="testGeoModel"

# run in specified directory
mkdir $file; cd $file

# run python test script
log_file="run_${package}_${file}.log"
out_file="out_${package}_${file}.root"
python -m $package.$file --outRootFile $out_file > $log_file 2>&1

# save return code and write to art-results output 
rc1=${PIPESTATUS[0]}
echo "art-result: $rc1 $file"

cd ../

