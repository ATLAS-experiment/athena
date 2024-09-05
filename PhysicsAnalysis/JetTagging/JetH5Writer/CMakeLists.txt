#
# Cmake for JetH5Writer
#

# Set the name of the package:
atlas_subdir( JetH5Writer )

# We don't want any warnings in compilation
add_compile_options(-Werror)

# common requirements
atlas_add_library(JetH5WriterLib
  src/EventInfoWriter.cxx
  src/IParticleWriter.cxx
  src/PrimitiveHelpers.cxx
  LINK_LIBRARIES
  AthenaBaseComps
  xAODEventInfo
  xAODBase
  HDF5Utils
  xAODBTagging
  PUBLIC_HEADERS
  JetH5Writer
  )
atlas_add_component(JetH5Writer
  src/EventInfoWriterAlg.cxx
  src/IParticleWriterAlg.cxx
  src/AlgHelpers.cxx
  src/components/JetH5Writer_entries.cxx
  LINK_LIBRARIES
  JetH5WriterLib
  IH5GroupSvc
  )


