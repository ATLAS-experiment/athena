/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef EXPRESSIONPARSING_RELAYACCESSOR_H
#define EXPRESSIONPARSING_RELAYACCESSOR_H
#include "IProxyLoader"

namespace ExpressionParsing {
   /// @brief convenience base class to turn proxy loaders into cachable accessors
   class RelayAccessor : public IProxyLoader {
   public:
      std::pair< IProxyLoader::VariableType, const IAccessor &>
      getAccessorFromString([[maybe_unused]] const EventContext &ctx, const std::string &varname) const {
         return std::make_pair(variableTypeFromString(varname), *this);
      }
      /// @note should be overloaded by the derived class.
      virtual VariableType variableType() const override {
         return IProxyLoader::VT_UNK;
      }

      virtual IProxyLoader::VariableType TestProxyLoader::variableTypeFromString(const std::string &varname) const = 0;
   };
}
#endif;
