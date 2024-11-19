/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/debug_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2015
 * @brief Regression tests for debug
 */


#undef NDEBUG
#include "AthContainers/debug.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/JaggedVec.h"
#include "AthContainers/PackedLink.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>


#ifndef XAOD_STANDALONE
#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( std::vector<int>, 12345, 0 )
#endif


struct A
  : public SG::AuxElement
{
  int& xint()
  { static const Accessor<int> acc ("xint");  return acc(*this); }
  int xint() const
  { static const Accessor<int> acc ("xint");  return acc(*this); }

  float& xfloat()
  { static const Accessor<float> acc ("xfloat");  return acc(*this); }
  float xfloat() const
  { static const Accessor<float> acc ("xfloat");  return acc(*this); }

  std::vector<int>& xvint()
  { static const Accessor<std::vector<int> > acc ("xvint");  return acc(*this); }
  const std::vector<int>& xvint() const
  { static const Accessor<std::vector<int> > acc ("xvint");  return acc(*this); }

  auto jvec()
  {
    static const Accessor<SG::JaggedVecElt<int> > acc("jvec");
    return acc(*this);
  }

#ifndef XAOD_STANDALONE
  using Cont_t = std::vector<int>;
  using Link_t = ElementLink<Cont_t>;
  using PLink_t = SG::PackedLink<Cont_t>;

  auto plink()
  {
    static const Accessor<PLink_t> acc("plink");
    return acc(*this);
  }

  auto pvlink()
  {
    static const Accessor<std::vector<PLink_t> > acc("pvlink");
    return acc(*this);
  }
#endif
};


void test1()
{
  std::cout << "test1\n";
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t id1 = r.getAuxID<int> ("foo");
  SG::auxid_t id2 = r.getAuxID<int> ("foo", "bar");
  assert (SGdebug::aux_var_name(id1) == "::foo");
  SGdebug::print_aux_var_name(id2);

  SG::auxid_t ltest1_id = r.getAuxID<int> ("ltest1", "",
                                           SG::AuxVarFlags::Linked);
  SG::auxid_t ltest2_id = r.getAuxID<float> ("ltest2", "",
                                             SG::AuxVarFlags::None,
                                             ltest1_id);
  SGdebug::print_aux_var_name(ltest1_id);
  SGdebug::print_aux_var_name(ltest2_id);
}


void test2()
{
  std::cout << "test2\n";
  DataVector<A> dv;
  SG::AuxStoreInternal store;
  dv.setStore (&store);

  dv.push_back (new A);
  dv.push_back (new A);
  dv.push_back (new A);

  dv[0]->xint() = 1;
  dv[1]->xint() = 2;
  dv[2]->xint() = 3;

  dv[0]->xfloat() = 1.5;
  dv[1]->xfloat() = 2.5;
  dv[2]->xfloat() = 3.5;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ltest1_id = r.getAuxID<int> ("ltest1", "",
                                           SG::AuxVarFlags::Linked);
  SG::auxid_t ltest2_id = r.getAuxID<float> ("ltest2", "",
                                             SG::AuxVarFlags::None,
                                             ltest1_id);
  int* vp1 = reinterpret_cast<int*> (store.getData (ltest1_id, 1, 1));
  vp1[0] = 1;
  float* vp2 = reinterpret_cast<float*> (store.getData (ltest2_id, 3, 3));
  vp2[0] = 1.5;
  vp2[1] = 2.5;
  vp2[2] = 3.5;

  dv.lock();
  SG::auxid_t decor_id = r.getAuxID<int> ("decor");
  store.getDecoration (decor_id, 3, 3);

  std::cout << "... set ...\n";
  SGdebug::print_aux_vars (store.getAuxIDs());
  std::cout << "... store ...\n";
  SGdebug::print_aux_vars (store);
  SGdebug::print_aux_vars (&store);
  std::cout << "... vec ...\n";
  SGdebug::print_aux_vars (dv);
  SGdebug::print_aux_vars (&dv);
  std::cout << "... elt ...\n";
  SGdebug::print_aux_vars (dv[1]);
  SGdebug::print_aux_vars (*dv[1]);
}


template <class T>
void test3_one (const char* name, const T& val)
{
  SG::auxid_t id = SG::AuxTypeRegistry::instance().getAuxID<T> (name);
  std::cout << name << " " << SGdebug::aux_var_as_string (id, &val, 0) << "\n";
}


