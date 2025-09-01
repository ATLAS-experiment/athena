/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaMonitoring/AthenaMonManager.h"

#include <cctype>

#include "TDirectory.h"
#include "TH1.h"
#include "TObject.h"
#include "TROOT.h"
#include "AthMonBench.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaKernel/ISGAudSvc.h"
#include "AthenaMonitoring/IMonitorToolBase.h"
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "CxxUtils/checker_macros.h"

#include "TrigNavTools/TrigNavigationThinningSvcMutex.h"

#include <limits.h>
#include <vector>


class AthenaMonManager::Imp {
public:
    Imp(AthenaMonManager*man,const std::string& name) :  m_theManager(man),
        m_sgAudSvc(0),
        m_isPrimaryManager(false),
        m_everyNevents(-1),
        m_eventCounter(-1),
        m_fileKeyProp(name),
        m_manualDataTypeSetupProp(false),
        m_dataTypeProp("userDefined"),
        m_environmentProp("user"),
        m_LBsLowStatProp(10),
        m_LBsMedStatProp(30),
        m_LBsHigStatProp(90),
        m_manualRunLBProp(false),
        m_runProp(0),
        m_lumiBlockProp(0),
        m_nMonGroupCopies(0),
	m_forkedProcess(false),
	m_lastPID(0),
    m_rootBackend(false),
	m_doResourceMon(false)
 {}
  
    ~Imp()
    {
        std::map<IMonitorToolBase*,ToolBench*>::iterator it, itE;

        for (it=m_toolbenchmarks.begin(),itE=m_toolbenchmarks.end(); it!=itE; ++it) {
            delete it->second;
        }

        m_toolbenchmarks.clear();
    }

    //These were protected before we moved to a private implementation:
    typedef std::map<std::string,TObject*>  ObjMap_t;
    ObjMap_t     m_objMap;

    AthenaMonManager * m_theManager;
    SmartIF<ISGAudSvc> m_sgAudSvc;

    bool         m_isPrimaryManager;

    long         m_everyNevents;
    long         m_eventCounter;

    std::string  m_fileKeyProp;

    bool         m_manualDataTypeSetupProp;
    std::string  m_dataTypeProp;
    std::string  m_environmentProp;
    unsigned int m_LBsLowStatProp, m_LBsMedStatProp, m_LBsHigStatProp;

    bool         m_manualRunLBProp;

    unsigned int m_runProp;
    unsigned int m_lumiBlockProp;

    unsigned m_nMonGroupCopies;

    bool m_forkedProcess;
    pid_t m_lastPID;

    bool m_rootBackend;

    //Hist leak checker is looking for
    //inappropriate usage of MonGroup copy constructors (temporary
    //until we outlaw copy/assignment of MonGroups):
    class HistLeakChecker {
        Imp * m_d;
        unsigned m_mgcopies_initval;
    public:
        HistLeakChecker(Imp*d) : m_d(d),
            m_mgcopies_initval(ManagedMonitorToolBase::MonGroup::ncopies()) {}
        ~HistLeakChecker()
        {   m_d->m_nMonGroupCopies += ManagedMonitorToolBase::MonGroup::ncopies()-m_mgcopies_initval;
        }
    };

    template<class T> void toolAudStart(const T&tool) {
        if (m_sgAudSvc) m_sgAudSvc->setFakeCurrentAlg(m_theManager->name()+"_MT_"+tool->name());
    }
    void toolAudEnd() {
        if (m_sgAudSvc) m_sgAudSvc->clearFakeCurrentAlg();
    }

    bool m_doResourceMon;

