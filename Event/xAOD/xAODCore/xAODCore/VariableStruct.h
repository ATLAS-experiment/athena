// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODCore/VariableStruct.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2026
 * @brief Define structures for fast filling of xAOD objects.
 */


#ifndef XAODCORE_VARIABLESTRUCT_H
#define XAODCORE_VARIABLESTRUCT_H


#include "AthContainers/AuxVectorData.h"
#include "AthContainers/Accessor.h"


namespace xAOD {


/**
 * @brief Define structures for fast filling of xAOD objects.
 *
 * Normally one initializes xAOD objects using the setter interfaces
 * of individual objects.  While these are designed to be fast, there is
 * some overhead which can add up if one is creating very many objects.
 * As a faster, but less convenient/safe alternative, one can retrieve
 * pointers to each variable array and then fill those directly without
 * going through the xAOD object interface.  The code here provides
 * some support for this.  Declare a structure deriving from @c VariableStruct.
 * Declare each variable using the @c AUXSTORE_VARSTRUCT_VAR macro.
 * Then when an instance of that structure is created, it will contain
 * pointers to the xAOD variables.  For example, for a fictional
 * Cluster xAOD object:
 *
 *@code
 *
 * struct ClusterVars : public xAOD::VariableStruct
 * {
 *   using xAOD::VariableStruct::VariableStruct;
 *
 *   AUXSTORE_VARSTRUCT_VAR(int,   id);
 *   AUXSTORE_VARSTRUCT_VAR(float, xpos);
 *   AUXSTORE_VARSTRUCT_VAR(float, ypos);
 * };
 *
 * ...
 *
 * xAOD::ClusterContainer cont = ...;
 * cont.push_new (nclust, [](){ return new xAOD::Custer; });
 * ClusterVars vars (cont);
 * for (size_t i = 0; i < nclust; i++) {
 *   vars.id[i] = clust_id(i);
 *   vars.xpos[i] = clust_xpos(i);
 *   vars.ypos[i] = clust_ypos(i);
 * }
 @endcode
 */
class VariableStruct
{
public:
  VariableStruct (SG::AuxVectorData& cont) : m_cont (cont) {}
  SG::AuxVectorData& m_cont;
};


} // namespace xAOD


/**
 * @brief Declare a member of an xAOD variable struct, giving the type
 *        and xAOD variable name.
 */
#define AUXSTORE_VARSTRUCT_VAR(TYPE, NAME)                      \
  static SG::Accessor<TYPE>::span getSpan##NAME(xAOD::VariableStruct& vars) { \
    static const SG::Accessor<TYPE> acc (#NAME);                \
    return acc.getDataSpan (vars.m_cont);                       \
  }                                                             \
  SG::Accessor<TYPE>::span NAME = getSpan##NAME(*this)


#endif // not XAODCORE_VARIABLESTRUCT_H