void test3()
{
  std::cout << "test3\n";
  test3_one<int> ("a_int", -2);
  test3_one<unsigned int> ("a_uint", 10);
  test3_one<short> ("a_short", 10);
  test3_one<unsigned short> ("a_ushort", 20);
  test3_one<char> ("a_char", 'a');
  test3_one<unsigned char> ("a_uchar", 'b');
  test3_one<long> ("a_long", 50);
  test3_one<unsigned long> ("a_ulong", 60);
  test3_one<long long> ("a_llong", 50);
  test3_one<unsigned long long> ("a_ullong", 60);
  test3_one<float> ("a_float", 123.5);
  test3_one<double> ("a_double", 223.5);
  test3_one<bool> ("a_bool", true);

  test3_one<std::vector<int> > ("a_intvec", std::vector<int>{1,-2,3});
  test3_one<std::vector<unsigned int> > ("a_uintvec", std::vector<unsigned int>{1,2,3});
  test3_one<std::vector<short> > ("a_shortvec", std::vector<short>{4,5,6});
  test3_one<std::vector<unsigned short> > ("a_ushortvec", std::vector<unsigned short>{7,8,9});
  test3_one<std::vector<char> > ("a_chartvec", std::vector<char>{'a','b','c'});
  test3_one<std::vector<unsigned char> > ("a_uchartvec", std::vector<unsigned char>{'d','e','f'});
  test3_one<std::vector<long> > ("a_longvec", std::vector<long>{10,11,12});
  test3_one<std::vector<unsigned long> > ("a_ulongvec", std::vector<unsigned long>{13,14,15});
  test3_one<std::vector<long long> > ("a_llongvec", std::vector<long long>{16,17,18});
  test3_one<std::vector<unsigned long long> > ("a_ullongvec", std::vector<unsigned long long>{19,20,21});
  test3_one<std::vector<float> > ("a_floatvec", std::vector<float>{1.1,1.2,1.3});
  test3_one<std::vector<double> > ("a_doublevec", std::vector<double>{2.1,2.2,2.3});
  test3_one<std::vector<bool> > ("a_boolvec", std::vector<bool>{true,false,true});

  test3_one<A> ("a_a", A());
}


void test4()
{
  std::cout << "test4\n";
  DataVector<A> dv;
  SG::AuxStoreInternal store;
  dv.setStore (&store);

  dv.push_back (new A);
  dv.push_back (new A);
  dv.push_back (new A);

  dv[0]->xint() = 1;
  dv[1]->xint() = 2;
  dv[2]->xint() = 3;

  dv[0]->xfloat() = 1.5;
  dv[1]->xfloat() = 2.5;
  dv[2]->xfloat() = 3.5;

  dv[0]->xvint() = std::vector<int> {10, 11, 12};
  dv[1]->xvint() = std::vector<int> {13, 14, 15};
  dv[2]->xvint() = std::vector<int> {16, 17, 18};

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ltest1_id = r.getAuxID<int> ("ltest1", "",
                                           SG::AuxVarFlags::Linked);
  SG::auxid_t ltest2_id = r.getAuxID<float> ("ltest2", "",
                                             SG::AuxVarFlags::None,
                                             ltest1_id);
  int* vp1 = reinterpret_cast<int*> (store.getData (ltest1_id, 1, 1));
  vp1[0] = 1;
  float* vp2 = reinterpret_cast<float*> (store.getData (ltest2_id, 3, 3));
  vp2[0] = 1.5;
  vp2[1] = 2.5;
  vp2[2] = 3.5;

  SGdebug::dump_aux_vars (store, 10);

  std::cout << "... print elt 1 from  store\n";
  SGdebug::dump_aux_vars (store, 1);
  std::cout << "... print elt 0 from  store\n";
  SGdebug::dump_aux_vars (&store, 0);

  std::cout << "\n";
  std::cout << "... print entire store\n";
  SGdebug::dump_aux_vars (store);
  SGdebug::dump_aux_vars (&store);

  std::cout << "\n";
  std::cout << "... print elt 1 from  vec\n";
  SGdebug::dump_aux_vars (dv, 1);
  std::cout << "... print elt 0 from  vec\n";
  SGdebug::dump_aux_vars (&dv, 0);

  std::cout << "\n";
  std::cout << "... print entire vec\n";
  SGdebug::dump_aux_vars (dv);
  SGdebug::dump_aux_vars (&dv);

  std::cout << "\n";
  std::cout << "... print elt 1\n";
  SGdebug::dump_aux_vars (*dv[1]);
  std::cout << "... print elt 0\n";
  SGdebug::dump_aux_vars (dv[0]);
}


// Test dumping JaggedVecElt variables.
void test5()
{
  std::cout << "test5\n";

  DataVector<A> dv;
  SG::AuxStoreInternal store;
  dv.setStore (&store);

  dv.push_back (new A);
  dv.back()->jvec() = std::vector<int>{5, 4, 3, 2};
  dv.push_back (new A);
  dv.back()->jvec() = std::vector<int>{8, 10};

  SGdebug::dump_aux_vars (store, 0);
}


// Test dumping PackedLink/DataLink variables.
void test6()
{
  std::cout << "test6\n";

#ifndef XAOD_STANDALONE
  std::unique_ptr<SGTest::TestStore> testStore = SGTest::getTestStore();
  SG::CurrentEventStore::setStore (testStore.get());
  {
    auto vi = std::make_unique<std::vector<int> >();
    vi->assign ({ 1, 2, 3, 4, 5});
    testStore->record (std::move (vi), "vi");
  }
  DataVector<A> dv;
  SG::AuxStoreInternal store;
  dv.setStore (&store);

  dv.push_back (new A);

  dv.back()->plink() = A::Link_t ("vi", 3);
  dv.back()->pvlink().push_back (A::Link_t ("vi", 2));
  dv.back()->pvlink().push_back (A::Link_t ());
  dv.back()->pvlink().push_back (A::Link_t ("vi", 1));

  std::ostringstream ss;
  SGdebug::dump_aux_vars (ss, store, 0);
  assert (ss.str() == "::plink PackedLink[1/3]\n"
          "  linked: ::plink_linked [DataLink[0/], DataLink[12345/vi]]\n"
          "::pvlink [PackedLink[1/2], PackedLink[0/0], PackedLink[1/1]]\n"
          "  linked: ::pvlink_linked [DataLink[0/], DataLink[12345/vi]]\n");

#endif
}


int main()
{
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  return 0;
}
