#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for Athena.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthenaExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=104
                        -DLCG_VERSION_POSTFIX="d_ATLAS_13"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v38r0.001/Gaudi-v38r0.001.tar.gz;URL_MD5;36f071bb81c0a85331c047107f829537"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v34.1.0.tar.gz;URL_HASH;SHA256=eed03c43ede98c0d511fb290c41689992aa5ed64193b9cf62151cfccf1cf26b3"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/5.4.0/GeoModel-5.4.0.tar.bz2;URL_MD5;c06fe74de34e6f89e527d83b366f7e03"
			-DATLAS_GEOMODEL_PATCH=""
                        -DATLAS_GEOMODEL_FORCEDOWNLOAD_MESSAGE="Forcing_the_re-download_of_GeoModel_(2024.01.27.)")
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
