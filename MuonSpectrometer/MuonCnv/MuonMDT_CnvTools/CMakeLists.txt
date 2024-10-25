# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( MuonMDT_CnvTools )

# External dependencies.
find_package( tdaq-common COMPONENTS eformat )

# Component(s) in the package.
atlas_add_library( MuonMDT_CnvToolsLib
   MuonMDT_CnvTools/*.h
   INTERFACE
   PUBLIC_HEADERS MuonMDT_CnvTools
   LINK_LIBRARIES GaudiKernel MuonCnvToolInterfacesLib )

atlas_add_component( MuonMDT_CnvTools
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} ByteStreamData GaudiKernel AthenaKernel CxxUtils
                  AthenaBaseComps StoreGateLib AtlasDetDescr Identifier ByteStreamCnvSvcBaseLib
                  GeoPrimitives MdtCalibInterfacesLib MuonReadoutGeometry MuonDigitContainer
                  MuonIdHelpersLib MuonRDO MuonPrepRawData MuonCablingData GeoModelUtilities MuonReadoutGeometryR4
                  MuonCnvToolInterfacesLib MuonMDT_CnvToolsLib xAODMuonPrepData)
