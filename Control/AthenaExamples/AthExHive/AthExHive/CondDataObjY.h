/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONDALGS_CONDDATAOBJY_H
#define CONDALGS_CONDDATAOBJY_H

class CondDataObjY {
 
public: 
 
  CondDataObjY():m_val(0) {};
  CondDataObjY(float f): m_val(f) {};
  virtual ~CondDataObjY(){};

  void val(float f) { m_val = f; }
  float val() const { return m_val; }

private:
  float m_val;
};


#include <format>
namespace std {

template <>
struct formatter<CondDataObjY>
  : public formatter<string_view>
{
  template <class FmtContext>
  FmtContext::iterator format (const CondDataObjY& io, FmtContext& ctx) const
  {
    return std::format_to (ctx.out(), "{}", io.val());
  }
};

} // namespace std


//using the macros below we can assign an identifier (and a version) 
//to the type CondDataObjY
//This is required and checked at compile time when you try to record/retrieve
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( CondDataObjY , 6664392 , 1 )
CLASS_DEF( CondCont<CondDataObjY> , 207255848 , 1 )


#endif
