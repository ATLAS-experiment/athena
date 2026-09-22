# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L1MuonS1TGCBitwiseTools )

atlas_add_component( L1MuonS1TGCBitwiseTools
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps L1MuonS1TGCToolInterfaces )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
