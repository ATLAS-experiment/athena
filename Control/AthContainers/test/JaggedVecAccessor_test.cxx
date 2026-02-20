/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/JaggedVecAccessor_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Regression tests for JaggedVecAccessor
 */

#undef NDEBUG
#include "AthContainers/JaggedVecAccessor.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/exceptions.h"
#include "TestTools/FLOATassert.h"
#include "TestTools/expect_exception.h"
#include <ranges>
#include <iostream>
#include <cassert>


using Athena_test::floatEQ;


namespace SG {
template <class T>
std::ostream& operator<< (std::ostream& s, const JaggedVecElt<T>& e)
{
  s << e.end();
  return s;
}
} // namespace SG


namespace {


template <class T>
void compareElts (const SG::JaggedVecElt<T>* ptr, size_t n,
                  const std::vector<SG::JaggedVecElt<T> >& v)
{
  assert (n >= v.size());
  for (size_t i = 0; i  < v.size(); ++i) {
    if (ptr[i] != v[i]) {
      std::cerr << "Elt comparison failure: [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << ptr[ii] << " ";
      std::cerr << "] [";
      for (size_t ii = 0; ii  < v.size(); ++ii)
        std::cerr << v[ii] << " ";
      std::cerr << "]\n";
      std::abort();
    }
  }
  SG::JaggedVecElt<T> tail (v.back().end());
  for (size_t i = v.size(); i  < n; ++i) {
    assert (ptr[i] == tail);
  }
}


template <class T>
void comparePayload (const T* ptr, const std::vector<T>& v)
{
  if (!std::equal (v.begin(), v.end(), ptr, floatEQ)) {
    std::cerr << "Payload comparison failure: [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << ptr[ii] << " ";
    std::cerr << "] [";
    for (size_t ii = 0; ii  < v.size(); ++ii)
      std::cerr << v[ii] << " ";
    std::cerr << "]\n";
    std::abort();
  }
}


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

  SG::Accessor<Elt> jtyp1 ("jvec");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t jvec_id = r.findAuxID ("jvec");
  SG::auxid_t payload_id = r.findAuxID ("jvec_linked");

  assert (jtyp1.auxid() == jvec_id);
  assert (jtyp1.linkedAuxid() == payload_id);

  {
    SG::Accessor<Elt> j2 (jvec_id);
    assert (j2.auxid() == jvec_id);
    EXPECT_EXCEPTION (SG::ExcAuxTypeMismatch, (SG::Accessor<SG::JaggedVecElt<double> > (jvec_id)));

    SG::auxid_t jveci_id = r.getAuxID<SG::JaggedVecElt<int> > ("jveci");
    EXPECT_EXCEPTION (SG::ExcNoLinkedVar, (SG::Accessor<SG::JaggedVecElt<int> > (jveci_id)));
  }

  SG::AuxElement b5;

  assert (!jtyp1.isAvailable(b5));
  assert (!jtyp1.isAvailableWritable(b5));

  SG::AuxVectorBase v;
  v.set (b5, 5);
  SG::AuxStoreInternal store;
  v.setStore (&store);

  assert (!jtyp1.isAvailable(v));
  assert (!jtyp1.isAvailableWritable(v));

  Elt* elt = reinterpret_cast<Elt*> (store.getData(jvec_id, 10, 10));
  Payload* payload = reinterpret_cast<Payload*> (store.getData(payload_id, 5, 5));
  std::ranges::copy (std::vector<Elt>{{2}, {2}, {2}, {2}, {2}, {5}}, elt);
  std::fill_n (elt+6, 4, Elt{5});
  std::ranges::copy (std::vector<float> {1.5, 2.5, 3.5, 4.5, 5.5}, payload);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (jvec_id);
  // [1.5 2.5] [] [] [] [] [3.5, 4.5, 5.5]

  assert (jtyp1.isAvailable(b5));
  assert (jtyp1.isAvailableWritable(b5));
  assert (jtyp1.isAvailable(v));
  assert (jtyp1.isAvailableWritable(v));

