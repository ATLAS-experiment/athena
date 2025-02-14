#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script building all the externals necessary for Athena.
#

# Set up the variables necessary for the script doing the heavy lifting.
ATLAS_PROJECT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
ATLAS_EXT_PROJECT_NAME="AthenaExternals"
ATLAS_BUILDTYPE="RelWithDebInfo"
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=106
                        -DLCG_VERSION_POSTFIX="b_ATLAS_1"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v38r0.004/Gaudi-v38r0.004.tar.gz;URL_MD5;245d7a6d173200060236ba7679688496"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v32.0.2.tar.gz;URL_HASH;SHA256=a817a9b6434ae206c36b1c25df6139ffcde95234944d09e718125e14da7996b8"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/5.0.3/GeoModel-5.0.3.tar.bz2;URL_MD5;8f00da4b18a60c574e6f3e4badcde7d2"
			-DATLAS_GEOMODEL_PATCH=""
                        -DATLAS_GEOMODEL_FORCEDOWNLOAD_MESSAGE="Forcing_the_re-download_of_GeoModel_(2024.01.27.)"
                        -DATLAS_VECMEM_SOURCE="URL;http://cern.ch/atlas-software-dist-eos/externals/vecmem/v1.13.0.tar.gz;https://github.com/acts-project/vecmem/archive/refs/tags/v1.13.0.tar.gz;URL_MD5;02fe327552c21779f501c224b8c95e87"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
