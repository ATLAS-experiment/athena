################################################################################
# Package: G4FastSimulation
################################################################################

# Declare the package name:
atlas_subdir( G4FastSimulation )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( XercesC )
find_package( ROOT COMPONENTS Core Tree RIO Hist Physics MathCore ROOTDataFrame )
find_package( FastCaloSim )

# Athena-only sources and dependencies:
set( extra_lib )
set( extra_src )
if( NOT SIMULATIONBASE )
   set( extra_src src/FastCaloSimParamHitAnalysis.cxx
                  src/FatrasG4.cxx src/FatrasG4Tool.cxx
                  src/AFatrasG4.cxx src/AFatrasG4Tool.cxx )
   set( extra_lib ISF_FastCaloSimInterfaces ISF_FastCaloSimParametrizationLib ISF_FastCaloSimEvent
        AthenaPoolUtilities AtlasHepMCLib CaloDetDescrLib
        GeneratorObjects GeoModelInterfaces LArElecCalib
        LArSimEvent NavFourMom TileConditionsLib TileDetDescr
        TileIdentifier TileSimEvent TrackRecordLib xAODCaloEvent
        InDetSimEvent )  # InDetSimEvent for ActsFatrasG4
endif()

# Component(s) in the package:
atlas_add_library( G4FastSimulation
                   src/DeadMaterialShower.cxx  src/DeadMaterialShowerTool.cxx  src/SimpleFastKiller.cxx  src/SimpleFastKillerTool.cxx
                   src/FastCaloSimModel.cxx  src/FastCaloSimTool.cxx
                   src/CaloCellContainerSD.cxx  src/CaloCellContainerSDTool.cxx
                   ${extra_src}
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${VDT_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${XERCESC_LIBRARIES} ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} ${FASTCALOSIM_LIBRARIES}
                       GaudiKernel G4AtlasToolsLib FCALGeoPlugin HitManagement
                       AthenaBaseComps AthenaKernel TruthUtils CxxUtils StoreGateLib MCTruth
                       CaloEvent CaloInterfaceLib CaloIdentifier Identifier
                       ${extra_lib}
                )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
