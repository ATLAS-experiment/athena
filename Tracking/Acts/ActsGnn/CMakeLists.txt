# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGnn )

# External dependencies:
find_package( Acts COMPONENTS Core PluginJson OPTIONAL_COMPONENTS PluginGnn )
find_package( Boost )

# The IGnnPipelineTool interface lives in ActsToolInterfaces alongside the other
# Acts tool interfaces, so no dedicated interface library is needed here.

# The Acts PluginGnn is optional: when it is not available the GnnPipeline
# component still builds and registers, but runs as a no-op.
set( _extraLibs )
if( TARGET Acts::PluginGnn )
  list( APPEND _extraLibs Acts::PluginGnn )
endif()

atlas_add_component( ActsGnn
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES}
                       Acts::Core
                       ${_extraLibs}
                       ActsEventLib
                       ActsInteropLib
                       ActsToolInterfacesLib
                       AthenaBaseComps
                       AthenaKernel
                       CxxUtils
                       GaudiKernel
                       Identifier
                       InDetIdentifier
                       StoreGateLib
                       xAODInDetMeasurement )
