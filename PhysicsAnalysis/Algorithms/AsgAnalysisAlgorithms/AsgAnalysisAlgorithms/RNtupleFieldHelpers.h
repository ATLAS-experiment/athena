/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ASGANALYSISALGORITHMS_RNTUPLEFIELDHELPERS_H
#define ASGANALYSISALGORITHMS_RNTUPLEFIELDHELPERS_H

// Framework include(s):
#include "AsgMessaging/AsgMessaging.h"
#include "AsgMessaging/AsgMessagingForward.h"
#include "AsgServices/ServiceHandle.h"
#include "AnaAlgorithm/AnaAlgorithm.h"
#include "PATInterfaces/SystematicSet.h"
#include "SystematicsHandles/SysListHandle.h"
#include <AsgAnalysisAlgorithms/TreeBranchHelpers.h>

// EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorBase.h"

// ROOT include(s):
#include <ROOT/RNTupleModel.hxx>

// System include(s):
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <typeinfo>
#include <set>

namespace SG {
   class IAuxTypeVectorFactory;
   class TypelessConstAccessor;
}

namespace CP {

   namespace RNtupleFieldHelpers {

#ifdef XAOD_STANDALONE
      using StoreType = asg::SgEvent;
#else
      using StoreType = StoreGateSvc;
#endif 
      typedef TreeBranchHelpers::BranchConfig BranchConfig;

      typedef TreeBranchHelpers::OutputBranchData OutputBranchData;

      struct FieldOps {
         std::function<void(size_t)> resize;
         std::function<void*()> getData;
      };

      // ----------------------------------------------------------------------
      // Field Processors
      // ----------------------------------------------------------------------
      class ElementFieldProcessor: public TreeBranchHelpers::IComponentProcessor  {
      public:
         ElementFieldProcessor() = default;
         virtual ~ElementFieldProcessor() = default;
         ElementFieldProcessor( const ElementFieldProcessor& ) = delete;
         ElementFieldProcessor& operator=( const ElementFieldProcessor& ) = delete;

         virtual StatusCode setup( ROOT::RNTupleModel& model,
                           const BranchConfig& branchConfig,
                           OutputBranchData& outputData,
                           MsgStream& msg ) override;

         StatusCode process( const SG::AuxElement& element, MsgStream& msg );
         virtual StatusCode setup ( TTree& /* tree */, 
                            const BranchConfig& /* branchConfig */, 
                            OutputBranchData& /* outputData */, 
                            MsgStream& msg ) override;

         std::string m_fieldName;
         std::unique_ptr< SG::TypelessConstAccessor > m_acc;
         const SG::IAuxTypeVectorFactory* m_factory = nullptr;
         std::shared_ptr<void> m_field;
         void* m_dataPtr = nullptr;
      };

      class ContainerFieldProcessor: public TreeBranchHelpers::IComponentProcessor {
      public:
         ContainerFieldProcessor() = default;
         virtual ~ContainerFieldProcessor() = default;
         ContainerFieldProcessor( const ContainerFieldProcessor& ) = delete;
         ContainerFieldProcessor& operator=( const ContainerFieldProcessor& ) = delete;

         virtual StatusCode setup( ROOT::RNTupleModel& model,
                           const BranchConfig& branchConfig,
                           OutputBranchData& outputData,
                           MsgStream& msg ) override;

         virtual StatusCode setup ( TTree& /* tree */, 
                            const BranchConfig& /* branchConfig */, 
                            OutputBranchData& /* outputData */, 
                            MsgStream& msg ) override;

         StatusCode resize( size_t size, MsgStream& msg );

         StatusCode process( const SG::AuxElement& element, size_t index, MsgStream& msg );
         
         void* getData() const { return m_ops.getData ? m_ops.getData() : nullptr; }


