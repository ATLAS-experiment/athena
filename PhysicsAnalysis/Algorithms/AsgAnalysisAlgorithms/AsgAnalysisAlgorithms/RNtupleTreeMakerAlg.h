/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ASGANALYSISALGORITHMS_RNTUPLETREEMAKERALG_H
#define ASGANALYSISALGORITHMS_RNTUPLETREEMAKERALG_H

#include "AnaAlgorithm/AnaAlgorithm.h"
#include "AsgTools/PropertyWrapper.h"

// RNTuple include(s):
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <TFile.h>

// Local include(s):
#include "AsgAnalysisAlgorithms/RNtupleFieldHelpers.h"

#include <string>
#include <vector>
#include <memory>

namespace CP {

   /// Algorithm that creates an RNTuple (instead of TTree)
   class RNtupleTreeMakerAlg : public EL::AnaAlgorithm {

   public:
      /// Algorithm constructor
      using EL::AnaAlgorithm::AnaAlgorithm;

      /// Function executed during algorithm initialization
      virtual StatusCode initialize() override;

      /// Function executed once per event
      virtual StatusCode execute(const EventContext& ctx) override;

      /// Function executed during algo finalization
      virtual StatusCode finalize() override;

   private:
      /// Function setting up the internal data structures on the first event
      StatusCode setupTree();

      /// @name Algorithm properties
      /// @{

      Gaudi::Property<std::string> m_modelName {
         this, "TreeName", "physics", "Name of the RNTuple model/tree"};

      Gaudi::Property<std::string> m_outputStreamName {
         this, "OutputStreamName", "ANALYSIS", "Name of the output stream"};

      Gaudi::Property<std::vector<std::string>> m_branches {
         this, "Branches", {}, "List of branches to create (format: Object.Var -> BranchName)"};

      Gaudi::Property<std::vector<std::string>> m_nonContainers {
         this, "NonContainers", {}, "List of objects to treat as non-containers"};

      Gaudi::Property<std::size_t> m_approxZippedClusterSize {
         this, "ApproxZippedClusterSize", 0,
         "Target compressed cluster size in bytes (0 = ROOT default)"};

      Gaudi::Property<std::size_t> m_maxUnzippedClusterSize {
         this, "MaxUnzippedClusterSize", 0,
         "Maximum uncompressed cluster size in bytes, bounds the write buffer memory (0 = ROOT default)"};

      /// @}

      /// Service handle for systematics
      ServiceHandle<ISystematicsSvc> m_systematicsService {
         this, "SystematicsSvc", "SystematicsSvc", "Systematics service"};

      /// @name Variables used for RNTuple
      /// @{

      std::unique_ptr<ROOT::RNTupleModel> m_model;
      std::unique_ptr<ROOT::RNTupleWriter> m_writer;

      CP::RNtupleFieldHelpers::ProcessorList m_processorList{this};

      bool m_isInitialized = false;

      /// @}

   }; // class RNtupleTreeMakerAlg

} // namespace CP

#endif // ASGANALYSISALGORITHMS_RNTUPLETREEMAKERALG_H
