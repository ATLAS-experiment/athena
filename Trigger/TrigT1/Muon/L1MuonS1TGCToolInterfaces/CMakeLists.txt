# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( L1MuonS1TGCToolInterfaces )

atlas_add_library( L1MuonS1TGCToolInterfaces
                   INTERFACE
                   PUBLIC_HEADERS L1MuonS1TGCToolInterfaces
                   LINK_LIBRARIES AthenaKernel GaudiKernel MuonRDO xAODL0MuonCand )
