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
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=109
                        -DLCG_VERSION_POSTFIX="a_ATLAS_5"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r4.000/Gaudi-v40r4.000.tar.gz;URL_MD5;c526bdc92244025cbcca6b44f4948274"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/releases/download/v46.5.0/acts-v46.5.0.tar.gz;URL_HASH;SHA256=8b300bb869edc72063822ff65c7dc6653ee39c5cd236b870cf3fabfb8ccfb2dd"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.27.0/GeoModel-6.27.0.tar.bz2;URL_MD5;2e6fb12f85e37636ecdfc5d1053745c1"
                        -DATLAS_VECMEM_SOURCE="URL;http://cern.ch/atlas-software-dist-eos/externals/vecmem/v1.24.0.tar.gz;https://github.com/acts-project/vecmem/archive/refs/tags/v1.24.0.tar.gz;URL_MD5;4ca66bf822528e0880581ad107efa911"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE
                        -DATLAS_ONNXRUNTIME_USE_CUDA=TRUE
                        -DATLAS_GAUDI_USE_CUDA=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
