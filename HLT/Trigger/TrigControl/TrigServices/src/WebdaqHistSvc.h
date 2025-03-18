/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef WEBDAQHISTSVC_THISTSVC_H
#define WEBDAQHISTSVC_THISTSVC_H 

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"

class TObject;
class TH1;
class TH2;
class TH3;
class TGraph;
class TTree;

#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <thread>

#include <boost/regex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <tbb/concurrent_hash_map.h>

// Helper to mark unsupported interfaces
#define NOSUPPORT(lvl, what) \
  do {                                                                  \
    ATH_MSG_LVL(MSG::lvl, what << "is not supported by this implementation"); \
    return {};                                                          \
  } while (0)


/**
 * HLT online histogram service
 *
 * The main difference to the offline THistSvc are:
 *  - do not write any output ROOT files
 *  - support start/stop
 *  - only supports one histogram instance per name
 *  - publish histograms via webdaq
 * The following environment variables are required:
 *  - TDAQ_WEBDAQ_BASE: should contain the protocol, hostname and, if required, the port 
 *                      number of webis_server to access. Example: http://localhost:8080 
 *  - TDAQ_PARTITION: the partition to publish to
 */
class WebdaqHistSvc: public extends<AthService, ITHistSvc, IIncidentListener>
{ 
public:
  WebdaqHistSvc(const std::string& name, ISvcLocator *svc );
  virtual ~WebdaqHistSvc() noexcept override {}

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
  virtual void handle( const Incident& incident ) override;
  virtual StatusCode stop() override;
  virtual StatusCode finalize() override;
  
  virtual StatusCode regHist(const std::string& name) override;
  virtual StatusCode regHist(const std::string& name, std::unique_ptr<TH1> hist) override;
  virtual StatusCode regHist(const std::string& name, TH1*) override;
  virtual StatusCode getHist(const std::string& id, TH1*& hist, size_t ind) const override;
  virtual StatusCode getHist(const std::string& id, TH2*& hist, size_t ind) const override;
  virtual StatusCode getHist(const std::string& id, TH3*& hist, size_t ind) const override;

  virtual StatusCode deReg(TObject* obj) override;            //<! very slow
  virtual StatusCode deReg(const std::string& name) override; //<! use this instead

  virtual std::vector<std::string> getHists() const override;
  std::set<std::string> getSet(boost::regex) const;

  virtual StatusCode regShared( const std::string&, std::unique_ptr<TH1>, LockedHandle<TH1>& ) override;
  virtual StatusCode regShared( const std::string&, std::unique_ptr<TH2>, LockedHandle<TH2>& ) override;
  virtual StatusCode regShared( const std::string&, std::unique_ptr<TH3>, LockedHandle<TH3>& ) override;
  virtual StatusCode getShared( const std::string&, LockedHandle<TH1>& ) const override;
  virtual StatusCode getShared( const std::string&, LockedHandle<TH2>& ) const override;
  virtual StatusCode getShared( const std::string&, LockedHandle<TH3>& ) const override;

  virtual StatusCode getTHists(TDirectory *td, TList &, bool recurse=false) const override;
  virtual StatusCode getTHists(const std::string& name, TList &, bool recurse=false) const override;
  virtual StatusCode getTHists(TDirectory *td, TList &tl, bool recurse=false, bool reg=false) override;
  virtual StatusCode getTHists(const std::string& name, TList &tl, bool recurse=false, bool reg=false) override;

  virtual bool exists( const std::string& name ) const override { return existsHist(name); }
  virtual bool existsHist( const std::string& name ) const override;

  /* Unsupported interfaces */
  virtual StatusCode regTree(const std::string&) override { NOSUPPORT(WARNING, "TTree"); }
  virtual StatusCode regTree(const std::string&, std::unique_ptr<TTree>) override { NOSUPPORT(WARNING, "TTree"); }
  virtual StatusCode regTree(const std::string&, TTree*) override { NOSUPPORT(WARNING, "TTree"); }
  virtual StatusCode getTree(const std::string&, TTree*&) const override { NOSUPPORT(WARNING, "TTree"); }

