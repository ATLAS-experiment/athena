/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiDatabaseConfig_h
#define dqiDatabaseConfig_h


#include <nlohmann/json.hpp>
#include "CoolKernel/pointers.h" //for IFolderPtr, IDatabasePtr typedef
#include <unordered_map>
#include <string>
namespace dqi {

class DatabaseConfig {

public:
  DatabaseConfig(std::string connectionString, long runNumber);
  nlohmann::json GetPayload(const std::string& tag);

  bool IsConnected() const;
  void Disconnect();

private:
  const std::string m_connectionString;
  const long m_runNumber{};

  bool m_dbConnected{};
  bool m_folderConnected{};

  std::unordered_map<std::string, nlohmann::json> m_jsonData;
  cool::IFolderPtr m_folder;
  cool::IDatabasePtr m_database;

  long GetRunNumber() const;
  nlohmann::json LoadPayload(const std::string& tag);

  void Connect();
};

}
#endif
