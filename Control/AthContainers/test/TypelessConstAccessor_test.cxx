/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/TypelessConstAccessor_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2023
 * @brief Regression tests for TypelessConstAccessor.
 */

#undef NDEBUG
#include "AthContainers/TypelessConstAccessor.h"
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
  virtual size_t size_v() const { return 10; }
  virtual size_t capacity_v() const { return 10; }

  using SG::AuxVectorData::setStore;
  void set (SG::AuxElement& b, size_t index)
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

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ftyp1_id = r.getAuxID<float> ("aFloat");

  SG::AuxElement b3;
  SG::AuxVectorBase v3;
  v3.set (b3, 6);
  SG::AuxStoreInternal store3;
  v3.setStore (&store3);

  float* aFloat = reinterpret_cast<float*> (store3.getData(ftyp1_id, 10, 10));
  aFloat[6] = 1.5;

  SG::TypelessConstAccessor ftyp1a ("aFloat");
  assert (ftyp1a.isAvailable (b3));
  assert (*reinterpret_cast<const float*>(ftyp1a (b3)) == 1.5);
  assert (*reinterpret_cast<const float*>(ftyp1a (v3, 6)) == 1.5);
  assert ((reinterpret_cast<const float*>(ftyp1a.getDataArray (v3)))[6] == 1.5);

  assert (ftyp1a.auxid() == ftyp1_id);

  EXPECT_EXCEPTION (SG::ExcUnknownAuxItem,
                    SG::AuxElement::TypelessConstAccessor ("adsasd"));
  SG::AuxElement::TypelessConstAccessor x1 (typeid(int), "adsasd");
  EXPECT_EXCEPTION (SG::ExcUnknownAuxItem,
                    SG::AuxElement::TypelessConstAccessor (typeid(SG::AuxVectorBase),
                                                           "x2"));
}


void test2()
{
  std::cout << "test2\n";

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp1_id = r.getAuxID<float> ("anInt");

  SG::AuxElement b2;
  SG::AuxVectorBase v2;
  v2.set (b2, 6);
  SG::AuxStoreInternal store2;
  v2.setStore (&store2);

  int* anInt = reinterpret_cast<int*> (store2.getData(ityp1_id, 10, 10));
  anInt[6] = 123;

  SG::ConstAuxElement::TypelessConstAccessor ityp1a ("anInt");
  assert (ityp1a.isAvailable (b2));
  assert (*reinterpret_cast<const int*>(ityp1a (b2)) == 123);
  assert (ityp1a.auxid() == ityp1_id);

  EXPECT_EXCEPTION (SG::ExcUnknownAuxItem,
                    SG::ConstAuxElement::TypelessConstAccessor ("adsasdxyz"));
  SG::ConstAuxElement::TypelessConstAccessor x1 (typeid(int), "adsasd");
  EXPECT_EXCEPTION (SG::ExcUnknownAuxItem,
                    SG::ConstAuxElement::TypelessConstAccessor (typeid(SG::AuxVectorBase),
                                                                "x2"));
}


int main()
{
  std::cout << "AthContainers/TypelessConstAccessor_test\n";
  test1();
  test2();
  return 0;
}
