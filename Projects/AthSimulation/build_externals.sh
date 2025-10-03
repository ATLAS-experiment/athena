#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for AthSimulation.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthSimulationExternals"
ATLAS_BUILDTYPE="Release"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=108
                        -DLCG_VERSION_POSTFIX="_ATLAS_5"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v40r0.003/Gaudi-v40r0.003.tar.gz;URL_MD5;c24aec64b186d4a3aec3a1c87a3e5eef"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.19.0/GeoModel-6.19.0.tar.bz2;URL_MD5;873cacbd5e1e89819b50894599e19a12"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
