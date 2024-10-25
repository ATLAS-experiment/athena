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
ATLAS_EXTRA_CMAKE_ARGS=(-DLCG_VERSION_NUMBER=106
                        -DLCG_VERSION_POSTFIX="_ATLAS_13"
                        -DATLAS_GAUDI_SOURCE="URL;https://gitlab.cern.ch/atlas/Gaudi/-/archive/v39r0.000/Gaudi-v39r0.000.tar.gz;URL_MD5;d6a88e827d52286bed248f7e4fba6b33"
                        -DATLAS_ACTS_SOURCE="URL;https://github.com/acts-project/acts/archive/refs/tags/v37.2.0.tar.gz;URL_HASH;SHA256=526ac9b25a2672de893c5f42f4a746156d99f00c2b2ca368c80c37fa72b1d004"
                        -DATLAS_GEOMODEL_SOURCE="URL;https://gitlab.cern.ch/GeoModelDev/GeoModel/-/archive/6.6.0/GeoModel-6.6.0.tar.bz2;URL_MD5;66c32047090b20afb75cd5befefce78a"
                        -DATLAS_VECMEM_SOURCE="URL;http://cern.ch/atlas-software-dist-eos/externals/vecmem/v1.5.0.tar.gz;https://github.com/acts-project/vecmem/archive/refs/tags/v1.5.0.tar.gz;URL_MD5;3cc5a3bb14b93f611513535173a6be28"
                        -DATLAS_GEANT4_USE_LTO=TRUE
                        -DATLAS_VECGEOM_USE_LTO=TRUE)
ATLAS_EXTRA_MAKE_ARGS=()

# Let "the common script" do all the heavy lifting.
source "${ATLAS_PROJECT_DIR}/../../Build/AtlasBuildScripts/build_project_externals.sh"
