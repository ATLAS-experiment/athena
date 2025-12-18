// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef ASGANALYSISALGORITHMS_ASGXAODNTUPLEMAKERALG_H
#define ASGANALYSISALGORITHMS_ASGXAODNTUPLEMAKERALG_H

// System include(s):
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <list>

// Framework include(s):
#include "AsgMessaging/AsgMessaging.h"
#include "AsgServices/ServiceHandle.h"
#include "AnaAlgorithm/AnaAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "SystematicsHandles/SysListHandle.h"
#include <AsgTools/PropertyWrapper.h>

// EDM include(s):
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainers/AuxElement.h"

// local include(s):
#include "AsgAnalysisAlgorithms/TreeBranchHelpers.h"

// Forward declaration(s):
class TClass;
class TTree;
class TVirtualCollectionProxy;
namespace SG {
   class AuxVectorBase;
   class IAuxTypeVectorFactory;
}

namespace CP {

   /// Algorithm that can write a simple ntuple from xAOD objects/variables
   ///
   /// This is meant as a simple tool for creating small ntuples in analyses,
   /// using a simple job configuration. It can create branches from any xAOD
   /// variables that are possible to write as an "xAOD variable" to begin with.
   ///
   /// It is *not* meant as a general purpose DAOD -> NTuple dumper however.
   /// It should only be used to create small ntuples, with highly processed
   /// variables.
   ///
   /// It's "main" property ("Branches") can be filled with entries of the form:
   ///
   /// <code>
   ///    writer = ...<br/>
   ///    writer.Branches = [ "<SG key>.<aux variable> -> <branch name>", ... ]
   /// </code>
   ///
   /// , where each entry sets up one branch for the output tree.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class ATLAS_NOT_THREAD_SAFE AsgxAODNTupleMakerAlg : public EL::AnaAlgorithm {

   public:
      /// Algorithm constructor
     using EL::AnaAlgorithm::AnaAlgorithm;

      /// @name Functions inherited from @c EL::AnaAlgorithm
      /// @{

      /// Function executed as part of the job initialisation
      StatusCode initialize() override;

      /// Function executed once per event
      StatusCode execute() override;

      /// Function executed as part of the job finalisation
      StatusCode finalize() override;

      /// @}

   private:

      /// @name Algorithm properties
      /// @{

      /// The name of the output tree to write
      Gaudi::Property<std::string> m_treeName {this, "TreeName", "physics", "Name of the tree to write"};
      /// The branches to write into this output tree
      Gaudi::Property<std::vector<std::string>> m_branches {this, "Branches", {}, "Branches to write to the output tree"};
      Gaudi::Property<std::vector<std::string>> m_nonContainers {this, "NonContainers", {"EventInfo"}, "List of objects that are single elements, not containers"};

      /// @}

      /// @name Variables used for the TTree filling
      /// @{

      TreeBranchHelpers::ProcessorList m_processorList {this};

      /// The tree being written
      TTree* m_tree = nullptr;

      /// \brief the handle for the systematics service
      ServiceHandle<ISystematicsSvc> m_systematicsService {this, "systematicsService", "SystematicsSvc", "systematics service"};

      /// @}

   }; // class AsgxAODNTupleMakerAlg

} // namespace CP

#endif // ASGANALYSISALGORITHMS_ASGXAODNTUPLEMAKERALG_H
