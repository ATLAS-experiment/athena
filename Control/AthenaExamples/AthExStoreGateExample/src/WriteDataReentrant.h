// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file WriteDataReentrant.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Testing reentrant algorithms.
 */


#ifndef ATHEXSTOREGATEEXAMPLE_WRITEDATAREENTRANT_H
#define ATHEXSTOREGATEEXAMPLE_WRITEDATAREENTRANT_H


#include <string>
#include "AthExStoreGateExample/MyDataObj.h"
#include "StoreGateExample_ClassDEF.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandleKeyArray.h"


class WriteDataReentrant
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute (const EventContext& ctx) const override final;

private:
  SG::WriteHandleKey<MyDataObj> m_dobjKey{this, "DObjKey", "dobj"};
  SG::WriteHandleKey<MyDataObj> m_dobjKey2{this, "DObjKey2", "dobj2"};
  SG::WriteHandleKey<MyDataObj> m_dobjKey3{this, "DObjKey3", ""};
  SG::WriteHandleKey<DataVector<MyContObj> > m_cobjKey{this, "CObjKey", "cobj"};
  SG::WriteHandleKey<std::vector<float> > m_vFloatKey{this, "VFloatKey", "vFloat"};
  SG::WriteHandleKey<MapStringFloat> m_mKey{this, "MKey", "mkey"};
  SG::WriteHandleKey<std::list<ElementLink<std::vector<float>>>> m_pLinkListKey{this,"PLinkListKey", ""};
  SG::WriteHandleKey<std::vector<ElementLink<MapStringFloat>>> m_linkVectorKey{this, "LinkVectorKey", "linkvec"};
  SG::WriteHandleKey<TestDataObject> m_testObjectKey{this, "TestObjectKey", "testobj"};

  SG::WriteHandleKeyArray<MyDataObj> m_dobjKeyArray{this, "DObjKeyArray", {"dobj_a1", "dobj_a2"}};

  SG::DataObjectSharedPtr<TestDataObject> m_testObject;
  StatusCode onError() const;
};


#endif // not ATHEXSTOREGATEEXAMPLE_WRITEDATAREENTRANT_H
