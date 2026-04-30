// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/TopBase.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2018
 * @brief Calculate topmost accessible base accessible via SG_BASES.
 */


#ifndef ATHENAKERNEL_TOPBASE_H
#define ATHENAKERNEL_TOPBASE_H


#include "AthenaKernel/BaseInfo.h"
#include "AthenaKernel/tools/safe_clid.h"
#include <type_traits>


namespace SG {


/**
 * @brief Calculate topmost base accessible via SG_BASES that also has
 *        a defined CLID.
 *
 * For example, if we have
 *
 *@code
 *   SG_BASES(C2, C1);
 *   SG_BASES(C3, C2);
 @endcode
 *
 * with all three classes having CLIDs defined,
 * then TopBase<C1>::type, TopBase<C2>::type, and TopBase<C3>::type all yield @c C1.
 *
 * If, on the other hand, there is no CLID for C2, then TopBase<C3>::type
 * will yield C3.
 *
 * For the use of this, see the comments for ReadDecorHandleKey.
 */
template <class T>
struct TopBase
{
  using Base1 = typename SG::Bases<T>::bases::Base1;
  static const bool has_base = !std::is_same<Base1, SG::NoBase>::value;
  static const bool base_has_clid = SG::safe_clid<Base1>();
  using type = typename std::conditional<has_base && base_has_clid,
                                         typename TopBase<Base1>::type,
                                         T>::type;
};


template <>
struct TopBase<SG::NoBase>
{
  using type = SG::NoBase;
};


} // namespace SG


#endif // not ATHENAKERNEL_TOPBASE_H
