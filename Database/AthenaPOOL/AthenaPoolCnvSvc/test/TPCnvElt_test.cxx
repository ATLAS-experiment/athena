/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaPoolCnvSvc/test/TPCnvElt_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Tests for TPCnvElt.
 */


#undef NDEBUG
#include "AthenaPoolCnvSvc/TPCnvElt.h"
#include "AthenaPoolCnvSvcTestDict.h"
#include "GaudiKernel/MsgStream.h"

#include "TSystem.h"
#include "TestConverterBase.h"

#include <iostream>
#include <typeinfo>
#include <cassert>


using namespace AthenaPoolCnvSvcTest;

class Token;

class XCnv_p1
  : public TestConverterBase
{
public:
  typedef X Trans_t;
  typedef X_p1 Pers_t;
  
  X* createTransientWithKey (const X_p1* pers, const std::string&, MsgStream& log)
  {
    return createTransient (pers, log);
  }
  X* createTransient (const X_p1* pers, MsgStream&)
  { return new X(pers->m_a*2); }

  void persToTrans (const X_p1* pers, X* trans, MsgStream&)
  {
    trans->m_a = pers->m_a*2;
  }
};

class TestConverter
{
public:
  TestConverter () {}
  
  bool compareClassGuid(const Token* token, const Guid& guid)
  { return guid == token->classID(); }

  template <class T>
  T* poolReadObject(const Token*) { return new T (10); }
};


void test1()
{
  std::cout << "test1\n";

  const std::string X_p1_guid = "6AD63B61-BE75-40FC-B0C6-DD3C7801D871";
  assert (AthenaPoolCnvSvc::guidFromTypeinfo (typeid (X_p1)) == Guid (X_p1_guid));
}


void test2()
{
  std::cout << "test2\n";
  MsgStream msg (nullptr, "");
  Token X_token;
  X_token.fromString("[DB=AEC1DFE2-010B-D811-9832-000347F31C25]"
		  "[CNT=TestContainer]"
		  "[CLID=CAE53A87-64AD-4576-A203-1A4142E1E10F]"
		  "[TECH=00000100]"
		  "[OID=00000000-00000000]");
  const Token* X_poolToken = &X_token;

  Token X_p1_token;
  X_p1_token.fromString("[DB=AEC1DFE2-010B-D811-9832-000347F31C25]"
		  "[CNT=TestContainer]"
		  "[CLID=6AD63B61-BE75-40FC-B0C6-DD3C7801D871]"
		  "[TECH=00000100]"
		  "[OID=00000000-00000000]");
  const Token* X_p1_poolToken = &X_p1_token;

  Token X_p2_token;
  X_p2_token.fromString("[DB=AEC1DFE2-010B-D811-9832-000347F31C25]"
		  "[CNT=TestContainer]"
		  "[CLID=0AAC9C99-726D-4CF4-B9F9-00B6674C57DD]"
		  "[TECH=00000100]"
		  "[OID=00000000-00000000]");
  const Token* X_p2_poolToken = &X_p2_token;


  AthenaPoolCnvSvc::TPCnvElt<TestConverter, XCnv_p1> tpcnv;

  TestConverter cnv1;
  {
    std::unique_ptr<X> xptr = tpcnv.createTransient (cnv1, X_p1_poolToken, "key", msg);
    assert (xptr->m_a == 20);
  }

  X x2 (0);
  assert (tpcnv.persToTrans (cnv1, &x2, X_p1_poolToken, "key", msg));
  assert (x2.m_a == 20);
  
  TestConverter cnv2;
  assert (tpcnv.createTransient (cnv2, X_p2_poolToken, "key", msg) == nullptr);
  assert (!tpcnv.persToTrans (cnv2, &x2, X_p2_poolToken, "key", msg));

  AthenaPoolCnvSvc::TPCnvElt<TestConverter, T_TPCnvNull<X> > tpcnv_null;
  TestConverter cnv3;
  {
    std::unique_ptr<X> xptr = tpcnv_null.createTransient (cnv3, X_poolToken, "key", msg);
    assert (xptr->m_a == 10);
  }
  assert (tpcnv_null.createTransient (cnv1, X_p1_poolToken, "key", msg) == nullptr);

  assert (tpcnv_null.persToTrans (cnv3, &x2, X_poolToken, "key", msg));
  assert (x2.m_a == 10);
  assert (!tpcnv_null.persToTrans (cnv1, &x2, X_p1_poolToken, "key", msg));
}

//coverity[root_function]
int main()
{
  gSystem->Load("libAthenaPoolCnvSvcTestDict");
  test1();
  test2();
  return 0;
}
