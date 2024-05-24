# Declare the package name:
atlas_subdir( MuonHoughDataNtuple )

# Component(s) in the package:
atlas_add_component( MuonHoughDataNtuple
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaKernel StoreGateLib GaudiKernel AthenaBaseComps
                                    xAODMuon xAODTruth MuonRecToolInterfaces MuonTesterTreeLib)


# Install files from the package:
atlas_install_python_modules( python/*.py )
