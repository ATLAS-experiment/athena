#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for VP1Light.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="VP1LightExternals"
ATLAS_BUILDTYPE="Release"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=107
                        -DLCG_VERSION_POSTFIX="a_ATLAS_11"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.11.1/GeoModel-6.11.1.tar.bz2;URL_MD5;5e95386417afd5df4069ca427b85c2c9")
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
