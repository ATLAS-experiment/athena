// this file is -*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//JetConstituentModSequence.h

#ifndef JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H
#define JETRECTOOLS_JETCONSTITUENTMODSEQUENCE_H

//
// Michael Nelson, CERN & Univesity of Oxford
// February, 2016

#include <string>
#include "xAODBase/IParticleHelpers.h"
#include "xAODBase/IParticleContainer.h"


#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetExecuteTool.h"
#include "JetInterface/IJetConstituentModifier.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKeyArray.h"
#include "AsgDataHandles/WriteHandleKey.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/CurrentContext.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODPFlow/PFO.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODPFlow/PFOAuxContainer.h"
#include "xAODPFlow/TrackCaloCluster.h"
#include "xAODPFlow/TrackCaloClusterContainer.h"
#include "xAODPFlow/TrackCaloClusterAuxContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/FlowElementAuxContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "AsgDataHandles/WriteHandle.h"


#ifndef XAOD_ANALYSIS
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#endif

class JetConstituentModSequence: public asg::AsgTool, virtual public IJetExecuteTool {
  // Changed from IJetExecute
  ASG_TOOL_CLASS(JetConstituentModSequence, IJetExecuteTool)
  public:
  JetConstituentModSequence(const std::string &name); // MEN: constructor
  StatusCode initialize();
  int execute() const;

protected:
  Gaudi::Property<std::string> m_inputContainer {this, "InputContainer", "", "The input container for the sequence"};
  Gaudi::Property<std::string> m_outputContainer = {this, "OutputContainer", "", "The output container for the sequence"};
  Gaudi::Property<bool> m_byVertex = {this, "DoByVertex", false, "True if we should match to each primary vertex, not just PV0"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{this, "VertexContainerKey", "PrimaryVertices", "Reconstructed primary vertex container name"};

  // P-A : the actual type
  // Define as a basic integer type because Gaudi
  // doesn't support arbitrary property types
  unsigned short m_inputType; //


  ToolHandleArray<IJetConstituentModifier> m_modifiers{this , "Modifiers" , {} , "List of constit modifier tools."};

#ifndef XAOD_ANALYSIS
  ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool","","Monitoring tool"};
#endif

  bool m_saveAsShallow = true;

  // note: not all keys will be used for a particular instantiation
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inClusterKey{this, "InClusterKey", "", "ReadHandleKey for unmodified CaloClusters"};
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outClusterKey{this, "OutClusterKey", "", "WriteHandleKey for modified CaloClusters"};

  SG::ReadHandleKey<xAOD::TrackCaloClusterContainer> m_inTCCKey{this, "InTCCKey", "", "ReadHandleKey for unmodified TrackCaloClusters"};
  SG::WriteHandleKey<xAOD::TrackCaloClusterContainer> m_outTCCKey{this, "OutTCCKey", "", "WriteHandleKey for modified TrackCaloClusters"};

  SG::ReadHandleKey<xAOD::PFOContainer> m_inChargedPFOKey{this, "InChargedPFOKey", "", "ReadHandleKey for modified Charged PFlow Objects"};
  SG::WriteHandleKey<xAOD::PFOContainer> m_outChargedPFOKey{this, "OutChargedPFOKey", "", "WriteHandleKey for modified Charged PFlow Objects"};

  SG::ReadHandleKey<xAOD::PFOContainer> m_inNeutralPFOKey{this, "InNeutralPFOKey", "", "ReadHandleKey for modified Neutral PFlow Objects"};
  SG::WriteHandleKey<xAOD::PFOContainer> m_outNeutralPFOKey{this, "OutNeutralPFOKey", "", "WriteHandleKey for modified Neutral PFlow Objects"};

  SG::WriteHandleKey<xAOD::PFOContainer> m_outAllPFOKey{this, "OutAllPFOKey", "", "WriteHandleKey for all modified PFlow Objects"};

  SG::ReadHandleKey<xAOD::FlowElementContainer> m_inChargedFEKey{this, "InChargedFEKey", "", "ReadHandleKey for modified Charged FlowElements"};
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_outChargedFEKey{this, "OutChargedFEKey", "", "WriteHandleKey for modified Charged FlowElements"};

  SG::ReadHandleKey<xAOD::FlowElementContainer> m_inNeutralFEKey{this, "InNeutralFEKey", "", "ReadHandleKey for modified Neutral FlowElements"};
  SG::WriteHandleKey<xAOD::FlowElementContainer> m_outNeutralFEKey{this, "OutNeutralFEKey", "", "WriteHandleKey for modified Neutral FlowElements"};

  SG::WriteHandleKey<xAOD::FlowElementContainer> m_outAllFEKey{this, "OutAllFEKey", "", "WriteHandleKey for all modified FlowElements"};

  // this ReadDecorHandleKeyArray holds necessary ReadDecorHandleKeys to make sure this algorithm is scheduled after all the relevant decorations are applied
  // otherwise FEs can be simultaneously decorated in other algorithms and deep-copied here, causing std::bad_array_new_length exceptions
  SG::ReadDecorHandleKeyArray<xAOD::FlowElementContainer> m_inChargedFEDecorKeys{this, "InChargedFEDecorKeys", {}, "Dummy keys that force all neccessary charged FlowElement decorations to be applied before running this algorithm"};
  SG::ReadDecorHandleKeyArray<xAOD::FlowElementContainer> m_inNeutralFEDecorKeys{this, "InNeutralFEDecorKeys", {}, "Dummy keys that force all neccessary neutral FlowElement decorations to be applied before running this algorithm"};

  /// helper function to cast, shallow copy and record a container.

  template<class T>
  StatusCode
  copyModRecord(const SG::ReadHandleKey<T>&,
                const SG::WriteHandleKey<T>&) const;

  template<class T, class U>
  StatusCode
  copyModRecordFlowLike(const SG::ReadHandleKey<T>&, const SG::ReadHandleKey<T>&, const SG::WriteHandleKey<T>&, const SG::WriteHandleKey<T>&, const SG::WriteHandleKey<T>&) const;
};

template<class T>
StatusCode
JetConstituentModSequence::copyModRecord(const SG::ReadHandleKey<T>& inKey,
                                         const SG::WriteHandleKey<T>& outKey) const {

  const EventContext& ctx = Gaudi::Hive::currentContext();

  /* Read in a container of (type is template parameter),
     optionally modify the elements of this container, and store.
     This puts a (modified) copy of the container  into storegate.
  */

  auto inHandle = makeHandle(inKey, ctx);
  if(!inHandle.isValid()){
    ATH_MSG_WARNING("Unable to retrieve input container from " << inKey.key());
    return StatusCode::FAILURE;
  }

  xAOD::ShallowCopyResult_t<T> newconstit = xAOD::shallowCopy(*inHandle);
  newconstit.second->setShallowIO(m_saveAsShallow);

  for (auto t : m_modifiers) {ATH_CHECK(t->process(newconstit.first.get()));}

  auto handle = makeHandle(outKey, ctx);
  ATH_CHECK(handle.record(std::move(newconstit.first), std::move(newconstit.second)));

  xAOD::setOriginalObjectLink(*inHandle, *handle);

  return StatusCode::SUCCESS;
}

template<class T, class U> StatusCode JetConstituentModSequence::copyModRecordFlowLike(const SG::ReadHandleKey<T>& inNeutralKey, const SG::ReadHandleKey<T>& inChargedKey, const SG::WriteHandleKey<T>& outNeutralKey, const SG::WriteHandleKey<T>& outChargedKey, const SG::WriteHandleKey<T>& outAllKey) const {

  const EventContext& ctx = Gaudi::Hive::currentContext();

  // Cannot be handled the same way as other objects (e.g. clusters),
  // because the data is split between two containers, but we need
  // information from both to do the modifications.
  //
  // The logic is:
  //   1. Copy the charged container via a shallow copy
  //   2. Create N copies of neutral container for by-vertex reconstruction OR
  //      Create a single copy of neutral container for regular jet reconstruction
  //   3. Merge into a combined view container
  //   4. Modify the combined container

  // 1. Retrieve the input containers
  SG::ReadHandle<T> inNeutralHandle = makeHandle(inNeutralKey, ctx);
  SG::ReadHandle<T> inChargedHandle = makeHandle(inChargedKey, ctx);
  if(!inNeutralHandle.isValid()){
    ATH_MSG_WARNING("Unable to retrieve input containers \""
                    << inNeutralKey.key() << "\" and \""
                    << inChargedKey.key() << "\"");
    return StatusCode::FAILURE;
  }

  unsigned numNeutralCopies = 1;
  if (m_byVertex){
    // Retrieve Primary Vertices
    auto handle = SG::makeHandle(m_vertexContainerKey);
    if (!handle.isValid()){
        ATH_MSG_WARNING(" This event has no primary vertex container" );
        return StatusCode::FAILURE;
    }

    const xAOD::VertexContainer* vertices = handle.cptr();
    if(vertices->empty()){
        ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container" );
        return StatusCode::FAILURE;
    }
    numNeutralCopies = static_cast<unsigned>(vertices->size());
  }

  // Copy the input containers individually, set I/O option and record
  // Charged elements
  SG::WriteHandle<T> outChargedHandle = makeHandle(outChargedKey, ctx);

  xAOD::ShallowCopyResult_t<T> chargedCopy =
    xAOD::shallowCopy(*inChargedHandle, ctx);
  chargedCopy.second->setShallowIO(m_saveAsShallow);
  xAOD::setOriginalObjectLink(*inChargedHandle, *chargedCopy.first);

  ATH_CHECK(outChargedHandle.record(std::move(chargedCopy.first),
                                    std::move(chargedCopy.second)));

  // Neutral elements
  SG::WriteHandle<T> outNeutralHandle = makeHandle(outNeutralKey, ctx);

  // Shallow copy
  if (m_saveAsShallow){

    xAOD::ShallowCopyResult_t<T> neutralCopy =
      xAOD::shallowCopy(*inNeutralHandle, ctx);
    neutralCopy.second->setShallowIO(true);
    xAOD::setOriginalObjectLink(*inNeutralHandle, *neutralCopy.first);

    ATH_CHECK(outNeutralHandle.record(std::move(neutralCopy.first),
                                      std::move(neutralCopy.second)));

  }
  // Deep copy
  else{
    // Define the accessor which will add the index
    const SG::AuxElement::Accessor<unsigned> copyIndex("ConstituentCopyIndex");

    // Create the new container and its auxiliary store.
    auto neutralCopies = std::make_unique<T>();
    auto neutralCopiesAux = std::make_unique<xAOD::AuxContainerBase>();
    neutralCopies->setStore(neutralCopiesAux.get()); //< Connect the two

    // Create N copies and set copy index
    // Necessary for by-vertex jet reconstruction
    for (unsigned i = 0; i < numNeutralCopies; i++){
      for (const U* fe : *inNeutralHandle) {
          U* copy = new U();
          neutralCopies->push_back(copy);
          *copy = *fe;
          copyIndex(*copy) = i;
          xAOD::setOriginalObjectLink(*fe, *copy);
      }
    }

    ATH_CHECK(outNeutralHandle.record(std::move(neutralCopies),
                                      std::move(neutralCopiesAux))
    );
  }


  // 2. Set up output handle for merged (view) container and record
  SG::WriteHandle<T> outAllHandle = makeHandle(outAllKey, ctx);
  ATH_CHECK(outAllHandle.record(std::make_unique<T>(SG::VIEW_ELEMENTS)));
  (*outAllHandle).assign((*outNeutralHandle).begin(), (*outNeutralHandle).end());
  (*outAllHandle).insert((*outAllHandle).end(),
      (*outChargedHandle).begin(),
      (*outChargedHandle).end());

  // 3. Now process modifications on all elements
  for (auto t : m_modifiers) {ATH_CHECK(t->process(&*outAllHandle));}
  return StatusCode::SUCCESS;
}


#endif
