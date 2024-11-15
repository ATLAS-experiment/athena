# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TGC_CondCabling )

# Component(s) in the package:
atlas_add_component( TGC_CondCabling
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel MuonCondInterface AthenaPoolUtilities PathResolver)

# Install files from the package:
atlas_install_runtime( share/*.db )
