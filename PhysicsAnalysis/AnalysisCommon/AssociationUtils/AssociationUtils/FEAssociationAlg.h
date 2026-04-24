/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_FEASSOCIATIONALG_H
#define ASSOCIATIONUTILS_FEASSOCIATIONALG_H

#ifndef XAOD_STANDALONE
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#else
#include "AnaAlgorithm/AnaAlgorithm.h"
#endif

#include "AsgTools/ToolHandle.h"
#include "AssociationUtils/IFEAssociationTool.h"

// Forward declarations to stay dual-use friendly
class ISvcLocator;
class EventContext;

namespace ORUtils {

class FEAssociationAlg final :
#ifndef XAOD_STANDALONE
  public AthReentrantAlgorithm
#else
  public EL::AnaAlgorithm
#endif
{
public:
  FEAssociationAlg(const std::string& name, ISvcLocator* svcLoc);
  virtual ~FEAssociationAlg() override = default;
  
  virtual StatusCode initialize() override;

#ifndef XAOD_STANDALONE
  virtual StatusCode execute(const EventContext& ctx) const override;
#else
  virtual StatusCode execute() override;
#endif

private:
  ToolHandle<ORUtils::IFEAssociationTool> m_tool{
    this,
    "FEAssociationTool",
    "ORUtils::FEAssociationTool/FEAssociationTool",
    "Tool building FE association map"
  };
};

} // namespace ORUtils

#endif