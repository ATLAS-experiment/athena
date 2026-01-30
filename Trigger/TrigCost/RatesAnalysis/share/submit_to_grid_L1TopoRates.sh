#!/bin/bash

DATASET="data24_13p6TeV.00482596.physics_EnhancedBias.merge.RAW"
OUTDS="user.${USER}.l1topoRates.EB_fullRun_00482596_1Oct25"

prun \
  --exec "bash -lc 'FILE=%IN; BASENAME=\$(basename \"\$FILE\"); \
    runL1TopoRates.py \
      --ratesJson=triggers.json \
      --filesInput %IN \
      Trigger.triggerConfig=DB \
      > logs.txt 2>&1; \
    if [ -f RatesHistograms.root ]; then mv RatesHistograms.root RatesHistograms_\${BASENAME}.root; else echo \"No RatesHistograms.root produced for \$BASENAME\" >> logs.txt; fi; \
    if [ -f logs.txt ]; then mv logs.txt logs_\${BASENAME}.txt; fi'" \
  --useAthenaPackages \
  --inDS ${DATASET} \
  --outDS ${OUTDS} \
  --outputs "RatesHistograms_*.root,logs_*.txt" \
  --nFiles 703 \
  --nFilesPerJob 2
