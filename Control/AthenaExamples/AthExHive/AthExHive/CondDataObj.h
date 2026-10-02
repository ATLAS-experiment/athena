/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONDALGS_CONDDATAOBJ_H
#define CONDALGS_CONDDATAOBJ_H

class CondDataObj {
 
public: 
 
  CondDataObj():m_val(0) {};
  CondDataObj(int i): m_val(i) {};
  virtual ~CondDataObj(){};

  void val(int i) { m_val = i; }
  int val() const { return m_val; }

private:
  int m_val;
};


#include <format>
namespace std {

template <>
struct formatter<CondDataObj>
  : public formatter<string_view>
{
  template <class FmtContext>
  FmtContext::iterator format (const CondDataObj& io, FmtContext& ctx) const
  {
    return std::format_to (ctx.out(), "{}", io.val());
  }
};

} // namespace std


//using the macros below we can assign an identifier (and a version) 
//to the type CondDataObj
//This is required and checked at compile time when you try to record/retrieve
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( CondDataObj , 232539028 , 1 )
CLASS_DEF( CondCont<CondDataObj> , 261788530 , 1 )


#endif
