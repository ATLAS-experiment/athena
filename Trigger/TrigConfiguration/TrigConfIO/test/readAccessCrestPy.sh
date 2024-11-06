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


echo "Diffing"
diff ${dirora} ${dircrest}
