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
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=110
                        -DLCG_VERSION_POSTFIX="_ATLAS_5"
                        -DCMAKE_CUDA_STANDARD=20 # See: ATLINFR-6204
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r4.002/Gaudi-v40r4.002.tar.gz;URL_MD5;72a2fa2008f37c0dc88fb1e5b039f295"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/releases/download/v48.0.1/acts-v48.0.1.tar.gz;URL_HASH;SHA256=cb9b196345119cbdbb73e4f53fae6c3ce6480560f9fd47a7164a17b51629a694"
                        -DATLAS_ACTS_BUILD_TRACCC=TRUE
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.29.0/GeoModel-6.29.0.tar.bz2;URL_MD5;0c21efe670b74278b2004d522bb3cb5e"
                        -DATLAS_VECMEM_SOURCE="URL;http://cern.ch/atlas-software-dist-eos/externals/vecmem/v1.27.0.tar.gz;https://github.com/acts-project/vecmem/archive/refs/tags/v1.27.0.tar.gz;URL_MD5;30ef85b2a9326c08d291f4fea949e097"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE
                        -DATLAS_GAUDI_USE_CUDA=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
