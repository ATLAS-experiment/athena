/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// IProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef IPROXY_LOADER_H
#define IPROXY_LOADER_H

#include "IAccessor.h"
#include <string>
#include <vector>
#include <utility>

namespace ExpressionParsing {
  class IProxyLoader : public IAccessor {
    public:
      using VariableType = IAccessor::VariableType;

      virtual ~IProxyLoader() { }

      virtual void reset() = 0;

      virtual std::pair< IAccessor::VariableType, const IAccessor &>
              getAccessorFromString(const EventContext &ctx, const std::string &varname) const = 0;

  };
}

#endif // IPROXY_LOADER_H
