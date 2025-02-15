#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for Athena.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthenaExternals"
ATLAS_BUILDTYPE="Release"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=107
                        -DLCG_VERSION_POSTFIX="a_ATLAS_1"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v39r2.000/Gaudi-v39r2.000.tar.gz;URL_MD5;72cf6f13c9461ab0a0e8447af80e092b"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v39.0.0.tar.gz;URL_HASH;SHA256=1c762b6194edd1b12db93822030e28cc34c41d3ea0c30239b08f727371ac3181"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.9.0/GeoModel-6.9.0.tar.bz2;URL_MD5;4696042fd5dbb95be6d76197097b5ed4"
                        -DATLAS_VECMEM_SOURCE="URL;http://cern.ch/atlas-software-dist-eos/externals/vecmem/v1.5.0.tar.gz;https://github.com/acts-project/vecmem/archive/refs/tags/v1.5.0.tar.gz;URL_MD5;3cc5a3bb14b93f611513535173a6be28"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
