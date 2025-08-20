#!/bin/sh
#
# art-description: test muon preparation data write with phase2 geometry building (RUN4 layout)
#
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-output: MuonReleaseTestsR4_test_muonHLTphase2.log
# art-output: RDO_TRIG.pool.root


# specify python test script 
package="MuonReleaseTestsR4"
file="testPrepDataWrite"

# run in specified directory
mkdir $file; cd $file

# run python test script
log_file="${package}_${file}.log"
python -m $package/$file.py --threads=1 \
        --defaultGeoFile=RUN4 --nEvents 100 > $log_file 2>&1  

# save return code and write to art-results output 
rc1=${PIPESTATUS[0]}
echo "art-result: $rc1 $file"

cd ../