    class ToolBench {
    public:
        ToolBench(IMonitorToolBase * t) :  m_theTool(t) {}
        const IMonitorToolBase * m_theTool;
        AthMonBench m_bench_alginit_retrieve;
        AthMonBench m_bench_algexec_fillHists;//NB: includes call to preselector
        AthMonBench m_bench_algexec_checkHists;
        AthMonBench m_bench_algexec_bookHists;
        //NB: So far we don't check the call to runStat()
        AthMonBench m_bench_algfin_finalHists;
        AthMonBench m_bench_algfin_checkHists;
        void report(AthenaMonManager*a) {
            MSG::Level l(AthMonBench::s_resourceMonThreshold);
            a->msg(l)<<"Init ResourceSummary ["<<m_theTool->name()<<"] --> load/create/initialize : "<<m_bench_alginit_retrieve<<endmsg;
            m_bench_algexec_checkHists.setUnitCount();
            m_bench_algexec_bookHists.setUnitCount();
            a->msg(l)<<"Exec ResourceSummary ["<<m_theTool->name()<<"] --> book : "<<m_bench_algexec_bookHists<<endmsg;
            a->msg(l)<<"Exec ResourceSummary ["<<m_theTool->name()<<"] --> check : "<<m_bench_algexec_checkHists<<endmsg;
            a->msg(l)<<"Exec ResourceSummary ["<<m_theTool->name()<<"] --> fill : "<<m_bench_algexec_fillHists<<endmsg;
            a->msg(l)<<"Fin ResourceSummary ["<<m_theTool->name()<<"] --> finalHists : "<<m_bench_algfin_finalHists<<endmsg;
            a->msg(l)<<"Fin ResourceSummary ["<<m_theTool->name()<<"] --> checkHists(fromFinalize==true) : "<<m_bench_algfin_checkHists<<endmsg;
        }

    };
    std::map<IMonitorToolBase*,ToolBench*> m_toolbenchmarks;
    ToolBench* getToolBench(IMonitorToolBase* t) {
        std::map<IMonitorToolBase*,ToolBench*>::iterator it = m_toolbenchmarks.find(t);
        if (it==m_toolbenchmarks.end()) {
            ToolBench*tb =new ToolBench(t);
            m_toolbenchmarks[t] = tb;
            return tb;
        }
        return it->second;
    }

    // The data below should be truly global for a job, and inheriting classes should
    // not have the option of modifying them.
    // The data are set from the corresponding m_* properties of the first AthenaMonManager
    // in the Athena algorithm chain (this manager sets s_staticDataAreInit to true).

    inline static std::atomic<ISvcLocator*> s_svcLocator{nullptr};

    inline static std::atomic<bool> s_staticDataAreInit{false};

    inline static std::string     s_dataTypeStr ATLAS_THREAD_SAFE {"userDefined"};
    inline static std::string     s_environmentStr ATLAS_THREAD_SAFE {"user"};
    inline static DataType_t      s_dataType ATLAS_THREAD_SAFE {userDefined};
    inline static Environment_t   s_environment ATLAS_THREAD_SAFE {user};

    inline static bool         s_runLBOverridden ATLAS_THREAD_SAFE {false};
    inline static unsigned int s_lumiBlock ATLAS_THREAD_SAFE {0};
    inline static unsigned int s_run ATLAS_THREAD_SAFE {0};
    inline static unsigned int s_fill ATLAS_THREAD_SAFE {0};
    inline static unsigned int s_LBsLowStat ATLAS_THREAD_SAFE {0};
    inline static unsigned int s_LBsMedStat ATLAS_THREAD_SAFE {0};
    inline static unsigned int s_LBsHigStat ATLAS_THREAD_SAFE {0};
};


namespace {

std::string strToLower( const std::string& str );

} // unnamed namespace

// *********************************************************************
// Public Methods
// *********************************************************************

AthenaMonManager::
AthenaMonManager( const std::string& name, ISvcLocator* pSvcLocator )
  : AthAlgorithm( name, pSvcLocator )
  , m_monTools( this )
  , m_THistSvc("THistSvc", name)
  , m_d(new Imp(this, name))
{
    declareProperty( "AthenaMonTools", m_monTools );
    declareProperty( "CheckEveryNoEvents", m_d->m_everyNevents );
    declareProperty( "FileKey", m_d->m_fileKeyProp );
    declareProperty( "ManualDataTypeSetup", m_d->m_manualDataTypeSetupProp );
    declareProperty( "DataType", m_d->m_dataTypeProp );
    declareProperty( "Environment", m_d->m_environmentProp );
    declareProperty( "LBsInLowStatInterval", m_d->m_LBsLowStatProp );
    declareProperty( "LBsInMediumStatInterval", m_d->m_LBsMedStatProp );
    declareProperty( "LBsInHighStatInterval", m_d->m_LBsHigStatProp );
    declareProperty( "ManualRunLBSetup", m_d->m_manualRunLBProp );
    declareProperty( "Run", m_d->m_runProp );
    declareProperty( "LumiBlock", m_d->m_lumiBlockProp );
    declareProperty( "ROOTBackend", m_d->m_rootBackend );

    if( !Imp::s_svcLocator )
        Imp::s_svcLocator = pSvcLocator;
}


