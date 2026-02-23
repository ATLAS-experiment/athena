/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ExpressionParsing_BaseAccessor_h_
#define ExpressionParsing_BaseAccessor_h_

#include <stdexcept>
#include <sstream>
#include "ExpressionEvaluation/IProxyLoader.h"
#include "ExpressionEvaluation/IAccessor.h"

namespace ExpressionParsing {

   /** Base class of xAOD object content accessors.
    */
   class BaseAccessor : public IAccessor {
   public:
      BaseAccessor(IAccessor::VariableType variable_type) : m_variableType(variable_type) {}

      static void throwInvalidHandle(const std::string &key) {
         std::stringstream msg;
         msg << "Failed to create read handle for " << key;
         throw std::runtime_error(key);
      }
      static void throwVectorContainsNotOneElement(const std::string &key, std::size_t n_elements) {
         std::stringstream msg;
         msg << "Cannot convert to scalar since the aux vector data container " << key << " does not contain exactly one element but " << n_elements;
         throw std::runtime_error(key);
      }
      virtual IAccessor::VariableType variableType([[maybe_unused]] const std::string &var_name) const override {
         return m_variableType;
      }

   private:
      IAccessor::VariableType m_variableType;
   };

   /** Special accessor to handle empty containers until the correct accessor can be created.
    */
   class EmptyVectorAccessor : public IAccessor {
   public :
      virtual IAccessor::VariableType variableType([[maybe_unused]] const std::string &var_name) const override { return IProxyLoader::VT_VECEMPTY; }
      virtual int loadInt([[maybe_unused]] const EventContext& ctx,[[maybe_unused]] const std::string &var_name) const override
        { throwEmptyVector(); return 0; }
      virtual double loadDouble([[maybe_unused]] const EventContext& ctx,[[maybe_unused]] const std::string &var_name) const override
        { throwEmptyVector(); return 0.;}
      virtual std::vector<int> loadVecInt([[maybe_unused]] const EventContext& ctx,[[maybe_unused]] const std::string &var_name) const override
        { return std::vector<int>(); }
      virtual std::vector<double> loadVec([[maybe_unused]] const EventContext& ctx,[[maybe_unused]] const std::string &var_name) const override
        { return std::vector<double>(); }
   private:
      void throwEmptyVector() const {
         throw std::runtime_error("Attempt to convert empty vector into scalar.");
      }
   };

}
#endif
