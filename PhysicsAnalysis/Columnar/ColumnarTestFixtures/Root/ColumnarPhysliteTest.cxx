/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>

#include <AsgTesting/UnitTest.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarInterfaces/ColumnInfo.h>
#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarToolWrapper/ColumnarToolHelpers.h>
#include <ColumnarToolWrapper/ColumnarToolWrapper.h>
#include <PATInterfaces/ISystematicsTool.h>
#include <TruthUtils/ParticleConstants.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/versions/MissingETAuxAssociationMap_v2.h>
#include <xAODMissingET/versions/MissingETBase.h>
#include <xAODCaloEvent/CaloClusterContainer.h>

#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

#ifndef XAOD_STANDALONE
#include <POOLRootAccess/TEvent.h>
#endif

#include <TFile.h>
#include <TLeaf.h>
#include <TTree.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

//
// method implementations
//

namespace columnar
{
  namespace
  {
    // the target time to run a given tool
    const auto targetTime = std::chrono::seconds(1);

    // the number of events per batch in columnar mode
    const unsigned int batchSize = 1000;


    class Benchmark final
    {
      std::string m_name;

      std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

      /// accumulated time m_ticks
      std::chrono::high_resolution_clock::duration m_ticks {};

      /// the number of times the timer has been started
      std::uint64_t m_count = 0;

    public:
      Benchmark (const std::string& val_name)
        : m_name (val_name)
      {}

      ~Benchmark ()
      {
        if (m_count > 0)
          std::cout << m_name << ": " << std::chrono::duration<std::uint64_t,std::nano> (m_ticks) / m_count << std::endl;
      }

      auto getTotalTime () const
      {
        return m_ticks;
      }

      void startTimer ()
      {
        m_start = std::chrono::high_resolution_clock::now();
      }

      void stopTimer ()
      {
        m_ticks += std::chrono::high_resolution_clock::now() - m_start;
        m_count += 1;
      }
    };
  }

  namespace PhysliteTestHelpers
  {
    // I never figured out how the keys get calculated, so I looked
    // at what's in the input file, and hard-coded it here.
    static const std::unordered_map<std::string,SG::sgkey_t> knownKeys =
    {
      {"AnalysisMuons", 0x3a6b126f},
      {"AnalysisElectrons", 0x3902fec0},
      {"AnalysisPhotons", 0x35d1472f},
      {"AnalysisJets", 0x1afd1919},
      {"egammaClusters", 0x15788d1f},
      {"GSFConversionVertices", 0x1f3e85c9},
      {"InDetTrackParticles", 0x1d3890db},
      {"CombinedMuonTrackParticles", 0x340d9196},
      {"ExtrapolatedMuonTrackParticles", 0x14e35e9f},
      {"GSFTrackParticles", 0x2e42db0b},
      {"InDetForwardTrackParticles", 0x143c6846},
      {"MuonSpectrometerTrackParticles", 0x3993c8f3},
    };

    template<typename T>
    class BranchReader final
    {
      std::string m_branchName;
      TBranch *m_branch = nullptr;
      bool m_isStatic = std::is_pod_v<T>;
      T *m_data {new T()};

    public:
      BranchReader (const std::string& val_branchName)
        : m_branchName (val_branchName)
      {
        if (m_branchName.find ("Aux.") != std::string::npos)
          m_isStatic = true;
      }

      ~BranchReader () noexcept
      {
        delete m_data;
      }

      BranchReader (const BranchReader&) = delete;
      BranchReader& operator= (const BranchReader&) = delete;

      void setIsStatic (bool isStatic)
      {
        m_isStatic = isStatic;
      }

      [[nodiscard]] const std::string& branchName () const
      {
        return m_branchName;
      }

      [[nodiscard]] std::string columnName () const
      {
        std::string columnName = m_branchName;
        if (auto index = columnName.find ("AuxDyn."); index != std::string::npos)
          columnName.replace (index, 6, "");
        else if (auto index = columnName.find ("Aux."); index != std::string::npos)
          columnName.replace (index, 3, "");
        else if (columnName.find (".") != std::string::npos)
          throw std::runtime_error ("branch name does not contain AuxDyn or Aux: " + m_branchName);
        return columnName;
      }

      [[nodiscard]] std::string containerName () const
      {
        if (auto index = m_branchName.find ("AuxDyn."); index != std::string::npos)
          return m_branchName.substr (0, index);
        else if (auto index = m_branchName.find ("Aux."); index != std::string::npos)
          return m_branchName.substr (0, index);
        else if (m_branchName.find (".") == std::string::npos)
          return m_branchName;
        else
          throw std::runtime_error ("branch name does not contain AuxDyn or Aux: " + m_branchName);
      }

      void connectTree (TTree *tree)
      {
        m_branch = tree->GetBranch (m_branchName.c_str());
        if (!m_branch)
          throw std::runtime_error ("failed to get branch: " + m_branchName);
        m_branch->SetMakeClass (1);
        if (m_isStatic)
          m_branch->SetAddress (m_data);
        else
          m_branch->SetAddress (&m_data);
      }

