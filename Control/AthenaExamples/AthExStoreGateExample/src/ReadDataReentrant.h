// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ReadDataReentrant.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Testing reentrant algorithms.
 */


#ifndef ATHEXSTOREGATEEXAMPLE_READDATAREENTRANT_H
#define ATHEXSTOREGATEEXAMPLE_READDATAREENTRANT_H

#include "MyContObj.h"
#include "MapStringFloat.h"

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthExStoreGateExample/MyDataObj.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

class ReadDataReentrant
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute (const EventContext& ctx) const override final;

private:
  SG::ReadHandleKey<DataVector<MyContObj>> m_cobjKey{this, "CObjKey", "cobj"};
  SG::ReadHandleKey<std::vector<float>> m_vFloatKey{this, "VFloatKey", "vFloat"};
  SG::ReadHandleKey<std::list<ElementLink<std::vector<float>>>> m_pLinkListKey{this, "PLinkListKey", "WriteDataReentrant"};
  SG::ReadHandleKey<std::vector<ElementLink<MapStringFloat>>> m_linkVectorKey{this, "LinkVectorKey", "linkvec"};
  SG::ReadHandleKey<TestDataObject> m_testObjectKey{this, "TestObjectKey", "testobj"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EvtInfo", "EventInfo", "EventInfo name"};
  SG::ReadHandleKeyArray<MyDataObj> m_dobjKeyArray{this, "DObjKeyArray", {"dobj_a1", "dobj_a2"}};
  SG::ReadHandleKey<MyDataObj> m_nonexistingKey{this, "NonExistingKey", "foo"};
};


#endif // not ATHEXSTOREGATEEXAMPLE_READDATAREENTRANT_H
