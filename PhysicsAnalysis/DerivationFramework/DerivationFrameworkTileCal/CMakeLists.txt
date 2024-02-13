################################################################################
# Package: DerivationFrameworkTileCal
################################################################################

# Declare the package name:
atlas_subdir( DerivationFrameworkTileCal )

# Component(s) in the package:
atlas_add_component( DerivationFrameworkTileCal
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES DerivationFrameworkInterfaces AthenaBaseComps xAODMuon xAODTracking CaloEvent CaloIdentifier AthContainers AthenaKernel xAODBase xAODCaloEvent GaudiKernel RecoToolInterfaces TileEvent TileIdentifier TileDetDescr TrkParameters TrkParametersIdentificationHelpers ParticlesInConeToolsLib )

atlas_install_python_modules( python/*.py
                              POST_BUILD_CMD ${ATLAS_FLAKE8} )

