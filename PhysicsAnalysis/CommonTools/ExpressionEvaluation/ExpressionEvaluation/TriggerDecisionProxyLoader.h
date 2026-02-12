/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TriggerDecisionProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch),
// James Catmore (james.catmore@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef TRIGGERDECISIONPROXYLOADER_H
#define TRIGGERDECISIONPROXYLOADER_H

#include "ExpressionEvaluation/RelayProxyLoader.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

namespace ExpressionParsing {
  class TriggerDecisionProxyLoader : public RelayProxyLoader {
    public:

      TriggerDecisionProxyLoader(ToolHandle<Trig::TrigDecisionTool>& trigDecTool) : m_trigDec(trigDecTool) { }
      virtual ~TriggerDecisionProxyLoader();
      virtual void reset() override;

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      ToolHandle<Trig::TrigDecisionTool> m_trigDec;
  };
}

#endif // TRIGGERDECISIONPROXYLOADER_H
