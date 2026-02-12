/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TestProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef TEST_PROXY_LOADER_H
#define TEST_PROXY_LOADER_H

#include "ExpressionEvaluation/RelayProxyLoader.h"

#include <atomic>

namespace ExpressionParsing {
  class TestProxyLoader : public RelayProxyLoader {
    public:
      TestProxyLoader() : m_intAccessCount(0) { }
      virtual ~TestProxyLoader();

      virtual void reset() override;

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      mutable std::atomic<unsigned int> m_intAccessCount;
  };
}

#endif // TEST_PROXY_LOADER_H
