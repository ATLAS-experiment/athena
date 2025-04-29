/************************************************************
 * @file FlowSelectorAlg.cxx
 * @brief Make a collection of constituents that aren't charged
 * **********************************************************/

#include "FlowSelectorAlg.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"

namespace ftag {

  FlowSelectorAlg::FlowSelectorAlg(
    const std::string& name, ISvcLocator* loc )
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode FlowSelectorAlg::initialize() {
    ATH_MSG_INFO( "Inizializing " << name() << "... " );

    ATH_CHECK( m_constituentKey.initialize() );
    ATH_CHECK( m_neutralConstituentOutKey.initialize() );
    ATH_CHECK( m_chargedConstituentOutKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode FlowSelectorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG( "Executing " << name() << "... " );

    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;

    SG::ReadDecorHandle<IPC, IPLV> constituents(m_constituentKey, ctx);
    SG::WriteDecorHandle<IPC, IPLV> neutralConstituentsOut(m_neutralConstituentOutKey, ctx);
    SG::WriteDecorHandle<IPC, IPLV> chargedConstituentsOut(m_chargedConstituentOutKey, ctx);


    for (const auto* obj: *constituents) {
      IPLV neutral_flows;
      IPLV charged_flows;
      for (const auto& link: constituents(*obj)) {
        if (!link.isValid()) throw std::runtime_error(
          "invalid constituent link");
        const auto* flow = dynamic_cast<const xAOD::FlowElement*>(*link);
        if (!flow) throw std::runtime_error("constituent isn't flow object");
        if (!flow->isCharged()) neutral_flows.push_back(link);
        else charged_flows.push_back(link);
      }
      neutralConstituentsOut(*obj) = neutral_flows;
      chargedConstituentsOut(*obj) = charged_flows;
    }

    return StatusCode::SUCCESS;
  }

} // end namespace ftag
