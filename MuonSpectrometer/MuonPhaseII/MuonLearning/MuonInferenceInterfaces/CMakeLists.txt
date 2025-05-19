# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonInferenceInterfaces
################################################################################

# Declare the package name:
atlas_subdir( MuonInferenceInterfaces )

find_package( onnxruntime REQUIRED)

atlas_add_library( MuonInferenceInterfaces
                   src/*.cxx
                   PUBLIC_HEADERS MuonInferenceInterfaces
                   INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIRS}
                   LINK_LIBRARIES MuonReadoutGeometryR4 AthenaBaseComps MuonPatternHelpers
                                  MuonSpacePoint MuonPatternEvent ${ONNXRUNTIME_LIBRARIES})

