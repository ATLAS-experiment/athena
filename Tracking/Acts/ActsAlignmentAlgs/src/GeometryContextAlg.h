/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_GeometryContextCondAlg_H
#define ACTSGEOMETRY_GeometryContextCondAlg_H

// ATHENA
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"

namespace ActsTrk {
  class GeometryContextAlg : public AthReentrantAlgorithm {
    public:
      GeometryContextAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual ~GeometryContextAlg();

      StatusCode initialize() override;
      StatusCode execute(const EventContext &ctx) const override;


    private:
      SG::ReadHandleKeyArray<ActsTrk::DetectorAlignStore> m_alignStoreKeys{this, "AlignmentStores", {}, ""};

      SG::WriteHandleKey<ActsGeometryContext> m_wchk{this, "ActsAlignmentKey", "ActsAlignment", "cond handle key"};
  };
}
#endif
