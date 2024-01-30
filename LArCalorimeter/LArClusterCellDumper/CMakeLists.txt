## This file goes INSIDE a package directory.
## Its responsible to point to all files within the project, in order to compile them.
## automatically generated CMakeLists.txt file
cmake_minimum_required( VERSION 3.10 )

# Declare the package
atlas_subdir( LArClusterCellDumper )

find_package( ROOT COMPONENTS MathCore RIO Core Tree Hist)

atlas_add_component (LArClusterCellDumper 
                     LArClusterCellDumper/*.h 
                     src/*.cxx 
                     src/components/*.cxx
       LINK_LIBRARIES AthenaBaseComps AthenaPoolUtilities CaloDetDescrLib xAODEventInfo xAODCaloEvent xAODTruth xAODEgamma CaloEvent LArIdentifier LArRecEvent LArCablingLib LArCOOLConditions LArDigitizationLib LArElecCalib LArHV CaloUtilsLib AthContainers Identifier LArRawEvent LArRawConditions xAODTracking EgammaAnalysisInterfacesLib CaloConditions LumiBlockData LArRecConditions CaloCondBlobObjs LArSimEvent
)

# Install python modules
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
