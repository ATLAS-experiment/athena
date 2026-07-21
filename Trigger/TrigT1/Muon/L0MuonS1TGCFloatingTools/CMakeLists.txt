# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L0MuonS1TGCFloatingTools )

atlas_add_component( L0MuonS1TGCFloatingTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps L0MuonS1TGCToolInterfaces MuonRDO )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
