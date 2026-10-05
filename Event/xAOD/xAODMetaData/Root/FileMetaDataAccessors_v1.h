// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
#ifndef XAODMETADATA_FILEMETADATAACCESSORS_V1_H
#define XAODMETADATA_FILEMETADATAACCESSORS_V1_H

// System include(s):
#include <cstdint>
#include <string>

// EDM include(s):
#include "AthContainers/AuxElement.h"

// Local include(s):
#include "xAODMetaData/versions/FileMetaData_v1.h"

namespace xAOD {

   /// Helper function for getting an accessor for a pre-defined property
   const SG::Accessor< std::string >*
   metaDataTypeStringAccessorV1( FileMetaData_v1::MetaDataType type );

   /// Helper function for getting an accessor for a pre-defined property
   const SG::Accessor< uint32_t >*
   metaDataTypeUIntAccessorV1( FileMetaData_v1::MetaDataType type );

   /// Helper function for getting an accessor for a pre-defined property
   const SG::Accessor< float >*
   metaDataTypeFloatAccessorV1( FileMetaData_v1::MetaDataType type );

   /// Helper function for getting an accessor for a pre-defined property
   const SG::Accessor< char >*
   metaDataTypeCharAccessorV1( FileMetaData_v1::MetaDataType type );

} // namespace xAOD

#endif // XAODMETADATA_FILEMETADATAACCESSORS_V1_H
