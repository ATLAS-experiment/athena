/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSMATERIALSTEPCONVERTERTOOL_H
#define ACTSGEOMETRY_ACTSMATERIALSTEPCONVERTERTOOL_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsGeometryInterfaces/IActsMaterialStepConverterTool.h"

class ActsMaterialStepConverterTool : public extends<AthAlgTool, IActsMaterialStepConverterTool>
{

public:
  virtual StatusCode initialize() override;

  ActsMaterialStepConverterTool(const std::string &type, const std::string &name,
                                const IInterface *parent);

  virtual
  const Acts::RecordedMaterialTrack
  convertToMaterialTrack(const Trk::MaterialStepCollection &colStep) const override;

  static constexpr double s_tolerance{0.1}; // mm

};
#endif
