/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "SortedEventTagWriter.h"

#include "DataModelRoot/RootType.h"
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "StorageSvc/APRDefaults.h"

#include <StoreGate/ReadHandle.h>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <typeinfo>

StatusCode SortedEventTagWriter::initialize()
{
  // Initialize the read handle key and retrieve the pool conversion service
  ATH_CHECK(m_inputAttList.initialize());
  ATH_CHECK(m_athenaPoolCnvSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode SortedEventTagWriter::execute(const EventContext& ctx)
{
  // Read the input Athena attribute list
  SG::ReadHandle<AthenaAttributeList> inputAttributes(m_inputAttList, ctx);
  if (!inputAttributes.isValid()) {
    ATH_MSG_ERROR("Did not find input AthenaAttributeList");
    return StatusCode::FAILURE;
  }
  if (!inputAttributes->exists(m_tokenAttribute.value())) {
    ATH_MSG_ERROR("Input AthenaAttributeList has no '" << m_tokenAttribute.value()
                  << "' attribute");
    return StatusCode::FAILURE;
  }

  if (!inputAttributes->exists(m_sortAttribute.value())) {
    ATH_MSG_ERROR("Input AthenaAttributeList has no '" << m_sortAttribute.value()
                  << "' sort attribute");
    return StatusCode::FAILURE;
  }

  // Keep a copy of the full input row (all attributes are propagated)
  m_rows.push_back(std::make_unique<AthenaAttributeList>(*inputAttributes));
  return StatusCode::SUCCESS;
}

StatusCode SortedEventTagWriter::finalize()
{
  // If no event-tag rows were collected, there is nothing to write
  if (m_rows.empty()) {
    ATH_MSG_WARNING("No event tags were collected; no output file was written");
    return StatusCode::SUCCESS;
  }

  // Sort the collected event-tag rows by the configured attribute
  // (compared in the attribute's native type, so any numeric type is exact)
  const std::string& sortKey = m_sortAttribute.value();
  const std::type_info& sortType = (*m_rows.front())[sortKey].specification().type();
  auto sortAs = [&]<typename T>() {
    if (sortType != typeid(T)) return false;
    std::stable_sort(m_rows.begin(), m_rows.end(),
                     [&](const auto& lhs, const auto& rhs) {
                       return (*lhs)[sortKey].template data<T>() <
                              (*rhs)[sortKey].template data<T>();
                     });
    return true;
  };
  const bool sorted =
    sortAs.operator()<int>() || sortAs.operator()<unsigned int>() ||
    sortAs.operator()<long>() || sortAs.operator()<unsigned long>() ||
    sortAs.operator()<long long>() || sortAs.operator()<unsigned long long>() ||
    sortAs.operator()<float>() || sortAs.operator()<double>();
  if (!sorted) {
    ATH_MSG_ERROR("SortAttribute '" << sortKey << "' is not of a supported numeric type");
    return StatusCode::FAILURE;
  }

  // Connect to the output POOL file
  const std::string outputFile = m_outputFile.value();
  ATH_CHECK(m_athenaPoolCnvSvc->connectOutput(outputFile));

  // Prepare the placement and data type of each output column
  const std::string eventTagName = APRDefaults::WriteConfig::getEventTagName();
  auto makePlacement = [&](const std::string& column) {
    Placement placement;
    placement.setFileName(outputFile);
    placement.setContainerName(eventTagName + "(" + column + ")");
    return placement;
  };
  // One column per input attribute, except the skipped ones
  const auto& skipped = m_skipAttributes.value();
  std::vector<std::string> columns;
  for (const auto& attribute : *m_rows.front()) {
    const std::string& name = attribute.specification().name();
    if (std::find(skipped.begin(), skipped.end(), name) == skipped.end()) {
      columns.push_back(name);
    }
  }
  std::vector<Placement> columnPlacements;
  std::vector<RootType> columnTypes;
  for (const std::string& column : columns) {
    columnPlacements.push_back(makePlacement(column));
    // The token string is persisted as a Token
    columnTypes.emplace_back(column == m_tokenAttribute.value()
      ? RootType("Token")
      : RootType((*m_rows.front())[column].specification().type()));
  }

  // Write one row: all its columns, then a commit. RNTuple columns are grouped
  // into one entry per transaction, so each row needs its own commit; hold
  // keeps the session open for the next row
  auto writeRow = [&](const AthenaAttributeList& row) {
    for (std::size_t i = 0; i < columns.size(); ++i) {
      const coral::Attribute& attribute = row[columns[i]];
      const void* data = columns[i] == m_tokenAttribute.value()
        ? static_cast<const void*>(attribute.data<std::string>().c_str())
        : attribute.addressOfData();
      std::unique_ptr<Token> columnToken(
        m_athenaPoolCnvSvc->registerForWrite(&columnPlacements[i], data, columnTypes[i]));
      if (!columnToken) {
        ATH_MSG_ERROR("Failed to write " << columns[i] << " event-tag column");
        return false;
      }
    }
    if (m_athenaPoolCnvSvc->commitOutput(outputFile, false).isFailure()) {
      ATH_MSG_ERROR("Failed to commit event-tag row");
      return false;
    }
    return true;
  };

  // Write all collected event-tag rows to the output POOL file
  const bool written = std::all_of(
    m_rows.begin(), m_rows.end(),
    [&](const std::unique_ptr<AthenaAttributeList>& row) { return writeRow(*row); });

  // Single disconnect for success and failure: it commits, flushes and closes the file
  const StatusCode disconnected = m_athenaPoolCnvSvc->disconnectOutput(outputFile);
  if (!written || disconnected.isFailure()) {
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Wrote " << m_rows.size() << " sorted event-tag rows to "
                        << outputFile);
  return StatusCode::SUCCESS;
}