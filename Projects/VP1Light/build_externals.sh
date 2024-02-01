#!/bin/bash
#
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for VP1Light.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="VP1LightExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=104
                        -DLCG_VERSION_POSTFIX="d_ATLAS_1"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/5.0.1/GeoModel-5.0.1.tar.bz2;URL_MD5;88d297f433ec15c26bd2dd120964813f"
                        -DATLAS_GEOMODEL_PATCH=""
                        -DATLAS_GEOMODEL_FORCEDOWNLOAD_MESSAGE="Forcing the re-download of GeoModel (2024.01.27.)")
>>>>>>> 232aab9b9a2 (Update to LCG_104d_ATLAS_1 and TDAQ/TDAQ-COMMON 11-02-01)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
