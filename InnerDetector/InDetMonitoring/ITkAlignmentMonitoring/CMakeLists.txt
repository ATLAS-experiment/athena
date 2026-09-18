################################################################################
# Package: ITkAlignmentMonitoring
################################################################################

# Declare the package name:
atlas_subdir( ITkAlignmentMonitoring )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( ITkAlignmentMonitoring
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} GaudiKernel AthenaBaseComps AthenaMonitoringLib AthContainers AtlasDetDescr Identifier EventPrimitives xAODEventInfo xAODTracking InDetIdentifier InDetPrepRawData InDetRIO_OnTrack Particle TrkGeometry TrkSurfaces TrkEventPrimitives TrkParameters TrkParticleBase TrkRIO_OnTrack TrkTrack TrkTrackSummary VxVertex TrkExInterfaces TrkToolInterfaces TrkVertexFitterInterfaces InDetTrackSelectionToolLib BeamSpotConditionsData InDetAlignGenToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.sh )
