/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _ExpressionParsing_IAccessor_h_
#define _ExpressionParsing_IAccessor_h_

#include <string>
#include <vector>
class EventContext;

namespace ExpressionParsing {

/** Interface of auxiliary classes to access xAOD object content.
    * Special implementations are provided for e.g. members of AuxElement or AuxVectorBase, decorations or
    * methods of particular xAOD objects.
    */
   class IAccessor {
   public :
      enum VariableType { VT_UNK, VT_INT, VT_DOUBLE, VT_VECINT, VT_VECDOUBLE, VT_VECEMPTY };

      virtual ~IAccessor() {}
      virtual VariableType variableType(const std::string &var_name) const =0;
      virtual int loadInt(const EventContext& ctx, const std::string &var_name) const = 0;
      virtual double loadDouble(const EventContext& ctx, const std::string &var_name) const = 0;
      virtual std::vector<int> loadVecInt(const EventContext& ctx, const std::string &var_name) const = 0;
      virtual std::vector<double> loadVec(const EventContext& ctx, const std::string &var_name) const = 0;
   };

}
#endif
