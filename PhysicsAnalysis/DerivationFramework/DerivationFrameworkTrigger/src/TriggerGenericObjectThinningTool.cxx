#include "TriggerGenericObjectThinningTool.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

namespace DerivationFramework{
    bool TriggerGenericObjectThinningTool::eventPassedFilter() const
    {
        if (msgLvl(MSG::VERBOSE)){
            const Trig::ChainGroup* Chain = m_trigDec->getChainGroup(".*");
            const std::vector<std::string> fired_triggers = Chain->getListOfTriggers();
            for (const std::string& fired : fired_triggers) {
                if (m_trigDec->isPassed(fired)) ATH_MSG_VERBOSE("Fired trigger "<<fired);
            }
        }

        unsigned int cntrAND{0}, cntrOR{0}, cntrORHLTOnly{0};
        for (const std::string& trig_and : m_triggerListAND) {
            ATH_MSG_DEBUG("AND - Trigger "<<trig_and<<" passed "<<m_trigDec->isPassed(trig_and));
            cntrAND+=(m_trigDec->isPassed(trig_and));
        }
        for (const std::string& trig_or : m_triggerListOR) {
            ATH_MSG_DEBUG("OR - Trigger "<<trig_or<<" passed "<<m_trigDec->isPassed(trig_or));
            cntrOR +=(m_trigDec->isPassed(trig_or));
        }
        for (const std::string& trig_orhltonly : m_triggerListORHLTOnly) {
            ATH_MSG_DEBUG("ORHLTOnly - Trigger "<<trig_orhltonly<<" passed "<<m_trigDec->isPassed(trig_orhltonly, TrigDefs::requireDecision));
            cntrORHLTOnly +=(m_trigDec->isPassed(trig_orhltonly, TrigDefs::requireDecision));
        }

        bool passAND = (cntrAND==m_triggerListAND.size() && !m_triggerListAND.empty());
        bool passOR = (cntrOR > 0);
        bool passORHLTOnly = (cntrORHLTOnly > 0);

        bool pass = passAND || passOR || passORHLTOnly;
        return pass;
    }

    TriggerGenericObjectThinningTool::TriggerGenericObjectThinningTool(const std::string& t,
                                                                    const std::string& n,
                                                                    const IInterface* p ) :
    base_class(t,n,p)
    {
    }

    // Destructor
    TriggerGenericObjectThinningTool::~TriggerGenericObjectThinningTool() {
    }

    // Athena initialize and finalize
    StatusCode TriggerGenericObjectThinningTool::initialize()
    {
        ATH_MSG_VERBOSE("initialize() ...");

        //check xAOD::InDetTrackParticle collection
        ATH_CHECK( m_SGKey.initialize (m_streamName) );
        ATH_MSG_INFO("Using " << m_SGKey << "as the source collection");
            
        return StatusCode::SUCCESS;
    }

    StatusCode TriggerGenericObjectThinningTool::finalize()
    {
        ATH_MSG_VERBOSE("finalize() ...");
        return StatusCode::SUCCESS;
    }

    // The thinning itself
    StatusCode TriggerGenericObjectThinningTool::doThinning() const
    {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        
        // Retrieve main object collection
        SG::ThinningHandle<xAOD::IParticleContainer> particles (m_SGKey, ctx);
    
        // Check the event contains objects
        unsigned int nObjects = particles->size();
        if (nObjects==0) return StatusCode::SUCCESS;

        //check if the trigger passed for the event
        bool keep_container = eventPassedFilter();
        
        // Set up a mask with the same entries as the full collection
        std::vector<bool> mask;
        mask.assign(nObjects,keep_container);
        

        // Execute the thinning based on the mask. Finish.
        particles.keep (mask);
        
        return StatusCode::SUCCESS;
    }
};