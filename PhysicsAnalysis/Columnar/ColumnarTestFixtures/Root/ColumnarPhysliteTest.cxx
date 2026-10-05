/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <AsgTesting/UnitTest.h>
#include <ColumnarTestFixtures/PerformanceData.h>
#include <ColumnarCore/ColumnInfoHelpers.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarInterfaces/KnownSgKeys.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>
#include <ColumnarTestFixtures/Benchmark.h>
#include <ColumnarTestFixtures/Configuration.h>
#include <ColumnarTestFixtures/PhysliteTest.h>
#include <ColumnarTestFixtures/ToolWrapper.h>
#include <PATInterfaces/ISystematicsTool.h>
#include <TruthUtils/ParticleConstants.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/versions/MissingETAuxAssociationMap_v2.h>
#include <xAODMissingET/versions/MissingETBase.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>

#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>
#include <xAODCore/ShallowCopy.h>

#ifndef XAOD_STANDALONE
#include <POOLRootAccess/TEvent.h>
#endif

#include <TFile.h>
#include <TLeaf.h>
#include <TTree.h>

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleInspector.hxx"
#include "ROOT/RNTupleReader.hxx"
#include <ROOT/RNTupleView.hxx>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <format>
#include <memory>
#include <span>
#include <vector>

#include <gtest/gtest.h>

//
// method implementations
//

namespace columnar 
{
  // I'm moving code to this namespace but some of the code in this file
  // is still just in the columnar namespace. As I evolve the code I'll
  // move more of it to TestUtils.
  using namespace TestUtils;

  namespace TestUtils 
  {

    struct RNTupleBackend {
      ROOT::RNTupleReader* reader = nullptr;
      ROOT::Experimental::RNTupleInspector* inspector = nullptr;
    };

    using Backend = std::variant<TTree*, RNTupleBackend*>;
    template <typename T>
    class BranchReader final 
    {
      std::string m_branchName;
      TBranch* m_branch = nullptr;
      bool m_isStatic = std::is_pod_v<T>;
      T* m_data{new T()};

    public:
      BranchReader(const std::string& val_branchName)
         : m_branchName(val_branchName) 
        {
         if (m_branchName.find("Aux.") != std::string::npos)
          m_isStatic = true;
        }

      ~BranchReader() noexcept 
      { 
        delete m_data; 
      }

      BranchReader(const BranchReader&) = delete;
      BranchReader& operator=(const BranchReader&) = delete;

      void setIsStatic(bool isStatic) 
      { 
        m_isStatic = isStatic; 
      }

      [[nodiscard]] const std::string& branchName() const 
      { 
        return m_branchName; 
      }

      [[nodiscard]] std::string columnName() const 
      {
        std::string columnName = m_branchName;
        if (auto index = columnName.find("AuxDyn."); index != std::string::npos)
          columnName.replace(index, 6, "");
        else if (auto index = columnName.find("Aux."); index != std::string::npos)
          columnName.replace(index, 3, "");
        else if (columnName.find(".") != std::string::npos)
          throw std::runtime_error("branch name does not contain AuxDyn or Aux: " +m_branchName);
        return columnName;
      }

      [[nodiscard]] std::string containerName() const 
      {
        if (auto index = m_branchName.find("AuxDyn."); index != std::string::npos)
          return m_branchName.substr(0, index);
        else if (auto index = m_branchName.find("Aux."); index != std::string::npos)
          return m_branchName.substr(0, index);
        else if (m_branchName.find(".") == std::string::npos)
          return m_branchName;
        else
          throw std::runtime_error("branch name does not contain AuxDyn or Aux: " +m_branchName);
      }

      void connectTree(TTree* tree) 
      {
        m_branch = tree->GetBranch(m_branchName.c_str());
        if (!m_branch)
          throw std::runtime_error("failed to get branch: " + m_branchName);
        m_branch->SetMakeClass(1);
        if (m_isStatic)
          m_branch->SetAddress(m_data);
        else
         m_branch->SetAddress(&m_data);
      }
  
      void connectTree(const Backend& b) 
      {
        auto* tree = std::get<TTree*>(b);  // throws if wrong backend
        connectTree(tree);
      }
  
      const T& getEntry(Long64_t entry) 
      {
        if (!m_branch)
          throw std::runtime_error("branch not connected: " + m_branchName);
        if (m_branch->GetEntry(entry) <= 0)
          throw std::runtime_error("failed to get entry " + std::to_string(entry) + " for branch: " + m_branchName);
        if (m_data == nullptr)
          throw std::runtime_error("got nullptr reading data for branch: " + m_branchName);
        return *m_data;
      }

      const T& getCachedEntry() const 
      { 
        return *m_data; 
      }

      std::optional<float> entrySize() const 
      {
        if (!m_branch)
          return std::nullopt;
        return static_cast<float>(m_branch->GetZipBytes()) / m_branch->GetEntries();
      }

      std::optional<float> uncompressedSize() const 
      {
        if (!m_branch)
          return std::nullopt;
        return static_cast<float>(m_branch->GetTotBytes()) / m_branch->GetEntries();
      }

      // technically this is const-correct, but I don't want to convince
      // the code checker of that
      std::optional<unsigned> numBaskets() 
      {
        if (!m_branch)
          return std::nullopt;
        return m_branch->GetListOfBaskets()->GetSize();
      }
    };

    template <typename T>
    class BranchReaderArray final 
    {
    public:
      std::string m_branchName;
      TBranch* m_branch = nullptr;
      std::vector<T> m_dataVec;

    public:
      BranchReaderArray(const std::string& val_branchName)
        : m_branchName(val_branchName) 
      {}

      BranchReaderArray(const BranchReaderArray&) = delete;
      BranchReaderArray& operator=(const BranchReaderArray&) = delete;

      [[nodiscard]] std::string columnName() const 
      {
        std::string columnName = m_branchName;
        if (auto index = columnName.find("AuxDyn."); index != std::string::npos)
          columnName.replace(index, 6, "");
        else if (auto index = columnName.find("Aux."); index != std::string::npos)
          columnName.replace(index, 3, "");
        else if (columnName.find(".") != std::string::npos)
          throw std::runtime_error("branch name does not contain AuxDyn or Aux: " + m_branchName);
        return columnName;
      }

      [[nodiscard]] std::string containerName() const 
      {
        if (auto index = m_branchName.find("AuxDyn."); index != std::string::npos)
          return m_branchName.substr(0, index);
        else if (auto index = m_branchName.find("Aux."); index != std::string::npos)
          return m_branchName.substr(0, index);
        else if (m_branchName.find(".") == std::string::npos)
          return m_branchName;
        else
          throw std::runtime_error("branch name does not contain AuxDyn or Aux: " + m_branchName);
      }
      void connectTree (TTree *tree)
      {
        m_branch = tree->GetBranch (m_branchName.c_str());
        if (!m_branch)
          throw std::runtime_error ("failed to get branch: " + m_branchName);
        m_branch->SetMakeClass (1);
        // FIX ME: I have to have some hard-coded size, see explanation
        // below.
        m_dataVec.resize (100);
        if (!m_dataVec.empty())
          m_branch->SetAddress (m_dataVec.data());
      }

      std::span<const T> getEntry (Long64_t entry, std::size_t size)
      {
        if (!m_branch)
          throw std::runtime_error ("branch not connected: " + m_branchName);
        if (m_dataVec.size() < size)
        {
          // FIX ME: in one of the latest releases the repointing below
          // breaks, and causes memory corruption. so I'm now
          // preallocating and fail rather than reallocate, and the
          // problem goes away. maybe it should be investigated at some
          // point, but this is a test and I already spend a fair amount
          // of time investigating this. the harm is that this test
          // consumes a few hundreds bytes more in memory and we may have
          // to occasionally increase the buffer size to cover all test
          // files and branch lengths.
          throw std::runtime_error ("requested size exceeds buffer size for branch: " + m_branchName);
          // m_dataVec.resize (size);
          // m_branch->SetAddress (m_dataVec.data());
        }
        if (size > 0 && m_branch->GetEntry (entry) <= 0)
          throw std::runtime_error ("failed to get entry " + std::to_string (entry) + " for branch: " + m_branchName);
        return std::span<const T>(m_dataVec.data(), size);
      }

      std::optional<float> entrySize () const
      {
        if (!m_branch)
          return std::nullopt;
        return static_cast<float>(m_branch->GetZipBytes()) / m_branch->GetEntries();
      }

