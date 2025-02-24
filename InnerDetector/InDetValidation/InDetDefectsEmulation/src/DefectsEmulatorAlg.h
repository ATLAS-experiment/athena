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
  /** Adapter to support different types of RDO collections, and ID helpers.
   * Must have the following traits:
   * - The adapter is constructable from the ID_Helper
   * - provides methods row_index and column_index to compute the row and column
   *   index from an Identifier
   * - provides a method cloneOrRejectHit which returns the number of newly added hits
   *   and takes as arguments: a module helper and matching emulated  defects conditions
   *   data, a module id hash, a row and column index, the input RDO, and the collection
   *   to which the "cloned" RDOs are added.
   */
  using T_ID_Adapter = DefectsEmulatorTraits<T_RDO_Container>::IDAdapter;
  using T_DefectsData = DefectsEmulatorTraits<T_RDO_Container>::DefectsData;
  using T_RDORawData =  DefectsEmulatorTraits<T_RDO_Container>::RDORawData;
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

   T_ID_Helper m_idHelper{};
};

}

#include "DefectsEmulatorAlg.icc"

#endif
