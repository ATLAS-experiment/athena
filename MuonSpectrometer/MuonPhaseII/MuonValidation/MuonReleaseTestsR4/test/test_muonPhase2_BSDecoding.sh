#!/bin/bash



nEvents=-1
threads=8

#### Launch pure BS decoding legacy geometry
python -m MuonReleaseTestsR4.testByteSreamDecoding \
       --threads ${threads} \
       --nEvents ${nEvents} 2>&1 > LegacyBS.log

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "Legacy bytestream decoder test failed "
    exit 1
fi

tar -xzf perfmonmt.json.tar.gz

mv perfmonmt.json LegacyBS.json

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "PerfMon MT file (legacyBS) not produced"
    exit 1
fi

### Launch BS decoding with PRD conversion (legacy geometry)
python -m MuonReleaseTestsR4.testByteSreamDecoding \
       --threads ${threads} \
       --nEvents ${nEvents} \
       --doRdoDecoding 2>&1 > LegacyBSwithPrd.log


if [ ${ret_code} -ne 0 ];then
    echo "Legacy bytestream decoder test with Prd failed "
    exit 1
fi


tar -xzf perfmonmt.json.tar.gz

mv perfmonmt.json LegacyBSwithPrd.json

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "PerfMon MT file (LegacyWithPrd) not produced"
    exit 1
fi





#### Launch pure BS decoding legacy geometry
python -m MuonReleaseTestsR4.testByteSreamDecoding \
       --threads ${threads} \
       --useSqLite \
       --nEvents ${nEvents} 2>&1 > NewBS.log

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "News bytestream decoder test failed "
    exit 1
fi

tar -xzf perfmonmt.json.tar.gz

mv perfmonmt.json NewBS.json

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "PerfMon MT file (NewBS) not produced"
    exit 1
fi

### Launch BS decoding with PRD conversion (legacy geometry)
python -m MuonReleaseTestsR4.testByteSreamDecoding \
       --threads ${threads} \
       --nEvents ${nEvents} \
       --useSqLite \
       --doRdoDecoding 2>&1 > NewBSwithPrd.log


if [ ${ret_code} -ne 0 ];then
    echo "New bytestream decoder test with Prd failed "
    exit 1
fi


tar -xzf perfmonmt.json.tar.gz

mv perfmonmt.json NewBSwithPrd.json

ret_code=$?
if [ ${ret_code} -ne 0 ];then
    echo "PerfMon MT file (NewBSwithPrd) not produced"
    exit 1
fi
