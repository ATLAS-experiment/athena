/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarTestFixtures/ColumnReaderBranch.h>

#include <xAODJet/JetContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

#include <TTree.h>

//
// method implementations
//

namespace columnar 
{
  namespace TestUtils 
  {
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

    template<typename T>
    struct ColumnReaderVectorSplitLink final : public TestUtils::ColumnReader
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

      ColumnReaderVectorSplitLink (const std::string& val_branchName)
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

    void makeTTreeColumnReaders (std::vector<std::shared_ptr<TestUtils::ColumnReader>>& knownColumns, TTree *tree)
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
                  knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::int32_t, BranchReader>>(branch->GetName()));
                  break;
                case kUInt_t:
                  knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::uint32_t, BranchReader>>(branch->GetName()));
                  break;
                case kULong_t:
                  knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::uint64_t, BranchReader>>(branch->GetName()));
                  break;
                case kULong64_t:
                  knownColumns.push_back(std::make_shared<ColumnReaderScalar<std::uint64_t, BranchReader>>(branch->GetName()));
                  break;
                case kFloat_t:
                  knownColumns.push_back(std::make_shared<ColumnReaderScalar<float, BranchReader>>(branch->GetName()));
                  break;
                default:
                  // no-op
                  break;
              }
            } else 
            {
              if (*branchClass->GetTypeInfo() == typeid(std::vector<float>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<float,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<char>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<char,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int8_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::int8_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint8_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::uint8_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int16_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::int16_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint16_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::uint16_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int32_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::int32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint32_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::uint32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int64_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::int64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint64_t>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVector<std::uint64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<float>>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVectorVector<float,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::int32_t>>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVectorVector<std::int32_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::uint64_t>>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVectorVector<std::uint64_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<std::size_t>>>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVectorVectorVector<std::size_t,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<unsigned char>>>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderVectorVectorVector<unsigned char,BranchReader>> (branch->GetName()));
              } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::string>))
              {
                knownColumns.push_back (std::make_shared<ColumnReaderMetNames<BranchReader>> (branch->GetName()));
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
      knownColumns.push_back(std::make_shared<ColumnReaderSamplingPattern<BranchReader>>("egammaClusters"));

      // For branches that are element links they need to be explicitly
      // declared to have the correct xAOD type, correct split setting,
      // and correct linked containers.
      
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::CaloClusterContainer,BranchReader>>("AnalysisElectronsAuxDyn.caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::TrackParticleContainer, BranchReader>>("AnalysisElectronsAuxDyn.trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::CaloClusterContainer,BranchReader>>("AnalysisPhotonsAuxDyn.caloClusterLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::VertexContainer, BranchReader>>("AnalysisPhotonsAuxDyn.vertexLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.inDetTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.combinedTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorSplitLink<xAOD::TrackParticleContainer>>("AnalysisMuonsAuxDyn.extrapolatedMuonSpectrometerTrackParticleLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorLink<xAOD::TrackParticleContainer, BranchReader>>("GSFConversionVerticesAuxDyn.trackParticleLinks"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorSplitLink<xAOD::TrackParticleContainer>>("GSFTrackParticlesAuxDyn.originalTrackParticle"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVariantLink<xAOD::IParticleContainer, BranchReader>>("AnalysisJetsAuxDyn.GhostTrack"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorLink<xAOD::JetContainer, BranchReader>>("METAssoc_AnalysisMETAux.jetLink"));
      knownColumns.push_back(std::make_shared<ColumnReaderVectorVectorVariantLink<xAOD::IParticleContainer, BranchReader>>("METAssoc_AnalysisMETAux.objectLinks"));
    }
  }
}