  SG::AuxElement b4;
  v.set (b4, 4);
  jtyp1.set (b4, std::vector<float> {10.1, 10.2});
  assert (linkedVec->size() == 7);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{2}, {2}, {2}, {2}, {4}, {7}});
  comparePayload (payload, {1.5, 2.5, 10.1, 10.2, 3.5, 4.5, 5.5});

  assert (jtyp1 (b5).size() == 3);
  assert (jtyp1 (b5)[1] == 4.5);
  assert (jtyp1 (b5).at(1) == 4.5);
  EXPECT_EXCEPTION (std::out_of_range, jtyp1 (b5).at(10));

  assert (jtyp1(v, 0).front() == 1.5);
  assert (jtyp1(v, 0).back() == 2.5);

  {
    const SG::AuxElement& cb5 = b5;
    assert (jtyp1(cb5).size() == 3);
    assert (jtyp1(cb5).size() == 3);
    assert (jtyp1(b5).size() == 3);
    assert (jtyp1(b5).size() == 3);

    jtyp1(b5) = std::vector<float> {20.5, 21.5};
    assert (jtyp1(cb5).size() == 2);
    assert (jtyp1(cb5)[0] == 20.5);
  }

  {
    std::vector<float> vf4 = jtyp1 (b4);
    assert (vfloatEQ (vf4, std::vector<float> {10.1, 10.2}));
    std::vector<float> vf4b = jtyp1 (b4).asVector();
    assert (vfloatEQ (vf4b, std::vector<float> {10.1, 10.2}));
  }
  jtyp1 (b4) = std::vector<float> {20.3, 20.4, 20.5};
  assert (linkedVec->size() == 7);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{2}, {2}, {2}, {2}, {5}, {7}});
  comparePayload (payload, {1.5, 2.5, 20.3, 20.4, 20.5, 20.5, 21.5});
  // [1.5 2.5] [] [] [] [20.3 20.4 20.5] [20.5 21.5]

  assert (jtyp1.getEltArray (v) == elt);
  assert (jtyp1.getPayloadArray (v) == payload);

  jtyp1(v, 0).push_back (3.5);
  assert (linkedVec->size() == 8);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{3}, {3}, {3}, {3}, {6}, {8}});
  comparePayload (payload, {1.5, 2.5, 3.5, 20.3, 20.4, 20.5, 20.5, 21.5});
  // [1.5 2.5 3.5] [] [] [] [20.3 20.4 20.5] [20.5 21.5]

  jtyp1(v, 0).clear();
  assert (linkedVec->size() == 5);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {0}, {0}, {0}, {3}, {5}});
  comparePayload (payload, {20.3, 20.4, 20.5, 20.5, 21.5});
  // [] [] [] [] [20.3 20.4 20.5] [20.5 21.5]

  jtyp1(v, 1).resize(3, 4.4);
  assert (linkedVec->size() == 8);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {6}, {8}});
  comparePayload (payload, {4.4, 4.4, 4.4, 20.3, 20.4, 20.5, 20.5, 21.5});
  // [] [4.4 4.4 4.4] [] [] [20.3 20.4 20.5] [20.5 21.5]

  jtyp1(v, 1).resize(1);
  assert (linkedVec->size() == 6);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {1}, {1}, {1}, {4}, {6}});
  comparePayload (payload, {4.4, 20.3, 20.4, 20.5, 20.5, 21.5});
  // [] [4.4] [] [] [20.3 20.4 20.5] [20.5 21.5]

  {
    auto e4 = jtyp1(v, 4);
    e4.erase (e4.begin() + 1);
  }
  assert (linkedVec->size() == 5);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {1}, {1}, {1}, {3}, {5}});
  comparePayload (payload, {4.4, 20.3, 20.5, 20.5, 21.5});
  // [] [4.4] [] [] [20.3 20.5] [20.5 21.5]

  {
    auto e5 = jtyp1(v, 5);
    e5.insert (e5.begin() + 1, 30.2);
  }
  assert (linkedVec->size() == 6);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {1}, {1}, {1}, {3}, {6}});
  comparePayload (payload, {4.4, 20.3, 20.5, 20.5, 30.2, 21.5});
  // [] [4.4] [] [] [20.3 20.5] [20.5 30.2 21.5]

  {
    auto e5 = jtyp1(v, 5);
    e5.erase (e5.begin(), e5.begin()+2);
  }
  assert (linkedVec->size() == 4);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {1}, {1}, {1}, {3}, {4}});
  comparePayload (payload, {4.4, 20.3, 20.5, 21.5});
  // [] [4.4] [] [] [20.3 20.5] [21.5]

  {
    auto e4 = jtyp1(v, 4);
    e4.insert (e4.begin()+1, 3, 30.5);
  }
  assert (linkedVec->size() == 7);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {1}, {1}, {1}, {6}, {7}});
  comparePayload (payload, {4.4, 20.3, 30.5, 30.5, 30.5, 20.5, 21.5});
  // [] [4.4] [] [] [20.3 30.5 30.5 30.5 20.5] [21.5]

  {
    auto e1 = jtyp1(v, 1);
    std::vector<float> vf {40.1, 40.2};
    e1.insert (e1.begin(), vf.begin(), vf.end());
  }
  assert (linkedVec->size() == 9);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {8}, {9}});
  comparePayload (payload, {40.1, 40.2, 4.4, 20.3, 30.5, 30.5, 30.5, 20.5, 21.5});
  // [] [40.1 40.2 4.4] [] [] [20.3 30.5 30.5 30.5 20.5] [21.5]

  jtyp1(v, 4).pop_back();
  assert (linkedVec->size() == 8);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {7}, {8}});
  comparePayload (payload, {40.1, 40.2, 4.4, 20.3, 30.5, 30.5, 30.5, 21.5});
  // [] [40.1 40.2 4.4] [] [] [20.3 30.5 30.5 30.5] [21.5]

  {
    auto e1 = jtyp1(v, 1);
    e1.insert_range (e1.begin()+2, std::vector<float> {50.1, 50.2});
  }
  assert (linkedVec->size() == 10);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {5}, {5}, {5}, {9}, {10}});
  comparePayload (payload, {40.1, 40.2, 50.1, 50.2, 4.4, 20.3, 30.5, 30.5, 30.5, 21.5});
  // [] [40.1 40.2 50.1 50.2 4.4] [] [] [20.3 30.5 30.5 30.5] [21.5]

  jtyp1(v, 5).append_range (std::vector<float> {60.4, 60.5});
  assert (linkedVec->size() == 12);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {5}, {5}, {5}, {9}, {12}});
  comparePayload (payload, {40.1, 40.2, 50.1, 50.2, 4.4, 20.3, 30.5, 30.5, 30.5, 21.5, 60.4, 60.5});
  // [] [40.1 40.2 50.1 50.2 4.4] [] [] [20.3 30.5 30.5 30.5] [21.5 60.4 60.5]

  jtyp1(v, 1).assign (3, 60.3);
  assert (linkedVec->size() == 10);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {7}, {10}});
  comparePayload (payload, {60.3, 60.3, 60.3, 20.3, 30.5, 30.5, 30.5, 21.5, 60.4, 60.5});
  // [] [60.3 60.3 60.3] [] [] [20.3 30.5 30.5 30.5] [21.5 60.4 60.5]

  {
    std::vector<float> vf {70.4, 71.5};
    jtyp1(v, 4).assign (vf.begin(), vf.end());
  }
  assert (linkedVec->size() == 8);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {5}, {8}});
  comparePayload (payload, {60.3, 60.3, 60.3, 70.4, 71.5, 21.5, 60.4, 60.5});
  // [] [60.3 60.3 60.3] [] [] [70.4 71.5] [21.5 60.4 60.5]

  jtyp1(v, 5).assign_range (std::vector<float> {72.6, 73.7});
  assert (linkedVec->size() == 7);
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{0}, {3}, {3}, {3}, {5}, {7}});
  comparePayload (payload, {60.3, 60.3, 60.3, 70.4, 71.5, 72.6, 73.7});
  // [] [60.3 60.3 60.3] [] [] [70.4 71.5] [72.6 73.7]


  {
    auto e4 = jtyp1(v, 4);
    std::vector<float> pvec (e4.begin(), e4.end());
    assert (vfloatEQ (pvec, {70.4, 71.5}));
  }

  {
    const auto c4 = jtyp1(v, 4);
    assert (floatEQ (c4[0], 70.4));
    assert (floatEQ (c4.at(1), 71.5));
    assert (floatEQ (c4.front(), 70.4));
    assert (floatEQ (c4.back(), 71.5));
    std::vector<float> pvec (c4.begin(), c4.end());
    assert (vfloatEQ (pvec, {70.4, 71.5}));
  }

  SG::Accessor<Elt> jtyp2 ("jvec2");
  jtyp2(b5) = std::vector<float> {20.7, 20.8};
}


