/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

namespace ExpressionParsing {
  class MultipleProxyLoader : public IProxyLoader {
    public:
      MultipleProxyLoader();
      virtual ~MultipleProxyLoader();

      IProxyLoader* push_back(std::unique_ptr<IProxyLoader> proxyLoader);

      virtual void reset();

      virtual IProxyLoader::VariableType variableTypeFromString(const std::string &varname) const;

      virtual int loadIntVariableFromString(const std::string &varname) const;
      virtual double loadDoubleVariableFromString(const std::string &varname) const;
      virtual std::vector<int> loadVecIntVariableFromString(const std::string &varname) const;
      virtual std::vector<double> loadVecDoubleVariableFromString(const std::string &varname) const;

    private:
      std::vector<std::unique_ptr<IProxyLoader> > m_proxyLoaders;

      using proxyCache_t = CxxUtils::ConcurrentStrMap<IProxyLoader*, CxxUtils::SimpleUpdater>;
      mutable proxyCache_t m_varnameToProxyLoader ATLAS_THREAD_SAFE;
  };
}

#endif // MULTIPLE_PROXY_LOADER_H
