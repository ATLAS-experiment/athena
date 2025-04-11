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

#ifdef XAOD_STANDALONE
#include <ComponentFactoryPreloader/ComponentFactoryPreloader.h>
#else
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
    class IColumnData
    {
    public:

      virtual ~IColumnData () noexcept = default;

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const IColumnData*>& sizeColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) = 0;

      virtual std::size_t getSize () const = 0;

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t entry) = 0;
    };

    struct ColumnDataEventCount final : public PhysliteTestHelpers::IColumnData
    {
      std::array<ColumnarOffsetType, 2> data = {0, 1};
  
      virtual bool connect (TTree * /*tree*/, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& /*sizeColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        if (requestedColumns.contains (numberOfEventsName))
        {
          requestedColumns.erase (numberOfEventsName);
          return true;
        }
        return false;
      }
  
      virtual std::size_t getSize () const override
      {
        return 1;
      }
  
      virtual void setData (ColumnarToolWrapperData& tool, Long64_t /*entry*/) override
      {
        tool.setColumn (numberOfEventsName, data.size(), data.data());
      }
    };
  
    template<typename T>
    struct ColumnDataScalar final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      std::string branchName;
      T data {};
      TBranch *branch = nullptr;
      Benchmark benchmark;

      ColumnDataScalar (const std::string& val_columnName, const std::string& val_branchName)
        : columnName (val_columnName), branchName (val_branchName), benchmark (columnName) {}

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& /*sizeColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (columnName);
        if (iter == requestedColumns.end())
          return false;

        branch = tree->GetBranch (branchName.c_str());
        if (!branch)
          throw std::runtime_error ("failed to get branch: " + branchName);
        branch->SetAddress (&data);

        requestedColumns.erase (iter);
        return true;
      }

      virtual std::size_t getSize () const override
      {
        return 1;
      }

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t entry) override
      {
        benchmark.startTimer ();
        branch->GetEntry (entry);
        benchmark.stopTimer ();
        tool.setColumn (columnName, 1, &data);
      } 
    };

    template<typename T>
    struct ColumnDataVector final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      std::string branchName;
      std::string offsetName;
      const PhysliteTestHelpers::IColumnData *sizeColumn = nullptr;
      std::array<ColumnarOffsetType, 2> offsets = {0, 0};
      std::vector<T> *data = nullptr;
      TBranch *branch = nullptr;
      Benchmark benchmark;

      ColumnDataVector (const std::string& val_columnName, const std::string& val_branchName)
        : columnName (val_columnName), branchName (val_branchName), benchmark (columnName) {}

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& sizeColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (columnName);
        if (iter == requestedColumns.end())
          return false;

        branch = tree->GetBranch (branchName.c_str());
        if (!branch)
          throw std::runtime_error ("failed to get branch: " + branchName);
        branch->SetAddress (&data);

        offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + columnName);

        requestedColumns.erase (iter);

        if (auto sizeIter = sizeColumns.find (offsetName); sizeIter != sizeColumns.end())
          sizeColumn = sizeIter->second;
        else
          sizeColumns.emplace (offsetName, this);

        iter = requestedColumns.find (offsetName);
        if (iter == requestedColumns.end())
          offsetName.clear();
        else
          requestedColumns.erase (iter);

        return true;
      }

      virtual std::size_t getSize () const override
      {
        return data->size();
      }

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t entry) override
      {
        benchmark.startTimer ();
        branch->GetEntry (entry);
        benchmark.stopTimer ();
        tool.setColumn (columnName, data->size(), data->data());
        if (!offsetName.empty())
        {
          offsets[1] = data->size();
          tool.setColumn (offsetName, offsets.size(), offsets.data());
        }
        if (sizeColumn && sizeColumn->getSize() != data->size())
          throw std::runtime_error ("size mismatch: " + columnName);
      }
    };

    template<typename T>
    struct ColumnDataOutVector final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      T defaultValue;
      const PhysliteTestHelpers::IColumnData *sizeColumn = nullptr;
      std::vector<T> outData;

      ColumnDataOutVector (const std::string& val_columnName, const T& val_defaultValue)
        : columnName (val_columnName), defaultValue (val_defaultValue) {}

      virtual bool connect (TTree * /*tree*/, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& sizeColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (columnName);
        if (iter == requestedColumns.end())
          return false;

        const auto & offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + columnName);

        requestedColumns.erase (iter);

        if (auto sizeIter = sizeColumns.find (offsetName); sizeIter != sizeColumns.end())
          sizeColumn = sizeIter->second;
        else
          throw std::runtime_error ("missing size column for: " + columnName);
        return true;
      }

      virtual std::size_t getSize () const override
      {
        return outData.size();
      }

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t /*entry*/) override
      {
        outData.clear ();
        outData.resize (sizeColumn->getSize(), defaultValue);
        tool.setColumn (columnName, outData.size(), outData.data());
      }
    };

    /// an implementation of @ref PhysliteTestHelpers::IColumnData that reads a split ElementLink
    ///
    /// I'm not sure this is the best way of doing it.  Ideally I'd read
    /// the branches directly, but that was crashing, so I'm relying on
    /// TLeaf instead.  This is not ideal, but it works for now.
    struct ColumnDataSplitLink final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      std::string branchBaseName;
      std::string offsetName;
      const PhysliteTestHelpers::IColumnData *sizeColumn = nullptr;
      const PhysliteTestHelpers::IColumnData *targetSizeColumn = nullptr;
      std::array<ColumnarOffsetType, 2> offsets = {0, 0};
      std::vector<ColumnarOffsetType> dataCopy;
      TBranch *mainBranch = nullptr;
      TBranch *indexBranch = nullptr;
      TLeaf *indexLeaf = nullptr;
      Benchmark benchmarkUnpack;
      Benchmark benchmark;

      ColumnDataSplitLink (const std::string& val_columnName, const std::string& val_branchBaseName)
        : columnName (val_columnName), branchBaseName (val_branchBaseName), benchmarkUnpack (columnName + " (unpack)"), benchmark (columnName) {}

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& sizeColumns, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (columnName);
        if (iter == requestedColumns.end())
          return false;

        mainBranch = tree->GetBranch (branchBaseName.c_str());
        if (!mainBranch)
          throw std::runtime_error ("failed to get branch: " + branchBaseName);
        indexBranch = tree->GetBranch ((branchBaseName + ".m_persIndex").c_str());
        if (!indexBranch)
          throw std::runtime_error ("failed to get branch: " + (branchBaseName + ".m_persIndex"));
        indexLeaf = indexBranch->GetLeaf ((branchBaseName + ".m_persIndex").c_str());

        offsetName = iter->second.offsetName;
        if (offsetName.empty())
          throw std::runtime_error ("missing offset column for: " + columnName);

        if (!iter->second.linkToName.empty())
        {
          auto linkIter = sizeColumns.find (iter->second.linkToName);
          if (linkIter == sizeColumns.end())
            throw std::runtime_error ("missing link column for: " + columnName);
          targetSizeColumn = linkIter->second;
        }

        if (auto sizeIter = sizeColumns.find (offsetName); sizeIter != sizeColumns.end())
          sizeColumn = sizeIter->second;
        else
          sizeColumns.emplace (offsetName, this);

        requestedColumns.erase (iter);

        iter = requestedColumns.find (offsetName);
        if (iter == requestedColumns.end())
          offsetName.clear();
        else
          requestedColumns.erase (iter);

        return true;
      }

      virtual std::size_t getSize () const override
      {
        return offsets[1];
      }

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t entry) override
      {
        benchmark.startTimer ();
        mainBranch->GetEntry (entry);
        indexBranch->GetEntry (entry);
        benchmark.stopTimer ();

        benchmarkUnpack.startTimer ();
        offsets[1] = indexLeaf->GetLen();
        dataCopy.resize (offsets[1]);
        for (std::size_t i = 0; i < dataCopy.size(); ++i)
        {
          auto value = indexLeaf->GetTypedValue<UInt_t> (i);
          if (value != std::numeric_limits<UInt_t>::max())
            dataCopy[i] = value;
          else
            dataCopy[i] = invalidObjectIndex;
        }
        tool.setColumn (columnName, dataCopy.size(), dataCopy.data());
        if (!offsetName.empty())
        {
          offsets[1] = dataCopy.size();
          tool.setColumn (offsetName, offsets.size(), offsets.data());
        }
        if (sizeColumn && sizeColumn->getSize() != dataCopy.size())
          throw std::runtime_error ("size mismatch: " + columnName);
        if (targetSizeColumn)
        {
          const auto targetSize = targetSizeColumn->getSize();
          for (auto& item : dataCopy)
          {
            if (item != invalidObjectIndex && item >= targetSize)
              throw std::runtime_error ("link out of range: " + columnName + " " + std::to_string (item) + " " + std::to_string (targetSize) + " " + std::to_string (std::numeric_limits<UInt_t>::max()));
          }
        }
        benchmarkUnpack.stopTimer ();
      }
    };

    template<typename T>
    struct ColumnDataVectorVector final : public PhysliteTestHelpers::IColumnData
    {
      std::string columnName;
      std::string branchName;
      std::string offsetName;
      std::vector<ColumnarOffsetType> offsets;
      std::vector<T> columnData;
      std::vector<std::vector<T>> *branchData = nullptr;
      TBranch *branch = nullptr;
      Benchmark benchmark;

      ColumnDataVectorVector (const std::string& val_columnName, const std::string& val_branchName)
        : columnName (val_columnName), branchName (val_branchName), benchmark (columnName) {}

      virtual bool connect (TTree *tree, std::unordered_map<std::string,const PhysliteTestHelpers::IColumnData*>& /*sizeColumns*/, std::unordered_map<std::string,ColumnInfo>& requestedColumns) override
      {
        auto iter = requestedColumns.find (columnName);
        if (iter == requestedColumns.end())
          return false;

        branch = tree->GetBranch (branchName.c_str());
        if (!branch)
          throw std::runtime_error ("failed to get branch: " + branchName);
        branch->SetAddress (&branchData);

        offsetName = iter->second.offsetName;

        requestedColumns.erase (iter);

        iter = requestedColumns.find (offsetName);
        if (iter == requestedColumns.end())
        {
          offsetName.clear();
          return true;
        }
        requestedColumns.erase (iter);
        return true;
      }

      virtual std::size_t getSize () const override
      {
        return columnData.size();
      }

      virtual void setData (ColumnarToolWrapperData& tool, Long64_t entry) override
      {
        benchmark.startTimer ();
        branch->GetEntry (entry);
        benchmark.stopTimer ();
        columnData.clear();
        offsets.clear();
        offsets.push_back (0);
        for (auto& data : *branchData)
        {
          columnData.insert (columnData.end(), data.begin(), data.end());
          offsets.push_back (columnData.size());
        }
        tool.setColumn (columnName, columnData.size(), columnData.data());
        if (!offsetName.empty())
          tool.setColumn (offsetName, offsets.size(), offsets.data());
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

      // Preload the component factories:  Alternately this could be
      // loaded and executed via a dictionary, but I had some technical
      // issue with that, and this seems to be working.
      CP::preloadComponentFactories();
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

    setupKnownColumns ();
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
    knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint32_t>> ("EventInfo.RandomRunNumber", "EventInfoAuxDyn.RandomRunNumber"));
    knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint64_t>> ("EventInfo.eventNumber", "EventInfoAuxDyn.eventNumber"));
    knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint32_t>> ("EventInfo.eventTypeBitmask", "EventInfoAuxDyn.eventTypeBitmask"));
    knownColumns.push_back (std::make_shared<ColumnDataScalar<std::uint32_t>> ("EventInfo.runNumber", "EventInfoAuxDyn.runNumber"));
    // knownColumns.push_back (std::make_shared<ColumnDataScalar<float>> ("EventInfo.NPV", "EventInfoAuxDyn.NPV"));
    knownColumns.push_back (std::make_shared<ColumnDataScalar<float>> ("EventInfo.averageInteractionsPerCrossing", "EventInfoAuxDyn.averageInteractionsPerCrossing"));
    // knownColumns.push_back (std::make_shared<ColumnDataScalar<int>> ("EventInfo.Njet", "EventInfoAuxDyn.Njet"));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisElectrons.pt", "AnalysisElectronsAuxDyn.pt"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisElectrons.eta", "AnalysisElectronsAuxDyn.eta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisElectrons.phi", "AnalysisElectronsAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisElectrons.m", "AnalysisElectronsAuxDyn.m"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<uint16_t>> ("AnalysisElectrons.author", "AnalysisElectronsAuxDyn.author"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<ColumnarOffsetType>> ("AnalysisElectrons.caloClusterLinks.data", "AnalysisElectronsAuxDyn.caloClusterIndex"));

    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisElectrons.ptOut", 0));

    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisElectrons.sfOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<char>> ("AnalysisElectrons.validOut", 0));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("egammaClusters.calE", "egammaClustersAuxDyn.calE"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("egammaClusters.calEta", "egammaClustersAuxDyn.calEta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("egammaClusters.calPhi", "egammaClustersAuxDyn.calPhi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("egammaClusters.ETACALOFRAME", "egammaClustersAuxDyn.ETACALOFRAME"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("egammaClusters.PHICALOFRAME", "egammaClustersAuxDyn.PHICALOFRAME"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<uint32_t>> ("egammaClusters.samplingPattern", "egammaClustersAuxDyn.samplingPattern"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("egammaClusters.e_sampl.data", "egammaClustersAuxDyn.e_sampl"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("egammaClusters.eta_sampl.data", "egammaClustersAuxDyn.eta_sampl"));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisMuons.pt", "AnalysisMuonsAuxDyn.pt"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisMuons.eta", "AnalysisMuonsAuxDyn.eta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisMuons.phi", "AnalysisMuonsAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisMuons.charge", "AnalysisMuonsAuxDyn.charge"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint16_t>> ("AnalysisMuons.muonType", "AnalysisMuonsAuxDyn.muonType"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<std::uint16_t>> ("AnalysisMuons.author", "AnalysisMuonsAuxDyn.author"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<std::int32_t>> ("AnalysisMuons.resolutionCategory", "AnalysisMuonsAuxDyn.resolutionCategory"));

    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.ptOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.chargeOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.InnerDetectorCharge", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.InnerDetectorPt", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.MuonSpectrometerCharge", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.MuonSpectrometerPt", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisMuons.sfOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<char>> ("AnalysisMuons.validOut", 0));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisJets.pt", "AnalysisJetsAuxDyn.pt"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisJets.eta", "AnalysisJetsAuxDyn.eta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisJets.phi", "AnalysisJetsAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("AnalysisJets.m", "AnalysisJetsAuxDyn.m"));
    // knownColumns.push_back (std::make_shared<ColumnDataVector<char>> ("AnalysisJets.IsBjet", "AnalysisJetsAuxDyn.IsBjet"));
    // knownColumns.push_back (std::make_shared<ColumnDataVector<int>> ("AnalysisJets.R10TruthLabel_R21Consolidated", "AnalysisJetsAuxDyn.R10TruthLabel_R21Consolidated"));

    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisJets.ptOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<float>> ("AnalysisJets.mOut", 0));
    knownColumns.push_back (std::make_shared<ColumnDataOutVector<char>> ("AnalysisJets.selection", 0));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("InDetTrackParticles.d0", "InDetTrackParticlesAuxDyn.d0"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("InDetTrackParticles.phi", "InDetTrackParticlesAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("InDetTrackParticles.qOverP", "InDetTrackParticlesAuxDyn.qOverP"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("InDetTrackParticles.theta", "InDetTrackParticlesAuxDyn.theta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("InDetTrackParticles.z0", "InDetTrackParticlesAuxDyn.z0"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("InDetTrackParticles.definingParametersCovMatrixDiag.data", "InDetTrackParticlesAuxDyn.definingParametersCovMatrixDiag"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("InDetTrackParticles.definingParametersCovMatrixOffDiag.data", "InDetTrackParticlesAuxDyn.definingParametersCovMatrixOffDiag"));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("CombinedMuonTrackParticles.d0", "CombinedMuonTrackParticlesAuxDyn.d0"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("CombinedMuonTrackParticles.phi", "CombinedMuonTrackParticlesAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("CombinedMuonTrackParticles.qOverP", "CombinedMuonTrackParticlesAuxDyn.qOverP"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("CombinedMuonTrackParticles.theta", "CombinedMuonTrackParticlesAuxDyn.theta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("CombinedMuonTrackParticles.z0", "CombinedMuonTrackParticlesAuxDyn.z0"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("CombinedMuonTrackParticles.definingParametersCovMatrixDiag.data", "CombinedMuonTrackParticlesAuxDyn.definingParametersCovMatrixDiag"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("CombinedMuonTrackParticles.definingParametersCovMatrixOffDiag.data", "CombinedMuonTrackParticlesAuxDyn.definingParametersCovMatrixOffDiag"));

    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("ExtrapolatedMuonTrackParticles.d0", "ExtrapolatedMuonTrackParticlesAuxDyn.d0"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("ExtrapolatedMuonTrackParticles.phi", "ExtrapolatedMuonTrackParticlesAuxDyn.phi"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("ExtrapolatedMuonTrackParticles.qOverP", "ExtrapolatedMuonTrackParticlesAuxDyn.qOverP"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("ExtrapolatedMuonTrackParticles.theta", "ExtrapolatedMuonTrackParticlesAuxDyn.theta"));
    knownColumns.push_back (std::make_shared<ColumnDataVector<float>> ("ExtrapolatedMuonTrackParticles.z0", "ExtrapolatedMuonTrackParticlesAuxDyn.z0"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixDiag.data", "ExtrapolatedMuonTrackParticlesAuxDyn.definingParametersCovMatrixDiag"));
    knownColumns.push_back (std::make_shared<ColumnDataVectorVector<float>> ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixOffDiag.data", "ExtrapolatedMuonTrackParticlesAuxDyn.definingParametersCovMatrixOffDiag"));

    knownColumns.push_back (std::make_shared<ColumnDataSplitLink> ("AnalysisMuons.inDetTrackParticleLink", "AnalysisMuonsAuxDyn.inDetTrackParticleLink"));
    knownColumns.push_back (std::make_shared<ColumnDataSplitLink> ("AnalysisMuons.combinedTrackParticleLink", "AnalysisMuonsAuxDyn.combinedTrackParticleLink"));
    knownColumns.push_back (std::make_shared<ColumnDataSplitLink> ("AnalysisMuons.extrapolatedMuonSpectrometerTrackParticleLink", "AnalysisMuonsAuxDyn.extrapolatedMuonSpectrometerTrackParticleLink"));
  }

  void ColumnarPhysLiteTest :: setupColumns (ColumnarToolWrapper& toolWrapper)
  {
    std::unordered_map<std::string,ColumnInfo> requestedColumns;
    for (auto& column : toolWrapper.getColumnInfo())
      requestedColumns[column.name] = std::move (column);

    for (auto& name : toolWrapper.getColumnNames())
      std::cout << "requested columns: " << name << std::endl;

    for (auto& column : knownColumns)
    {
      if (column->connect (tree, sizeColumns, requestedColumns))
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
      setupColumns (toolWrapper);

      Benchmark benchmark (name);

      const PhysliteTestHelpers::IColumnData *sizeColumn = nullptr;
      if (!container.empty())
      {
        auto iter = sizeColumns.find (container);
        if (iter == sizeColumns.end())
          throw std::runtime_error ("missing size column: " + container);
        sizeColumn = iter->second;
      }

      const auto numberOfEvents = tree->GetEntries();
      std::uint64_t totalSize = 0;
      Long64_t entry = 0;
      for (; benchmark.getTotalTime() < targetTime; ++entry)
      {
        ColumnarToolWrapperData columnData (&toolWrapper);
        for (auto& column : usedColumns)
          column->setData (columnData, entry % numberOfEvents);
        if (sizeColumn)
        {
          totalSize += sizeColumn->getSize();
          if (entry + 1 == numberOfEvents)
            std::cout << "average size: " << float (totalSize) / numberOfEvents << std::endl;
        }
        benchmark.startTimer ();
        columnData.call ();
        benchmark.stopTimer ();
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