         std::string m_fieldName;
         std::unique_ptr< SG::TypelessConstAccessor > m_acc;
         const SG::IAuxTypeVectorFactory* m_factory = nullptr;
         std::shared_ptr<void> m_field;
         void* m_dataPtr = nullptr;
         // Field operation is needed here but not in ttree because ttree works by creating a branch and feed a pointer to the data to manage. 
         // You can pre-define how to resize aux vector in compile time for ttree, aka "IAuxTypeVector"
         // In RNTuple, "model" will give you a shared pointer of the primitive type you requested. This request is made in run-time.
         // That is why unlike ttree, we need an operator that takes lambda function to do the resize in the run time.
         FieldOps m_ops;
      };


      // ----------------------------------------------------------------------
      // Object Processors Interface
      // ----------------------------------------------------------------------

      class ElementProcessor : public asg::AsgMessaging, public TreeBranchHelpers::IObjectProcessor {
      public:
         ElementProcessor(const std::string& sgName);
         virtual ~ElementProcessor() = default;
         ElementProcessor( const ElementProcessor& ) = delete;
         ElementProcessor& operator=( const ElementProcessor& ) = delete;

         virtual StatusCode retrieveProcess( StoreType& evtStore ) override;

         virtual StatusCode addBranch( ROOT::RNTupleModel& model,
                               const BranchConfig& branchConfig,
                               OutputBranchData& outputData ) override;

         virtual StatusCode addBranch( TTree& tree,
                               const BranchConfig& branchConfig,
                               OutputBranchData& outputData ) override;

      protected:
         std::string m_sgName;
         std::vector< std::unique_ptr<ElementFieldProcessor> > m_fields;
      };


      class ATLAS_NOT_THREAD_SAFE ContainerProcessor : public asg::AsgMessaging, public TreeBranchHelpers::IObjectProcessor {
      public:
         ContainerProcessor(const std::string& sgName);
         virtual ~ContainerProcessor() = default;
         ContainerProcessor( const ContainerProcessor& ) = delete;
         ContainerProcessor& operator=( const ContainerProcessor& ) = delete;

         virtual StatusCode retrieveProcess( StoreType& evtStore ) override;

         virtual StatusCode addBranch( ROOT::RNTupleModel& model,
                               const BranchConfig& branchConfig,
                               OutputBranchData& outputData ) override;

         virtual StatusCode addBranch( TTree& tree,
                               const BranchConfig& branchConfig,
                               OutputBranchData& outputData ) override;

      private:
         std::string m_sgName;
         std::vector< std::unique_ptr<ContainerFieldProcessor> > m_fields;
         TVirtualCollectionProxy* m_collProxy = nullptr;
         int m_auxElementOffset = -1;
      };

      class ElementProcessorMet : public ElementProcessor {
      public:
         ElementProcessorMet(const std::string& sgName, const std::string& termName);
         virtual ~ElementProcessorMet() = default;
         ElementProcessorMet( const ElementProcessorMet& ) = delete;
         ElementProcessorMet& operator=( const ElementProcessorMet& ) = delete;
         virtual StatusCode retrieveProcess( StoreType& evtStore ) override;
      private:
         std::string m_termName;
      };

      class ProcessorList : public asg::AsgMessagingForward {
      public:
         using AsgMessagingForward::AsgMessagingForward;
         virtual ~ProcessorList() = default;
         ProcessorList( const ProcessorList& ) = delete;
         ProcessorList& operator=( const ProcessorList& ) = delete;

         StatusCode setupTree( const std::vector<std::string>& branches,
                               std::unordered_set<std::string> nonContainers,
                               ISystematicsSvc& sysSvc,
                               ROOT::RNTupleModel& model );

         StatusCode setupBranch( const BranchConfig& branchConfig,
                                 OutputBranchData& outputData,
                                 ROOT::RNTupleModel& model );

         StatusCode process( StoreType& evtStore );
         
         TreeBranchHelpers::IObjectProcessor& getObjectProcessor( const BranchConfig& branchConfig, const std::string& sgName );
         std::optional<int> defaultBasketSize;
         std::unordered_set<std::string> m_nonContainers;
         std::unordered_map< std::string, std::unique_ptr<TreeBranchHelpers::IObjectProcessor> > m_processors;
      };

   } // namespace RNtupleFieldHelpers
} // namespace CP

#endif // ASGANALYSISALGORITHMS_RNTUPLEFIELDHELPERS_H
