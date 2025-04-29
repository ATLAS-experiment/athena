/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTUPLEWRITERHELPER_H
#define RNTUPLEWRITERHELPER_H

#include "AthenaBaseComps/AthMessaging.h"

#include "ROOT/REntry.hxx"
#include "ROOT/RField.hxx"
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleWriteOptions.hxx"
#include "ROOT/RNTupleWriter.hxx"

#include <tuple>

#if ROOT_VERSION_CODE < ROOT_VERSION( 6, 35, 0 )
namespace ROOT {
  using REntry = ROOT::Experimental::REntry;
  using RNTupleWriter = ROOT::Experimental::RNTupleWriter;
  using RNTupleWriteOptions = ROOT::Experimental::RNTupleWriteOptions;
  using RNTupleModel = ROOT::Experimental::RNTupleModel;
  using RFieldBase = ROOT::Experimental::RFieldBase;
}
#endif

namespace RootStorageSvc {

class RNTupleWriterHelper : public AthMessaging {
 public:
  /// Constructor
  RNTupleWriterHelper(TFile* file, const std::string& ntupleName,
                      bool enableBufferedWrite, bool enableMetrics);

  /// Default Destructor
  ~RNTupleWriterHelper() = default;

  /// Create a new empty RNTuple row with the current model (fields)
  void makeNewEntry();

  /// Add a new field to the RNTuple, collect the data pointer for the commit
  // The convention for the tuple is <name, type, data>
  typedef std::tuple<std::string, std::string, void*> attrDataTuple;
  void addAttribute(const attrDataTuple& in);

  /// Add a new field to the RNTuple
  void addField(const std::string& field_name, const std::string& attr_type);

  /// Supply data address for a given field
  void addFieldValue(const std::string& field_name, void* attr_data);

  /// Commit the data
  int commit();

  /// Name of the RNTuple
  const std::string& getName() const { return m_ntupleName; }

  /// Size of the RNTuple
  size_t size() const { return m_rowN; }

  /// Check if any data needs to be committed
  bool needsCommit() const { return m_needsCommit; }

  /// Is this RNTuple used by more than one APR container?
  bool isGrouped() const { return m_clients > 1; }

  /// Keep track of how many APR containers are writing to this RNTuple
  void increaseClientCount() { m_clients++; }

  /// Close the writer
  void close();

 private:
  /// Store data ptr for the first row, when only creating the model
  std::map<std::string, void*> m_attrDataMap;

  /// Internal cache for the RNTuple model
  /// Before first commit the fields are added to the model
  /// At the first commit the model passed to the RNTupleWriter
  /// which takes its ownership
  std::unique_ptr<ROOT::RNTupleModel> m_model;

  /// Internal cache for the RNEntry
  std::unique_ptr<ROOT::REntry> m_entry;

  /// Internal cache for the native RNTupleWriter
  std::unique_ptr<ROOT::RNTupleWriter> m_ntupleWriter;

  std::string m_ntupleName;
  TFile* m_tfile;
  ROOT::RNTupleWriteOptions m_opts;
  int m_rowN = 0;

  /// Count how many APR Containers are writing to this RNTuple (more than one
  /// makes a Group)
  int m_clients = 0;
  bool m_needsCommit = false;

  /// Enable/Disable Metric Collection
  bool m_collectMetrics;
};

}  // namespace RootStorageSvc
#endif
