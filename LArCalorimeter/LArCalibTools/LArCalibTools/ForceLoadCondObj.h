/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORCELOADCONDOBJ_H
#define FORCELOADCONDOBJ_H

// ForceLoadCondObj.h

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include <vector>
#include <string>

class StoreGateSvc;
class IClassIDSvc;

class ForceLoadCondObj: public AthReentrantAlgorithm
{
public:
    using AthReentrantAlgorithm ::AthReentrantAlgorithm;
    virtual ~ForceLoadCondObj();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
  ServiceHandle<IClassIDSvc> p_clidsvc { this, "ClassIDSvc", "ClassIDSvc" };

  StringArrayProperty m_objectList { this, "ObjectList", {}, "list of 'object#key'" };
};

#endif // REGISTRATIONSVC_OUTPUTCONDALG_H
