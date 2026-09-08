/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DBREPLICASVC_DBREPLICASVC_H
#define DBREPLICASVC_DBREPLICASVC_H
// DBReplicaSvc.h - concrete implementation of service implementating
// CORAL IReplicaSortingAlgorithm
// Richard Hawkings, started 24/4/07

#include <string>

#include "AthenaBaseComps/AthService.h"
#include "CoralKernel/Context.h"
#include "DBReplicaSvc/IDBReplicaSvc.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"

class DBReplicaSvc : public extends<AthService, IDBReplicaSvc> {
 public:
  using base_class::base_class;

  virtual StatusCode initialize() override;

  void sort(std::vector<const coral::IDatabaseServiceDescription*>& replicaSet) override;
  
 private:
  StatusCode readConfig();

  Gaudi::Property<std::string> m_configfile{this, "ConfigFile", "dbreplica.config"};
  Gaudi::Property<std::string> m_testhost{this, "TestHost", ""};
  Gaudi::Property<std::string> m_coolsqlitepattern{this, "COOLSQLiteVetoPattern", ""};
  Gaudi::Property<bool> m_usecoolsqlite{this, "UseCOOLSQLite", true};
  Gaudi::Property<bool> m_usecoolfrontier{this, "UseCOOLFrontier", true};
  Gaudi::Property<bool> m_usegeomsqlite{this, "UseGeomSQLite", true};
  Gaudi::Property<bool> m_nofailover{this, "DisableFailover", false};

  /// ConnectionRetrialPeriod, retry period for CORAL Connection Service: default = 30 seconds
  Gaudi::Property<int> m_retrialPeriod{this, "ConnectionRetrialPeriod", 300};
  /// ConnectionRetrialTimeOut, the retrial time out for CORAL Connection Service: default = 300 seconds
  Gaudi::Property<int> m_retrialTimeOut{this, "ConnectionRetrialTimeOut", 3600};
  /// ConnectionTimeOut, the time out for CORAL Connection Service: default = 5 seconds
  Gaudi::Property<int> m_timeOut{this, "ConnectionTimeOut", 5};
  /// ConnectionCleanUp - whether to use CORAL connection management thread: default = false.
  Gaudi::Property<bool> m_connClean{this, "ConnectionCleanUp", false};
  /// Frontier proprties, compression level and list of schemas to be refreshed: default = 5
  Gaudi::Property<int> m_frontierComp{this, "FrontierCompression", 5};
  Gaudi::Property<std::vector<std::string>> m_frontierRefresh{this, "FrontierRefreshSchema", {}};

  coral::Context* m_context{nullptr};
  bool m_frontiergen{false};
  std::string m_hostname;
  typedef std::pair<std::string, int> ServerPair;  //<! (priority, name) pair
  std::vector<ServerPair> m_servermap;
};

#endif  // DBREPLICASVC_DBREPLICASVC_H
