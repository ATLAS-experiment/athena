# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthDeviceComps)

# External dependencies.
find_package(vecmem)

# Component(s) in the package.
atlas_add_component(AthDeviceComps
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps AthDeviceInterfacesLib vecmem::core)

# Install files from the package.
atlas_install_python_modules(python/*.py)
