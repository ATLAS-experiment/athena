/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MultipleProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef MULTIPLE_PROXY_LOADER_H
#define MULTIPLE_PROXY_LOADER_H

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/ConcurrentStrMap.h"
#include "CxxUtils/SimpleUpdater.h"
#include "ExpressionEvaluation/IProxyLoader.h"

#include <vector>
#include <map>
#include <utility>

namespace ExpressionParsing {
  class MultipleProxyLoader : public IProxyLoader {
    public:
      MultipleProxyLoader();
      virtual ~MultipleProxyLoader();

      IProxyLoader* push_back(std::unique_ptr<IProxyLoader> proxyLoader);

      virtual void reset() override;
      virtual IAccessor::VariableType variableType(const std::string &var_name) const override;

      virtual std::pair< IAccessor::VariableType, const IAccessor &>
              getAccessorFromString(const EventContext &ctx, const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      std::vector<std::unique_ptr<IProxyLoader> > m_proxyLoaders;

     using proxyCache_t = CxxUtils::ConcurrentStrMap<const IAccessor *, CxxUtils::SimpleUpdater>;
      mutable proxyCache_t m_varnameToProxyLoader ATLAS_THREAD_SAFE;
  };
}

#endif // MULTIPLE_PROXY_LOADER_H