AthenaMonManager::
~AthenaMonManager()
{
    delete m_d;

    // If it is known that ManagedMonitorToolBase objects are deleted _after_ this object,
    // then one should call setMonManager(0) on each of them.
}


AthenaMonManager::Environment_t
AthenaMonManager::
envStringToEnum( const std::string& str )
{
    std::string lcstr( strToLower(str) );

    if( lcstr == "user" )
        return user;
    else if( lcstr == "nooutput" )
        return noOutput;
    else if( lcstr == "online" )
        return online;
    else if( lcstr == "tier0" )
        return tier0;
    else if( lcstr == "tier0raw" )
        return tier0Raw;
    else if( lcstr == "tier0esd" )
        return tier0ESD;
    else if( lcstr == "aod" )
        return AOD;
    else if( lcstr == "altprod" )
        return altprod;

    if( Imp::s_svcLocator ) {
        SmartIF<IMessageSvc> ms{Imp::s_svcLocator.load()->service( "MessageSvc" )};
        if( ms.isValid() ) {
            MsgStream log( ms, "AthenaMonManager::envStringToEnum()" );
            log << MSG::WARNING << "Unknown AthenaMonManager::Environment_t \""
                << str << "\", returning \"user\"" << endmsg;
        }
    }

    return user;
}


AthenaMonManager::DataType_t
AthenaMonManager::
dataTypeStringToEnum( const std::string& str )
{
    std::string lcstr( strToLower(str) );

    if( lcstr == "userdefined" )
        return userDefined;
    else if( lcstr == "montecarlo" )
        return monteCarlo;
    else if( lcstr == "collisions" )
        return collisions;
    else if( lcstr == "cosmics" )
        return cosmics;
    else if( lcstr == "heavyioncollisions" )
        return heavyIonCollisions;

    if( Imp::s_svcLocator ) {
        SmartIF<IMessageSvc> ms{Imp::s_svcLocator.load()->service( "MessageSvc" )};
        if( ms.isValid() ) {
            MsgStream log( ms, "AthenaMonManager::dataTypeStringToEnum()" );
            log << MSG::WARNING << "Unknown AthenaMonManager::DataType_t \""
                << str << "\", returning \"userDefined\"" << endmsg;
        }
    }

    return userDefined;
}


AthenaMonManager::Environment_t
AthenaMonManager::
environment()
{
    return Imp::s_environment;
}

unsigned int AthenaMonManager::getLBsLowStat() {
    return Imp::s_LBsLowStat;
}
unsigned int AthenaMonManager::getLBsMedStat() {
    return Imp::s_LBsMedStat;
}
unsigned int AthenaMonManager::getLBsHigStat() {
    return Imp::s_LBsHigStat;
}

AthenaMonManager::DataType_t
AthenaMonManager::
dataType()
{
    return Imp::s_dataType;
}

bool 
AthenaMonManager::
forkedProcess() {
  return m_d->m_forkedProcess;
}


unsigned int
AthenaMonManager::
lumiBlockNumber()
{
    if (Imp::s_runLBOverridden) {
        return Imp::s_lumiBlock;
    } else {
        return Gaudi::Hive::currentContext().eventID().lumi_block();
    }
}


unsigned int
AthenaMonManager::
runNumber()
{
    if (Imp::s_runLBOverridden) {
        return Imp::s_run;
    } else {
        return Gaudi::Hive::currentContext().eventID().run_number();
    }
}


unsigned int
AthenaMonManager::
fillNumber()
{
    return Imp::s_fill;
}


