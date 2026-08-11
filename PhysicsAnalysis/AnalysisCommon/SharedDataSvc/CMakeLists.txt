# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack


atlas_subdir( SharedDataSvc )

atlas_add_library( SharedDataSvcLib
   SharedDataSvc/*.h SharedDataSvc/*.icc Root/*.cxx
   PUBLIC_HEADERS SharedDataSvc
   LINK_LIBRARIES AsgServicesLib CxxUtils
   PRIVATE_LINK_LIBRARIES )

atlas_add_component( SharedDataSvc
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES SharedDataSvcLib )