      std::optional<float> uncompressedSize () const
      {
        if (!m_branch)
          return std::nullopt;
        return static_cast<float>(m_branch->GetTotBytes()) / m_branch->GetEntries();
      }

      // technically this is const-correct, but I don't want to convince
      // the code checker of that
      std::optional<unsigned> numBaskets ()
      {
        if (!m_branch)
          return std::nullopt;
        return m_branch->GetListOfBaskets()->GetSize();
      }
    };

    class LinkColumnVector final
    {
      /// Public Members
      /// ==============
    public:

      using CM = ColumnarModeArray;

      ~LinkColumnVector () noexcept
      {
        if (!m_unknownKeysAllowedTargets.empty())
        {
          std::cout << "found unknown keys for " << m_columnName << ":";
          for (auto& [key, allowedSet] : m_unknownKeysAllowedTargets)
          {
            std::cout << "  " << std::hex << key << std::dec << " (allowed targets:";
            for (auto index : allowedSet)
              std::cout << " " << m_targetNames.at(index);
            std::cout << ")";
          }
        }
      }

      [[nodiscard]] std::vector<std::string> connect (const ColumnInfo& columnInfo, const std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, const std::unordered_map<std::string,ColumnInfo>& requestedColumns)
      {
        m_columnName = columnInfo.name;
        std::vector<std::string> keyColumnNames;
        if (!columnInfo.soleLinkTargetName.empty())
        {
          addTarget (columnInfo.soleLinkTargetName, offsetColumns, columnInfo.soleLinkTargetClid);
        } else
        {
          for (auto& [requestedName, requestedInfo] : requestedColumns)
          {
            if (requestedInfo.keyColumnForVariantLink == m_columnName)
            {
              keyColumnNames.push_back (requestedName);
              m_keysColumns.emplace_back();
              for (const auto& targetName : requestedInfo.variantLinkTargetNames)
                addTarget (targetName, offsetColumns);
            }
          }
          if (m_keysColumns.empty())
            throw std::runtime_error ("no key column found for variant link: " + m_columnName);
        }
        return keyColumnNames;
      }

      void clear ()
      {
        m_columnData.clear();
      }

      void checkOffsets (unsigned eventIndex)
      {
        for (std::size_t i = 0; i < m_targetNames.size(); ++ i)
        {
          auto& targetOffsetColumn = *m_targetOffsetColumns.at(i);
          if (eventIndex + 1 >= targetOffsetColumn.size())
            throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(i) + " when checking link column " + m_columnName);
        }
      }

      template<typename T>
      void addLink (const ElementLink<T>& element, unsigned eventIndex)
      {
        if (element.isDefault())
        {
          addEmptyLink();
          return;
        }

        addSplitLink (element.index(), element.key(), eventIndex);
      }

      void addEmptyLink ()
      {
        m_columnData.push_back (invalidObjectIndex);
      }

      void addSplitLink (std::size_t linkIndex, SG::sgkey_t linkKey, unsigned eventIndex)
      {
        if (linkIndex == 0 && linkKey == 0)
        {
          addEmptyLink();
          return;
        }

        unsigned targetIndex = 0u;
        while (targetIndex < m_targetKeys.size() && m_targetKeys.at(targetIndex) != linkKey)
          ++ targetIndex;

        // We didn't find the key, so we try to figure out which of the
        // targets it could be. The idea is that you wouldn't rely on
        // this for real tests, but that you then go and fill in those
        // keys in the central lookup table. It will always record and
        // report, that means if there is a variant link with extra
        // targets you didn't declare you will get a diagnostic. This
        // may be overly cautious, but it gives an extra diagnostic if
        // maybe you missed a target.
        if (targetIndex == m_targetKeys.size())
        {
          if (!m_unknownKeysAllowedTargets.contains (linkKey))
          {
            auto& allowedSet = m_unknownKeysAllowedTargets[linkKey];
            for (std::size_t i = 0; i < m_targetKeys.size(); ++ i)
            {
              if (m_targetKeys.at(i) == 0)
                allowedSet.insert(i);
            }
          }
          auto& allowedSet = m_unknownKeysAllowedTargets[linkKey];
          for (auto iter = allowedSet.begin(); iter != allowedSet.end();)
          {
            auto index = *iter;
            auto& targetOffsetColumn = *m_targetOffsetColumns.at(index);
            if (eventIndex + 1 >= targetOffsetColumn.size())
              throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(index));
            if (targetOffsetColumn.at(eventIndex) + linkIndex >= targetOffsetColumn.at(eventIndex + 1))
              iter = allowedSet.erase(iter);
            else
              ++ iter;
          }
          // Not quite sure whether it is safer to use or not use one of
          // the targets from the allowed set in this case. In general
          // tools are expected to handle invalid links gracefully,
          // worst case they throw an exception when trying to access
          // it. So what I came up with is that for variant links we
          // assume it invalid, but for non-variant links the tool
          // expects exactly one target and we either found it or throw
          // an exception.
          if (m_keysColumns.empty())
          {
            if (allowedSet.size() == 1 && m_targetKeys.at(*allowedSet.begin()) == 0 && m_unknownKeysAllowedTargets.size() == 1)
              targetIndex = *allowedSet.begin();
            else
            {
              std::ostringstream error;
              error << "target key mismatch: read sgkey " << std::hex << linkKey << std::dec;
              error << " for column " << m_columnName << " with element index " << linkIndex << " targeting " << m_targetNames.at(0);
              if (m_targetKeys.at(0) != 0u)
              {
                error << ", expected sgkey " << std::hex << m_targetKeys.at(0) << std::dec;
              } else if (m_unknownKeysAllowedTargets.size() > 1)
              {
                error << ", alternate key found for non-variant link:";
                for (auto& [key, allowedSet] : m_unknownKeysAllowedTargets)
                {
                  if (key != linkKey)
                    error << " " << std::hex << key << std::dec;
                }
              } else
              {
                error << ", no expected sgkey configured but the maximum allowed index for the target is " << m_targetOffsetColumns.at(0)->at(eventIndex + 1) - m_targetOffsetColumns.at(0)->at(eventIndex) - 1;
              }
              throw std::runtime_error (std::move (error).str());
            }
          }
        }

        if (targetIndex == m_targetKeys.size())
        {
          // this creates a link with an unknown key, which the user
          // will ignore
          m_columnData.push_back (CM::mergeLinkKeyIndex (0xff, linkIndex));
          return;
        }

        auto& targetOffsetColumn = *m_targetOffsetColumns.at(targetIndex);
        if (eventIndex + 1 >= targetOffsetColumn.size())
          throw std::runtime_error ("target offset column not yet filled for: " + m_targetNames.at(targetIndex));
        auto myLinkIndex = linkIndex + targetOffsetColumn.at(eventIndex);
        if (myLinkIndex >= targetOffsetColumn.at(eventIndex + 1))
          throw std::runtime_error ("index out of range for link: " + m_columnName + " with element index " + std::to_string(linkIndex) + " targeting " + m_targetNames.at(targetIndex) + " with offset " + std::to_string(targetOffsetColumn.at(eventIndex)) + " and next offset " + std::to_string(targetOffsetColumn.at(eventIndex + 1)));

        m_columnData.push_back (CM::mergeLinkKeyIndex (targetIndex, myLinkIndex));
      }

      [[nodiscard]] std::size_t size () const noexcept
      {
        return m_columnData.size();
      }

      [[nodiscard]] const typename CM::LinkIndexType* data () const noexcept
      {
        return m_columnData.data();
      }

      [[nodiscard]] auto begin () const noexcept { return m_columnData.begin(); }
      [[nodiscard]] auto end () const noexcept { return m_columnData.end(); }

      [[nodiscard]] const std::vector<typename CM::LinkKeyType>& keysColumn (std::size_t index) const
      {
        return m_keysColumns.at(index);
      }



      /// Private Members
      /// ===============
    private:

      std::vector<typename CM::LinkIndexType> m_columnData;

      std::string m_columnName;

      std::vector<std::string> m_targetNames;
      std::vector<SG::sgkey_t> m_targetKeys;
      std::vector<const std::vector<ColumnarOffsetType>*> m_targetOffsetColumns;

      // there can be multiple keys-columns, hence this is a vector of
      // vectors. if this is empty, then it is a single-target link
      std::vector<std::vector<typename CM::LinkKeyType>> m_keysColumns;

      std::unordered_map<SG::sgkey_t,std::unordered_set<std::size_t>> m_unknownKeysAllowedTargets;



