/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/Decorator_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2023
 * @brief Regression tests for Decorator.
 */

#undef NDEBUG
#include "AthContainers/Decorator.h"
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
  SG::ConstAuxElement bc;

  SG::Decorator<int> ityp1 ("anInt");
  SG::Decorator<float> ftyp1 ("aFloat");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp1_id = r.getAuxID<int> ("anInt");
  SG::auxid_t ftyp1_id = r.getAuxID<float> ("aFloat");

  assert (ityp1.auxid() == ityp1_id);
  assert (ftyp1.auxid() == ftyp1_id);

  {
    SG::Decorator<int> i2 (ityp1_id);
    assert (i2.auxid() == ityp1_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::Decorator<float> (ityp1_id)));
  }

  assert (!ityp1.isAvailable(b));
  assert (!ftyp1.isAvailable(b));
  assert (!ityp1.isAvailable(bc));
  assert (!ftyp1.isAvailable(bc));
  assert (!ityp1.isAvailableWritable(b));
  assert (!ftyp1.isAvailableWritable(b));
  assert (!ityp1.isAvailableWritable(bc));
  assert (!ftyp1.isAvailableWritable(bc));

  SG::AuxVectorBase v;
  v.set (b, 5);
  v.set (bc, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  const SG::AuxVectorBase& vc = v;

  assert (!ityp1.isAvailable(v));
  assert (!ftyp1.isAvailable(v));
  assert (!ityp1.isAvailable(vc));
  assert (!ftyp1.isAvailable(vc));
  assert (!ityp1.isAvailableWritable(v));
  assert (!ftyp1.isAvailableWritable(v));
  assert (!ityp1.isAvailableWritable(vc));
  assert (!ftyp1.isAvailableWritable(vc));

  int* anInt = reinterpret_cast<int*> (store.getData(ityp1_id, 10, 10));
  anInt[5] = 3;

  assert (ityp1.isAvailable(b));
  assert (ityp1.isAvailable(bc));
  assert (ityp1.isAvailableWritable(b));
  assert (ityp1.isAvailableWritable(bc));
  assert (ityp1.isAvailable(v));
  assert (ityp1.isAvailable(vc));
  assert (ityp1.isAvailableWritable(v));
  assert (ityp1.isAvailableWritable(vc));

  assert (ityp1(b) == 3);
  assert (ityp1(bc) == 3);

  ityp1(b) = 4;

  assert (ityp1(b) == 4);
  assert (ityp1(bc) == 4);

  ityp1(bc) = 5;

  assert (ityp1(b) == 5);
  assert (ityp1(bc) == 5);

  ityp1.set(b, 6);

  assert (ityp1(b) == 6);
  assert (ityp1(bc) == 6);

  ityp1.set(b, 7);

  assert (ityp1(b) == 7);
  assert (ityp1(bc) == 7);

  assert (ityp1.getDataArray(v) == anInt);
  assert (ityp1.getDecorationArray(v) == anInt);

  {
    auto ispan = ityp1.getDecorationSpan (v);
    assert (ispan.size() == 10);
    assert (ispan[5] == 7);
    ispan[5] = 8;
  }

  {
    auto ispan = ityp1.getDataSpan (v);
    assert (ispan.size() == 10);
    assert (ispan[5] == 8);
  }

  v.lock();
  assert (ityp1.isAvailable(b));
  assert (ityp1.isAvailable(bc));
  assert (!ityp1.isAvailableWritable(b));
  assert (!ityp1.isAvailableWritable(bc));
  assert (ityp1.isAvailable(v));
  assert (ityp1.isAvailable(vc));
  assert (!ityp1.isAvailableWritable(v));
  assert (!ityp1.isAvailableWritable(vc));

  EXPECT_EXCEPTION (SG::ExcStoreLocked, ityp1(b));
  EXPECT_EXCEPTION (SG::ExcStoreLocked, ityp1(bc));

  assert (!ftyp1.isAvailable(b));
  assert (!ftyp1.isAvailable(bc));
  assert (!ftyp1.isAvailableWritable(b));
  assert (!ftyp1.isAvailableWritable(bc));
  assert (!ftyp1.isAvailable(v));
  assert (!ftyp1.isAvailable(vc));
  assert (!ftyp1.isAvailableWritable(v));
  assert (!ftyp1.isAvailableWritable(vc));

  ftyp1(bc) = 1.5;

  assert (ftyp1.isAvailable(b));
  assert (ftyp1.isAvailable(bc));
  assert (ftyp1.isAvailableWritable(b));
  assert (ftyp1.isAvailableWritable(bc));
  assert (ftyp1.isAvailable(v));
  assert (ftyp1.isAvailable(vc));
  assert (ftyp1.isAvailableWritable(v));
  assert (ftyp1.isAvailableWritable(vc));

  assert (ftyp1(b) == 1.5);
  assert (ftyp1(bc) == 1.5);

  ftyp1.set (bc, 2.5);
  assert (ftyp1(bc) == 2.5);

  assert (ityp1.getDataArray(v) == anInt);
  assert (ftyp1.getDataArray(v)[5] == 2.5);

  EXPECT_EXCEPTION (SG::ExcStoreLocked, ityp1.getDecorationArray(v));
  assert (ftyp1.getDecorationArray(v)[5] == 2.5);

  EXPECT_EXCEPTION (SG::ExcStoreLocked, ityp1.getDecorationSpan(v));
  {
    auto fspan = ftyp1.getDecorationSpan (v);
    assert (fspan.size() == 10);
    assert (fspan[5] == 2.5);
    fspan[5] = 3.5;
  }

  {
    auto ispan = ityp1.getDataSpan (v);
    auto fspan = ftyp1.getDataSpan (v);
    assert (ispan.size() == 10);
    assert (fspan.size() == 10);
    assert (ispan[5] == 8);
    assert (fspan[5] == 3.5);
  }
}


int main()
{
  std::cout << "AthContainers/Decorator_test\n";
  test1();
  return 0;
}