      const T& getEntry (Long64_t entry)
      {
        if (!m_branch)
          throw std::runtime_error ("branch not connected: " + m_branchName);
        if (m_branch->GetEntry (entry) <= 0)
          throw std::runtime_error ("failed to get entry " + std::to_string (entry) + " for branch: " + m_branchName);
        if (m_data == nullptr)
          throw std::runtime_error ("got nullptr reading data for branch: " + m_branchName);
        return *m_data;
      }

      const T& getCachedEntry () const
      {
        return *m_data;
      }
    };

    template<typename T>
    class BranchReaderArray final
    {
    public:
      std::string m_branchName;
      TBranch *m_branch = nullptr;
      std::vector<T> m_dataVec;

    public:
      BranchReaderArray (const std::string& val_branchName)
        : m_branchName (val_branchName)
      {}

      BranchReaderArray (const BranchReaderArray&) = delete;
      BranchReaderArray& operator= (const BranchReaderArray&) = delete;

      [[nodiscard]] std::string columnName () const
      {
        std::string columnName = m_branchName;
        if (auto index = columnName.find ("AuxDyn."); index != std::string::npos)
          columnName.replace (index, 6, "");
        else if (auto index = columnName.find ("Aux."); index != std::string::npos)
          columnName.replace (index, 3, "");
        else if (columnName.find (".") != std::string::npos)
          throw std::runtime_error ("branch name does not contain AuxDyn or Aux: " + m_branchName);
        return columnName;
      }

      [[nodiscard]] std::string containerName () const
      {
        if (auto index = m_branchName.find ("AuxDyn."); index != std::string::npos)
          return m_branchName.substr (0, index);
        else if (auto index = m_branchName.find ("Aux."); index != std::string::npos)
          return m_branchName.substr (0, index);
        else if (m_branchName.find (".") == std::string::npos)
          return m_branchName;
        else
          throw std::runtime_error ("branch name does not contain AuxDyn or Aux: " + m_branchName);
      }

      void connectTree (TTree *tree)
      {
        m_branch = tree->GetBranch (m_branchName.c_str());
        if (!m_branch)
          throw std::runtime_error ("failed to get branch: " + m_branchName);
        m_branch->SetMakeClass (1);
        if (!m_dataVec.empty())
          m_branch->SetAddress (m_dataVec.data());
      }

      std::span<const T> getEntry (Long64_t entry, std::size_t size)
      {
        if (!m_branch)
          throw std::runtime_error ("branch not connected: " + m_branchName);
        if (m_dataVec.size() < size)
        {
          m_dataVec.resize (size);
          m_branch->SetAddress (m_dataVec.data());
        }
        if (size > 0 && m_branch->GetEntry (entry) <= 0)
          throw std::runtime_error ("failed to get entry " + std::to_string (entry) + " for branch: " + m_branchName);
        return std::span<const T>(m_dataVec.data(), size);
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
      };
      std::vector<OutputColumnInfo> outputColumns;

      virtual ~IColumnData () noexcept = default;

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) = 0;

      virtual void clearColumns () = 0;

      virtual void getEntry (Long64_t entry) = 0;

      virtual void setData (ColumnarToolWrapperData& tool) = 0;
    };

    struct ColumnDataEventCount final : public PhysliteTestHelpers::IColumnData
    {
      std::array<ColumnarOffsetType, 2> data = {0, 0};

      ColumnDataEventCount ()
      {
        outputColumns.push_back ({.name = numberOfEventsName, .isOffset = true});
      }
  
      virtual bool connect (TTree * /*tree*/, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        if (requestedColumns.contains (outputColumns.at(0).name))
        {
          requestedColumns.erase (outputColumns.at(0).name);
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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, data.size(), data.data());
      }
    };
  
    template<typename T>
    struct ColumnDataScalar final : public PhysliteTestHelpers::IColumnData
    {
      BranchReader<T> branchReader;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;
      std::vector<T> outData;

