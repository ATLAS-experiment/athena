/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONDALGS_CONDDATAOBJZ_H
#define CONDALGS_CONDDATAOBJZ_H

class CondDataObjZ {
 
public: 
 
  CondDataObjZ():m_val(0) {};
  CondDataObjZ(float f): m_val(f) {};
  virtual ~CondDataObjZ(){};

  void val(float f) { m_val = f; }
  float val() const { return m_val; }

private:
  float m_val;
};


#include <format>
namespace std {

template <>
struct formatter<CondDataObjZ>
  : public formatter<string_view>
{
  template <class FmtContext>
  FmtContext::iterator format (const CondDataObjZ& io, FmtContext& ctx) const
  {
    return std::format_to (ctx.out(), "{}", io.val());
  }
};

} // namespace std


//using the macros below we can assign an identifier (and a version) 
//to the type CondDataObjZ
//This is required and checked at compile time when you try to record/retrieve
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( CondDataObjZ , 6664395 , 1 )
CLASS_DEF( CondCont<CondDataObjZ> , 210255841 , 1 )


#endif
