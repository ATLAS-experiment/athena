/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkStripCablingTool_h
#define ITkStripCablingTool_h
/**
 * @file ITkStripCablingTool.h
 * Header file for  ITkStrip cabling service
 * Based on code by Susumu Oda
 * @author Shaun Roe
 * @author Edson Carquin
 * @author Daniel Torres
 **/

#include "ITkStripCabling/IITkStripCablingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Identifier/IdentifierHash.h"
#include "ITkStripCabling/ITkStripCablingData.h"
#include "StoreGate/ReadCondHandleKey.h"

// Gaudi includes
#include "GaudiKernel/EventContext.h"

//STL includes
#include <string>
#include <vector>

//fwd declarations
class StatusCode;
class SCT_ID;
class Identifier;

/**
 * @class ITkStripCablingTool, providing mappings of online and offline identifiers and also serial numbers
 **/
class ITkStripCablingTool: public extends<AthAlgTool, IITkStripCablingTool> {
 public:

  //@name Tool methods, reimplemented
  //@{
  using base_class::base_class;
  virtual ~ITkStripCablingTool() = default;
  virtual StatusCode initialize() override;
  //@}
  
  //@name ITkStripCablingTool methods implemented, these are visible to clients
  //@{
  
  virtual IdentifierHash getHashFromOnlineId(const ITkStripOnlineId& onlineId, const EventContext& ctx, const bool withWarnings = true) const;

  /// return the online Id, given a hash (used by simulation encoders)
  virtual ITkStripOnlineId getOnlineIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const override;
  virtual ITkStripOnlineId getOnlineIdFromHash(const IdentifierHash& hash) const override;    
  
  /// return the rob/rod Id, given a hash (used by simulation encoders)
  virtual std::uint32_t getRobIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const override;
  virtual std::uint32_t getRobIdFromHash(const IdentifierHash& hash) const override;
    
  /// fill a users vector with all the RodIds
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector, const EventContext& ctx) const override;
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector) const override;

  //@}

 private:
  SG::ReadCondHandleKey<ITkStripCablingData> m_data{this, "ITkStripCablingData", "ITkStripCablingData", "ITkStripCablingData created by ITkStripCablingCondAlgFromCoraCool"};
  StringProperty m_cablingDataSource{this, "DataSource", "", "the name of the data source"};
  bool m_usingDatabase{true};

  const SCT_ID* m_idHelper{nullptr}; //!< helper for offlineId/hash conversions

  const ITkStripCablingData* getData(const EventContext& ctx) const;
};

#endif // ITkStripCablingTool_h