  virtual std::vector<std::string> getTrees() const override { NOSUPPORT(DEBUG, "TTree"); }
  virtual StatusCode getTTrees(TDirectory*, TList&, bool) const override { NOSUPPORT(DEBUG, "TTree"); }
  virtual StatusCode getTTrees(const std::string&, TList&, bool) const override { NOSUPPORT(DEBUG, "TTree"); }
  virtual StatusCode getTTrees(TDirectory*, TList&, bool, bool) override { NOSUPPORT(DEBUG, "TTree"); }
  virtual StatusCode getTTrees(const std::string&, TList&, bool, bool) override { NOSUPPORT(DEBUG, "TTree"); }

  virtual StatusCode regGraph(const std::string&) override { NOSUPPORT(WARNING, "TGraph"); }
  virtual StatusCode regGraph(const std::string&, std::unique_ptr<TGraph>) override { NOSUPPORT(WARNING, "TGraph"); }
  virtual StatusCode regGraph(const std::string&, TGraph*) override { NOSUPPORT(WARNING, "TGraph"); }
  virtual std::vector<std::string> getGraphs() const override { NOSUPPORT(DEBUG, "TGraph"); }
  virtual StatusCode getGraph(const std::string&, TGraph*&) const override { NOSUPPORT(DEBUG, "TGraph"); }

  virtual StatusCode regEfficiency(const std::string&) override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode regEfficiency(const std::string&, std::unique_ptr<TEfficiency>) override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode regEfficiency(const std::string&, TEfficiency*) override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode getEfficiency(const std::string&, TEfficiency*&) const override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual std::vector<std::string> getEfficiencies() const override { NOSUPPORT(WARNING, "TEfficiency"); }

  virtual StatusCode getTEfficiencies(TDirectory*, TList&, bool) const override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode getTEfficiencies(const std::string&, TList&, bool) const override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode getTEfficiencies(TDirectory*, TList&, bool, bool) override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode getTEfficiencies(const std::string&, TList&, bool, bool) override { NOSUPPORT(WARNING, "TEfficiency"); }

  virtual StatusCode regShared( const std::string&, std::unique_ptr<TGraph>, LockedHandle<TGraph>& ) override { NOSUPPORT(WARNING, "TGraph"); }
  virtual StatusCode getShared( const std::string&, LockedHandle<TGraph>& ) const override { NOSUPPORT(WARNING, "TGraph"); }
  virtual StatusCode regShared( const std::string&, std::unique_ptr<TEfficiency>, LockedHandle<TEfficiency>& ) override { NOSUPPORT(WARNING, "TEfficiency"); }
  virtual StatusCode getShared( const std::string&, LockedHandle<TEfficiency>& ) const override { NOSUPPORT(WARNING, "TEfficiency"); }

  virtual StatusCode merge( const std::string& ) override  { NOSUPPORT(WARNING, "merge"); }
  virtual StatusCode merge( TObject* ) override  { NOSUPPORT(WARNING, "merge"); }

  virtual bool existsTree( const std::string& ) const override { NOSUPPORT(DEBUG, "TTree"); }
  virtual bool existsGraph( const std::string& ) const override { NOSUPPORT(WARNING, "TGraph"); }
  virtual bool existsEfficiency( const std::string& ) const override { NOSUPPORT(WARNING, "TEfficiency"); }

private:
  /// Helper struct that bundles the histogram, name and mutex
  struct THistID {
    THistID(const std::string& s, TObject* o) : id(s), obj(o) {};
    THistID() : id(""), obj(nullptr) {}; 
    ~THistID() { delete mutex; }
    std::string id;
    TObject* obj{nullptr};
    std::mutex* mutex{nullptr};
  };

  /// Flag to stop the monitoring task
  std::atomic<bool> m_stopFlag{false};
  /// The actual publication Task
  void monitoringTask(int, int, std::atomic<bool>&, boost::regex);
  /// Sync the publication to a multiple of the interval
  void syncPublish(long int, boost::posix_time::ptime);
  /// Sleep for a duration or until the stop flag is set
  void conditionedSleep(std::chrono::milliseconds, const std::atomic<bool>&);
  /// Publication thread
  std::thread m_thread;
  std::thread m_threadFast;
  /// The partition to publish to
  std::string m_partition;
  /// Webdaq configuration variable, see https://gitlab.cern.ch/atlas-tdaq-software/webdaq
  std::string m_tdaqWebdaqBase;
  /// The OH server name (TDAQ_OH_SERVER if defined, m_OHServerName otherwise)
  std::string m_tdaqOHServerName;
  /// Flag to indicate when the histogram map is updated
  std::atomic<bool> m_histoMapUpdated{false};
  /// Flag to indicate when the histogram map is updated for the fast publication  
  std::atomic<bool> m_histoMapUpdatedFast{false};

