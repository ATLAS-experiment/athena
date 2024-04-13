# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VP1AlgsEventProd )

# Component(s) in the package:
atlas_add_component( VP1AlgsEventProd
   VP1AlgsEventProd/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps GaudiKernel PathResolver
   VP1UtilsBase EventDisplaysOnlineLib)

atlas_install_python_modules( python/*.py)
