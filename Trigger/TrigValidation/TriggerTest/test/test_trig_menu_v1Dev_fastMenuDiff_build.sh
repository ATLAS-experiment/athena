#!/usr/bin/env bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Compare default (fast) and slow menu generation
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena

menu="Dev_pp_run3_v1"

set -e   # exit on failure

echo "Fast menu generation of ${menu}"
mkdir -p ${menu}_fast && cd ${menu}_fast
{ time test_HLTmenu.py Trigger.triggerMenuSetup="${menu}" Trigger.fastMenuGeneration=True &> athena.log; } 2>&1
cd ..

echo
echo "Slow menu generation of ${menu}"
mkdir -p ${menu}_slow && cd ${menu}_slow
{ time test_HLTmenu.py Trigger.triggerMenuSetup="${menu}" Trigger.fastMenuGeneration=False &> athena.log; } 2>&1
cd ..

set +e

status=0
echo
echo "Comparing menu json files"
for json in `ls ${menu}_fast/*.json`; do
    json=`basename ${json}`
    diff ${menu}_fast/${json} ${menu}_slow/${json}
    status=$((status + $?))
done

echo "Comparing job configuration"
confTool.py --diff ${menu}_fast/${menu}.pkl ${menu}_slow/${menu}.pkl
status=$((status + $?))

if [ $status -eq 0 ]; then
    echo "No differences found"
fi
exit $status
