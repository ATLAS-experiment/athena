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
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=107
                        -DLCG_VERSION_POSTFIX="a_ATLAS_13"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v39r4.001/Gaudi-v39r4.001.tar.gz;URL_MD5;62e12e0d519bf0b5baf263c321b50b9e"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.13.0/GeoModel-6.13.0.tar.bz2;URL_MD5;10b990af8f033d5af673e6de01524378"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
