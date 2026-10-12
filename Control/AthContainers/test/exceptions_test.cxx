/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/exceptions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for exceptions.
 */


#undef NDEBUG
#include "AthContainers/exceptions.h"
#include "AthContainers/AuxTypeRegistry.h"
#include <print>


void test1()
{
  std::println ("test1");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t id1 = r.getAuxID<int> ("foo");
  SG::auxid_t id2 = r.getAuxID<float> ("bar", "cl");

  std::println ("{}",  SG::ExcNoAuxStore(id1).what());
  std::println ("{}",  SG::ExcNoAuxStore(id2).what());
  std::println ("{}",  SG::ExcNoAuxStore(SG::null_auxid).what());
  std::println ("{}",  SG::ExcNoAuxStore("op").what());

  std::println ("{}",  SG::ExcBadAuxVar(id1).what());

  std::println ("{}",  SG::ExcConstAuxData("someop", id1).what());
  std::println ("{}",  SG::ExcConstAuxData("otherop").what());

  std::println ("{}",  SG::ExcUntrackedSetStore().what());

  std::println ("{}",  SG::ExcBadPrivateStore("op").what());

  std::println ("{}",  SG::ExcAuxTypeMismatch(id1, typeid(int), typeid(float),
                                              "aaa", "aaa").what());
  std::println ("{}",  SG::ExcAuxTypeMismatch(id2, typeid(int), typeid(float),
                                              "aaa", "bbb").what());

  std::println ("{}",  SG::ExcInsertionInBaseClass("op", typeid(int), typeid(float)).what());

  std::println ("{}",  SG::ExcStoreLocked(id1).what());
  std::println ("{}",  SG::ExcStoreLocked("op").what());
  std::println ("{}",  SG::ExcNonowningContainer().what());
  std::println ("{}",  SG::ExcUnknownAuxItem("foo").what());
  std::println ("{}",  SG::ExcUnknownAuxItem("foo", "bar").what());
  std::println ("{}",  SG::ExcUnknownAuxItem("foo", "bar", &typeid(int)).what());
  std::println ("{}",  SG::ExcDVToELV("why").what());
  std::println ("{}",  SG::ExcViewVectorNotView().what());
  std::println ("{}",  SG::ExcMissingViewVectorCLID(typeid(int)).what());
  std::println ("{}",  SG::ExcInsertMoveOwnershipMismatch().what());
  std::println ("{}",  SG::ExcFlagMismatch(id2, typeid(int),
                                           SG::AuxVarFlags::Atomic,
                                           SG::AuxVarFlags::Atomic | SG::AuxVarFlags::Linked).what());
  std::println ("{}",  SG::ExcLinkMismatch(id1, typeid(int), id2, SG::null_auxid).what());
  std::println ("{}",  SG::ExcNoLinkedVar(id1, typeid(int)).what());
  std::println ("{}",  SG::ExcInvalidThinningTarget(1234, "asd").what());
  std::println ("{}",  SG::ExcBadIterSwap().what());
  std::println ("{}",  SG::ExcAllocOwnership().what());
  std::println ("{}",  SG::ExcBadVarName("foo asd").what());
  std::println ("{}",  SG::ExcJaggedVecOverlappingCopy().what());
}


int main()
{
  test1();
  return 0;
}
