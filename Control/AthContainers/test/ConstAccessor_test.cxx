/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/ConstAccessor_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Regression tests for ConstAccessor.
 */

#undef NDEBUG
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/exceptions.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <cassert>


namespace SG {


class AuxVectorBase
  : public SG::AuxVectorData
{
public:
  virtual size_t size_v() const override { return 10; }
  virtual size_t capacity_v() const override { return 10; }

  using SG::AuxVectorData::setStore;
  void set (SG::AuxElement& b, size_t index)
  {
    b.setIndex (index, this);
  }
  void set (SG::ConstAuxElement& b, size_t index)
  {
    b.setIndex (index, this);
  }
  void clear (SG::AuxElement& b)
  {
    b.setIndex (0, 0);
  }

  static
  void clearAux (SG::AuxElement& b)
  {
    b.clearAux();
  }

  static
  void copyAux (SG::AuxElement& a, const SG::AuxElement& b)
  {
    a.copyAux (b);
  }

  static
  void testAuxElementCtor (SG::AuxVectorData* container,
                           size_t index)
  {
    SG::AuxElement bx (container, index);
    assert (bx.index() == index);
    assert (bx.container() == container);
  }
};



} // namespace SG


void test1()
{
  std::cout << "test1\n";

  SG::AuxElement b;
  SG::ConstAccessor<int> ityp1 ("anInt");
  SG::ConstAccessor<float> ftyp1 ("aFloat");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp1_id = r.getAuxID<int> ("anInt");
  SG::auxid_t ftyp1_id = r.getAuxID<float> ("aFloat");

  assert (ityp1.auxid() == ityp1_id);
  assert (ftyp1.auxid() == ftyp1_id);

  {
    SG::ConstAccessor<int> i2 (ityp1_id);
    assert (i2.auxid() == ityp1_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::ConstAccessor<float> (ityp1_id)));
  }

  assert (!ityp1.isAvailable(b));
  assert (!ftyp1.isAvailable(b));
  assert (ityp1.withDefault(b, -1) == -1);
  assert (ftyp1.withDefault(b, -1) == -1);

  SG::AuxVectorBase v;
  v.set (b, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  assert (ityp1.withDefault(v, 5, -1) == -1);
  assert (ftyp1.withDefault(v, 5, -1) == -1);
  assert (!ityp1.isAvailable(b));
  assert (!ftyp1.isAvailable(b));
  assert (!ityp1.isAvailable(v));
  assert (!ftyp1.isAvailable(v));

  int* anInt = reinterpret_cast<int*> (store.getData(ityp1_id, 10, 10));
  anInt[5] = 3;
  float* aFloat = reinterpret_cast<float*> (store.getData(ftyp1_id, 10, 10));
  aFloat[5] = 1.5;

  assert (ityp1.isAvailable(b));
  assert (ftyp1.isAvailable(b));
  assert (ityp1.isAvailable(v));
  assert (ftyp1.isAvailable(v));

  assert (ityp1 (b) == 3);
  assert (ftyp1 (b) == 1.5);
  assert (ityp1.getDataArray (v) == anInt);
  assert (ftyp1.getDataArray (v) == aFloat);
  assert (ityp1.withDefault(b, -1) == 3);
  assert (ftyp1.withDefault(b, -1) == 1.5);
  assert (ityp1.withDefault(v, 5, -1) == 3);
  assert (ftyp1.withDefault(v, 5, -1) == 1.5);

  auto ispan = ityp1.getDataSpan (v);
  auto fspan = ftyp1.getDataSpan (v);
  assert (ispan.size() == 10);
  assert (fspan.size() == 10);
  assert (ispan[5] == 3);
  assert (fspan[5] == 1.5);
}


void test2()
{
  std::cout << "test2\n";

  SG::ConstAuxElement b;
  SG::ConstAccessor<int> ityp1 ("anInt");
  SG::ConstAccessor<float> ftyp1 ("aFloat");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp1_id = r.getAuxID<int> ("anInt");
  SG::auxid_t ftyp1_id = r.getAuxID<float> ("aFloat");

  assert (ityp1.auxid() == ityp1_id);
  assert (ftyp1.auxid() == ftyp1_id);

  assert (!ityp1.isAvailable(b));
  assert (!ftyp1.isAvailable(b));

  SG::AuxVectorBase v;
  v.set (b, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);

  assert (!ityp1.isAvailable(v));
  assert (!ftyp1.isAvailable(v));

  int* anInt = reinterpret_cast<int*> (store.getData(ityp1_id, 10, 10));
  anInt[5] = 3;
  float* aFloat = reinterpret_cast<float*> (store.getData(ftyp1_id, 10, 10));
  aFloat[5] = 1.5;

  assert (ityp1.isAvailable(b));
  assert (ftyp1.isAvailable(b));
  assert (ityp1.isAvailable(v));
  assert (ftyp1.isAvailable(v));

  assert (ityp1 (b) == 3);
  assert (ftyp1 (b) == 1.5);
  assert (ityp1.getDataArray (v) == anInt);
  assert (ftyp1.getDataArray (v) == aFloat);

  auto ispan = ityp1.getDataSpan (v);
  auto fspan = ftyp1.getDataSpan (v);
  assert (ispan.size() == 10);
  assert (fspan.size() == 10);
  assert (ispan[5] == 3);
  assert (fspan[5] == 1.5);
}


int main()
{
  std::cout << "AthContainers/ConstAccessor_test\n";
  test1();
  test2();
  return 0;
}
