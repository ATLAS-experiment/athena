/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef EXPRESSIONPARSING_RRELAYPROXYLOADER_H
#define EXPRESSIONPARSING_RRELAYPROXYLOADER_H
#include "IProxyLoader.h"

namespace ExpressionParsing {
   class RelayProxyLoader : public IProxyLoader {
   public:
      virtual std::pair< IAccessor::VariableType, const IAccessor &>
      getAccessorFromString([[maybe_unused]] const EventContext &ctx, const std::string &varname) const override {
         return {variableTypeFromString(varname), *this};
      }
      virtual VariableType variableType([[maybe_unused]] const std::string &var_name) const override {
         return IProxyLoader::VT_UNK;
      }

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const = 0;
   };
}


#endif
