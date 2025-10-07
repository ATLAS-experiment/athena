/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InDetRawDataCLASS_DEF.h
//   Header file for class InDetRawDataCLASS_DEF
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Class to contain all the CLASS_DEF for Containers and Collections
///////////////////////////////////////////////////////////////////
// Version 1.0 25/09/2002 Veronique Boisvert
///////////////////////////////////////////////////////////////////

#ifndef INDETRAWDATA_INDETRAWDATACLASS_DEF_H
#define INDETRAWDATA_INDETRAWDATACLASS_DEF_H

// Include all headers here - just the containers and collections are enough
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetRawData/TRT_RDO_Container.h"

#include "InDetRawData/PixelRDO_Collection.h"
#include "InDetRawData/SCT_RDO_Collection.h"
#include "InDetRawData/TRT_RDO_Collection.h"

// Explicit template instantiations to ensure dictionary coverage.
// No dummy globals → no static initialization → Coverity clean.

template class DataVector<InDetRawDataCollection<TRT_LoLumRawData>>;
template class DataVector<InDetRawDataCollection<SCT1_RawData>>;
template class DataVector<InDetRawDataCollection<Pixel1RawData>>;
template class DataVector<SCT1_RawData>;

template class InDetRawDataCollection<Pixel1RawData>;
template class InDetRawDataCollection<SCT1_RawData>;
template class InDetRawDataCollection<SCT_RDORawData>;
template class InDetRawDataCollection<TRT_LoLumRawData>;
template class InDetRawDataCollection<TRT_RDORawData>;

#endif // INDETRAWDATA_INDETRAWDATACLASS_DEF_H