StatusCode
AthenaMonManager::
initialize()
{
    Imp::HistLeakChecker lc(m_d);

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "AthenaMonManager::initialize():" << endmsg;

    m_d->m_sgAudSvc = Imp::s_svcLocator.load()->service("SGAudSvc", false/*do not create*/);
    m_d->m_doResourceMon = msgLvl(AthMonBench::s_resourceMonThreshold);

    ATH_CHECK( m_THistSvc.retrieve() );

    if( !Imp::s_staticDataAreInit ) {
        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Initializing static data" << endmsg;

        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "      * \"ManualDataTypeSetup\" = " << m_d->m_manualDataTypeSetupProp << endmsg;
        }

#if 0  // The two branches of the conditional are identical.
        if( m_d->m_manualDataTypeSetupProp ) {
            Imp::s_dataTypeStr = m_d->m_dataTypeProp;
            Imp::s_dataType = dataTypeStringToEnum( m_d->m_dataTypeProp );
        }
        else
#endif
        {
            Imp::s_dataTypeStr = m_d->m_dataTypeProp;
            Imp::s_dataType = dataTypeStringToEnum( m_d->m_dataTypeProp );
        }

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "      * \"DataType\" = " << Imp::s_dataType << endmsg;

        Imp::s_environmentStr = m_d->m_environmentProp;
        Imp::s_environment = envStringToEnum( m_d->m_environmentProp );

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "      * \"Environment\" = " << Imp::s_environment << endmsg;

        Imp::s_LBsLowStat = m_d->m_LBsLowStatProp;
        Imp::s_LBsMedStat = m_d->m_LBsMedStatProp;
        Imp::s_LBsHigStat = m_d->m_LBsHigStatProp;
        if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "      * \"LBsInLowStatInterval\" = " << Imp::s_LBsLowStat << endmsg;
            msg(MSG::DEBUG) << "      * \"LBsInMediumStatInterval\" = " << Imp::s_LBsMedStat << endmsg;
            msg(MSG::DEBUG) << "      * \"LBsInHighStatInterval\" = " << Imp::s_LBsHigStat << endmsg;
        }

        m_d->m_isPrimaryManager = true;
        Imp::s_staticDataAreInit = true;

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "      * \"ManualRunLBSetup\" = " << m_d->m_manualRunLBProp << endmsg;

        if( m_d->m_manualRunLBProp ) {
            Imp::s_run = m_d->m_runProp;
            Imp::s_lumiBlock = m_d->m_lumiBlockProp;
            Imp::s_runLBOverridden = true;
            if (msgLvl(MSG::DEBUG))
                msg(MSG::DEBUG) << "          --> using run = " << Imp::s_run << ", lumiBlock = " << Imp::s_lumiBlock << endmsg;
        } else {
            if (msgLvl(MSG::DEBUG))
                msg(MSG::DEBUG) << "          --> using run = " << Imp::s_run << ", lumiBlock = " << Imp::s_lumiBlock << endmsg;
        }
    }

    // Ensure consistency among all managers
    m_d->m_dataTypeProp    = Imp::s_dataTypeStr;
    m_d->m_environmentProp = Imp::s_environmentStr;

    // Provide a virtual Algorithm with a subset of the properties
    ServiceHandle<Gaudi::Interfaces::IOptionsSvc> joSvc("JobOptionsSvc", name());
    ATH_CHECK( joSvc.retrieve() );
    ATH_MSG_DEBUG("  --> Found service \"JobOptionsSvc\"");

    const std::string client = name() + std::string("Properties");
    ATH_MSG_DEBUG("  --> Adding properties under name \"" << client << "\"");
    joSvc->set( client + ".FileKey", m_d->m_fileKeyProp );
    joSvc->set( client + ".DataType", m_d->m_dataTypeProp );
    joSvc->set( client + ".Environment", m_d->m_environmentProp );

    if( m_monTools.size() > 0 ) {
      ATH_CHECK( m_monTools.retrieve() );
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Retrieved AthenaMonTools" << endmsg;
    }

    m_d->m_eventCounter = m_d->m_everyNevents;

    ToolHandleArray<IMonitorToolBase>::iterator monToolsEnd = m_monTools.end();
    for( ToolHandleArray<IMonitorToolBase>::iterator i = m_monTools.begin(); i != monToolsEnd; ++i ) {
        ToolHandle<IMonitorToolBase>& tool = *i;
        AthMonBench bench_tmp;
        IMonitorToolBase* mon = tool.operator->();
        ManagedMonitorToolBase* managed = dynamic_cast<ManagedMonitorToolBase*>( mon );
        if( managed != 0 ) {
            managed->setMonManager( this );
        }
        if (m_d->m_doResourceMon) {
            Imp::ToolBench* tb = m_d->getToolBench(mon);
            tb->m_bench_alginit_retrieve = bench_tmp;
        }

    }
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Exiting successfully" << endmsg;

    return StatusCode::SUCCESS;
}


