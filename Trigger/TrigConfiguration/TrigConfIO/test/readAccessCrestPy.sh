#!/usr/bin/bash

dirora=dbaccessOracle
dircrest=dbaccessCrest

rm -rf ${dirora}
mkdir -p ${dirora}
cd ${dirora}
TriggerMenuRW.py --db TRIGGERDB_RUN3 --smk 3370 -w
TriggerMenuRW.py --db TRIGGERDB_RUN3 --l1psk 15049 -w
TriggerMenuRW.py --db TRIGGERDB_RUN3 --hltpsk 11179 -w
cd -

rm -rf ${dircrest}
mkdir -p ${dircrest}
cd ${dircrest}
TriggerMenuRW.py --db TRIGGERDB_RUN3 --smk 3370 --use-crest -w
TriggerMenuRW.py --db TRIGGERDB_RUN3 --l1psk 15049 --use-crest -w
TriggerMenuRW.py --db TRIGGERDB_RUN3 --hltpsk 11179 --use-crest -w
cd -

rm -rf ${dircrest}2
mkdir -p ${dircrest}2
cd ${dircrest}2
TriggerMenuRW.py --db CONF_DATA_RUN3 --smk 3370 --use-crest -w
TriggerMenuRW.py --db CONF_DATA_RUN3 --l1psk 15049 --use-crest -w
TriggerMenuRW.py --db CONF_DATA_RUN3 --hltpsk 11179 --use-crest -w
cd -


echo "Diffing ${dirora} ${dircrest}"
if ! diff -qr "${dirora}" "${dircrest}" >/dev/null; then
    echo "Directories differ: $dirora vs $dircrest"
    exit 1
fi

echo "Diffing ${dirora} ${dircrest}2"
if ! diff -qr "${dirora}" "${dircrest}2" >/dev/null; then
    echo "Directories differ: $dirora vs $dircrest"2
    exit 1
fi

exit 0
