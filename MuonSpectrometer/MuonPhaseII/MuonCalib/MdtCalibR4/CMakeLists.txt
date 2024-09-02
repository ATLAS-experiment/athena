################################################################################
# Package: MdtCalibR4
################################################################################

# Declare the package name:
atlas_subdir( MdtCalibR4 )

# Declare the package's dependencies:
atlas_add_library( MdtCalibR4
                      src/*.cxx
                      PUBLIC_HEADERS MdtCalibR4
                      LINK_LIBRARIES AthenaKernel MuonIdHelpersLib GaudiKernel AthenaPoolUtilities MuonReadoutGeometryR4
                                    MdtCalibData )