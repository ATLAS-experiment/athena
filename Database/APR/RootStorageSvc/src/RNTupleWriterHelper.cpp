/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RNTupleWriterHelper.h"

#include "ROOT/RNTupleModel.hxx"
#include "RootUtils/APRDefaults.h"
#include "TFile.h"

namespace RootStorageSvc {

RNTupleWriterHelper::RNTupleWriterHelper(TFile* file,
                                         const std::string& ntupleName,
                                         bool enableBufferedWrite,
                                         bool enableMetrics)
    : pool::APRMessaging(std::string("RNTupleWriterHelper[") + ntupleName + "]"),
      m_model(ROOT::RNTupleModel::Create()),
      m_ntupleName(ntupleName),
      m_tfile(file),
      m_collectMetrics(enableMetrics) {
  m_opts.SetCompression(m_tfile->GetCompressionSettings());
  m_opts.SetUseBufferedWrite(enableBufferedWrite);
  m_model->SetDescription(ntupleName);
  RNTupleWriterHelper::addField(APRDefaults::IndexColName, "std::uint64_t");
}

void RNTupleWriterHelper::makeNewEntry() {
  if (m_model) {
    // prepare for writing of the first row
    if (!m_tfile) {
      throw std::runtime_error(std::string("Attempt to write RNTuple ") +
                               m_ntupleName + " without valid TFile ptr");
    } else {
      // write into existing file
      ATH_MSG_DEBUG("Creating RNTuple " << m_tfile->GetName() << "/"
                                        << m_ntupleName);
      m_ntupleWriter = ROOT::RNTupleWriter::Append(std::move(m_model), m_ntupleName,
                                             *m_tfile, m_opts);
      if (m_collectMetrics)
        m_ntupleWriter->EnableMetrics();
    }
  }
  m_entry = m_ntupleWriter->GetModel().CreateBareEntry();
}

void RNTupleWriterHelper::addAttribute(const attrDataTuple& in) {
  const std::string& name = std::get<0>(in);
  const std::string& type = std::get<1>(in);
  void* data = std::get<2>(in);
  ATH_MSG_DEBUG("Adding Dynamic Attribute " << name << " of type " << type);
  if (m_attrDataMap.find(name) == m_attrDataMap.end()) {
    addField(name, type);
  }
  addFieldValue(name, data);
}

/// Add a new field to the RNTuple
/// Used for data objects from RNTupleContainer, not for dynamic attributes
void RNTupleWriterHelper::addField(const std::string& field_name,
                                   const std::string& attr_type) {
  if (m_attrDataMap.find(field_name) != m_attrDataMap.end()) {
    throw std::runtime_error(
        std::string("Attempt to add existing field.  name: ") + field_name +
        "new type: " + attr_type);
  }
  ATH_MSG_DEBUG("Adding new object column, name=" << field_name << " of type "
                                                  << attr_type);
  auto field = ROOT::RFieldBase::Create(field_name, attr_type).Unwrap();
  if (!m_model) {
    // first write was already done, need to update the model
    ATH_MSG_DEBUG("Adding late attribute " << field_name);
    auto updater = m_ntupleWriter->CreateModelUpdater();
    updater->BeginUpdate();
    updater->AddField(std::move(field));
    updater->CommitUpdate();
  } else {
    m_model->AddField(std::move(field));
  }
  m_attrDataMap[field_name] = nullptr;
}

/// Supply data address for a given field
void RNTupleWriterHelper::addFieldValue(const std::string& field_name,
                                        void* attr_data) {
  auto field_iter = m_attrDataMap.find(field_name);
  if (field_iter == m_attrDataMap.end()) {
    std::stringstream msg;
    msg << "Attempt to write unknown Field with name: '" << field_name
        << std::ends;
    throw std::runtime_error(msg.str());
  }
  // already started writing
  field_iter->second = attr_data;
  m_needsCommit = true;
}

int RNTupleWriterHelper::commit() {
  // write only if there was data added, ignore empty commits
  if (!needsCommit()) {
    ATH_MSG_DEBUG("Empty Commit");
    return 0;
  }
  ATH_MSG_DEBUG("Commit, row=" << m_rowN << " : " << m_ntupleName);
  if (!m_entry)
    makeNewEntry();
  // update index field before commit

  int num_bytes = 0;
  ATH_MSG_DEBUG(m_ntupleName << " has " << m_attrDataMap.size()
                             << " attributes");
  int attrN = 0;
  for (auto& attr : m_attrDataMap) {
    ATH_MSG_VERBOSE("Setting data ptr for field# "
                    << ++attrN << ": " << attr.first << "  data=" << std::hex
                    << attr.second << std::dec);
    // If an object already exists bind it to the field
    // Otherwise, create a new value and use its address
    if (attr.second) {
      m_entry->BindRawPtr(attr.first, attr.second);
    } else {
      m_entry->EmplaceNewValue(attr.first);
      attr.second = m_entry->GetPtr<void>(attr.first).get();
    }
  }
  num_bytes += m_ntupleWriter->Fill(*m_entry);
  ATH_MSG_DEBUG("Filled RNTuple Row, bytes written: " << num_bytes);

  m_entry.reset();
  // forget all values to see if any object is missing in a new commit
  for (auto& attr : m_attrDataMap) {
    attr.second = nullptr;
  }
  m_needsCommit = false;
  m_rowN++;

  return num_bytes;
}

void RNTupleWriterHelper::close() {
  // Print metrics if enabled - this can become DEBUG/VERBOSE
  if (m_ntupleWriter->GetMetrics().IsEnabled()) {
    auto& log = msg(MSG::INFO);
    log << "Printing I/O Statistics\n";
    m_ntupleWriter->GetMetrics().Print(log.stream());
    log << endmsg;
  }
  // delete the generated default fields (RField should delete the default data
  // objects)
  m_ntupleWriter.reset();
  m_entry.reset();
  m_model.reset();
  m_rowN = 0;
}

}  // namespace RootStorageSvc
