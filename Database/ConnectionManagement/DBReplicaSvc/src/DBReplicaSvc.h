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
#include "DBReplicaSvc/IDBReplicaSvc.h"
#include "RelationalAccess/IDatabaseServiceDescription.h"

class DBReplicaSvc : public extends<AthService, IDBReplicaSvc>
{
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

  bool m_frontiergen{false};
  std::string m_hostname;
  typedef std::pair<std::string, int> ServerPair;  //<! (priority, name) pair
  std::vector<ServerPair> m_servermap;
};

#endif // DBREPLICASVC_DBREPLICASVC_H
