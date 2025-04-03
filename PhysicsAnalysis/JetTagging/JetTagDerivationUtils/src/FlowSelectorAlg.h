#ifndef FLOW_SELECTOR_ALG_HH
#define FLOW_SELECTOR_ALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/AuxElement.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODBase/IParticleContainer.h"
#include "AthLinks/ElementLink.h"

namespace ftag {

  class FlowSelectorAlg :  public AthReentrantAlgorithm {
  public:

    /**< Constructors */
    FlowSelectorAlg(const std::string& name, ISvcLocator *pSvcLocator);

    /**< Main routines */
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&)  const override;


  private:

    using IPC = xAOD::IParticleContainer;

    SG::ReadDecorHandleKey<IPC> m_constituentKey {
      this, "Constituents", "Something.constituentLinks",
      "Key for the input flow links"};

    SG::WriteDecorHandleKey<IPC> m_neutralConstituentOutKey {
      this, "OutConstituentsNeutral", "Something.constituentLinks",
      "Link to be added to the Jet for neutral constituents"};

    SG::WriteDecorHandleKey<IPC> m_chargedConstituentOutKey {
      this, "OutConstituentsCharged", "Something.constituentLinks",
      "Link to be added to the Jet fr charged constituents"};
  };

} // end namespace ftag

#endif
