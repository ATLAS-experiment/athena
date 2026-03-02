/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file xAODCore/test/ut_xaodcore_variablestruct_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2026
 * @brief Unit test for VariableStruct.
 */


#undef NDEBUG
#include "xAODCore/VariableStruct.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include <iostream>
#include <cassert>


class ClusterAuxContainer : public xAOD::AuxContainerBase
{
public:
  AUXVAR_DECL( int, id );
  AUXVAR_DECL( float, xpos );
  AUXVAR_DECL( float, ypos );
};


class Cluster : public SG::AuxElement
{
public:
  int id() const;
  float xpos() const;
  float ypos() const;
};

AUXSTORE_PRIMITIVE_GETTER(Cluster, int, id);
AUXSTORE_PRIMITIVE_GETTER(Cluster, float, xpos);
AUXSTORE_PRIMITIVE_GETTER(Cluster, float, ypos);

using ClusterContainer = DataVector<Cluster>;


struct ClusterVars : public xAOD::VariableStruct
{
  AUXSTORE_VARSTRUCT_VAR(int, id);
  AUXSTORE_VARSTRUCT_VAR(float, xpos);
  AUXSTORE_VARSTRUCT_VAR(float, ypos);
};


void test1()
{
  std::cout << "test1\n";

  ClusterContainer cont;
  ClusterAuxContainer store;
  cont.setStore (&store);

  const size_t N = 10;
  cont.push_new (N, [](){ return new Cluster; });

  ClusterVars vars (cont);
  assert (vars.id != nullptr);
  assert (vars.xpos != nullptr);
  assert (vars.ypos != nullptr);

  for (size_t i = 0; i < N; i++) {
    vars.id[i] = i+10;
    vars.xpos[i] = i + 20.5;
    vars.ypos[i] = i + 30.5;
  }

  assert (cont.size() == N);
  for (size_t i = 0; const Cluster* c : cont) {
    assert (c->id() == static_cast<int>(i+10));
    assert (c->xpos() == i+20.5);
    assert (c->ypos() == i+30.5);
    ++i;
  }
}


int main()
{
  std::cout << "xAODCore/ut_xaodcore_variablestruct_test\n";
  test1();
  return 0;
}
