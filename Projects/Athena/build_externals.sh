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
                        -DLCG_VERSION_POSTFIX="a_ATLAS_5"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r2.001/Gaudi-v40r2.001.tar.gz;URL_MD5;e292ba3d71e4a88675d8632c2001cfb1"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/releases/download/v45.0.0/acts-v45.0.0.tar.gz;URL_HASH;SHA256=9297cd08235dce50b87404a84a0e0a2bc08b0b94c41a7694068179b2e621b7fd"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.22.0/GeoModel-6.22.0.tar.bz2;URL_MD5;c7ec128777884387ed75ae4fcc95f42a"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE
                        -DATLAS_ONNXRUNTIME_USE_CUDA=TRUE
                        -DATLAS_GAUDI_USE_CUDA=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
