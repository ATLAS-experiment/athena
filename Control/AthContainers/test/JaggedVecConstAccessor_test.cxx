/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecConstAccessor_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Regression tests for PackedLinkConstAccessor.
 */

#undef NDEBUG
#include "AthContainers/JaggedVecConstAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/exceptions.h"
#include "TestTools/expect_exception.h"
#include "TestTools/FLOATassert.h"
#include <ranges>
#include <iostream>
#include <cassert>


using Athena_test::floatEQ;


namespace {


bool vfloatEQ (const std::vector<float>& a, const std::vector<float>& b)
{
  return std::ranges::equal (a, b, floatEQ);
}


} // anonymous namespace


namespace SG {


class AuxVectorBase
  : public SG::AuxVectorData
{
public:
  AuxVectorBase (size_t sz = 10) : m_sz (sz) {}
  virtual size_t size_v() const override { return m_sz; }
  virtual size_t capacity_v() const override { return m_sz; }

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

private:
  size_t m_sz;
};



} // namespace SG


void test1()
{
  std::cout << "test1\n";

  using Payload = float;
  using Elt = SG::JaggedVecElt<Payload>;

  SG::ConstAccessor<Elt> jtyp1 ("jvec");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t jvec_id = r.findAuxID ("jvec");
  SG::auxid_t payload_id = r.findAuxID ("jvec_linked");

  assert (jtyp1.auxid() == jvec_id);
  assert (jtyp1.linkedAuxid() == payload_id);

  {
    SG::ConstAccessor<Elt> j2 (jvec_id);
    assert (j2.auxid() == jvec_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::ConstAccessor<SG::JaggedVecElt<double> > (jvec_id)));

    SG::auxid_t jveci_id = r.getAuxID<SG::JaggedVecElt<int> > ("jveci");
    EXPECT_EXCEPTION (SG::ExcNoLinkedVar, (SG::ConstAccessor<SG::JaggedVecElt<int> > (jveci_id)));
  }

  SG::AuxElement b;
  SG::ConstAuxElement cb;
  assert (!jtyp1.isAvailable(b));
  assert (!jtyp1.isAvailable(cb));

  SG::AuxVectorBase v;
  v.set (b, 5);
  v.set (cb, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  const SG::AuxVectorBase& cv = v;

  assert (!jtyp1.isAvailable(v));
  assert (!jtyp1.isAvailable(cv));

  Elt* elt = reinterpret_cast<Elt*> (store.getData(jvec_id, 10, 10));
  Payload* payload = reinterpret_cast<Payload*> (store.getData(payload_id, 5, 5));
  elt[0] = Elt (2);
  elt[1] = Elt (2);
  elt[2] = Elt (2);
  elt[3] = Elt (2);
  elt[4] = Elt (2);
  elt[5] = Elt (5);
  std::ranges::copy (std::vector<float> {1.5, 2.5, 3.5, 4.5, 5.5}, payload);

  assert (jtyp1.isAvailable(b));
  assert (jtyp1.isAvailable(cb));
  assert (jtyp1.isAvailable(b));
  assert (jtyp1.isAvailable(cb));

  assert (jtyp1 (b).size() == 3);
  assert (jtyp1 (b)[1] == 4.5);
  assert (jtyp1 (b).at(1) == 4.5);
  EXPECT_EXCEPTION (std::out_of_range, jtyp1 (b).at(10));
  {
    std::vector<float> vv = jtyp1 (b);
    //assert (vfloatEQ (vv, std::vector<float> {3.5, 4.5, 5.5}));
    assert (vfloatEQ (vv, std::vector<float> {3.5, 4.5, 5.5}));
    std::vector<float> vv2 = jtyp1 (b).asVector();
    assert (vfloatEQ (vv2, std::vector<float> {3.5, 4.5, 5.5}));
  }
  assert (jtyp1 (cb).size() == 3);
  assert (jtyp1 (cb)[0] == 3.5);
  assert (jtyp1 (cb)[2] == 5.5);
  assert (jtyp1 (cb).at(0) == 3.5);
  EXPECT_EXCEPTION (std::out_of_range, jtyp1 (cb).at(10));

  assert (jtyp1 (v, 5).size() == 3);
  assert (jtyp1 (v, 5)[1] == 4.5);

  assert (jtyp1.getEltArray (v) == elt);
  assert (jtyp1.getPayloadArray (v) == payload);
}


// spans
void test2()
{
  std::cout << "test2\n";

  using Payload = float;
  using Elt = SG::JaggedVecElt<Payload>;

  SG::ConstAccessor<Elt> jtyp1 ("jvec");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t jvec_id = r.findAuxID ("jvec");
  SG::auxid_t payload_id = r.findAuxID ("jvec_linked");

  SG::AuxVectorBase v (5);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  Elt* elt = reinterpret_cast<Elt*> (store.getData(jvec_id, 5, 5));
  Payload* payload = reinterpret_cast<Payload*> (store.getData(payload_id, 6, 6));

  // [1.1 1.2] [1.3] [1.4 1.5] [] [1.6]
  std::vector<float> vpayload  {1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
  std::ranges::copy (std::vector<Elt> {{2}, {3}, {5}, {5}, {6}}, elt);
  std::ranges::copy (vpayload, payload);

  v.setStore (static_cast<SG::IAuxStore*>(nullptr));
  v.setStore (static_cast<SG::IConstAuxStore*>(&store));
  auto elt_span = jtyp1.getEltSpan (v);
  auto payload_span = jtyp1.getPayloadSpan (v);
  assert (elt_span.size() == 5);
  assert (elt_span[1] == Elt(3));
  assert (elt_span[2] == Elt(5));
  assert (payload_span.size() == 6);
  assert (floatEQ (payload_span[1], 1.2));
  assert (floatEQ (payload_span[3], 1.4));

  auto span = jtyp1.getDataSpan (v);
  assert (span.size() == 5);
  assert (!span.empty());
  assert (span[1].size() == 1);
  assert (floatEQ (span[1][0], 1.3));
  assert (span[2].size() == 2);
  assert (floatEQ (span[2][1], 1.5));
  assert (floatEQ (span.front().back(), 1.2));
  assert (floatEQ (span.back().front(), 1.6));

  std::vector<float> pvec;
  for (const auto v : span) {
    pvec.insert (pvec.end(), v.begin(), v.end());
  }
  assert (vfloatEQ (pvec, vpayload));
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainers/JaggedVecConstAccessor_test\n";
  test1();
  test2();
  return 0;
}
