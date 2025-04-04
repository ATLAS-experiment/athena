################################################################################
# Package: LArCondUtils
################################################################################

# Declare the package name:
atlas_subdir( LArCondUtils )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( LArCondUtils
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${CORAL_LIBRARIES} AthenaBaseComps StoreGateLib AthenaPoolUtilities Identifier GaudiKernel CaloDetDescrLib CaloIdentifier CaloGeoHelpers AthenaKernel LArCablingLib LArHV LArReadoutGeometry LArIdentifier LArRawConditions LArRecConditions RegistrationServicesLib CaloInterfaceLib
                     PRIVATE_LINK_LIBRARIES PathResolver )

# Install files from the package:
atlas_install_runtime( share/LArEmLvl1.data share/LArHecLvl1.data share/LArFcalLvl1.data )
atlas_install_scripts( share/LArBuildHVPathologies.sh )

atlas_install_scripts( share/LArHVPathology2Ascii.py
		       share/fillPileUpNoiseLumi.py
		       share/dumpHVPathFromNtuple.py
		       POST_BUILD_CMD ${ATLAS_FLAKE8} )