StatusCode
AthenaMonManager::
execute()
{
    m_d->m_forkedProcess=false;
    pid_t currPID=getpid();
    //m_lastPID 0 means the execute method was not called yet. 
    if (m_d->m_lastPID!=0 && m_d->m_lastPID!=currPID) {
      m_d->m_forkedProcess=true;
      ATH_MSG_INFO("Forked event discovered!");
    }
    m_d->m_lastPID=currPID;

    Imp::HistLeakChecker lc(m_d);
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "AthenaMonManager::execute():" << endmsg;

    // This is legacy R2 monitoring.
    // We only permit serial access (over all slots) to both HLT monitoring AND navigation thinning, as both use the same underlying thread un-safe navigation tool
    // All of these elements are deprecated for R3 and are in the process of being replaced. 
    std::unique_lock<std::mutex> hltLock(TrigNavigationThinningSvcMutex::s_mutex, std::defer_lock);
    if (name() == "HLTMonManager") {
        ATH_MSG_DEBUG("HLTMonManager is obtaining the TrigNavigationThinningSvc lock in slot " 
            << Gaudi::Hive::currentContext().slot() << " for event " << Gaudi::Hive::currentContext().eventID().event_number() );
        hltLock.lock();
    }

    StatusCode sc;

    ToolHandleArray<IMonitorToolBase>::iterator monToolsEnd = m_monTools.end();
    for( ToolHandleArray<IMonitorToolBase>::iterator i = m_monTools.begin(); i != monToolsEnd; ++i ) {
        ToolHandle<IMonitorToolBase>& tool = *i;
        m_d->toolAudStart(tool);
        AthMonBench bench_tmp;
        if (m_d->m_doResourceMon)
            bench_tmp.startMeasurement();
        if( tool->preSelector() ) {

            sc = tool->fillHists();

            if( !sc.isSuccess() ) {
                if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::fillHists() unsuccessful" << endmsg;
            }
        }


        if (m_d->m_doResourceMon) {
            bench_tmp.finishMeasurement();
            Imp::ToolBench* tb =  m_d->getToolBench(tool.operator->());
            tb->m_bench_algexec_fillHists += bench_tmp;//fixme: count events and subdivide
            if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> m_doResourceMon is True" << endmsg;
        }
        m_d->toolAudEnd();
    }
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Done calling IMonitorToolBase::fillHists()" << endmsg;

    if( m_d->m_eventCounter > 0 ) {
        m_d->m_eventCounter--;
    }
    else if( m_d->m_eventCounter == 0 ) {
        for( ToolHandleArray<IMonitorToolBase>::iterator i = m_monTools.begin(); i != monToolsEnd; ++i ) {
            ToolHandle<IMonitorToolBase>& tool = *i;
            m_d->toolAudStart(tool);
            AthMonBench bench_tmp;

            if (m_d->m_doResourceMon)
                bench_tmp.startMeasurement();
            sc = tool->checkHists(false);
            if (m_d->m_doResourceMon) {
                bench_tmp.finishMeasurement();
                Imp::ToolBench* tb =  m_d->getToolBench(tool.operator->());
                tb->m_bench_algexec_checkHists += bench_tmp;//fixme: count events and subdivide
            }
            if( !sc.isSuccess() ) {
                if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::checkHists() unsuccessful" << endmsg;
            }
            m_d->toolAudEnd();
        }
        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Done calling IMonitorToolBase::checkHists()" << endmsg;

        m_d->m_eventCounter = m_d->m_everyNevents;
    }
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "  --> Exiting successfully" << endmsg;

    ATH_MSG_DEBUG(name() << " is releasing the TrigNavigationThinningSvc lock");
    return StatusCode::SUCCESS;
}


StatusCode
AthenaMonManager::
finalize()
{
    return StatusCode::SUCCESS;
}

