/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef xAODAUXILLARYMEASUREMENT_DICT_H
#define xAODAUXILLARYMEASUREMENT_DICT_H

#include "xAODCore/tools/DictHelpers.h"

#include "xAODAuxillaryMeasurement/AuxillaryMeasurement.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementContainer.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer1D.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer2D.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurementAuxContainer3D.h"


// Instantiate all necessary types for the dictionary.
namespace {
struct GCCXML_DUMMY_INSTANTIATION_XAODMUONPRD {
    // Type(s) needed for the dictionary generation to succeed.
    XAOD_INSTANTIATE_NS_CONTAINER_TYPES(xAOD, AuxillaryMeasurementContainer);
};
}  // namespace

#endif