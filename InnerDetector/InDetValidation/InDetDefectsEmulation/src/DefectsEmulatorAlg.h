/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_DEFECTSEMULATOR_H
#define INDET_DEFECTSEMULATOR_H

#include "DefectsEmulatorBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "TH2.h"

namespace InDet {
template <class T_RDO_Container>
struct DefectsEmulatorTraits;

/** Algorithm template to selectivly copy RDOs from an InDetRawDataCollection
 *
 * Hits will be copied unless they are marked as defects in the "defects"
 * conditions data.
 */
template <class T_RDO_Container>
class DefectsEmulatorAlg : public DefectsEmulatorBase {
public:
  using T_ID_Helper =   DefectsEmulatorTraits<T_RDO_Container>::ID_Helper;
  using T_DefectsData = DefectsEmulatorTraits<T_RDO_Container>::DefectsData;
  using T_RDORawData =  DefectsEmulatorTraits<T_RDO_Container>::RDORawData;
  using T_RDORawDataConcreteType = DefectsEmulatorTraits<T_RDO_Container>::RDORawDataConcreteType;
  using T_ModuleHelper = DefectsEmulatorTraits<T_RDO_Container>::ModuleHelper;

  using DefectsEmulatorBase::DefectsEmulatorBase;
  DefectsEmulatorAlg &operator=(const DefectsEmulatorAlg&) = delete;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
   SG::ReadCondHandleKey<T_DefectsData> m_emulatedDefects
      {this, "EmulatedDefectsKey", ""};
   SG::ReadHandleKey<T_RDO_Container> m_origRdoContainerKey
      {this, "InputKey", ""};
   SG::WriteHandleKey<T_RDO_Container> m_rdoOutContainerKey
      {this, "OutputKey", ""};
   Gaudi::Property<std::string> m_idHelperName
      {this, "IDHelper",""};

   const T_ID_Helper* m_idHelper = nullptr;
};

}

#include "DefectsEmulatorAlg.icc"

#endif