// spans
void test2()
{
  std::cout << "test2\n";

  using Payload = float;
  using Elt = SG::JaggedVecElt<Payload>;

  SG::Accessor<Elt> jtyp1 ("jvec");

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t jvec_id = r.findAuxID ("jvec");
  SG::auxid_t payload_id = r.findAuxID ("jvec_linked");

  SG::AuxVectorBase v (10);
  SG::AuxStoreInternal store;
  v.setStore (&store);
  Elt* elt = reinterpret_cast<Elt*> (store.getData(jvec_id, 10, 10));
  Payload* payload = reinterpret_cast<Payload*> (store.getData(payload_id, 5, 5));
  std::ranges::copy (std::vector<Elt>{{2}, {2}, {2}, {2}, {2}, {5}}, elt);
  std::fill_n (elt+6, 4, Elt{5});
  std::ranges::copy (std::vector<float> {1.5, 2.5, 3.5, 4.5, 5.5}, payload);
  SG::IAuxTypeVector* linkedVec = store.linkedVector (jvec_id);
  // [1.5 2.5] [] [] [] [] [3.5, 4.5, 5.5]

  auto elt_span = jtyp1.getEltSpan (v);
  auto payload_span = jtyp1.getPayloadSpan (v);
  assert (elt_span.size() == 10);
  assert (elt_span[1] == Elt(2));
  assert (elt_span[5] == Elt(5));
  assert (payload_span.size() == 5);
  assert (floatEQ (payload_span[1], 2.5));
  assert (floatEQ (payload_span[3], 4.5));

  auto span = jtyp1.getDataSpan (v);
  assert (span.size() == 10);
  assert (!span.empty());
  assert (span[0].size() == 2);
  assert (span[1].size() == 0);
  assert (span[5].size() == 3);
  assert (span[0][1] == 2.5);
  assert (span[5][2] == 5.5);
  assert (span.front()[0] == 1.5);
  assert (span.back().empty());
  assert (vfloatEQ (span[0], std::vector<float>{1.5, 2.5}));

  span[0][1] = 9.5;
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{2}, {2}, {2}, {2}, {2}, {5}});
  comparePayload (payload, {1.5, 9.5, 3.5, 4.5, 5.5});

  span[1] = std::vector<float> {20.5, 21.5, 22.5};
  payload = reinterpret_cast<Payload*> (linkedVec->toPtr());
  compareElts (elt, 10, {{2}, {5}, {5}, {5}, {5}, {8}});
  comparePayload (payload, {1.5, 9.5, 20.5, 21.5, 22.5, 3.5, 4.5, 5.5});

  std::vector<float> pvec;
  for (const auto v : span) {
    pvec.insert (pvec.end(), v.begin(), v.end());
  }
  assert (vfloatEQ (pvec, {1.5, 9.5, 20.5, 21.5, 22.5, 3.5, 4.5, 5.5}));
}

//coverity[root_function]
int main()
{
  std::cout << "AthContainers/JaggedVecAccessor_test\n";
  test1();
  test2();
  return 0;
}
