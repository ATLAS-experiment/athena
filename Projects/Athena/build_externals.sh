#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for Athena.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthenaExternals"
ATLAS_BUILDTYPE="Release"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=108
                        -DLCG_VERSION_POSTFIX="_ATLAS_1"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r0.002/Gaudi-v40r0.002.tar.gz;URL_MD5;09dfaa50aaad6d1347363e008c3b405c"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v43.0.0.tar.gz;URL_HASH;SHA256=c5529a421b78384af8fc503060d73c10b813b8e902368ffc7b6265af46a33a05"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.17.0/GeoModel-6.17.0.tar.bz2;URL_MD5;51a8479d9bc46b97617fd0be29bb2ea9"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE
                        -DATLAS_ONNXRUNTIME_USE_CUDA=TRUE
                        -DATLAS_GAUDI_USE_CUDA=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
