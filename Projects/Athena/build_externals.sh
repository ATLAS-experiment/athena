#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for Athena.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthenaExternals"
ATLAS_BUILDTYPE="Release"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=108
                        -DLCG_VERSION_POSTFIX="a_ATLAS_9"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r3.000/Gaudi-v40r3.000.tar.gz;URL_MD5;f73b109129b595a9c0299e8e025b7119"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/releases/download/v45.5.0/acts-v45.5.0.tar.gz;URL_HASH;SHA256=ca711f28d1443c47f7341049849199619fb7734e525a17cc20d2b5a4231cdbe7"
                        -DATLAS_ACTS_USE_GNN=FALSE
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.24.0/GeoModel-6.24.0.tar.bz2;URL_MD5;5d436565dd92671aa58cef737bbc2ab6"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE
                        -DATLAS_ONNXRUNTIME_USE_CUDA=TRUE
                        -DATLAS_GAUDI_USE_CUDA=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
