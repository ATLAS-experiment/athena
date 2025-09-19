#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script to remove ART output directory / files if needed

AtlasBuildBranch=$1
AtlasProject=$2
AtlasBuildStamp=$3
Architecture="${AtlasProject}_PLATFORM"
testName="${ArtJobName%.sh}"
# echo /eos/atlas/atlascerngroupdisk/data-art/grid-output/$AtlasBuildBranch/$AtlasProject/${!Architecture}/$AtlasBuildStamp/InDetPhysValMonitoring/$testName

! ([[ ${!Architecture} == "x86_64-el9-gcc14-opt" ]] && [[ $AtlasBuildBranch == "main" ]])  && rmOutput=true || rmOutput=false   #Placeholder

skip_files=(
  "physval.ntuple.root"
  "physval.root"
  "physval_lrt.ntuple.root"
  "physval_idtide.ntuple.root"
  "HitValid.root"
  "SiHitValid.root"
  "RDOAnalysis.root"
  "idpvm.root"
  "idpvm.acts.root"
  "idpvm.athena.root"
  "idpvm.athena.ckf.root"
  "idpvm.ckf.root"
  "idpvm.ambi.root"
  "idpvm.ambi.scored.root"
  "idpvm.gbts.root"
  "idpvm.acts.timed.root"
  "ActsMonitoringOutput.root"
  "acts-analysis.gbts.root"
  "acts-analysis.acts.root"
)

if [[ $rmOutput == true ]]; then
    for item in {art_core,*.root}; do
        [[ -e "$item" ]] || continue
        remove=true
        for pat in "${skip_files[@]}"; do
            if [[ $item == $pat ]]; then
                remove=false
                break
            fi
	done

	if [[ $remove == true ]]; then
            rm -rf -- "$item"
        fi
    done
fi
