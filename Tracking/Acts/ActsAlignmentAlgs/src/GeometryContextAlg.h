/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GeometryContextCondAlg_H
#define ACTSGEOMETRY_GeometryContextCondAlg_H

// ATHENA
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

// PACKAGE
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"

namespace ActsTrk {
  class GeometryContextAlg : public AthReentrantAlgorithm {
    public:
      using AthReentrantAlgorithm::AthReentrantAlgorithm;
      virtual ~GeometryContextAlg();

      StatusCode initialize() override;
      StatusCode execute(const EventContext &ctx) const override;


    private:
      SG::ReadHandleKeyArray<DetectorAlignStore> m_alignStoreKeys{this, "AlignmentStores", {}, ""};

      SG::WriteHandleKey<GeometryContext> m_wchk{this, "ActsAlignmentKey", "ActsAlignment", "cond handle key"};
  };
}
#endif