      void addTarget (const std::string& name, const std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::uint32_t clid = 0)
      {
        unsigned targetIndex = 0;
        while (targetIndex < m_targetNames.size() && m_targetNames.at(targetIndex) != name)
          ++ targetIndex;
        if (targetIndex == m_targetNames.size())
        {
          m_targetNames.push_back(name);
          if (auto offsetIter = offsetColumns.find (name); offsetIter != offsetColumns.end())
            m_targetOffsetColumns.push_back (offsetIter->second);
          else
            throw std::runtime_error ("missing offset column: " + name);
          if (clid != 0)
            m_targetKeys.push_back (computeSgKey (name, clid));
          else if (auto keyIter = knownSgKeys.find (name); keyIter != knownSgKeys.end())
            m_targetKeys.push_back (keyIter->second);
          else
            m_targetKeys.push_back (0);
        }
        if (!m_keysColumns.empty())
          m_keysColumns.back().push_back (targetIndex);
      }
    };

    template <typename T>
    class RNTFieldReader final 
    {
      std::string m_FieldName;
      std::unique_ptr<ROOT::RNTupleView<T>> m_view;
      ROOT::Experimental::RNTupleInspector* m_inspector = nullptr;
      ROOT::RNTupleReader* m_reader = nullptr;
      const T* m_data = nullptr;

    public:
      RNTFieldReader(const std::string& val_fieldName)
      : m_FieldName(val_fieldName) 
      {}

      ~RNTFieldReader() noexcept {}
      RNTFieldReader(const RNTFieldReader&) = delete;
      RNTFieldReader& operator=(const RNTFieldReader&) = delete;

      [[nodiscard]] const std::string& fieldName() const 
      { 
        return m_FieldName; 
      }

      [[nodiscard]] std::string columnName() const 
      {
        std::string columnName = m_FieldName;
        if (auto index = columnName.find("AuxDyn:"); index != std::string::npos)
          columnName.replace(index, 6, "");
        else if (auto index = columnName.find("Aux:."); index != std::string::npos)
          columnName.replace(index, 4, "");
        else if (auto index = columnName.find("Aux:"); index != std::string::npos)
          columnName.replace(index, 3, "");
        else if (columnName.find(":") != std::string::npos)
          throw std::runtime_error("field name does not contain AuxDyn or Aux: " + m_FieldName);
        std::replace(columnName.begin(), columnName.end(), ':', '.');

        return columnName;
      }

      [[nodiscard]] std::string containerName() const 
      {
        if (auto index = m_FieldName.find("AuxDyn:"); index != std::string::npos)
          return m_FieldName.substr(0, index);
        else if (auto index = m_FieldName.find("Aux:"); index != std::string::npos)
          return m_FieldName.substr(0, index);
        else if (m_FieldName.find(":") == std::string::npos)
          return m_FieldName;
        else
          throw std::runtime_error("field name does not contain AuxDyn or Aux: " + m_FieldName);
      }

      void connectRNTuple(ROOT::RNTupleReader* reader,
                      ROOT::Experimental::RNTupleInspector* inspector) 
      {
        m_inspector = inspector;
        m_reader = reader;
        m_view = std::make_unique<ROOT::RNTupleView<T>>(reader->GetView<T>(m_FieldName));

        if (!m_view)
          throw std::runtime_error("failed to get field: " + m_FieldName);
      }

      void connectTree(const Backend& b) 
      {
        auto* rntbackend = std::get<RNTupleBackend*>(b);  // throws if wrong backend

        if (!rntbackend->reader || !rntbackend->inspector)
          throw std::runtime_error("RNTuple backend not properly initialized");
        connectRNTuple(rntbackend->reader, rntbackend->inspector);
      }
  
  
      const T& getEntry(Long64_t entry) 
      {
        if (!m_view)
          throw std::runtime_error("field not connected: " + m_FieldName);

        m_data = &((*m_view)(static_cast<ROOT::NTupleSize_t>(entry)));

        if (m_data == nullptr)
          throw std::runtime_error("got nullptr reading data for field: " + m_FieldName);
        return *m_data;
      }

      const T& getCachedEntry() const 
      {  
        return *m_data; 
      }

      std::optional<float> entrySize() const 
      {

        const ROOT::Experimental::RNTupleInspector::RFieldTreeInspector& fieldTreeInspector = m_inspector->GetFieldTreeInspector(m_FieldName);
        return static_cast<float>(fieldTreeInspector.GetCompressedSize()) /
          m_inspector->GetDescriptor().GetNEntries();
      }

      std::optional<float> uncompressedSize() const 
      {

        const ROOT::Experimental::RNTupleInspector::RFieldTreeInspector& fieldTreeInspector = m_inspector->GetFieldTreeInspector(m_FieldName);

        return static_cast<float>(fieldTreeInspector.GetUncompressedSize()) /
          m_inspector->GetDescriptor().GetNEntries();
      }

      std::optional<unsigned> numBaskets() 
      {
       // placeholder
       return std::nullopt;
      }
    };

    class IColumnData
    {
    public:

      struct OutputColumnInfo
      {
        std::string name;
        bool isOffset = false;
        bool primary = false;
        bool enabled = false;
        std::size_t columnIndex = ColumnVectorHeader::nullIndex;
      };
      std::vector<OutputColumnInfo> outputColumns;

      virtual ~IColumnData () noexcept = default;

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) = 0;

      /// @brief lookup and store column indices from the header for all enabled output columns
      void connectColumnIndices (const ColumnVectorHeader& header)
      {
        for (auto& col : outputColumns)
        {
          if (col.enabled)
            col.columnIndex = header.getColumnIndex (col.name);
        }
      }

      virtual void clearColumns () = 0;

      virtual void getEntry (Long64_t entry) = 0;

      virtual void setData (ColumnVectorData& columnData) = 0;

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) = 0;

      virtual void collectColumnData () = 0;
    };

    struct ColumnDataEventCount final : public TestUtils::IColumnData
    {
      std::array<ColumnarOffsetType, 2> data = {0, 0};

      ColumnDataEventCount ()
      {
        outputColumns.push_back ({.name = eventRangeColumnName, .isOffset = true});
      }

      virtual bool connect(Backend /*source*/, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/,std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       if (requestedColumns.contains(outputColumns.at(0).name)) 
       {
         requestedColumns.erase(outputColumns.at(0).name);
         outputColumns.at(0).enabled = true;
         return true;
       }
       return false;
      }
      virtual void clearColumns () override
      {
        data[0] = 0;
        data[1] = 0;
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        data[1] += 1;
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, data.size(), data.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = "EventCount(auto)";
        return result;
      }

      virtual void collectColumnData () override
      {}
    };


    template <typename T, template <typename> class Reader>
    struct ColumnDataScalar final : public TestUtils::IColumnData
    {
      Reader<T> branchReader;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      std::vector<T> outData;
      unsigned entries = 0;

      explicit ColumnDataScalar (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
      }

      virtual bool connect( Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/,std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;
        requestedColumns.erase (iter);

        branchReader.connectTree (source);

        return true;
      }
      virtual void clearColumns () override
      {
        outData.clear ();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        outData.push_back (branchData);
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVector final : public TestUtils::IColumnData 
    {
      Reader<std::vector<T>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> outData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnDataVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          requestedColumns.erase (iter);
          outputColumns.at(1).enabled = true;
        }

        return true;
      }

      virtual void clearColumns () override
      {
        offsets.clear ();
        offsets.push_back (0);
        outData.clear ();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        outData.insert (outData.end(), branchData.begin(), branchData.end());
        offsets.push_back (outData.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataOutVector final : public TestUtils::IColumnData 
    {
      T defaultValue;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<T> outData;
      unsigned entries = 0;

      ColumnDataOutVector (const std::string& val_columnName, const T& val_defaultValue)
        : defaultValue (val_defaultValue)
      {
        outputColumns.push_back ({.name = val_columnName});
      }

      virtual bool connect([[maybe_unused]]Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        const auto & offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name);

        if (auto offsetIter = offsetColumns.find (offsetName); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name + " (offset column: " + offsetName + ")");

        // IMPORTANT: This has to be erased at the end of the function,
        // since erasing it earlier would invalidate the offsetName
        // reference.
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        outData.clear ();
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        outData.resize (offsetColumn->back(), defaultValue);
      }

      virtual void setData (ColumnVectorData& columnData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          columnData.setColumn (outputColumns.at(0).columnIndex, outData.size(), outData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = outputColumns.at(0).name + "(out)";
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += outData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorVector final : public TestUtils::IColumnData 
    {
      Reader<std::vector<std::vector<T>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnDataVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        requestedColumns.erase (iter);
        outputColumns.at(1).enabled = true;
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          columnData.insert (columnData.end(), data.begin(), data.end());
          offsets.push_back (columnData.size());
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorVectorLink final : public TestUtils::IColumnData 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> eventOffsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      explicit ColumnDataVectorVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

     virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
     {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);
        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        requestedColumns.erase (iter);
        outputColumns.at(1).enabled = true;
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
        eventOffsets.clear();
        eventOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (eventOffsets.size() - 1);
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            columnData.addLink (element, eventOffsets.size()-1);
          }
          offsets.push_back (columnData.size());
        }
        eventOffsets.push_back (offsets.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorVectorVector final : public TestUtils::IColumnData 
    {
      std::string columnName;
      Reader<std::vector<std::vector<std::vector<T>>>> branchReader;
      std::vector<ColumnarOffsetType> outerOffsets = {0};
      std::vector<ColumnarOffsetType> innerOffsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      explicit ColumnDataVectorVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".innerOffset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + ".outerOffset", .isOffset = true});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
          return true;
        outputColumns.at(1).enabled = true;

        if (iter->second.offsetName != outputColumns.at(2).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(2).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(2).name);
        if (iter == requestedColumns.end())
          return true;
        outputColumns.at(2).enabled = true;
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        innerOffsets.clear();
        innerOffsets.push_back (0);
        outerOffsets.clear();
        outerOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& outerData : branchData)
        {
          for (auto& innerData : outerData)
          {
            columnData.insert (columnData.end(), innerData.begin(), innerData.end());
            innerOffsets.push_back (columnData.size());
          }
          outerOffsets.push_back (innerOffsets.size()-1);
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, innerOffsets.size(), innerOffsets.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, outerOffsets.size(), outerOffsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorLink final : public TestUtils::IColumnData 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnDataVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }


      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);
        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }

        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (offsets.size() - 1);
        for (auto& element : branchData)
          columnData.addLink (element, offsets.size()-1);
        offsets.push_back (columnData.size());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorRLink final : public TestUtils::IColumnData 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnDataVectorRLink(const std::string& val_branchName)
      : branchReader(val_branchName), benchmarkUnpack(branchReader.columnName() + "(unpack)"), benchmark(branchReader.columnName()) 
      {
        outputColumns.push_back({.name = branchReader.columnName()});
        outputColumns.push_back({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
        auto iter = requestedColumns.find(outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);
        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }
        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        return true;
      }

      virtual void clearColumns() override 
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back(0);
      }

      virtual void getEntry(Long64_t entry) override 
      {
        benchmark.startTimer();
        const auto& branchData = branchReader.getEntry(entry);
        benchmark.stopTimer();
        benchmarkUnpack.startTimer();

       columnData.checkOffsets (offsets.size() - 1);
       for (const auto& element : branchData) 
       {
         if (element.isDefault() || element.index() == static_cast<unsigned int>(-1)) 
           columnData.addEmptyLink ();
          else
            columnData.addSplitLink (element.index(), element.key(), offsets.size()-1);
       }
      
    
      offsets.push_back(columnData.size());

      if (offsetColumn) {
        if (offsetColumn->size() != offsets.size()) 
        {
          throw std::runtime_error("offset column not filled yet: " + outputColumns.at(1).name);
        }
        if (offsetColumn->back() != offsets.back()) 
        {
          throw std::runtime_error("offset column does not match: " + outputColumns.at(1).name);
       }
      }

     benchmarkUnpack.stopTimer();
    }

    virtual void setData(ColumnVectorData& colData) override 
    {
      if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
        colData.setColumn(outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
      if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex) 
        colData.setColumn(outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
      for (std::size_t i = 2; i < outputColumns.size(); ++ i)
      {
        if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
      }
    }

    [[nodiscard]] virtual BranchPerfData getPerfData(float emptyTime) override 
    {
      BranchPerfData result;
      result.name = branchReader.columnName();
      result.timeRead = benchmark.getEntryTime(emptyTime);
      result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
      benchmark.setSilence();
      benchmarkUnpack.setSilence();
      result.entrySize = branchReader.entrySize();
      result.uncompressedSize = branchReader.uncompressedSize();
      result.numBaskets = branchReader.numBaskets();
      result.entries = entries;
      result.nullEntries = nullEntries;
      return result;
    }

   virtual void collectColumnData() override 
   {
      entries += columnData.size();
      for (const auto& index : columnData) 
      {
        if (index == invalidObjectIndex)
          nullEntries += 1;
        }
    }
  };


    template<typename T>
    struct ColumnDataVectorSplitLink final : public TestUtils::IColumnData
    {
      using CM = ColumnarModeArray;
      BranchReader<Int_t> branchReaderSize;
      BranchReaderArray<UInt_t> branchReaderKey;
      BranchReaderArray<UInt_t> branchReaderIndex;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      ColumnDataVectorSplitLink (const std::string& val_branchName)
        : branchReaderSize (val_branchName), branchReaderKey (val_branchName + ".m_persKey"), branchReaderIndex (val_branchName + ".m_persIndex"), benchmarkUnpack (branchReaderSize.columnName()+"(unpack)"), benchmark (branchReaderSize.columnName())
      {
        outputColumns.push_back ({.name = branchReaderSize.columnName()});
        outputColumns.push_back ({.name = branchReaderSize.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect (Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;
        auto* tree = std::get<TTree*>(source);
        branchReaderSize.connectTree (tree);
        branchReaderKey.connectTree (tree);
        branchReaderIndex.connectTree (tree);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }
        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (outputColumns.at(1).name); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          offsetColumns.emplace (outputColumns.at(1).name, &offsets);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        std::size_t branchDataSize = branchReaderSize.getEntry (entry);
        auto branchDataKey = branchReaderKey.getEntry (entry, branchDataSize);
        auto branchDataIndex = branchReaderIndex.getEntry (entry, branchDataSize);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (offsets.size() - 1);
        for (std::size_t index = 0; index < branchDataSize; ++index)
        {
          if (branchDataIndex[index] == static_cast<UInt_t>(-1))
            columnData.addEmptyLink ();
          else
            columnData.addSplitLink (branchDataIndex[index], branchDataKey[index], offsets.size()-1);
        }
        offsets.push_back (columnData.size());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReaderSize.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReaderSize.entrySize().value() + branchReaderKey.entrySize().value() + branchReaderIndex.entrySize().value();
        result.uncompressedSize = branchReaderSize.uncompressedSize().value() + branchReaderKey.uncompressedSize().value() + branchReaderIndex.uncompressedSize().value();
        result.numBaskets = branchReaderSize.numBaskets().value() + branchReaderKey.numBaskets().value() + branchReaderIndex.numBaskets().value();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
      }
    };

    template <typename T, template <typename> class Reader>
    struct ColumnDataVectorVectorVariantLink final : public TestUtils::IColumnData 
    {
      using CM = ColumnarModeArray;
      Reader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> eventOffsets = {0};
      LinkColumnVector columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;
      unsigned nullEntries = 0;

      explicit ColumnDataVectorVectorVariantLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

       branchReader.connectTree(source);

       if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        for (const auto & keyColumn : columnData.connect (iter->second, offsetColumns, requestedColumns))
        {
          outputColumns.push_back ({.name = keyColumn, .primary = false, .enabled = true});
          requestedColumns.erase (keyColumn);
        }

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        iter = requestedColumns.find (outputColumns.at(2).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(2).enabled = true;
          requestedColumns.erase (iter);
        }
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
        eventOffsets.clear();
        eventOffsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        columnData.checkOffsets (eventOffsets.size() - 1);
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            columnData.addLink (element, eventOffsets.size()-1);
          }
          offsets.push_back (columnData.size());
        }
        eventOffsets.push_back (offsets.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        for (std::size_t i = 2; i < outputColumns.size(); ++ i)
        {
          if (outputColumns.at(i).columnIndex != ColumnVectorHeader::nullIndex)
            colData.setColumn (outputColumns.at(i).columnIndex, columnData.keysColumn(i-2).size(), columnData.keysColumn(i-2).data());
        }
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        result.nullEntries = nullEntries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
        for (const auto& index : columnData)
        {
          if (index == invalidObjectIndex)
            nullEntries += 1;
        }
      }
    };

    template <template <typename> class Reader>
    struct ColumnDataMetNames final : public TestUtils::IColumnData 
    {
      Reader<std::vector<std::string>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<char> columnData;
      std::vector<std::size_t> columnHashData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnDataMetNames (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + "Hash"});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);

       if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
        {
          return true;
        }
        outputColumns.at(1).enabled = true;
        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(2).name);
        if (iter != requestedColumns.end())
        {
          outputColumns.at(2).enabled = true;
          requestedColumns.erase (iter);
        }
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
        columnHashData.clear();
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          columnData.insert (columnData.end(), data.begin(), data.end());
          offsets.push_back (columnData.size());
          columnHashData.push_back (std::hash<std::string> () (data));
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, columnHashData.size(), columnHashData.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName();
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        return result;
      }

      virtual void collectColumnData () override
      {}
    };

    template <template <typename> class Reader>
    struct ColumnDataOutputMet final : public TestUtils::IColumnData 
    {
     std::vector<std::string> termNames;
      const std::vector<ColumnarOffsetType>* offsetColumns = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<ColumnarOffsetType> namesOffsets = {0};
      std::vector<char> namesData;
      std::vector<std::size_t> namesHash;

      ColumnDataOutputMet (const std::string& val_columnName, std::vector<std::string> val_termNames)
        : termNames (std::move (val_termNames))
      {
        outputColumns.push_back ({.name = val_columnName, .isOffset = true});
        outputColumns.push_back ({.name = val_columnName + ".name.data"});
        outputColumns.push_back ({.name = val_columnName + ".name.offset", .isOffset = true});
        outputColumns.push_back ({.name = val_columnName + ".nameHash"});
      }

      virtual bool connect([[maybe_unused]]Backend source, std::unordered_map<std::string, const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       if (auto iter = requestedColumns.find (outputColumns.at(0).name);
            iter != requestedColumns.end())
          requestedColumns.erase (iter);
        else
          return false;
        outputColumns.at(0).enabled = true;

        if (auto iter = requestedColumns.find (outputColumns.at(1).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(1).enabled = true;
          requestedColumns.erase (iter);
        }

        if (auto iter = requestedColumns.find (outputColumns.at(2).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(2).enabled = true;
          requestedColumns.erase (iter);
        }

        if (auto iter = requestedColumns.find (outputColumns.at(3).name);
            iter != requestedColumns.end())
        {
          outputColumns.at(3).enabled = true;
          requestedColumns.erase (iter);
        }

        // For multi-tool support, skip if offset column already registered
        if (auto offsetIter = offsetColumns.find (outputColumns.at(0).name); offsetIter == offsetColumns.end())
          offsetColumns.emplace (outputColumns.at(0).name, &offsets);

        return true;
      }

      virtual void clearColumns () override
      {
        offsets.clear ();
        offsets.push_back (0);
        namesData.clear ();
        namesOffsets.clear ();
        namesOffsets.push_back (0);
        namesHash.clear ();
      }

      virtual void getEntry (Long64_t /*entry*/) override
      {
        for (const auto& termName : termNames)
        {
          namesData.insert (namesData.end(), termName.begin(), termName.end());
          namesOffsets.push_back (namesData.size());
          namesHash.push_back (std::hash<std::string> () (termName));
        }
        offsets.push_back (namesHash.size());
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, offsets.size(), offsets.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, namesData.size(), namesData.data());
        if (outputColumns.at(2).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(2).columnIndex, namesOffsets.size(), namesOffsets.data());
        if (outputColumns.at(3).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(3).columnIndex, namesHash.size(), namesHash.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float /*emptyTime*/) override
      {
        BranchPerfData result;
        result.name = outputColumns.at(0).name + "(met-out)";
        return result;
      }

      virtual void collectColumnData () override
      {}
    };

    template <template <typename> class Reader>
    struct ColumnDataSamplingPattern final : public TestUtils::IColumnData 
    {
      Reader<xAOD::CaloClusterContainer> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<std::uint32_t> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      unsigned entries = 0;

      ColumnDataSamplingPattern (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+".samplingPattern(fallback)(unpack)"), benchmark (branchReader.columnName() + ".samplingPattern(fallback)")
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".samplingPattern"});
        outputColumns.push_back ({.name = branchReader.columnName(), .isOffset = true, .primary = false});
      }

      virtual bool connect(Backend source, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string, ColumnInfo>& requestedColumns) override 
      {
       auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree(source);
       if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (outputColumns.at(1).name);
        if (iter == requestedColumns.end())
        {
          return true;
        }
        outputColumns.at(1).enabled = true;
        requestedColumns.erase (iter);
        return true;
      }

      virtual void clearColumns () override
      {
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto data : branchData)
        {
          columnData.push_back (data->samplingPattern());
        }
        offsets.push_back (columnData.size());
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnVectorData& colData) override
      {
        if (outputColumns.at(0).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(0).columnIndex, columnData.size(), columnData.data());
        if (outputColumns.at(1).columnIndex != ColumnVectorHeader::nullIndex)
          colData.setColumn (outputColumns.at(1).columnIndex, offsets.size(), offsets.data());
      }

      [[nodiscard]] virtual BranchPerfData getPerfData (float emptyTime) override
      {
        BranchPerfData result;
        result.name = branchReader.columnName() + "(fallback)";
        result.timeRead = benchmark.getEntryTime(emptyTime);
        result.timeUnpack = benchmarkUnpack.getEntryTime(emptyTime);
        benchmark.setSilence();
        benchmarkUnpack.setSilence();
        result.entrySize = branchReader.entrySize();
        result.uncompressedSize = branchReader.uncompressedSize();
        result.numBaskets = branchReader.numBaskets();
        result.entries = entries;
        return result;
      }

      virtual void collectColumnData () override
      {
        entries += columnData.size();
      }
    };

   namespace
    {
      /// @brief wrapper around a tool and its benchmarks for mode 2
      struct ToolData
      {
        std::string name;
        ColumnarTool<ColumnarModeArray>* tool = nullptr;
        std::unique_ptr<ToolColumnVectorMap> toolWrapper;
        bool noRepeatCall = false;
        bool runToolTwice = false;

        Benchmark benchmarkCall;
        Benchmark benchmarkCall2;

        ToolData (const UserConfiguration& config, const TestDefinition& td,
                  ColumnVectorHeader& columnHeader)
          : name (td.name)
          , noRepeatCall (td.noRepeatCall)
          , runToolTwice (config.runToolTwice)
          , benchmarkCall ("", config.batchSize)
          , benchmarkCall2 ("", config.batchSize)
        {
          tool = dynamic_cast<ColumnarTool<ColumnarModeArray>*>(td.tool);
          if (!tool)
            throw std::runtime_error ("tool is not a ColumnarTool<ColumnarModeArray>: " + td.name);
          if (!td.containerRenames.empty())
            renameContainers (*tool, td.containerRenames);
          toolWrapper = std::make_unique<ToolColumnVectorMap> (columnHeader, *tool);
        }

        /// @brief call the tool with timing, optionally twice
        void call (ColumnVectorData& columnData)
        {
          benchmarkCall.startTimer ();
          columnData.callNoCheck (*tool);
          benchmarkCall.stopTimer ();
          if (runToolTwice && !noRepeatCall)
          {
            benchmarkCall2.startTimer ();
            columnData.callNoCheck (*tool);
            benchmarkCall2.stopTimer ();
          }
        }
      };
    }
  }




  ColumnarPhysLiteTest ::
  ColumnarPhysLiteTest ()
  {
    static std::once_flag flag;
    std::call_once (flag, [] ()
    {
#ifdef XAOD_STANDALONE
      xAOD::Init().ignore();
#else
      POOL::Init();
#endif
    });

    auto userConfiguration = TestUtils::UserConfiguration::fromEnvironment();
    if (userConfiguration.isrntuple) 
    {
      auto* fileName = getenv("ASG_TEST_FILE_RNTUPLE_LITE_MC");
      if (fileName == nullptr)
        throw std::runtime_error("missing ASG_TEST_FILE_RNTUPLE_LITE_MC");
      rntreader = ROOT::RNTupleReader::Open("EventData", fileName);
      inspector = ROOT::Experimental::RNTupleInspector::Create("EventData", fileName);
      rntbackend = new TestUtils::RNTupleBackend{rntreader.get(), inspector.get()};
      if (!rntreader or !inspector)
        throw std::runtime_error("failed to open rntuple");
    } else 
    {
      auto* fileName = getenv("ASG_TEST_FILE_LITE_MC");
      if (fileName == nullptr)
        throw std::runtime_error("missing ASG_TEST_FILE_LITE_MC");
      file.reset(TFile::Open(fileName, "READ"));
      if (!file)
        throw std::runtime_error("failed to open file");
      tree = dynamic_cast<TTree*>(file->Get("CollectionTree"));
      if (!tree)
        throw std::runtime_error("failed to open rntuple");
    }
  }

  ColumnarPhysLiteTest ::~ColumnarPhysLiteTest() 
  {
    if (rntbackend)
      delete rntbackend;
  }

  std::string ColumnarPhysLiteTest :: makeUniqueName ()
  {
    static std::atomic<unsigned> index = 0;
    return "UniquePhysliteTestTool" + std::to_string(++index);
  }

  bool ColumnarPhysLiteTest ::
  checkMode ()
  {
    return true;
  }

  void ColumnarPhysLiteTest :: setupKnownColumns (std::span<const TestDefinition> testDefinitions)
  {
    using namespace TestUtils;

    knownColumns.push_back (std::make_shared<ColumnDataEventCount> ());

    if (tree) 
    {
      tree->SetMakeClass(1);
      {
        std::unordered_map<std::string, TBranch*> branches;
        {
          TIter branchIter(tree->GetListOfBranches());
          TObject* obj = nullptr;
          while ((obj = branchIter())) 
          {
            TBranch* branch = nullptr;
            if ((branch = dynamic_cast<TBranch*>(obj))) 
            {
              branches.emplace(branch->GetName(), branch);
              TIter subBranchIter(branch->GetListOfBranches());
              while ((obj = subBranchIter())) 
              {
                if (auto subBranch = dynamic_cast<TBranch*>(obj))
                  branches.emplace(subBranch->GetName(), subBranch);
              }
            }
          }
        }

        for (const auto& [name, branch] : branches) 
        {
          if (name.find("AuxDyn.") != std::string::npos ||
               name.find("Aux.") != std::string::npos) 
          {
            TClass* branchClass = nullptr;
            EDataType branchType{};
            branch->GetExpectedType(branchClass, branchType);
            if (branchClass == nullptr) 
            {
              switch (branchType) 
              {
                case kInt_t:
                  knownColumns.push_back(std::make_shared<ColumnDataScalar<std::int32_t, BranchReader>>(branch->GetName()));
                  break;
                case kUInt_t:
                  knownColumns.push_back(std::make_shared<ColumnDataScalar<std::uint32_t, BranchReader>>(branch->GetName()));
                  break;
                case kULong_t:
                  knownColumns.push_back(std::make_shared<ColumnDataScalar<std::uint64_t, BranchReader>>(branch->GetName()));
                  break;
                case kULong64_t:
                  knownColumns.push_back(std::make_shared<ColumnDataScalar<std::uint64_t, BranchReader>>(branch->GetName()));
                  break;
                case kFloat_t:
                  knownColumns.push_back(std::make_shared<ColumnDataScalar<float, BranchReader>>(branch->GetName()));
                  break;
                default:
                  // no-op
                  break;
              }
            } else 
            {
              if (*branchClass->GetTypeInfo() == typeid(std::vector<float>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<float,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<char>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<char,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int8_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::int8_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint8_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint8_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int16_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::int16_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint16_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint16_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int32_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::int32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint32_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int64_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::int64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint64_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<float>>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::int32_t>>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVectorVector<std::int32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::uint64_t>>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVectorVector<std::uint64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<std::size_t>>>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVectorVectorVector<std::size_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<unsigned char>>>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataVectorVectorVector<unsigned char,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::string>))
              {
                knownColumns.push_back (std::make_shared<ColumnDataMetNames<BranchReader>> (branch->GetName()));
              }
            }
          }
        }  
      }
      // This is a fallback for the case that we don't have an explicit
      // `samplingPattern` branch in our input file (i.e. an older file),
      // to allow us to still test tools needing it.  This is likely not
      // something that actual users can do (they need the new files), but
      // for testing it seems like a reasonable workaround.
      knownColumns.push_back(std::make_shared<ColumnDataSamplingPattern<BranchReader>>("egammaClusters"));

      // For branches that are element links they need to be explicitly
      // declared to have the correct xAOD type, correct split setting,
      // and correct linked containers.
      
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer,BranchReader>>("AnalysisElectronsAuxDyn.caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::TrackParticleContainer, BranchReader>>("AnalysisElectronsAuxDyn.trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer,BranchReader>>("AnalysisPhotonsAuxDyn.caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::VertexContainer, BranchReader>>("AnalysisPhotonsAuxDyn.vertexLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.inDetTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.combinedTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.extrapolatedMuonSpectrometerTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::TrackParticleContainer, BranchReader>>("GSFConversionVerticesAuxDyn.trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>>("GSFTrackParticlesAuxDyn.originalTrackParticle"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVariantLink<xAOD::IParticleContainer, BranchReader>>("AnalysisJetsAuxDyn.GhostTrack"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorLink<xAOD::JetContainer, BranchReader>>("METAssoc_AnalysisMETAux.jetLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVariantLink<xAOD::IParticleContainer, BranchReader>>("METAssoc_AnalysisMETAux.objectLinks"));
      
    }else if (rntbackend) 
    {
      std::unordered_map<std::string, ROOT::DescriptorId_t> fields;
      {
       const auto& desc = rntreader->GetDescriptor();

       for (const auto& field : desc.GetTopLevelFields()) 
       {
          fields.emplace(desc.GetQualifiedFieldName(field.GetId()), field.GetId());

         std::vector<ROOT::DescriptorId_t> subFieldIds{field.GetId()};
         while (!subFieldIds.empty()) 
         {
           const auto parentId = subFieldIds.back();
           auto parentname=desc.GetQualifiedFieldName(parentId);
           subFieldIds.pop_back();

           for (const auto& subField : desc.GetFieldIterable(parentId)) 
           {
             auto subFieldName = desc.GetQualifiedFieldName(subField.GetId());

             fields.emplace(desc.GetQualifiedFieldName(subField.GetId()), subField.GetId());

             subFieldIds.push_back(subField.GetId());
           }
          }
        }
      }

      const auto& desc = rntreader->GetDescriptor();
      for (const auto& [name, fieldId] : fields) 
      {
        auto fieldName = desc.GetQualifiedFieldName(fieldId);

        if (name.find("AuxDyn:") != std::string::npos ||
          name.find("Aux:") != std::string::npos) 
        {

          const auto& fieldDesc = desc.GetFieldDescriptor(fieldId);
          const std::string typeName = desc.GetTypeNameForComparison(fieldDesc);
          if (typeName == "std::int32_t" || typeName == "int") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataScalar<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::uint32_t" || typeName == "unsigned int") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataScalar<std::uint32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::uint64_t" || typeName == "unsigned long" || typeName == "unsigned long long") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataScalar<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName == "float") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataScalar<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<float>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<char>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<char, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int8_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<std::int8_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint8_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<std::uint8_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int16_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<std::int16_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint16_t>") 
          {
            knownColumns.push_back(std::make_shared<ColumnDataVector<std::uint16_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int32_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVector<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint32_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVector<std::uint32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::int64_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVector<std::int64_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::uint64_t>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVector<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<float>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVector<float, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<std::int32_t>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVector<std::int32_t, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::vector<std::uint64_t>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVector<std::uint64_t, RNTFieldReader>>(name));
          } else if (typeName =="std::vector<std::vector<std::vector<std::size_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVector<std::size_t, RNTFieldReader>>(name));
          }else if (typeName =="std::vector<std::vector<std::vector<std::uint64_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVector<std::uint64_t, RNTFieldReader>>(name));
          }else if (typeName =="std::vector<std::vector<std::vector<std::uint8_t>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVector<std::uint8_t, RNTFieldReader>>(name));
          } else if (typeName =="std::vector<std::vector<std::vector<unsigned char>>>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVector<unsigned char, RNTFieldReader>>(name));
          } else if (typeName == "std::vector<std::string>") 
          {
             knownColumns.push_back(std::make_shared<ColumnDataMetNames<RNTFieldReader>>(name));
          }
        }
      }
      knownColumns.push_back(std::make_shared<ColumnDataSamplingPattern<RNTFieldReader>>("egammaClusters"));

      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer,RNTFieldReader>>("AnalysisElectronsAuxDyn:caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::TrackParticleContainer, RNTFieldReader>>("AnalysisElectronsAuxDyn:trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer,RNTFieldReader>>("AnalysisPhotonsAuxDyn:caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::VertexContainer, RNTFieldReader>>("AnalysisPhotonsAuxDyn:vertexLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("AnalysisMuonsAuxDyn:inDetTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("AnalysisMuonsAuxDyn:combinedTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorRLink< xAOD::TrackParticleContainer, RNTFieldReader>>("AnalysisMuonsAuxDyn:extrapolatedMuonSpectrometerTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorLink<xAOD::TrackParticleContainer, RNTFieldReader>>("GSFConversionVerticesAuxDyn:trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorRLink<xAOD::TrackParticleContainer,RNTFieldReader>>("GSFTrackParticlesAuxDyn:originalTrackParticle"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVariantLink<xAOD::IParticleContainer, RNTFieldReader>>("AnalysisJetsAuxDyn:GhostTrack"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorLink<xAOD::JetContainer, RNTFieldReader>>("METAssoc_AnalysisMETAux:.jetLink"));
      knownColumns.push_back(std::make_shared<ColumnDataVectorVectorVariantLink<xAOD::IParticleContainer, RNTFieldReader>>("METAssoc_AnalysisMETAux:.objectLinks"));

    }


    // For METMaker we need to preplace all of the MET terms that we
    // expect to be used, that's what this line does.
    std::vector<std::string> allMetTermNames;
    for (const auto& td : testDefinitions)
    {
      for (const auto& name : td.metTermNames)
      {
        if (std::find (allMetTermNames.begin(), allMetTermNames.end(), name) == allMetTermNames.end())
          allMetTermNames.push_back (name);
      }
    }
   

    if (tree) 
    {
       if (!allMetTermNames.empty())
         knownColumns.push_back(std::make_shared<ColumnDataOutputMet<BranchReader>>("OutputMET",allMetTermNames));

      // For METMaker we need various extra columns to run. This may need
      // some work to avoid, but would likey be worth it.
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<std::uint16_t, BranchReader>>("AnalysisMuons.objectType", xAOD::Type::Muon));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, BranchReader>>("AnalysisMuons.m", ParticleConstants::muonMassInMeV));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<std::uint16_t, BranchReader>>("AnalysisJets.objectType", xAOD::Type::Jet));

      // These are columns that represent variables that are normally held
      // by METAssociationHelper, or alternatively are decorated on the
      // MET terms (even though they are per object).
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, BranchReader>>("AnalysisMuons.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, BranchReader>>("AnalysisJets.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, BranchReader>>("AnalysisJets.MetObjectWeightSoft", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<MissingETBase::Types::bitmask_t,BranchReader>>("METAssoc_AnalysisMET.useObjectFlags", 0));
    } else if (rntbackend) 
    {
      if (!allMetTermNames.empty())
         knownColumns.push_back(std::make_shared<ColumnDataOutputMet<BranchReader>>("OutputMET",allMetTermNames));

      // For METMaker we need various extra columns to run. This may need
      // some work to avoid, but would likey be worth it.
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<std::uint16_t, RNTFieldReader>>("AnalysisMuons.objectType", xAOD::Type::Muon));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, RNTFieldReader>>("AnalysisMuons.m", ParticleConstants::muonMassInMeV));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<std::uint16_t, RNTFieldReader>>("AnalysisJets.objectType", xAOD::Type::Jet));

      // These are columns that represent variables that are normally held
      // by METAssociationHelper, or alternatively are decorated on the
      // MET terms (even though they are per object).
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, RNTFieldReader>>("AnalysisMuons.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, RNTFieldReader>>("AnalysisJets.MetObjectWeight", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<float, RNTFieldReader>>("AnalysisJets.MetObjectWeightSoft", 0));
      knownColumns.push_back(std::make_shared<ColumnDataOutVector<MissingETBase::Types::bitmask_t,RNTFieldReader>>("METAssoc_AnalysisMET.useObjectFlags", 0));
    }
  }  // namespace columnar

  void ColumnarPhysLiteTest :: setupColumns (const ColumnVectorHeader& columnHeader)
  {
    using namespace asg::msgUserCode;

    // Get all column info directly from the header (all tools have already
    // registered their columns via ToolColumnVectorMap)
    auto requestedColumns = columnHeader.getAllColumnInfo();

    // Print requested columns
    for (auto& [name, info] : requestedColumns)
      std::cout << "requested columns: " << name << std::endl;

    for (auto& column : knownColumns)
    {
      if (tree) 
      {
        if (column->connect (tree, offsetColumns, requestedColumns))
          usedColumns.push_back (column);
      } else if (rntbackend) 
      {
        if (column->connect(rntbackend, offsetColumns, requestedColumns))
          usedColumns.push_back(column);
      }
    }

    std::set<std::string> unclaimedColumns;
    for (auto& column : requestedColumns)
    {
      if (!column.second.isOptional)
        unclaimedColumns.insert (column.first);
      else
        std::cout << "optional column not claimed: " << column.first << std::endl;
    }
    std::erase_if (unclaimedColumns, [&] (auto& columnName)
    {
      const auto& info = requestedColumns.at (columnName);
      if (info.accessMode != ColumnAccessMode::output || !info.fixedDimensions.empty())
        return false;
      auto offsetIter = std::find_if (usedColumns.begin(), usedColumns.end(), [&] (const std::shared_ptr<TestUtils::IColumnData>& column)
      {
        for (auto& output : column->outputColumns)
        {
          if (output.name == info.offsetName)
            return true;
        }
        return false;
      });
      if (offsetIter == usedColumns.end())
        return false;
      std::shared_ptr<TestUtils::IColumnData> myColumn;
      if (tree) 
      {
        if (*info.type == typeid(float))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<float, BranchReader>>(info.name, 0);
        else if (*info.type == typeid(char))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<char, BranchReader>>(info.name, 0);
        else if (*info.type == typeid(std::uint16_t))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<std::uint16_t, BranchReader>>(info.name, 0);
        else if (*info.type == typeid(std::uint64_t))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<std::uint64_t, BranchReader>>(info.name, 0);
        else 
        {
          ANA_MSG_WARNING("unhandled column type: " << info.name << " "<< info.type->name());
          return false;
        }
      } else if (rntbackend) 
      {
        if (*info.type == typeid(float))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<float, RNTFieldReader>>(info.name,0);
        else if (*info.type == typeid(char))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<char, RNTFieldReader>>(info.name, 0);
        else if (*info.type == typeid(std::uint16_t))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<std::uint16_t, RNTFieldReader>>(info.name, 0);
        else if (*info.type == typeid(std::uint64_t))
          myColumn = std::make_shared<TestUtils::ColumnDataOutVector<std::uint64_t, RNTFieldReader>>(info.name, 0);
        else 
        {
          ANA_MSG_WARNING("unhandled column type: " << info.name << " " << info.type->name());
          return false;
        }
      }
      knownColumns.push_back(myColumn);
      if (tree) {
        if (!myColumn->connect(tree, offsetColumns, requestedColumns)) 
        {
          ANA_MSG_WARNING("failed to connect dynamic output column: " << info.name);
          return false;
        }
      } else if (rntbackend) 
      {
        if (!myColumn->connect(rntbackend, offsetColumns, requestedColumns)) 
        {
          ANA_MSG_WARNING("failed to connect dynamic output column: " << info.name);
          return false;
        }
      }
      usedColumns.push_back(std::move(myColumn));
      return true;
    });
    if (!unclaimedColumns.empty()) 
    {
      std::string message = "columns not claimed:";
      for (auto& column : unclaimedColumns)
        message += " " + column;
      throw std::runtime_error(message);
    }
  }

  void ColumnarPhysLiteTest :: doCall (const TestDefinition& testDefinition)
  {
    doCallMulti ({testDefinition});
  }


void ColumnarPhysLiteTest ::doCallMulti(
    const std::vector<TestDefinition>& testDefinitions) {
  using namespace asg::msgUserCode;
  auto userConfiguration = TestUtils::UserConfiguration::fromEnvironment();

  // apply systematics for all test definitions
  for (const auto& td : testDefinitions) {
    if (!td.sysName.empty()) {
      auto* sysTool = dynamic_cast<CP::ISystematicsTool*>(td.tool);
      if (!sysTool)
        throw std::runtime_error("tool does not support systematics");
      std::cout << "applying systematic variation: " << td.sysName << std::endl;
      if (sysTool->applySystematicVariation(CP::SystematicSet(td.sysName))
              .isFailure())
        throw std::runtime_error("failed to apply systematic variation: " +
                                 td.sysName);
    }
  }

  if constexpr (columnarAccessMode == 2) {
    // Create shared column header for all tools
    ColumnVectorHeader columnHeader;

    // Build vector of ToolData from all testDefinitions
    std::vector<TestUtils::ToolData> toolDataVec;
    for (const auto& td : testDefinitions)
      toolDataVec.emplace_back(userConfiguration, td, columnHeader);

    setupKnownColumns(testDefinitions);
    // Set up columns using the shared header (all tools have already
    // registered their columns via ToolColumnVectorMap, so we get all columns
    // from the header)
    setupColumns(columnHeader);

    // connect column indices from header to each column for direct setting
    for (auto& column : usedColumns)
      column->connectColumnIndices(columnHeader);

    Benchmark benchmarkEmpty("empty");
    Benchmark benchmarkCheck("", userConfiguration.batchSize);
    auto numberOfEvents = 0;
    if (tree) {
      numberOfEvents = tree->GetEntries();
    } else if (rntbackend) {
      numberOfEvents = rntreader->GetNEntries();
    }
    Long64_t entry = 0;
    const auto startTime = std::chrono::high_resolution_clock::now();
    bool endLoop = false;
    for (; !endLoop; ++entry) {
      // just sample how much overhead there is for starting and
      // stopping the timer
      benchmarkEmpty.startTimer();
      benchmarkEmpty.stopTimer();
      ColumnVectorData columnData(&columnHeader);
      for (auto& column : usedColumns)
        column->getEntry(entry % numberOfEvents);
      if ((entry + 1) % userConfiguration.batchSize == 0) {
        if (entry < numberOfEvents) {
          for (auto& column : usedColumns)
            column->collectColumnData();
        }
        for (auto& column : usedColumns)
          column->setData(columnData);

        // Check data once (shared column data)
        benchmarkCheck.startTimer();
        columnData.checkData();
        benchmarkCheck.stopTimer();
        // Call each tool
        for (auto& toolData : toolDataVec) {
          toolData.call(columnData);
        }
        for (auto& column : usedColumns)
          column->clearColumns();
        if ((std::chrono::high_resolution_clock::now() - startTime) >
            userConfiguration.targetTime)
          endLoop = true;
      } else if (entry + 1 == numberOfEvents) {
        for (auto& column : usedColumns)
          column->collectColumnData();
      }
    }
      std::cout << "Entries in file: " << numberOfEvents << std::endl;
      std::cout << "Total entries read: " << entry << std::endl;
      const float emptyTime = benchmarkEmpty.getEntryTime(0).value();
      std::cout << "Empty benchmark time: " << emptyTime << "ns (tick=" << Benchmark::getTickDuration() << "ns)" << std::endl;
      benchmarkEmpty.setSilence();
      const auto checkTime = benchmarkCheck.getEntryTime(emptyTime);
      if (checkTime)
        std::cout << "Check data time: " << checkTime.value() << "ns" << std::endl;
      benchmarkCheck.setSilence();
      {
        std::vector<TestUtils::BranchPerfData> branchPerfData;
        TestUtils::BranchPerfData summary;
        summary.name = "total";
        summary.timeRead = 0;
        summary.timeUnpack = 0;
        summary.timeShallowCopy = 0;
        summary.entrySize = 0;
        summary.uncompressedSize = 0;
        summary.numBaskets = 0;
        summary.entries = std::nullopt;
        summary.nullEntries = std::nullopt;
        for (auto& column : usedColumns)
        {
          branchPerfData.push_back (column->getPerfData (emptyTime));
          summary.timeRead.value() += branchPerfData.back().timeRead.value_or(0);
          summary.timeUnpack.value() += branchPerfData.back().timeUnpack.value_or(0);
          summary.entrySize.value() += branchPerfData.back().entrySize.value_or(0);
          summary.uncompressedSize.value() += branchPerfData.back().uncompressedSize.value_or(0);
          summary.numBaskets.value() += branchPerfData.back().numBaskets.value_or(0);
          summary.timeShallowCopy.value() += branchPerfData.back().timeShallowCopy.value_or(0);
        }
        std::sort (branchPerfData.begin(), branchPerfData.end(), [] (const auto& a, const auto& b) {return a.name < b.name;});
        branchPerfData.insert (branchPerfData.end(), summary);
        const std::size_t nameWidth = std::max_element (branchPerfData.begin(), branchPerfData.end(), [] (const auto& a, const auto& b) {return a.name.size() < b.name.size();})->name.size();
        std::string label = userConfiguration.isrntuple ? "field name" : "branch name";
        std::string header = std::format ("{:{}} | read(ns) | unpack(ns) | size(B) | rate(MB/s) | compression | baskets | entries | null", label, nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string (header.size(), '-') << std::endl;
        for (auto& data : branchPerfData)
        {
          if (data.name == "total")
            std::cout << std::string (header.size(), '-') << std::endl;
          std::cout << std::format ("{:{}} |", data.name, nameWidth);
          if (data.timeRead)
            std::cout << std::format ("{:>9.0f} |", data.timeRead.value());
          else
            std::cout << "          |";
          if (data.timeUnpack)
            std::cout << std::format ("{:>11.1f} |", data.timeUnpack.value());
          else
            std::cout << "            |";
          if (data.entrySize)
            std::cout << std::format ("{:>8.1f} |", data.entrySize.value());
          else
            std::cout << "         |";
          if (data.timeRead && data.entrySize)
            std::cout << std::format ("{:>11.1f} |", (data.entrySize.value() / (data.timeRead.value() * 1e-3 * 1.024 * 1.024)));
          else
            std::cout << "            |";
          if (data.entrySize && data.uncompressedSize)
            std::cout << std::format ("{:>12.2f} |", float (data.uncompressedSize.value()) / data.entrySize.value());
          else
            std::cout << "             |";
          if (data.numBaskets)
            std::cout << std::format ("{:>8} |", data.numBaskets.value());
          else
            std::cout << "         |";
          if (data.entries)
            std::cout << std::format ("{:>8.2f} |", static_cast<float>(data.entries.value())/numberOfEvents);
          else
            std::cout << "         |";
          if (data.nullEntries && data.entries)
            std::cout << std::format ("{:>4.0f}%", static_cast<float>(data.nullEntries.value()) / data.entries.value() * 100.0f);
          std::cout << std::endl;
        }
      }
      {
        std::vector<TestUtils::ToolPerfData> toolPerfData;
        for (auto& toolData : toolDataVec)
        {
          toolPerfData.emplace_back ();
          toolPerfData.back().name = toolData.name;
          toolPerfData.back().timeCall = toolData.benchmarkCall.getEntryTime (emptyTime);
          if (userConfiguration.runToolTwice)
            toolPerfData.back().timeCall2 = toolData.benchmarkCall2.getEntryTime (emptyTime);
        }
        const std::size_t nameWidth = std::max_element (toolPerfData.begin(), toolPerfData.end(), [] (const auto& a, const auto& b) {return a.name.size() < b.name.size();})->name.size();
        std::string header = std::format ("{:{}} | call(ns) | call2(ns)", "tool name", nameWidth);
        std::cout << "\n" << header << std::endl;
        std::cout << std::string (header.size(), '-') << std::endl;
        for (auto& data : toolPerfData)
        {
          std::cout << std::format ("{:{}} |", data.name, nameWidth);
          if (data.timeCall)
            std::cout << std::format ("{:>9.0f} |", data.timeCall.value());
          else
            std::cout << "          |";
          if (data.timeCall2)
            std::cout << std::format ("{:>10.0f}", data.timeCall2.value());
          else
            std::cout << "          ";
          std::cout << std::endl;
        }
        // Add totals line for multiple tools
        if (toolPerfData.size() > 1)
        {
          std::optional<float> totalCall, totalCall2;
          for (const auto& data : toolPerfData)
          {
            if (data.timeCall)
              totalCall = totalCall.value_or (0) + data.timeCall.value();
            if (data.timeCall2)
              totalCall2 = totalCall2.value_or (0) + data.timeCall2.value();
          }
          std::cout << std::string (header.size(), '-') << std::endl;
          std::cout << std::format ("{:{}} |", "total", nameWidth);
          if (totalCall)
            std::cout << std::format ("{:>9.0f} |", totalCall.value());
          else
            std::cout << "          |";
          if (totalCall2)
            std::cout << std::format ("{:>10.0f}", totalCall2.value());
          else
            std::cout << "          ";
          std::cout << std::endl;
        }
      }
    } else if constexpr (columnarAccessMode == 0)
    {
      TestUtils::runXaodTest (userConfiguration, testDefinitions, file.get());
    } else if constexpr (columnarAccessMode == 100)
    {
      const auto& testDefinition = testDefinitions[0];
      TestUtils::runXaodArrayTest (userConfiguration, testDefinition, file.get());
    }
  }
}
