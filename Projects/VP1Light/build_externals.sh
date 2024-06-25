#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for VP1Light.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="VP1LightExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=104
                        -DLCG_VERSION_POSTFIX="d_ATLAS_19"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.0.0/GeoModel-6.0.0.tar.bz2;URL_MD5;3e239a848c7622fc894152e100bec27e")
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