      explicit ColumnDataScalar (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;
        requestedColumns.erase (iter);

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, outData.size(), outData.data());
      } 
    };

    template<typename T>
    struct ColumnDataVector final : public PhysliteTestHelpers::IColumnData
    {
      BranchReader<std::vector<T>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> outData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      explicit ColumnDataVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, outData.size(), outData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
        if (offsetColumn)
        {
          if (offsetColumn->size() != offsets.size())
            throw std::runtime_error ("offset column not filled yet: " + outputColumns.at(1).name);
          if (offsetColumn->back() != offsets.back())
            throw std::runtime_error ("offset column does not match: " + outputColumns.at(1).name);
        }
      }
    };

    template<typename T>
    struct ColumnDataOutVector final : public PhysliteTestHelpers::IColumnData
    {
      T defaultValue;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<T> outData;

      ColumnDataOutVector (const std::string& val_columnName, const T& val_defaultValue)
        : defaultValue (val_defaultValue)
      {
        outputColumns.push_back ({.name = val_columnName});
      }

      virtual bool connect (TTree * /*tree*/, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        // WARNING: absolutely do not switch the next line to a
        // reference, the pointed to element gets deleted below.
        const auto offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name);

        requestedColumns.erase (iter);

        if (auto offsetIter = offsetColumns.find (offsetName); offsetIter != offsetColumns.end())
          offsetColumn = offsetIter->second;
        else
          throw std::runtime_error ("missing offset column for: " + outputColumns.at(0).name);
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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, outData.size(), outData.data());
      }
    };

    template<typename T>
    struct ColumnDataVectorVector final : public PhysliteTestHelpers::IColumnData
    {
      BranchReader<std::vector<std::vector<T>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      explicit ColumnDataVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
      }
    };

    template<typename T>
    struct ColumnDataVectorVectorLink final : public PhysliteTestHelpers::IColumnData
    {
      using CM = ColumnarModeArray;
      BranchReader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<typename CM::LinkIndexType> columnData;
      const std::vector<ColumnarOffsetType>* targetOffsetColumn = nullptr;
      SG::sgkey_t targetKey = 0;
      std::string targetContainerName;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      explicit ColumnDataVectorVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        targetContainerName = iter->second.linkToName;
        if (auto keyIter = knownKeys.find (targetContainerName); keyIter != knownKeys.end())
          targetKey = keyIter->second;
        if (auto offsetIter = offsetColumns.find (iter->second.linkToName); offsetIter != offsetColumns.end())
          targetOffsetColumn = offsetIter->second;
        else
          throw std::runtime_error ("missing offset column: " + iter->second.linkToName);

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
        if (targetOffsetColumn->size() < 2)
          throw std::runtime_error ("target offset column not yet filled for: " + outputColumns.at(0).name);
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            if (element.isDefault() || (element.key() == 0 && element.index() == 0))
              columnData.push_back (invalidObjectIndex);
            else
            {
              columnData.push_back (element.index() + targetOffsetColumn->at (targetOffsetColumn->size()-2));
              if (element.key() != targetKey)
              {
                if (targetKey == 0)
                {
                  targetKey = element.key();
                  std::cout << "assume target key for " << targetContainerName << " is " << std::hex << targetKey << std::dec << std::endl;
                } else
                {
                  throw std::runtime_error(
                      std::format("target key mismatch: {:x} != {:x} for {} with element index {}",
                                  element.key(), targetKey, outputColumns.at(0).name, element.index())
                  );
                }
              }
            }
          }
          offsets.push_back (columnData.size());
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
      } 
    };

    template<typename T>
    struct ColumnDataVectorVectorVector final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      BranchReader<std::vector<std::vector<std::vector<T>>>> branchReader;
      std::vector<ColumnarOffsetType> outerOffsets = {0};
      std::vector<ColumnarOffsetType> innerOffsets = {0};
      std::vector<T> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      explicit ColumnDataVectorVectorVector (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".innerOffset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + ".outerOffset", .isOffset = true});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, innerOffsets.size(), innerOffsets.data());
        if (outputColumns.at(2).enabled)
          tool.setColumn (outputColumns.at(2).name, outerOffsets.size(), outerOffsets.data());
      } 
    };

    template<typename T>
    struct ColumnDataVectorLink final : public PhysliteTestHelpers::IColumnData
    {
      using CM = ColumnarModeArray;
      BranchReader<std::vector<ElementLink<T>>> branchReader;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<typename CM::LinkIndexType> columnData;
      const std::vector<ColumnarOffsetType>* targetOffsetColumn = nullptr;
      SG::sgkey_t targetKey = 0;
      std::string targetContainerName;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnDataVectorLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName()});
        outputColumns.push_back ({.name = branchReader.containerName(), .isOffset = true, .primary = false});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        targetContainerName = iter->second.linkToName;
        if (auto keyIter = knownKeys.find (targetContainerName); keyIter != knownKeys.end())
          targetKey = keyIter->second;
        if (auto targetOffsetIter = offsetColumns.find (iter->second.linkToName); targetOffsetIter != offsetColumns.end())
          targetOffsetColumn = targetOffsetIter->second;
        else
          throw std::runtime_error ("missing offset column: " + iter->second.linkToName);

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
        if (targetOffsetColumn->size() < 2)
          throw std::runtime_error ("target offset column not yet filled for: " + outputColumns.at(0).name);
        for (auto& element : branchData)
        {
          if (element.isDefault())
            columnData.push_back (invalidObjectIndex);
          else
          {
            columnData.push_back (element.index() + targetOffsetColumn->at (targetOffsetColumn->size()-2));
            if (element.key() != targetKey)
            {
              if (targetKey == 0)
              {
                targetKey = element.key();
                std::cout << "assume target key for " << targetContainerName << " is " << std::hex << targetKey << std::dec << std::endl;
              } else
              {
                throw std::runtime_error ("target key mismatch: " + std::to_string (element.key()) + " != " + std::to_string (targetKey) + " for " + outputColumns.at(0).name);
              }
            }
          }
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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
      }
    };

    template<typename T>
    struct ColumnDataVectorSplitLink final : public PhysliteTestHelpers::IColumnData
    {
      using CM = ColumnarModeArray;
      BranchReader<Int_t> branchReaderSize;
      BranchReaderArray<UInt_t> branchReaderKey;
      BranchReaderArray<UInt_t> branchReaderIndex;
      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<typename CM::LinkIndexType> columnData;
      std::vector<const std::vector<ColumnarOffsetType>*> targetOffsetColumns;
      std::vector<SG::sgkey_t> targetKeys;
      std::vector<typename CM::LinkKeyType> keyColumnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnDataVectorSplitLink (const std::string& val_branchName)
        : branchReaderSize (val_branchName), branchReaderKey (val_branchName + ".m_persKey"), branchReaderIndex (val_branchName + ".m_persIndex"), benchmarkUnpack (branchReaderSize.columnName()+"(unpack)"), benchmark (branchReaderSize.columnName())
      {
        outputColumns.push_back ({.name = branchReaderSize.columnName()});
        outputColumns.push_back ({.name = branchReaderSize.containerName(), .isOffset = true, .primary = false});
        outputColumns.push_back ({.name = branchReaderSize.columnName() + ".keys", .primary = false});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReaderSize.connectTree (tree);
        branchReaderKey.connectTree (tree);
        branchReaderIndex.connectTree (tree);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);

        auto linkContainers = iter->second.variantLinkContainers;
        if (linkContainers.empty())
        {
          if (iter->second.linkToName.empty())
            throw std::runtime_error ("missing link container for: " + outputColumns.at(0).name);
          linkContainers.push_back (iter->second.linkToName);
        } else
        {
          if (!iter->second.linkToName.empty())
            throw std::runtime_error ("link container and variant link containers both set for: " + outputColumns.at(0).name);
        }
        for (const auto& container : linkContainers)
        {
          if (auto keyIter = knownKeys.find (container); keyIter != knownKeys.end())
            targetKeys.push_back (keyIter->second);
          else
            throw std::runtime_error ("no key known for link container: " + container);
          if (auto targetOffsetIter = offsetColumns.find (container); targetOffsetIter != offsetColumns.end())
            targetOffsetColumns.push_back (targetOffsetIter->second);
          else
            throw std::runtime_error ("missing offset column: " + container);
          keyColumnData.push_back (keyColumnData.size());
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
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        std::size_t branchDataSize = branchReaderSize.getEntry (entry);
        auto branchDataKey = branchReaderKey.getEntry (entry, branchDataSize);
        auto branchDataIndex = branchReaderIndex.getEntry (entry, branchDataSize);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& targetOffsetColumn : targetOffsetColumns)
        {
          if (targetOffsetColumn->size() <= offsets.size())
            throw std::runtime_error ("target offset column not yet filled for: " + outputColumns.at(0).name);
        }
        for (std::size_t index = 0; index < branchDataSize; ++index)
        {
          if (branchDataIndex[index] == static_cast<UInt_t>(-1))
            columnData.push_back (invalidObjectIndex);
          else
          {
            CM::LinkIndexType keyIndex = CM::invalidLinkValue;
            if (auto keyIter = std::find(targetKeys.begin(), targetKeys.end(), branchDataKey[index]); keyIter != targetKeys.end())
            {
              keyIndex = std::distance(targetKeys.begin(), keyIter);
            } else if (targetKeys.empty())
            {
              targetKeys.push_back (branchDataKey[index]);
              keyIndex = 0;
              std::cout << "assume target key for " << outputColumns.at(0).name << " is " << std::hex << branchDataKey[index] << std::dec << std::endl;
            } else
            {
              std::ostringstream error;
              error << "target key mismatch: read " << std::hex << branchDataKey[index];
              error << ", expected one of";
              for (const auto& key : targetKeys)
                error << " " << key;
              error << " for " << outputColumns.at(0).name;
              throw std::runtime_error (std::move (error).str());
            }
            auto& targetOffsetColumn = *targetOffsetColumns.at(keyIndex);
            auto targetOffset = targetOffsetColumn.at (offsets.size()-1);
            CM::LinkIndexType linkIndex = branchDataIndex[index];
            linkIndex += targetOffset;
            if (linkIndex >= targetOffsetColumn.at(offsets.size()))
              throw std::runtime_error (std::format ("index out of range for link: {} >= {} (base index {})", outputColumns.at(0).name, linkIndex, targetOffsetColumn.at(offsets.size()), targetOffset));
            columnData.push_back (CM::mergeLinkKeyIndex (keyIndex, branchDataIndex[index] + targetOffset));
          }
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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
        if (outputColumns.at(2).enabled)
          tool.setColumn (outputColumns.at(2).name, keyColumnData.size(), keyColumnData.data());
      }
    };

    template<typename T>
    struct ColumnDataVectorVectorVariantLink final : public PhysliteTestHelpers::IColumnData
    {
      using CM = ColumnarModeArray;
      BranchReader<std::vector<std::vector<ElementLink<T>>>> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<typename CM::LinkIndexType> columnData;
      std::vector<typename CM::LinkKeyType> keysColumn;
      std::vector<std::string> containers;
      std::vector<SG::sgkey_t> containerKeys;
      std::vector<const std::vector<ColumnarOffsetType>*> containerOffsets;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      bool checkUnknownKeys = false;
      std::unordered_map<SG::sgkey_t,std::unordered_set<std::string>> unknownKeys;

      explicit ColumnDataVectorVectorVariantLink (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+"(unpack)"), benchmark (branchReader.columnName())
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".data"});
        outputColumns.push_back ({.name = branchReader.columnName() + ".offset", .isOffset = true});
        outputColumns.push_back ({.name = branchReader.columnName() + ".keys"});
      }

      ~ColumnDataVectorVectorVariantLink ()
      {
        // print unknown keys and containers they may be associated
        // with, based on whether they were always within the range of
        // elements allowed for the container.
        for (auto& [key, forbiddenContainer] : unknownKeys)
        {
          std::cout << "unknown key: " << std::hex << key << std::dec << ", allowed containers:";
          for (const auto& container : containers)
          {
            if (forbiddenContainer.find (container) == forbiddenContainer.end())
              std::cout << " " << container;
          }
          std::cout << std::endl;
        }
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

        if (iter->second.offsetName != outputColumns.at(1).name)
          throw std::runtime_error ("offset name mismatch: " + iter->second.offsetName + " != " + outputColumns.at(1).name);
        containers = iter->second.variantLinkContainers;
        if (containers.empty() || iter->second.variantLinkKeyColumn.empty())
          throw std::runtime_error ("no variant link containers for: " + outputColumns.at(0).name);
        if (iter->second.variantLinkKeyColumn != outputColumns.at(2).name)
          throw std::runtime_error ("variant link key column mismatch: " + iter->second.variantLinkKeyColumn + " != " + outputColumns.at(2).name);

        for ([[maybe_unused]] auto& container : containers)
        {
          keysColumn.push_back (keysColumn.size()+1);
          containerOffsets.push_back (offsetColumns.at (container));
          if (auto iter = knownKeys.find (container); iter != knownKeys.end())
          {
            containerKeys.push_back (iter->second);
          } else
          {
            checkUnknownKeys = true;
            containerKeys.push_back (0u);
          }
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
      }

      virtual void getEntry (Long64_t entry) override
      {
        benchmark.startTimer ();
        const auto& branchData = branchReader.getEntry (entry);
        benchmark.stopTimer ();
        benchmarkUnpack.startTimer ();
        for (auto& data : branchData)
        {
          for (auto& element : data)
          {
            if (element.isDefault())
              columnData.push_back (invalidObjectIndex);
            else
            {
              typename CM::LinkIndexType key = 0xff;
              typename CM::LinkIndexType index = 0;
              for (std::size_t i = 0; i < containers.size(); ++i)
              {
                if (element.key() == containerKeys[i])
                {
                  if (containerOffsets[i]->back() <= element.index())
                    throw std::runtime_error ("invalid index: " + std::to_string (element.index()) + " in container: " + containers[i] + " with size: " + std::to_string (containerOffsets[i]->back()));
                  key = keysColumn[i];
                  if (containerOffsets[i]->size() < 2)
                    throw std::runtime_error ("container offset not yet filled for: " + containers[i]);
                  index = containerOffsets[i]->at (containerOffsets[i]->size()-2) + element.index();
                  break;
                }
              }
              if (key == 0xff && checkUnknownKeys)
              {
                // this records which containers the unknown key is
                // compatible with, so that I may figure out which
                // container it is and hard-code it above.
                auto& forbiddenContainers = unknownKeys[element.key()];
                for (std::size_t i = 0; i < containers.size(); ++i)
                {
                  if (containerOffsets[i]->back() <= containerOffsets[i]->at (containerOffsets[i]->size()-2) + element.index())
                    forbiddenContainers.insert (containers[i]);
                }
              }
              columnData.push_back (CM::mergeLinkKeyIndex (key, index));
            }
          }
          offsets.push_back (columnData.size());
        }
        benchmarkUnpack.stopTimer ();
      }

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
        if (outputColumns.at(2).enabled)
          tool.setColumn (outputColumns.at(2).name, keysColumn.size(), keysColumn.data());
      } 
    };

    struct ColumnDataMetNames final : public PhysliteTestHelpers::IColumnData
    {
      BranchReader<std::vector<std::string>> branchReader;
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

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
        if (outputColumns.at(2).enabled)
          tool.setColumn (outputColumns.at(2).name, columnHashData.size(), columnHashData.data());
      } 
    };

    struct ColumnDataOutputMet final : public PhysliteTestHelpers::IColumnData
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

      virtual bool connect (TTree * /*tree*/, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& offsetColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
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

        if (auto offsetIter = offsetColumns.find (outputColumns.at(0).name); offsetIter != offsetColumns.end())
          throw std::runtime_error ("duplicate size column: " + outputColumns.at(0).name);
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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, offsets.size(), offsets.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, namesData.size(), namesData.data());
        if (outputColumns.at(2).enabled)
          tool.setColumn (outputColumns.at(2).name, namesOffsets.size(), namesOffsets.data());
        if (outputColumns.at(3).enabled)
          tool.setColumn (outputColumns.at(3).name, namesHash.size(), namesHash.data());
      }
    };

    struct ColumnDataSamplingPattern final : public PhysliteTestHelpers::IColumnData
    {
      BranchReader<xAOD::CaloClusterContainer> branchReader;
      std::vector<ColumnarOffsetType> offsets = {0};
      std::vector<std::uint32_t> columnData;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnDataSamplingPattern (const std::string& val_branchName)
        : branchReader (val_branchName), benchmarkUnpack (branchReader.columnName()+".samplingPattern(fallback)(unpack)"), benchmark (branchReader.columnName() + ".samplingPattern(fallback)")
      {
        outputColumns.push_back ({.name = branchReader.columnName() + ".samplingPattern"});
        outputColumns.push_back ({.name = branchReader.columnName(), .isOffset = true, .primary = false});
      }

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const std::vector<ColumnarOffsetType>*>& /*offsetColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (outputColumns.at(0).name);
        if (iter == requestedColumns.end())
          return false;
        outputColumns.at(0).enabled = true;

        branchReader.connectTree (tree);

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

      virtual void setData (ColumnarToolWrapperData& tool) override
      {
        if (outputColumns.at(0).enabled)
          tool.setColumn (outputColumns.at(0).name, columnData.size(), columnData.data());
        if (outputColumns.at(1).enabled)
          tool.setColumn (outputColumns.at(1).name, offsets.size(), offsets.data());
      } 
    };
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

    auto *fileName = getenv ("ASG_TEST_FILE_LITE_MC");
    if (fileName == nullptr)
      throw std::runtime_error ("missing ASG_TEST_FILE_LITE_MC");
    file.reset (TFile::Open (fileName, "READ"));
    if (!file)
      throw std::runtime_error ("failed to open file");
    tree = dynamic_cast<TTree*> (file->Get ("CollectionTree"));
    if (!tree)
      throw std::runtime_error ("failed to open tree");
  }

  ColumnarPhysLiteTest :: ~ColumnarPhysLiteTest () = default;

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

  void ColumnarPhysLiteTest :: setupKnownColumns ()
  {
    using namespace PhysliteTestHelpers;

    knownColumns.push_back (std::make_shared<ColumnDataEventCount> ());

    tree->SetMakeClass (1);
    {
      std::unordered_map<std::string,TBranch*> branches;
      {
        TIter branchIter (tree->GetListOfBranches());
        TObject *obj = nullptr;
        while ((obj = branchIter()))
        {
          TBranch *branch = nullptr;
          if ((branch = dynamic_cast<TBranch*>(obj)))
          {
            branches.emplace (branch->GetName(), branch);
            TIter subBranchIter (branch->GetListOfBranches());
            while ((obj = subBranchIter()))
            {
              if (auto subBranch = dynamic_cast<TBranch*>(obj))
                branches.emplace (subBranch->GetName(), subBranch);
            }
          }
        }
      }

      for (const auto& [name, branch] : branches)
      {
        if (name.find ("AuxDyn.") != std::string::npos ||
            name.find ("Aux.") != std::string::npos)
        {
          TClass *branchClass = nullptr;
          EDataType branchType {};
          branch->GetExpectedType (branchClass, branchType);
          if (branchClass == nullptr)
          {
            switch (branchType)
            {
              case kInt_t:
                knownColumns.push_back (std::make_shared<ColumnDataScalar<std::int32_t>> (branch->GetName()));
                break;
              case kUInt_t:
                knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint32_t>> (branch->GetName()));
                break;
              case kULong_t:
                knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint64_t>> (branch->GetName()));
                break;
              case kULong64_t:
                knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint64_t>> (branch->GetName()));
                break;
              case kFloat_t:
                knownColumns.push_back (std::make_shared<ColumnDataScalar<float>> (branch->GetName()));
                break;
              default:
                // no-op
                break;
            }
          } else
          {
            if (*branchClass->GetTypeInfo() == typeid(std::vector<float>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<float>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<char>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<char>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int8_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::int8_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint8_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint8_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int16_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::int16_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint16_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint16_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int32_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::int32_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint32_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint32_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::int64_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::int64_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::uint64_t>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint64_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<float>>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::int32_t>>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVectorVector<std::int32_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::uint64_t>>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVectorVector<std::uint64_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<std::size_t>>>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVectorVectorVector<std::size_t>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::vector<std::vector<unsigned char>>>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataVectorVectorVector<unsigned char>> (branch->GetName()));
            } else if (*branchClass->GetTypeInfo() == typeid(std::vector<std::string>))
            {
              knownColumns.push_back (std::make_shared<ColumnDataMetNames> (branch->GetName()));
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
    knownColumns.push_back (std::make_shared<ColumnDataSamplingPattern> ("egammaClusters"));

    // For branches that are element links they need to be explicitly
    // declared to have the correct xAOD type, correct split setting,
    // and correct linked containers.
    knownColumns.push_back (std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer>> ("AnalysisElectronsAuxDyn.caloClusterLinks"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVectorLink<xAOD::CaloClusterContainer>> ("AnalysisPhotonsAuxDyn.caloClusterLinks"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVectorLink<xAOD::VertexContainer>> ("AnalysisPhotonsAuxDyn.vertexLinks"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>> ("AnalysisMuonsAuxDyn.inDetTrackParticleLink"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>> ("AnalysisMuonsAuxDyn.combinedTrackParticleLink"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorSplitLink<xAOD::TrackParticleContainer>> ("AnalysisMuonsAuxDyn.extrapolatedMuonSpectrometerTrackParticleLink"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVectorLink<xAOD::TrackParticleContainer>> ("GSFConversionVerticesAuxDyn.trackParticleLinks"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorLink<xAOD::JetContainer>>("METAssoc_AnalysisMETAux.jetLink"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVectorVariantLink<xAOD::IParticleContainer>>("METAssoc_AnalysisMETAux.objectLinks"));

    // For METMaker we need to preplace all of the MET terms that we
    // expect to be used, that's what this lined does.
    knownColumns.push_back (std::make_shared<ColumnDataOutputMet> ("OutputMET", std::vector<std::string>{"Muons", "RefJet", "MuonEloss", "PVSoftTrk"}));

    // For METMaker we need various extra columns to run. This may need
    // some work to avoid, but would likey be worth it.
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<std::uint16_t>> ("AnalysisMuons.objectType", xAOD::Type::Muon));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.m", ParticleConstants::muonMassInMeV));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<std::uint16_t>> ("AnalysisJets.objectType", xAOD::Type::Jet));

    // These are columns that represent variables that are normally held
    // by METAssociationHelper, or alternatively are decorated on the
    // MET terms (even though they are per object).
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.MetObjectWeight", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisJets.MetObjectWeight", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisJets.MetObjectWeightSoft", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<MissingETBase::Types::bitmask_t>> ("METAssoc_AnalysisMET.useObjectFlags", 0));
  }

  void ColumnarPhysLiteTest :: setupColumns (ColumnarToolWrapper& toolWrapper)
  {
    using namespace asg::msgUserCode;

    std::unordered_map<std::string,ColumnInfo> requestedColumns;
    for (auto& column : toolWrapper.getColumnInfo())
      requestedColumns[column.name] = std::move (column);

    for (auto& name : toolWrapper.getColumnNames())
      std::cout << "requested columns: " << name << std::endl;

    for (auto& column : knownColumns)
    {
      if (column->connect (tree, offsetColumns, requestedColumns))
        usedColumns.push_back (column);
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
      auto offsetIter = std::find_if (usedColumns.begin(), usedColumns.end(), [&] (const std::shared_ptr<PhysliteTestHelpers::IColumnData>& column)
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
      std::shared_ptr<PhysliteTestHelpers::IColumnData> myColumn;
      if (*info.type == typeid(float))
        myColumn = std::make_shared<PhysliteTestHelpers::ColumnDataOutVector<float>> (info.name, 0);
      else if (*info.type == typeid(char))
        myColumn = std::make_shared<PhysliteTestHelpers::ColumnDataOutVector<char>> (info.name, 0);
      else if (*info.type == typeid(std::uint16_t))
        myColumn = std::make_shared<PhysliteTestHelpers::ColumnDataOutVector<std::uint16_t>> (info.name, 0);
      else if (*info.type == typeid(std::uint64_t))
        myColumn = std::make_shared<PhysliteTestHelpers::ColumnDataOutVector<std::uint64_t>> (info.name, 0);
      else
      {
        ANA_MSG_WARNING ("unhandled column type: " << info.name << " " << info.type->name());
        return false;
      }
      knownColumns.push_back (myColumn);
      if (!myColumn->connect (tree, offsetColumns, requestedColumns))
      {
        ANA_MSG_WARNING ("failed to connect dynamic output column: " << info.name);
        return false; 
      }
      usedColumns.push_back (myColumn);
      return true;
    });
    if (!unclaimedColumns.empty())
    {
      std::string message = "columns not claimed:";
      for (auto& column : unclaimedColumns)
        message += " " + column;
      throw std::runtime_error (message);
    }
  }

  void ColumnarPhysLiteTest :: doCall (asg::AsgTool& tool, const std::string& name, const std::string& container, std::function<void(XAODArgs&)> callXAOD, const std::vector<std::pair<std::string,std::string>>& containerRenames, const std::string& sysName)
  {
    using namespace asg::msgUserCode;

    if (!sysName.empty())
    {
      auto *sysTool = dynamic_cast<CP::ISystematicsTool*>(&tool);
      if (!sysTool)
        throw std::runtime_error ("tool does not support systematics");
      std::cout << "applying systematic variation: " << sysName << std::endl;
      if (sysTool->applySystematicVariation (CP::SystematicSet (sysName)).isFailure())
        throw std::runtime_error ("failed to apply systematic variation: " + sysName);
    }
    if constexpr (columnarAccessMode == 2)
    {
      auto *myTool = dynamic_cast<ColumnarTool<ColumnarModeArray>*>(&tool);
      if (!containerRenames.empty())
        renameContainers (*myTool, containerRenames);
      ColumnarToolWrapper toolWrapper (myTool);

      setupKnownColumns ();
      setupColumns (toolWrapper);

      Benchmark benchmark (name);

      const std::vector<ColumnarOffsetType>* offsetColumn = nullptr;
      if (!container.empty())
      {
        auto iter = offsetColumns.find (container);
        if (iter == offsetColumns.end())
          throw std::runtime_error ("missing size column: " + container);
        offsetColumn = iter->second;
      }

      const auto numberOfEvents = tree->GetEntries();
      std::uint64_t totalSize = 0;
      Long64_t entry = 0;
      for (; benchmark.getTotalTime() < targetTime; ++entry)
      {
        ColumnarToolWrapperData columnData (&toolWrapper);
        for (auto& column : usedColumns)
          column->getEntry (entry % numberOfEvents);
        if (offsetColumn)
        {
          if (entry + 1 == numberOfEvents)
            std::cout << "average size: " << float (totalSize + offsetColumn->back()) / numberOfEvents << std::endl;
        }
        if ((entry + 1) % batchSize == 0)
        {
          if (offsetColumn)
            totalSize += offsetColumn->back();
          for (auto& column : usedColumns)
            column->setData (columnData);
          benchmark.startTimer ();
          columnData.call ();
          benchmark.stopTimer ();
          for (auto& column : usedColumns)
            column->clearColumns ();
        }
      }
      std::cout << "Total entries read: " << entry << std::endl;
    } else if constexpr (columnarAccessMode == 0)
    {
      // this test simply doesn't work in Athena
#ifdef XAOD_STANDALONE
      xAOD::TEvent event;
      xAOD::TStore store;
#else
      POOL::TEvent event;
#endif
      ANA_CHECK_THROW (event.readFrom (file.get()));

#ifdef XAOD_STANDALONE
      Benchmark benchmarkEmptyClear (name + " empty clear");
      Benchmark benchmarkCallClear (name + " call clear");
      Benchmark benchmarkPrepClear (name + " prep clear");
#endif
      Benchmark benchmarkCall (name + " call");
      Benchmark benchmarkPrep (name + " prep");
      Benchmark benchmarkGetEntry (name + " getEntry");

      const auto numberOfEvents = event.getEntries();
#ifdef XAOD_STANDALONE
      std::cout << "known container keys:" << std::endl;
      for (auto& [container, key] : columnar::PhysliteTestHelpers::knownKeys)
      {
        std::cout << std::format ("  {} -> 0x{:x}, 0x{:x} -> {}", container, event.getHash (container), key, event.getName (key)) << std::endl;
      }
#endif
      if (numberOfEvents == 0){
        throw std::runtime_error ("ColumnarPhysLiteTest: numberOfEvents == 0");
      }
      Long64_t entry = 0;

      // Instead of running for a fixed number of events, we run for a
      // fixed amount of time.  That is because individual tools can
      // vary wildly in how long they take to run, and we mostly want to
      // make sure that we ran the tool enough to get a precise
      // performance estimate.  As a fail-safe it also bounds the time
      // spend in i/o, which can be significant in Athena, but at a much
      // higher level.
      for (; benchmarkCall.getTotalTime() < targetTime && benchmarkPrep.getTotalTime() + benchmarkGetEntry.getTotalTime() < 20 * targetTime; ++entry)
      {
        benchmarkGetEntry.startTimer ();
        event.getEntry (entry % numberOfEvents);
        benchmarkGetEntry.stopTimer ();
        XAODArgs args;
        args.inputContainer = container;
        args.outputContainer = container + "Copy1";
        args.isPrepCall = true;
        benchmarkPrep.startTimer ();
        callXAOD (args);
        benchmarkPrep.stopTimer ();
        args.outputContainer = container + "Copy2";
        args.isPrepCall = false;
#ifdef XAOD_STANDALONE
        benchmarkPrepClear.startTimer ();
        store.clear ();
        benchmarkPrepClear.stopTimer ();
#endif
        benchmarkCall.startTimer ();
        callXAOD (args);
        benchmarkCall.stopTimer ();
#ifdef XAOD_STANDALONE
        benchmarkCallClear.startTimer ();
        store.clear ();
        benchmarkCallClear.stopTimer ();
        benchmarkEmptyClear.startTimer ();
        store.clear ();
        benchmarkEmptyClear.stopTimer ();
#endif
      }
      std::cout << "Total entries read: " << entry << std::endl;
    }
  }
}
