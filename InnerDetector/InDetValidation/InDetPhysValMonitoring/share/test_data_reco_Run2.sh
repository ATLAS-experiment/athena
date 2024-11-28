#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART jobs with Run 2 Data Reco config

ArtProcess=$1
ArtInFile=$2
dcubeRef=$3
conditions="CONDBR2-BLKPA-RUN2-11"
geotag="ATLAS-R2-2016-01-00-01"

script=test_data_reco.sh

"$script" ${ArtProcess} ${ArtInFile} ${dcubeRef} ${conditions} ${geotag}