StatusCode
AthenaMonManager::
stop()
{
    Imp::HistLeakChecker lc(m_d);
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "AthenaMonManager::finalize():" << endmsg;

    StatusCode sc;

    ToolHandleArray<IMonitorToolBase>::iterator monToolsEnd = m_monTools.end();
    for( ToolHandleArray<IMonitorToolBase>::iterator i = m_monTools.begin(); i != monToolsEnd; ++i ) {
        ToolHandle<IMonitorToolBase>& tool = *i;
        m_d->toolAudStart(tool);

        sc = tool->runStat();
        if( !sc.isSuccess() ) {
            if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::runStat() unsuccessful" << endmsg;
        }

        Imp::ToolBench* tb =  m_d->m_doResourceMon ? m_d->getToolBench(tool.operator->()) : 0;
        if (tb)
            tb->m_bench_algfin_finalHists.startMeasurement();
        sc = tool->finalHists();
        if (tb)
            tb->m_bench_algfin_finalHists.finishMeasurement();
        if( !sc.isSuccess() ) {
            if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::finalHists() unsuccessful" << endmsg;
        }
        if (tb)
            tb->m_bench_algfin_checkHists.startMeasurement();
        sc = tool->checkHists(true);
        if (tb)
            tb->m_bench_algfin_checkHists.finishMeasurement();
        if( !sc.isSuccess() ) {
            if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::checkHists() unsuccessful" << endmsg;
        }
        if (tb)
            tb->report(this);
        m_d->toolAudEnd();
    }
    if (msgLvl(MSG::DEBUG)) {
        msg(MSG::DEBUG) << "  --> Done calling IMonitorToolBase::finalHists() and IMonitorToolBase::checkHists()" << endmsg;
        msg(MSG::DEBUG) << "  --> Exiting successfully" << endmsg;
    }
    return StatusCode::SUCCESS;
}


StatusCode
AthenaMonManager::
start()
{
    Imp::HistLeakChecker lc(m_d);
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "AthenaMonManager::start():" << endmsg;

    StatusCode sc;

    ToolHandleArray<IMonitorToolBase>::iterator monToolsEnd = m_monTools.end();
    for( ToolHandleArray<IMonitorToolBase>::iterator i = m_monTools.begin(); i != monToolsEnd; ++i ) {
        ToolHandle<IMonitorToolBase>& tool = *i;
        m_d->toolAudStart(tool);
        AthMonBench bench_tmp;
        if (m_d->m_doResourceMon)
            bench_tmp.startMeasurement();
        sc = tool->bookHists();
        if (m_d->m_doResourceMon) {
            bench_tmp.finishMeasurement();
            Imp::ToolBench* tb =  m_d->getToolBench(tool.operator->());
            tb->m_bench_algexec_bookHists += bench_tmp;
        }
        if( !sc.isSuccess() ) {
            if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "IMonitorToolBase::bookHists() unsuccessful" << endmsg;
        }
        m_d->toolAudEnd();
    }
    if (msgLvl(MSG::DEBUG)) {
        msg(MSG::DEBUG) << "  --> Done calling IMonitorToolBase::bookHists()" << endmsg;
        msg(MSG::DEBUG) << "  --> Exiting successfully" << endmsg;
    }

    return StatusCode::SUCCESS;
}


std::string
AthenaMonManager::
fileKey() const
{
    return m_d->m_fileKeyProp;
}


void
AthenaMonManager::
passOwnership( TObject* h, const std::string& key )
{
    m_d->m_objMap.emplace( key, h );
}


void
AthenaMonManager::
writeAndDelete( const std::string& key )
{
    // If is owned TObject, we deregister from THistSvc, write it in a
    // root file, and delete it.

    Imp::ObjMap_t::iterator i = m_d->m_objMap.find( key );
    if( i != m_d->m_objMap.end() ) {
        TObject* o = i->second;
        TH1* h = dynamic_cast<TH1*>( o );
        if( h != 0 ) {
            // By default, ROOT does its own bookkeeping of references to
            // histograms, and this leads to a significant amount of time
            // spent in the TObject d'tor when there are >= O(1000) TDirectories.
            // So, turn this bookkeeping off when deleting these histograms.
            bool doRecursiveReferenceDelete = gROOT->MustClean();
            gROOT->SetMustClean(false);
            TDirectory* dir = h->GetDirectory();
            TDirectory* g = gDirectory;
            if( dir )
                dir->cd();
            h->Write();
            g->cd();

            StatusCode sc = m_THistSvc->deReg( h );
            if( !sc.isSuccess() ) {
                if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "AthenaMonManager::WriteAndDeleteHist(): Failure to deReg( TObject* )" << endmsg;
            }
            delete o;
            gROOT->SetMustClean(doRecursiveReferenceDelete);
            m_d->m_objMap.erase( key );
        }
    }
}

// *********************************************************************
// Protected Methods
// *********************************************************************


// *********************************************************************
// Private Methods
// *********************************************************************
namespace {

std::string strToLower( const std::string& str )
{
    std::string lstr(str);
    std::string::const_iterator  from   = str.begin();
    std::string::const_iterator  strend = str.end();
    std::string::iterator        to     = lstr.begin();
    while( from != strend ) {
        *to++ = tolower(*from++);
    }
    return lstr;
}

} // unnamed namespace
