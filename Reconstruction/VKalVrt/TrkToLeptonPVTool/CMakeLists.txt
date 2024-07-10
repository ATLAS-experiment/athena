################################################################################
# Package: TrkToLeptonPVTool
################################################################################

# Declare the package name:
atlas_subdir( TrkToLeptonPVTool )

# External dependencies:
find_package( ROOT COMPONENTS Core )


# Component(s) in the package:
atlas_add_library  ( TrkToLeptonPVToolLib 
                     src/*.cxx  
                     PUBLIC_HEADERS TrkToLeptonPVTool
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} LINK_LIBRARIES ${ROOT_LIBRARIES}
                     AthenaBaseComps xAODTracking xAODEventInfo GaudiKernel 
                     TrkVKalVrtFitterLib TrkVertexFitterInterfaces BeamSpotConditionsData AthContainers)

atlas_add_component  ( TrkToLeptonPVTool
                       src/components/*.cxx
                       LINK_LIBRARIES TrkToLeptonPVToolLib )

# Install files from the package:
#atlas_install_python_modules( python/*.py )
