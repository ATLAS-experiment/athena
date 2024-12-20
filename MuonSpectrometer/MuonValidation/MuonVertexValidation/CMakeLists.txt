#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# Declare the package
atlas_subdir( MuonVertexValidation )

# Declare external dependencies
find_package( ROOT COMPONENTS MathCore RIO Core Tree Hist Gpad Graf)

# Declare package as a component library
atlas_add_component ( MuonVertexValidation src/*.cxx src/components/*.cxx
                      NO_PUBLIC_HEADERS
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} 
                                     AthAnalysisBaseCompsLib StoreGateLib MuonTesterTreeLib MuonPRDTestLib
                                     FourMomUtils CxxUtils
                                     xAODEventInfo xAODTruth xAODTracking xAODMuon xAODJet xAODMissingET)

# Declare the validation macros as a library, excluding the cxx files that will be compiled to executables 
atlas_add_library( MuonVertexValidationLibMacro util/[!make]*.cxx
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} 
                                   AthAnalysisBaseCompsLib StoreGateLib FourMomUtils )

# declare executables for the plot makers
atlas_add_executable( makeValidationPlots util/makeValidationPlots.cxx
                      LINK_LIBRARIES MuonVertexValidationLibMacro )
                      
atlas_add_executable( makeValidationPlotsComparison util/makeValidationPlotsComparison.cxx
                      LINK_LIBRARIES MuonVertexValidationLibMacro )


atlas_install_python_modules( python/*.py ) # contains the joboptions