  /// joboptions service
  ServiceHandle<Gaudi::Interfaces::IOptionsSvc> m_jobOptionsSvc{this, "JobOptionsSvc", "JobOptionsSvc"};
  
  /// Map of the registered histograms
  tbb::concurrent_hash_map<std::string, THistID> m_hists;

  /// Does the histogram follow the naming rules ?
  bool isObjectAllowed(const std::string& path, const TObject *o) const;

  /// Get TList of registered histograms
  StatusCode getTHists_i(const std::string& name, TList &) const;

  // Templated methods implementing the virtual interface methods
  template <typename T> StatusCode regHist_i(std::unique_ptr<T> hist, const std::string& name, bool shared, THistID*& phid);
  template <typename T> T* getHist_i(const std::string& id, const size_t& ind, bool quiet = false) const;
  template <typename T> LockedHandle<T> regShared_i( const std::string& id, std::unique_ptr<T> hist);
  template <typename T> LockedHandle<T> getShared_i( const std::string& id ) const;

  // Properties
  Gaudi::Property<std::string> m_excludeType{this, "ExcludeType", "()"};
  Gaudi::Property<std::string> m_includeType{this, "IncludeType", ".+"};
  Gaudi::Property<std::string> m_excludeName{this, "ExcludeName", ".*\\..*"};
  Gaudi::Property<std::string> m_includeName{this, "IncludeName",
                                             "^/((run_[0-9]+/lb_[0-9]+/LB)|(SHIFT)|(EXPERT)|(DEBUG)|(EXPRESS)|(RUNSTAT))/.+/.+"};
  
  //New properties for the monitoring task
  Gaudi::Property<int> m_numSlots{this, "NumSlots", 8, "Number of slots for the main monitoring task"};
  Gaudi::Property<int> m_numSlotsFast{this, "NumSlotsFast", 1, "Number of slots for the fast monitoring task"};
  Gaudi::Property<int> m_intervalSeconds{this, "IntervalSeconds", 80, "Interval between histogram publications periods in seconds"};
  Gaudi::Property<int> m_intervalSecondsFast{this, "IntervalSecondsFast", 10, "Interval between histogram publications periods in seconds for the fast publication"};
  Gaudi::Property<std::string> m_OHServerName{this, "OHServerName", "Histogramming", "Name of the OH server to publish histograms into"};
  Gaudi::Property<std::string> m_PublicationIncludeName{this, "PublicationIncludeName",".*","Regex to select histograms for publication"};
  Gaudi::Property<std::string> m_fastPublicationIncludeName{this, "FastPublicationIncludeName","^.EXPERT.HLTFramework.TrigSignatureMoni.*","Regex to select histograms for fast publication"};

  // Dummy properties for compatibility with THistSvc
  Gaudi::Property<int> m_autoSave{this, "AutoSave", 0, "Not supported by WebdaqHistSvc"};
  Gaudi::Property<int> m_autoFlush{this, "AutoFlush", 0, "Not supported by WebdaqHistSvc"};
  Gaudi::Property<bool> m_print{this, "PrintAll", false, "Not supported by WebdaqHistSvc"};
  Gaudi::Property<int>  m_maxFileSize{this, "MaxFileSize", 10240, "Not supported by WebdaqHistSvc"};
  Gaudi::Property<std::vector<std::string>> m_outputfile{this, "Output", {}, "Not supported by WebdaqHistSvc"};
  Gaudi::Property<std::vector<std::string>> m_inputfile{this, "Input", {}, "Not supported by WebdaqHistSvc"};

  // compiled regexes
  boost::regex m_excludeTypeRegex;
  boost::regex m_includeTypeRegex;
  boost::regex m_excludeNameRegex;
  boost::regex m_includeNameRegex;
  boost::regex m_PublicationIncludeNameRegex;
  boost::regex m_fastPublicationIncludeNameRegex;

};

#endif // WEBDAQHISTSVC_THISTSVC_H
