#!/bin/bash
# art-description: Run 4 configuration, ITK recontruction with special persistification of space points
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5

run () {
    name="${1}"
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    echo "art-result: $rc ${name}"
    return $rc
}

checkCollectionOnFile() {
    local FileName="$1"
    shift
    local EdmCollections=("$@")
    checkxAOD.py ${FileName} >& collections.txt
    echo " * checking AOD file ("${FileName}") content for collections:"
    for var in "${EdmCollections[@]}"; do
	echo "   - checking collection: "${var//\"/}
	grep -e " ${var//\"/} " collections.txt >& tmp.log
	res=$?
	if [ $res != 0 ]; then
	    return ${res}
	fi
    done
    return 0
}

# RECONSTRUCTION
export ATHENA_CORE_NUMBER=1
run "ACTS creation" \
    Reco_tf.py \
    --preExec "flags.Exec.FPE=-1; \
    	       flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True; \
	       flags.Tracking.StoreSlimmedDataPreparation=True;" \
    --conditionsTag ${conditions_tag} \
    --inputRDOFile ${input_rdo} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${n_events} \
    --multithreaded

res=$?
if [ $res != 0 ]; then
    echo "- AOD creation failure"
    exit ${res}
fi

run "AOD inspection" \
    checkCollectionOnFile \
    AOD.pool.root \
    "ITkPixelSpacePoints" "ITkStripSpacePoints" "ITkStripOverlapSpacePoints"

res=$?
if [ $res != 0 ]; then
    echo "- AOD content failure"
    exit ${res}
fi

# DERIVATION
run "DAOD FTAG creation" \
    Derivation_tf.py \
    --inputAODFile AOD.pool.root \
    --outputDAODFile DAOD.pool.root \
    --formats FTAG1 \
    --maxEvents -1 \
    --multithreaded

res=$?
if [ $res != 0 ]; then
    echo "- DAOD (FTAG1) creation failure"
    exit ${res}
fi

run "DAOD FTAG inspection" \
    checkCollectionOnFile \
    DAOD_FTAG1.DAOD.pool.root \
    "ITkPixelSpacePoints" "ITkStripSpacePoints" "ITkStripOverlapSpacePoints"

res=$?
if [ $res != 0 ]; then
    echo "- DAOD (FTAG1) content failure"
    exit ${res}
fi
