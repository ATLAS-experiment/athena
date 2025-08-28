/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DataQualityUtils/MonitoringFile.h"

#include <TDirectory.h>
#include <TEfficiency.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TIterator.h>
#include <TKey.h>
#include <TObject.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>

#include <cmath>
#include <cstdio>
#include <ctime>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "DataQualityInterfaces/HanApp.h"
#include "DataQualityInterfaces/HanUtils.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

ClassImp(dqutils::MonitoringFile)

namespace dqutils {

  class dbgPrint {

   public:
    dbgPrint(const debugLevel_t setLvl = none) : m_currLvl(setLvl) {};
    inline void operator()(const debugLevel_t level, const std::string& msg) const {
      if (level <= m_currLvl)
        std::cout << msg << std::endl;
    }
    void setLvl(const debugLevel_t lvl) { m_currLvl = lvl; }
    debugLevel_t getLvl() const { return m_currLvl; }

   private:
    debugLevel_t m_currLvl;
  };

  static dbgPrint s_dbg;
  static bool s_checkEquality=false;

  std::optional<std::regex> checkRegEx(const std::string& re) {
    if (re.empty())
      return std::nullopt;

    std::regex reNew(re);
    try {
      // this should fail if there are any problems with re!
      std::string test("Test String");
      std::regex_match(test, reNew);
    } catch (std::exception& e) {
      std::cout << "ERROR: Invalid RegEx string \"" << re << "\"." << std::endl;
      std::cout << "See http://www.boost.org/doc/libs/1_42_0/libs/regex/doc/html/boost_regex/syntax.html for allowed regular expression syntax" << std::endl;
      return std::nullopt;
    }
    return reNew;
  }

  // Internally used data-structures:

  class histCollection {

   public:
    typedef std::map<std::string, std::vector<std::string>> fileLBMap_t;
    explicit histCollection(TFile* out, bool skipExisting = false) : m_out{out}, m_skipExisting(skipExisting) {};
    histCollection() = delete;

    ~histCollection();

    void addDirectory(TDirectory* dir, const std::string& dirName, const std::string& filename = "");
    size_t size() { return m_data.size(); };
    void print();
    void write();  // Destructive method, will delete internal data after writing

    void addDirExclusion(const std::optional<std::regex>& dirEx);
    void addHistExclusion(const std::optional<std::regex>& histEx);

    unsigned size() const;
    void clear();

    fileLBMap_t getFileLBMapAndClear() {
      m_data.clear();
      return std::move(m_fileLBMap);
    }

    void printTiming();

   private:
    class histPerDir_t {
     public:
      histPerDir_t(const std::string& nameIn, std::unique_ptr<TObject>&& objIn, TTree* md);
      histPerDir_t(dqutils::histCollection::histPerDir_t&& other)
          : name(std::move(other.name)), obj(std::move(other.obj)), metadata(std::move(other.metadata)), mergeMethod(other.mergeMethod) {}
    
      std::string name;
      std::unique_ptr<TObject> obj;
      std::array<std::string, 3> metadata{"unset","","<default>"};
      std::clock_t cpuSum = 0;
      void (*mergeMethod)(TObject* a, const TObject* b) = nullptr;
      void merge(TObject* other); 

     private:
      bool fillMD(TTree* mdTree);
    };

    struct histDir_t {
      std::unordered_map<std::string, histPerDir_t> histos;
      void writeMD(TDirectory* outDir) const;
    };

   private:
    TFile* m_out;
    bool m_skipExisting;
    std::unordered_map<std::string, histDir_t> m_data;
    std::optional<std::regex> m_dirExclusion;
    std::optional<std::regex> m_histExclusion;
    fileLBMap_t m_fileLBMap;
  };

  void histCollection::clear() {
    m_data.clear();
    m_fileLBMap.clear();
  }

  void histCollection::printTiming() {
    std::vector<std::pair<std::string, clock_t>> cpuPerHistVec;
    for (auto& [dirname, histDir] : m_data) {
      for (auto& [histname, histo] : histDir.histos) {
        cpuPerHistVec.emplace_back(dirname + "/" + histname, histo.cpuSum);
      }
    }
    auto ordering = [](std::pair<std::string, clock_t> a, std::pair<std::string, clock_t> b) { return a.second < b.second; };
    std::sort(cpuPerHistVec.begin(), cpuPerHistVec.end(), ordering);
    for (const auto& [name, time] : cpuPerHistVec) {
      const double tSec = double(time) / CLOCKS_PER_SEC;
      std::cout << std::format("{:<30} : {:10.3f}", name, tSec) << std::endl;
    }
    return;
  }

  bool histCollection::histPerDir_t::fillMD(TTree * md) {
    TTreeReader reader(md);
    TTreeReaderArray<char> i_name(reader, "Name");
    TTreeReaderArray<char> i_interval(reader, "Interval");
    TTreeReaderArray<char> i_chain(reader, "TriggerChain");
    TTreeReaderArray<char> i_merge(reader, "MergeMethod");

    bool found = false;
    while (reader.Next()) {
      const std::string nameStr(static_cast<char*>(i_name.GetAddress()));
      if (name == nameStr) {
        metadata = {static_cast<char*>(i_interval.GetAddress()), static_cast<char*>(i_chain.GetAddress()), static_cast<char*>(i_merge.GetAddress())};
        found = true;
        break;
      }
    }
    return found;
  }

  void histCollection::histPerDir_t::merge(TObject * other) {
    if (obj && other) {
      const std::clock_t cpuStart = std::clock();
      this->mergeMethod(obj.get(), other);
      cpuSum += std::clock() - cpuStart;
    }
    return;
  }

  void histCollection::histDir_t::writeMD(TDirectory * out) const {

    // Check if there is already a metadata-tree. Merge content if necessary
    std::map<std::string, std::array<std::string, 3>> metadatamap;
    std::unique_ptr<TTree> oldMD((TTree*)out->Get("metadata"));
    if (oldMD) {
      TTreeReader reader(oldMD.get());
      TTreeReaderArray<char> i_name(reader, "Name");
      TTreeReaderArray<char> i_interval(reader, "Interval");
      TTreeReaderArray<char> i_chain(reader, "TriggerChain");
      TTreeReaderArray<char> i_merge(reader, "MergeMethod");

      while (reader.Next()) {
        const std::string name(static_cast<char*>(i_name.GetAddress()));
        metadatamap[name] = {static_cast<char*>(i_interval.GetAddress()), static_cast<char*>(i_chain.GetAddress()), static_cast<char*>(i_merge.GetAddress())};
      }
    }

    for (const auto& [key, h] : histos) {
      if (h.metadata[0]!="unset") //Ignore dummy-metadata (eg HLTMon use-case)
	  metadatamap[key] = h.metadata;
    }

    if (metadatamap.empty()) return; //Do not write empty metadata tree
    std::string interval, chain, merge;
    char histname[1024];  // FIXME, no idea why this works only in this old-fashioned way
    std::unique_ptr<TTree> mdTree = std::make_unique<TTree>("metadata", "Monitoring Metadata");
    mdTree->SetDirectory(out);

    mdTree->Branch("Name", (void*)nullptr, "Name/C");
    mdTree->Branch("Interval", interval.data(), "Interval/C");
    mdTree->Branch("TriggerChain", chain.data(), "TriggerChain/C");
    mdTree->Branch("MergeMethod", merge.data(), "MergeMethod/C");

    mdTree->SetBranchAddress("Name", histname);

    for (auto& [key, h] : metadatamap) {
      strncpy(histname, key.c_str(), 1023);
      interval = h[0];
      chain = h[1];
      merge = h[2];
      mdTree->Fill();
    }
    mdTree->Write(0, TObject::kOverwrite);
  }

  histCollection::~histCollection() {}

  void histCollection::addDirExclusion(const std::optional<std::regex>& dir) {
    m_dirExclusion = dir;
    return;
  }

  void histCollection::addHistExclusion(const std::optional<std::regex>& dir) {
    m_histExclusion = dir;
    return;
  }

  unsigned histCollection::size() const {
    unsigned s = 0;
    for (const auto& it : m_data) {
      s += it.second.histos.size();
    }
    return s;
  }

  void histCollection::print() {
    for (const auto& it : m_data) {
      const histDir_t& hd = it.second;
      std::cout << "Dir: " << it.first << " has " << hd.histos.size() << " histos" << std::endl;
      for (const auto& it1 : hd.histos)
        std::cout << "\t" << it1.second.name << std::endl;
    }
    return;
  }

  template <class HIST>
  void defaultMerge(TObject * a, const TObject* b) {
    (static_cast<HIST*>(a))->Add(static_cast<const HIST*>(b));
    return;
  }

  void weightedAverage ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!b1 || !a1)
      std::cout << "ERROR in weightedAverageTH1: Object not of type TH1" << std::endl;
    else {
      if (b1->GetEntries()==0) return;
      dqutils::MonitoringFile::merge_weightedAverage(*a1, *b1);
    }
    return;
  }

  void weightedEff ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = (dynamic_cast<const TH1*>(b));
    if (!b1 || !a1)
      std::cout << "ERROR in weightedEff: Object not of type TH1" << std::endl;
    else {
      if (b1->GetEntries()==0) return;
      dqutils::MonitoringFile::merge_weightedEff(*a1, *b1);
    }
    return;
  }

  void mergeRMS ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!b1 || !a1)
      std::cout << "ERROR in mergeRMS: Object not of type TH1" << std::endl;
    else {
      if (b1->GetEntries()==0) return;
      dqutils::MonitoringFile::merge_RMS(*a1, *b1);
    }
    return;
  }

  void RMSpercentDeviation ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!b1 || !a1)
      std::cout << "ERROR in RMSpercentDeviation: Object not of type TH1" << std::endl;
    else {
      if (b1->GetEntries()==0) return;
      dqutils::MonitoringFile::merge_RMSpercentDeviation(*a1, *b1);
    }
    return;
  }

  void perBinEffPerCent ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!b1 || !a1)
      std::cout << "ERROR in getBinEffPerCent: Object not of type TH1" << std::endl;
    else {
      if (b1->GetEntries()==0) return;
      dqutils::MonitoringFile::merge_perBinEffPerCent(*a1, *b1);
    }
    return;
  }

  void lowerLB ATLAS_NOT_THREAD_SAFE(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!b1 || !a1)
      std::cout << "ERROR in lowerLB: Object not of type TH1" << std::endl;
    else
      dqutils::MonitoringFile::merge_lowerLB(*a1, *b1);
    return;
  }

  template <class HIST>
  void identical(TObject * a, const TObject* b) {
    if (!s_checkEquality)
      return; //quasi null-operation
    HIST* a1 = (dynamic_cast<HIST*>(a));
    const HIST* b1 = dynamic_cast<const HIST*>(b);
    if (!b1 || !a1){
      std::cout << "ERROR in identical: Object not of correct type" << std::endl;
      return;
    }
    dqutils::MonitoringFile::merge_identical(*a1, *b1);
    return;
  }

  void merge_rebinned(TObject * a, const TObject* b) {
    TH1* a1 = (dynamic_cast<TH1*>(a));
    const TH1* b1 = dynamic_cast<const TH1*>(b);
    if (!a1 || !b1) {
      std::cout << "ERROR, in merge_rebinned: Object not of type TH1";
      return;
    }
    TH1* b2 = const_cast<TH1*>(b1);
    dqutils::MonitoringFile::merge_Rebinned(*a1, *b2);
    return;
  }

  void merge_eventSample(TObject * a, const TObject* b) {
    TH2* a1 = (dynamic_cast<TH2*>(a));
    const TH2* b1 = dynamic_cast<const TH2*>(b);
    if (!a1 || !b1) {
      std::cout << "ERROR in merge_eventSample: Object not of type TH2" << std::endl;
      return;
    }
    dqutils::MonitoringFile::merge_eventSample(*a1, *b1);
  }

  void merge_TEfficency(TObject * a, const TObject* b) {
    TEfficiency* a1 = dynamic_cast<TEfficiency*>(a);
    const TEfficiency* b1 = dynamic_cast<const TEfficiency*>(b);
    TEfficiency* b2 = const_cast<TEfficiency*>(b1);
    if (!a1 || !b1) {
      std::cout << "ERROR in merge_TEfficiency: Object not of type TEfficiency" << std::endl;
      return;
    }
    TList listE;
    listE.Add(b2);
    a1->Merge(&listE);
    listE.Clear();
  }

  void merge_TTree(TObject * a, const TObject* b) {
    TTree* a1 = dynamic_cast<TTree*>(a);
    const TTree* b1 = dynamic_cast<const TTree*>(b);
    if (!a1 || !b1) {
      std::cout << "ERROR in merge_TTree: Object not of type TTree" << std::endl;
      return;
    }
    TTree* b2 = const_cast<TTree*>(b1);
    TList listT;
    listT.Add(b2);
    a1->Merge(&listT);
    listT.Clear();
  }

  histCollection::histPerDir_t::histPerDir_t(const std::string& nameIn, std::unique_ptr<TObject>&& objIn, TTree* mdTree)
      : name(nameIn), obj(std::move(objIn)), mergeMethod(nullptr) {
    // Some sanity checks:
    if (!obj) {
      std::cout << "ERROR while adding " << nameIn << ": Histogram pointer is NULL" << std::endl;
      return;
    }

    if (mdTree) {
      fillMD(mdTree);
    }
    else {
      s_dbg(VERBOSE,"No matadata found for " + name +", use defaults");
    }
    const std::string& howToMerge = metadata[2];
    s_dbg(VERBOSE, "Name: " + name + " mergeMethod=" + howToMerge);

    TH1* th1 = dynamic_cast<TH1*>(obj.get());
    TH2* th2 = dynamic_cast<TH2*>(obj.get());
    TEfficiency* teff = dynamic_cast<TEfficiency*>(obj.get());
    if (th1) {
      th1->SetDirectory(nullptr);  // Get ownership of this hist
      if (howToMerge == "<default>") {
        if (th2) {
          mergeMethod = &defaultMerge<TH2>;
        } else { //TH1 case
          mergeMethod = &defaultMerge<TH1>;
        }
      }
      else if (howToMerge == "weightedAverage" ||  howToMerge=="weightedAverage2D")
        mergeMethod = &weightedAverage;
      else if (howToMerge == "weightedEff")
        mergeMethod = &weightedEff;
      else if (howToMerge == "mergeRMS")
        mergeMethod = &mergeRMS;
      else if (howToMerge == "RMSpercentDeviation")
        mergeMethod = &RMSpercentDeviation;
      else if (howToMerge == "perBinEffPerCent")
        mergeMethod = &perBinEffPerCent;
      else if (howToMerge == "lowerLB")
        mergeMethod = &lowerLB;
      else if (howToMerge == "identical")
        if (th2) {
          mergeMethod = &identical<TH2>;
        } else { //TH1 case
          mergeMethod = &identical<TH1>;
        }
      else if ((howToMerge == "mergeRebinned") || (howToMerge == "merge"))
        mergeMethod = &merge_rebinned;
      else {
        std::cout << "ERROR: Unknown merging method (" << howToMerge << ") for object of type TH1 named " << nameIn << std::endl;
        obj.reset(nullptr);
      }
    }  // end if TH1
    else if (teff) {
      teff->SetDirectory(nullptr);
      if (howToMerge == "<default>")
        mergeMethod = &merge_TEfficency;
      else
        std::cout << "ERROR: Unknown merging method (" << howToMerge << ") for object of type TEfficiency named " << nameIn << std::endl;
    }  // end if TEfficiency
    else if (nullptr != dynamic_cast<TTree*>(obj.get())) {
      mergeMethod = &merge_TTree;
    } else {
      std::cout << "ERROR Object " << name << " has unkown type" << std::endl;
      obj.reset(nullptr);
    }
  }

  void histCollection::addDirectory(TDirectory * dir, const std::string& dirName, const std::string& filename) {

    s_dbg(VERBOSE, "Working on directory " + dirName);
    if (m_dirExclusion && !std::regex_search(dirName, *m_dirExclusion)) {
      s_dbg(DEBUG, "Path " + dirName + " is excluded");
      return;
    }

    for (TObject* oKey : *dir->GetListOfKeys()) {
      TKey* key = static_cast<TKey*>(oKey);
      const std::string name = key->GetName();
      const std::string classname = key->GetClassName();
      if ((classname == "TTree") && (name == "metadata")) {
        continue;
      }

      s_dbg(VERBOSE, "Found name " + name + ", classname=" + classname);

      const std::string newName = dirName.empty() ? name : dirName + "/" + name;
      auto itDir = m_data.find(dirName);

      if (classname.starts_with("TH") || classname.starts_with("TProfile") || classname.starts_with("TEfficiency") || classname == "TTree") {
        if (m_histExclusion && !std::regex_search(name, *m_histExclusion)) {
          s_dbg(DEBUG, "Histogram with name " + name + " is excluded");
          continue;
        }

        // arrive here if we have at least one histogram in this directory
        if (m_skipExisting) {
          // Check if this object exists already in the output-file
          std::unique_ptr<TObject> existingObj(m_out->Get(newName.c_str()));
          if (existingObj)
            continue;
        }

        std::unique_ptr<TTree> md;
        if (itDir == m_data.end()) {
          // Have not seen this dirName yet
          itDir = m_data.emplace(dirName, histDir_t()).first;
          s_dbg(VERBOSE, "Registering new directory " + dirName);
        }

        // Check if we already have this histogram in the list
        auto itH = itDir->second.histos.find(name);
        if (itH == itDir->second.histos.end()) {
          // New histogram (or Tree):
          if (!md) {
            // Metadata tree not yet read in this directory
            md.reset((TTree*)dir->Get("metadata"));
          }
	  
          std::unique_ptr<TObject> obj{key->ReadObj()};
          TTree* treeObj = dynamic_cast<TTree*>(obj.get());
          if (treeObj) {
            TDirectory* outDir = m_out->GetDirectory(dirName.c_str());
            if (!outDir)
              outDir = m_out->mkdir(dirName.c_str());
            // TTree need special treatment ...
            TDirectory* currentDir = gDirectory;
            outDir->cd();
            TTree* cloneTree = treeObj->CloneTree();
            // this disconnects parent tree
            obj.reset(cloneTree);
            currentDir->cd();
          }
          histPerDir_t histo(name, std::move(obj), md.get());
          itH = itDir->second.histos.emplace(name, std::move(histo)).first;  //Take owernship of object here!
          s_dbg(VERBOSE, "Cloning histogram " + name + " in dir " + dirName);
        } else {
          // Histogram already known .. merge it
          std::unique_ptr<TObject> other(key->ReadObj());
          if (!other) {
            std::cout << "ERROR, got NULL key";
          } else {
            itH->second.merge(other.get());  // Release object in this case
            s_dbg(VERBOSE, "Merging histogram " + name + " in dir " + dirName);
          }
        }
      } else if (classname.starts_with("TDirectory")) {
        std::unique_ptr<TObject> obj(key->ReadObj());
        TDirectory* subdir = dynamic_cast<TDirectory*>(obj.get());
        if (subdir) {
          if (filename.empty()) {
            this->addDirectory(subdir, newName, filename);
          } else {
            if (!name.starts_with("lb_") && !name.starts_with("lowStat_LB")) {
              this->addDirectory(subdir, newName, filename);
            } else {
              m_fileLBMap[newName].push_back(filename);
            }
          }
        }
      } else {
        std::cout << "Ignored objects '" << name << "' of type " << classname << std::endl;
      }
    }
    return;
  }

  void histCollection::write() {
    unsigned nWritten = 0;
    unsigned nIgnored = 0;
    unsigned nDirs = 0;
    for (auto& it : m_data) {
      const std::string fulldir = it.first;
      TDirectory* histDir = m_out->GetDirectory(fulldir.c_str());
      if (histDir == nullptr) {  // Create the directory if it doesn't exist yet
        histDir = m_out->mkdir(fulldir.c_str());
        if (histDir == nullptr) {
          std::cout << "ERROR, failed to create directory " << fulldir << std::endl;
          break;
        } else {
          s_dbg(VERBOSE, "Created directory " + fulldir + " in file " + m_out->GetName());
        }
      }
      m_out->cd(fulldir.c_str());
      ++nDirs;
      for (auto& [name, histo] : it.second.histos) {
        if (histo.obj) {
          histo.obj->Write();
          ++nWritten;
        } else {
          std::cout << "NOT writing " << name << ". Invalid." << std::endl;
          ++nIgnored;
        }
      }  // End loop over histograms in one directory
      it.second.writeMD(histDir);
    }  // End loop over directories;
    std::cout << "Wrote " << nWritten << " histograms to " << nDirs << " directories in output file " << m_out->GetName() << std::endl;
    if (nIgnored)
      std::cout << " Omitting " << nIgnored << " histograms." << std::endl;
  }

  // *********************************************************************
  // Public Methods
  // *********************************************************************

  MonitoringFile::OutputMetadata::OutputMetadata(TTree * metadata) : m_metadata(metadata) {
    makeBranch("Name", "Name/C");
    makeBranch("Interval", "Interval/C");
    makeBranch("TriggerChain", "TriggerChain/C");
    makeBranch("MergeMethod", "MergeMethod/C");
  }

  void MonitoringFile::OutputMetadata::makeBranch(const char* branchName, const char* branchstr) {
    if (!m_metadata->GetBranch(branchName)) {
      m_metadata->Branch(branchName, (void*)nullptr, branchstr);
    }
  }

  void MonitoringFile::OutputMetadata::fill(const std::string& theName, const std::string& theInterval, const std::string& theChain,
                                            const std::string& theMerge) {
    std::string name = theName;
    std::string interval = theInterval;
    std::string chain = theChain;
    std::string merge = theMerge;
    m_metadata->SetBranchAddress("Name", name.data());
    m_metadata->SetBranchAddress("Interval", interval.data());
    m_metadata->SetBranchAddress("TriggerChain", chain.data());
    m_metadata->SetBranchAddress("MergeMethod", merge.data());
    m_metadata->Fill();
  }

  MonitoringFile::MonitoringFile() : m_file(0) {
    m_fileCompressionLevel = 1;
    m_doTiming = false;
    MonitoringFile::clearData();
  }

  bool MonitoringFile::setFile(const std::string& fileName) {
    clearData();
    m_file = TFile::Open(fileName.c_str());
    if (m_file != 0)
      return true;
    return false;
  }

  MonitoringFile::MonitoringFile(const std::string& fileName) : m_file(0) {
    m_fileCompressionLevel = 1;
    m_doTiming = false;
    MonitoringFile::clearData();
    MonitoringFile::setFile(fileName);
  }

  MonitoringFile::~MonitoringFile() {
    dqi::DisableMustClean disabled;

    delete m_file;
  }

  bool MonitoringFile::setHistogramRegEx(const std::string& re) {
    m_mergeMatchHistoRE = checkRegEx(re);
    return m_mergeMatchHistoRE.has_value();
  }

  bool MonitoringFile::setDirectoryRegEx(const std::string& re) {
    m_mergeMatchDirRE = checkRegEx(re);
    return m_mergeMatchDirRE.has_value();
  }

  void MonitoringFile::getAllDirs(DirMap_t & dirmap, TDirectory * dir, const std::string& dirName) {
    if (dir == 0)
      return;

    if (dirName != "") {
      DirMap_t::value_type dirmapVal(dirName, dir);
      dirmap.insert(dirmapVal);
    }

    TIter next(dir->GetListOfKeys());
    TKey* key;
    while ((key = dynamic_cast<TKey*>(next())) != 0) {
      // don't delete TDirectories
      TObject* obj = key->ReadObj();
      TDirectory* subdir = dynamic_cast<TDirectory*>(obj);
      if (subdir != 0) {
        std::string subdirName(subdir->GetName());
        std::string fName("");
        if (dirName != "") {
          fName += dirName;
          fName += '/';
        }
        fName += subdirName;
        getAllDirs(dirmap, subdir, fName);
      } else {
        delete obj;
      }
    }
  }

  TDirectory* MonitoringFile::createDir(DirMap_t & dirmap, TDirectory * dir, const std::string& parent, const std::string& path) {
    if (dir == 0)
      return 0;

    TDirectory* subdir(0);
    DirMap_t::const_iterator diter;
    std::string::size_type i = path.find_first_of('/');
    std::string fName("");
    if (parent != "") {
      fName += parent;
      fName += '/';
    }

    if (i != std::string::npos) {
      std::string dName(path, 0, i);
      std::string pName(path, i + 1, std::string::npos);
      fName += dName;
      if (dName != "") {
        diter = dirmap.find(fName);
        if (diter != dirmap.end()) {
          subdir = diter->second;
        } else {
          subdir = dir->mkdir(dName.c_str());
          DirMap_t::value_type dirmapVal(fName, subdir);
          dirmap.insert(dirmapVal);
        }
      } else {
        subdir = dir;
      }
      return createDir(dirmap, subdir, fName, pName);
    }

    fName += path;

    diter = dirmap.find(fName);
    if (diter != dirmap.end()) {
      return diter->second;
    }

    subdir = dir->mkdir(path.c_str());
    DirMap_t::value_type dirmapVal(fName, subdir);
    dirmap.insert(dirmapVal);
    return subdir;
  }

  TKey* MonitoringFile::getObjKey(TDirectory * dir, const std::string& path) {
    if (dir == 0)
      return 0;

    TKey* key(0);

    std::string::size_type i = path.find_first_of('/');
    if (i != std::string::npos) {
      std::string dName(path, 0, i);
      std::string pName(path, i + 1, std::string::npos);
      if (dName != "") {
        key = dir->FindKey(dName.c_str());
        if (key != 0) {
          TDirectory* subDir = dynamic_cast<TDirectory*>(key->ReadObj());
          if (subDir) {
            return getObjKey(subDir, pName);
          }  // else fall through
        }
        return 0;
      }
      return getObjKey(dir, pName);
    }

    return dir->FindKey(path.c_str());
  }

  void MonitoringFile::fillMetaDataMap(std::map<std::string, dqutils::MonitoringFile::MetaData> & mdMap, TDirectory * dir) {
    if (dir == 0)
      return;
    TTree* md = dynamic_cast<TTree*>(dir->Get("metadata"));
    if (md == 0)
      return;

    TTreeReader reader(md);
    TTreeReaderArray<char> i_name(reader, "Name");
    TTreeReaderArray<char> i_interval(reader, "Interval");
    TTreeReaderArray<char> i_chain(reader, "TriggerChain");
    TTreeReaderArray<char> i_merge(reader, "MergeMethod");

    while (reader.Next()) {
      const std::string nameStr(static_cast<char*>(i_name.GetAddress()));
      if (mdMap.find(nameStr) == mdMap.end()) {
        MetaData md(nameStr, static_cast<char*>(i_interval.GetAddress()), static_cast<char*>(i_chain.GetAddress()), static_cast<char*>(i_merge.GetAddress()));
        std::map<std::string, MetaData>::value_type mdVal(nameStr, md);
        mdMap.insert(mdVal);
      }
    }

    delete md;
  }

  int MonitoringFile::mergeFiles(const std::string& outFileName, const std::vector<std::string>& files) {
    std::cout << "Writing file: " << outFileName << std::endl;
    std::cout << "Start merging [" << files.size() << "] histogram files" << std::endl;
    dqi::DisableMustClean disabled;
    TH1::AddDirectory(false);
    if (m_mergeMatchDirRE.has_value() || m_mergeMatchHistoRE.has_value()) {
      std::cout << " ========== Using regular expressions for selective merging ========== " << std::endl;
    }
    if (m_doTiming) {
      std::cout << "CPU time measurement activated " << std::endl;
    }

    const size_t nFiles = files.size();

    if (nFiles < 1)
      return -1;

    if (nFiles == 1) {
      std::cout << "Got exactly one input file. Will copy input -> output" << std::endl;
      if (m_mergeMatchDirRE.has_value() || m_mergeMatchHistoRE.has_value()) {
        std::cout << "regular expressions for selective merging will have no effect!" << std::endl;
      }

      std::filesystem::path inPath(files[0]);
      std::filesystem::path outPath(outFileName);
      std::filesystem::copy_file(inPath, outPath, std::filesystem::copy_options::overwrite_existing);
      return 0;
    }

    std::unique_ptr<TFile> outfile(TFile::Open(outFileName.c_str(), "RECREATE", outFileName.c_str(), m_fileCompressionLevel));
    if (outfile.get() == 0) {
      std::cout << " ERROR, cound not open output file " << outFileName << std::endl;
      return -1;
    }
    std::cout << "Opened/created output file " << outFileName << std::endl;


    auto TFiledel = [](TFile* f) {
        f->Delete("");
        f->Close();
        delete f;
      };

    histCollection hc(outfile.get());
    hc.addDirExclusion(m_mergeMatchDirRE);
    hc.addHistExclusion(m_mergeMatchHistoRE);

    // Open first input file, mostly to get the run-directory
    std::unique_ptr<TFile,decltype(TFiledel)> in1(TFile::Open(files[0].c_str()));
    if (!in1) {
      std::cout << "ERROR, could not open input file " << files[0] << std::endl;
      return -1;
    }
    std::cout << "Working on file 1/" << nFiles << ": " << files[0] << std::endl;
    std::string runDir, runDirFwd;
    const std::regex runDirPattern("run_[0-9]*");
    TIter next(in1->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
      const char* name = key->GetName();
      if (std::regex_match(name, runDirPattern)) {
        if (runDir.size() > 0) {
          std::cout << "ERROR More than one run_XXX directory found! Ignoring " << name << std::endl;
        } else
          runDir = name;
      }
    }
    if (runDir.empty()) {
      std::cout << "No run-directory found, start with '/'" << std::endl;
      runDir="/";
      runDirFwd="";
    }
    else {
      std::cout << "Found run directory " << runDir << std::endl;
      runDirFwd=runDir;
    }

    
    TDirectory* dir(dynamic_cast<TDirectory*>(in1->GetDirectory(runDir.c_str())));
    if (!dir) {
      std::cout << "ERROR, can't access directory  " << runDir;
      return -1;
    }

    hc.addDirectory(dir, runDirFwd, files[0]);

    // Close first input file
    in1.reset(nullptr);

    for (size_t i = 1; i < files.size(); ++i) {
      std::cout << "Working on file " << 1+i << "/" << nFiles << ": " << files[i] << std::endl;
      std::unique_ptr<TFile,decltype(TFiledel)> in(TFile::Open(files[i].c_str()));
      if (!in) {
        std::cout << "ERROR, could not open input file " << files[i] << std::endl;
        return -1;
      }
      TDirectory* dir(dynamic_cast<TDirectory*>(in->GetDirectory(runDir.c_str())));
      if (not dir){
        std::cout << "ERROR, could not cast to directory" << std::endl;
        return -1;
      }
      hc.addDirectory(dir, runDirFwd, files[i]);
    }

    std::cout << "Accumulated a total of " << hc.size() << " histograms." << std::endl;

    std::cout << "Start writing output ..." << std::endl;
    hc.write();

    if (m_doTiming) {
      std::cout << "CPU time for histogram merging: (regular histograms)" << std::endl;
      hc.printTiming();
    }
    const auto lbmap = hc.getFileLBMapAndClear();
    if (!lbmap.empty()) {
      std::cout << "Start merging lb_nnn and lowStat_LB directories (" << lbmap.size() << " in total)" << std::endl;
      histCollection hclb(outfile.get());
      hclb.addDirExclusion(m_mergeMatchDirRE);
      hclb.addHistExclusion(m_mergeMatchHistoRE);

      // Sort lb/file list by file-name to avoid re-oping the same files:
      // Copy map to vector<pair> ...
      std::vector<std::pair<std::string, std::vector<std::string>>> lbToFiles;
      for (const auto& val : lbmap) {
        if (val.second.size() > 0)
          lbToFiles.emplace_back(val);
      }

      //..and sort the vector
      std::sort(lbToFiles.begin(), lbToFiles.end(),
                [](const decltype(lbToFiles)::value_type& a, const decltype(lbToFiles)::value_type& b) { return a.second[0] < b.second[0]; });

      size_t counter = 0;
      std::unique_ptr<TFile, decltype(TFiledel)> in;
      for (const auto& [dir, filenames] : lbToFiles) {
        std::cout << "Merging/copying directory " << dir << " from " << filenames.size() << "input file(s) (" << ++counter << "/" << lbToFiles.size() << ")"
                  << std::endl;
        for (const std::string& fName : filenames) {
          if (!in || strcmp(in->GetName(), fName.c_str()) != 0) {
            in.reset(TFile::Open(fName.c_str()));
            s_dbg(DEBUG, "Opening input file " + fName);
          } else {
            s_dbg(DEBUG, "Input file " + fName + " already open");
          }

          if (!in) {
            std::cout << "ERROR, could not open input file " << fName << std::endl;
            return -1;
          }
          TDirectory* tDir = (dynamic_cast<TDirectory*>(in->Get(dir.c_str())));
          if (!tDir) {
            std::cout << "ERROR, failed to get directory " << dir << " from file " << fName << std::endl;
          } else {
            hclb.addDirectory(tDir, dir);
          }
        }  // end loop over filenames
        hclb.write();
        if (m_doTiming) {
          std::cout << "CPU time for histogram merging: (lumiblock-histograms)" << std::endl;
          hclb.printTiming();
        }
        hclb.clear();
      }  // end loop over lbmap
      in.reset(nullptr);
    }
    outfile->Close();
    return 0;
  }

  int MonitoringFile::mergeFiles(const std::string& outFileName, const std::string& listFileName) {
    typedef std::vector<std::string> FileList_t;

    const unsigned int nFilesAtOnce = 50;

    FileList_t allFiles;
    bool success = setListFromFile(allFiles, listFileName);
    if (!success) {
      std::cout << "ERROR Failed ot read list of input files" << std::endl;
      return -1;
    }

    if (allFiles.size() <= nFilesAtOnce) {
      return mergeFiles(outFileName, allFiles);
    }

    FileList_t procFiles, tmpIntermediateFiles;

    FileList_t::const_iterator filesEnd = allFiles.end();
    FileList_t::const_iterator fi = allFiles.begin();

    unsigned int counter = 0;
    std::string tmpInputFile("");
    std::string tmpOutputFile("");

    // new logic: merge intermediately, then merge intermediate files
    while (fi != filesEnd) {

      procFiles.push_back(*fi);
      ++counter;
      ++fi;
      if (counter % nFilesAtOnce == 0 || fi == filesEnd) {
        std::ostringstream nameStream;
        nameStream << "tmp_merge_" << counter << ".root";
        tmpOutputFile = nameStream.str();
        tmpIntermediateFiles.push_back(tmpOutputFile);
        int stat=mergeFiles(tmpOutputFile, procFiles);
        if (stat) return stat;
        procFiles.clear();
      }
    }

    int stat=mergeFiles(outFileName, tmpIntermediateFiles);
    if (stat) return stat;

    for (const auto& tmpFile : tmpIntermediateFiles) {
      std::remove(tmpFile.c_str());
    }
    return 0;
  }

  void MonitoringFile::printStatistics() {
    if (m_file == 0) {
      std::cerr << "MonitoringFile::printStatistics(): "
                << "No input file is open\n";
      return;
    }

    DirMap_t indirmap;

    getAllDirs(indirmap, m_file, "");

    DirMap_t::const_iterator idirend = indirmap.end();
    for (DirMap_t::const_iterator idir = indirmap.begin(); idir != idirend; ++idir) {
      std::string idirName = idir->first;

      GatherStatistics stat_shift(idirName);
      GatherStatistics stat_all(idirName);

      loopOnHistogramsInMetadata(stat_shift, idir->second);
      loopOnHistograms(stat_all, idir->second);

      std::cout.setf(std::ios_base::left, std::ios_base::adjustfield);
      std::cout.width(80);
      std::cout << idirName << "  ";

      std::cout.setf(std::ios_base::right, std::ios_base::adjustfield);
      std::cout << "  shift: ";
      std::cout.width(3);
      std::cout << stat_shift.m_nHist1D << " ";
      std::cout.width(5);
      std::cout << stat_shift.m_nHist1DBins << " ";
      std::cout.width(3);
      std::cout << stat_shift.m_nHist2D << " ";
      std::cout.width(7);
      std::cout << stat_shift.m_nHist2DBins << " ";
      std::cout.width(3);
      std::cout << stat_shift.m_nGraph << " ";
      std::cout.width(5);
      std::cout << stat_shift.m_nGraphPoints << "   ";

      std::cout << "  all: ";
      std::cout << stat_all.m_nHist1D << " ";
      std::cout.width(5);
      std::cout << stat_all.m_nHist1DBins << " ";
      std::cout.width(3);
      std::cout << stat_all.m_nHist2D << " ";
      std::cout.width(7);
      std::cout << stat_all.m_nHist2DBins << " ";
      std::cout.width(3);
      std::cout << stat_all.m_nGraph << " ";
      std::cout.width(5);
      std::cout << stat_all.m_nGraphPoints << "\n";

      std::cout << std::flush;
    }
  }

  bool MonitoringFile::copyHistograms(const std::string& outFileName, const std::string& dirName) {
    dqi::DisableMustClean disabled;
    //  bool useRecursiveDelete = gROOT->MustClean();
    //  gROOT->SetMustClean(false);

    if (m_file == 0) {
      std::cerr << "MonitoringFile::copyHistograms(): "
                << "No input file is open\n";
      return false;
    }

    DirMap_t indirmap;
    DirMap_t reducedmap;
    DirMap_t outdirmap;

    if (dirName != "all") {
      TKey* dkey = getObjKey(m_file, dirName);
      if (dkey == 0) {
        std::cerr << "MonitoringFile::copyHistograms(): "
                  << "Directory \'" << dirName << "\' not found in input file\n";
        return false;
      }

      TDirectory* fromDir = dynamic_cast<TDirectory*>(dkey->ReadObj());

      DirMap_t::value_type dirmapVal(dirName, fromDir);
      indirmap.insert(dirmapVal);
    } else {
      std::cout << "Building list of all TDirectories in file...\n" << std::flush;
      getAllDirs(indirmap, m_file, "");
    }

    DirMap_t::const_iterator idirend = indirmap.end();
    for (DirMap_t::const_iterator idir = indirmap.begin(); idir != idirend; ++idir) {

      std::string idirName = idir->first;
      std::cout << "Checking " << idirName << "\n" << std::flush;
      // std::string::size_type j = idirName.find( "L1Calo/1_PPr_EmFADCTiming" );
      // if( j != std::string::npos ) {
      //   std::cerr << "Skipping directory \"" << idirName << "\"\n";
      //   std::cerr << std::flush;
      //   continue;
      // }

      if (!dirHasHistogramsInMetadata(idir->second)) {
        continue;
      }

      reducedmap.insert(*idir);
    }

    std::unique_ptr<TFile> outfile(TFile::Open(outFileName.c_str(), "RECREATE", outFileName.c_str(), m_fileCompressionLevel));
    if (outfile.get() == 0) {
      std::cerr << "MonitoringFile::copyHistograms(): "
                << "Output file not opened\n";
      return false;
    }

    idirend = reducedmap.end();
    for (DirMap_t::const_iterator idir = reducedmap.begin(); idir != idirend; ++idir) {

      std::string idirName = idir->first;
      std::cout << "Processing " << idirName << "\n" << std::flush;

      TDirectory* toDir = createDir(outdirmap, outfile.get(), "", idirName);
      if (toDir == 0) {
        std::cerr << "MonitoringFile::copyHistograms(): "
                  << "Directory \'" << idirName << "\' not created in output file\n";
        return false;
      }

      CopyHistogram copyFcn(toDir, idirName);

      loopOnHistogramsInMetadata(copyFcn, idir->second);
    }

    outfile->Write();
    outfile->Close();

    //  gROOT->SetMustClean(useRecursiveDelete);
    return true;
  }

  std::string MonitoringFile::getHanResults(const std::string& hanResultsDir, const std::string& input, const std::string& hcfg,
                                            const std::string& hcfg_lowStat, const std::string& hcfg_medStat) {
    // DisableMustClean disabled;

    std::cout << "\nUsing han configurations:\n"
              << "  entire run: " << hcfg << "\n"
              << "  low stat interval: " << hcfg_lowStat << "\n"
              << "  medium stat interval: " << hcfg_medStat << "\n\n"
              << std::flush;

    TFile* infile = TFile::Open(input.c_str());
    if (infile == 0) {
      std::cerr << "MonitoringFile::getHanResults(): "
                << "Cannot open input file \"" << input << "\"\n";
      return "";
    }

    std::vector<std::string> run_dirs;
    std::vector<std::string> lowStat_dirs;
    std::vector<std::string> medStat_dirs;

    TIter next_run(infile->GetListOfKeys());
    TKey* key_run(0);
    while ((key_run = dynamic_cast<TKey*>(next_run())) != 0) {
      TObject* obj_run = key_run->ReadObj();
      TDirectory* tdir_run = dynamic_cast<TDirectory*>(obj_run);
      if (tdir_run != 0) {
        std::string tdir_run_name(tdir_run->GetName());
        if (tdir_run_name.find("run") != std::string::npos) {
          run_dirs.push_back(tdir_run_name);
          TIter next_minutes(tdir_run->GetListOfKeys());
          TKey* key_minutes(0);
          while ((key_minutes = dynamic_cast<TKey*>(next_minutes())) != 0) {
            TObject* obj_minutes = key_minutes->ReadObj();
            TDirectory* tdir_minutes = dynamic_cast<TDirectory*>(obj_minutes);
            if (tdir_minutes != 0) {
              std::string tdir_minutes_name(tdir_minutes->GetName());
              if (tdir_minutes_name.find("lowStat") != std::string::npos) {
                lowStat_dirs.push_back(tdir_run_name + '/' + tdir_minutes_name);
              } else if (tdir_minutes_name.find("medStat") != std::string::npos) {
                medStat_dirs.push_back(tdir_run_name + '/' + tdir_minutes_name);
              }
            }
            delete obj_minutes;
          }
        }
      }
      delete obj_run;
    }

    delete infile;

    dqi::HanApp han;

    std::string fileList = " ";
    std::vector<std::string>::const_iterator dirs_end;
    std::vector<std::string>::const_iterator dir;

    dirs_end = run_dirs.end();
    for (dir = run_dirs.begin(); dir != dirs_end; ++dir) {
      const std::string& tdir_run_name = *dir;
      std::string han_output_run = hanResultsDir + '/' + tdir_run_name + "_han.root";
      std::cout << "Calling han( " << hcfg << ", " << input << ", " << tdir_run_name << ", " << han_output_run << " ):\n" << std::flush;
      han.Analyze(hcfg, input, han_output_run, tdir_run_name);
      std::cout << "\n";
      fileList += han_output_run + " " + tdir_run_name + "\n";
    }

    dirs_end = lowStat_dirs.end();
    for (dir = lowStat_dirs.begin(); dir != dirs_end; ++dir) {
      const std::string& tdir_minutes_path = *dir;

      std::string tdir_minutes_underscore = tdir_minutes_path;
      std::string::size_type tdir_minutes_i = tdir_minutes_underscore.find('/');
      tdir_minutes_underscore.replace(tdir_minutes_i, 1, "_");

      std::string han_output_lowStat = hanResultsDir + '/' + tdir_minutes_underscore + "_han.root";
      std::cout << "Running han, writing to " << han_output_lowStat << ":\n" << std::flush;
      han.Analyze(hcfg_lowStat, input, han_output_lowStat, tdir_minutes_path);
      std::cout << "\n";
      std::string subdirname(tdir_minutes_path, tdir_minutes_i + 1, std::string::npos);
      std::string dirname(tdir_minutes_path, 0, tdir_minutes_i);
      fileList += han_output_lowStat + " " + subdirname + " " + dirname + " " + subdirname + "\n";
    }

    dirs_end = medStat_dirs.end();
    for (dir = medStat_dirs.begin(); dir != dirs_end; ++dir) {
      const std::string& tdir_minutes_path = *dir;

      std::string tdir_minutes_underscore = tdir_minutes_path;
      std::string::size_type tdir_minutes_i = tdir_minutes_underscore.find('/');
      tdir_minutes_underscore.replace(tdir_minutes_i, 1, "_");

      std::string han_output_medStat = hanResultsDir + '/' + tdir_minutes_underscore + "_han.root";
      std::cout << "Running han, writing to " << han_output_medStat << ":\n" << std::flush;
      han.Analyze(hcfg_medStat, input, han_output_medStat, tdir_minutes_path);
      std::cout << "\n";
      std::string subdirname(tdir_minutes_path, tdir_minutes_i + 1, std::string::npos);
      std::string dirname(tdir_minutes_path, 0, tdir_minutes_i);
      fileList += han_output_medStat + " " + subdirname + " " + dirname + " " + subdirname + "\n";
    }

    return fileList;
  }

  void MonitoringFile::printHanConfig() {
    if (m_file == 0) {
      std::cerr << "MonitoringFile::printHanConfig(): "
                << "No input file is open\n";
      return;
    }

    DirMap_t indirmap;

    getAllDirs(indirmap, m_file, "");

    std::string indent, indent_p, indent_c;
    std::string idirName_p;
    DirMap_t::const_iterator idirend = indirmap.end();
    for (DirMap_t::const_iterator idir = indirmap.begin(); idir != idirend; ++idir) {
      std::string idirName = idir->first;
      std::string::size_type shortNameIndex = idirName.rfind('/');
      std::string shortName = idirName.substr(shortNameIndex + 1, std::string::npos);

      std::string::size_type fsIndex = idirName.find('/');
      std::string shortPath;
      if (fsIndex != shortNameIndex)
        shortPath = idirName.substr(fsIndex + 1, shortNameIndex);
      else
        shortPath = idirName.substr(fsIndex + 1, std::string::npos);

      std::cout << idirName << "\n";
      std::cout << shortPath << ", " << shortName << "\n";
      /*
      indent = getIndentation(idirName,"");
      if(int(indent.size())==in_p){
        std::cout << indent << "} \n";
        std::cout << indent << "dir "  << shortName << "  { \n";
        std::cout << indent << "  output " << idirName << "\n";
        std::cout << indent << "  hist all_in_dir  { \n " << indent << " } \n";
      }
      else if (int(indent.size()) > in_p){
        std::cout << indent << "dir "  << shortName << "  { \n";
        std::cout << indent << "  output " << idirName << "\n";
        std::cout << indent << "  hist all_in_dir  { \n " << indent << " } \n";
      }
      else{
        //find common part + number of common '/'
        std::string common = FindCommon(idirName,idirName_p);
        indent_c = getIndentation(common,"");
        int counter = (indent_p.size() - indent_c.size())/2;
        for (int i = counter; i>0; i--){
    std::string temp = indent_c;
    for (int j = 0; j< i; j++){
      temp+="  ";
    }
    std::cout << temp << "} \n" ;
        }
        std::cout << indent << "} \n";
        std::cout << indent << "dir "  << shortName << "  { \n";
        std::cout << indent << "  output " << idirName << "\n";
        std::cout << indent << "  hist all_in_dir  { \n " << indent << " } \n";
      }
      indent_p = indent;
      in_p = indent_p.size();
      idirName_p = idirName;
      */
    }
  }

  std::string MonitoringFile::getIndentation(const std::string& pathName, const std::string& leadingSpace) {
    std::string space = leadingSpace;
    std::string::size_type i = pathName.find_first_of('/');
    if (i != std::string::npos) {
      std::string subPath(pathName, i + 1, std::string::npos);
      space += "  ";
      return getIndentation(subPath, space);
    }
    return space;
  }

  std::string MonitoringFile::FindCommon(const std::string& name1, const std::string& name2) const {
    int length = (name1.size() < name2.size()) ? name1.size() : name2.size();
    bool found = true;
    int count = 0;
    while (found == true && count < length) {
      if (name1[count] == name2[count]) {
        count++;
      } else {
        found = false;
      }
    }
    return (name1.substr(0, count));
  }

  // *********************************************************************
  // Protected Methods
  // *********************************************************************

  MonitoringFile::CopyHistogram::CopyHistogram(TDirectory * target, const std::string& dirName) : m_target(target), m_dirName(dirName), m_metadata(0) {
    m_metadata = new TTree("metadata", "Monitoring Metadata");
    m_metadata->SetDirectory(0);
    m_metadata->Branch("Name", (void*)nullptr, "Name/C");
    m_metadata->Branch("Interval", (void*)nullptr, "Interval/C");
    m_metadata->Branch("TriggerChain", (void*)nullptr, "TriggerChain/C");
    m_metadata->Branch("MergeMethod", (void*)nullptr, "MergeMethod/C");
  }

  MonitoringFile::CopyHistogram::~CopyHistogram() {
    m_target->cd();
    m_metadata->SetDirectory(m_target);
    m_metadata->Write();
    delete m_metadata;
  }

  bool MonitoringFile::CopyHistogram::execute(TH1 * hist) {
    m_target->cd();
    hist->SetDirectory(m_target);
    hist->Write();

    return true;
  }

  bool MonitoringFile::CopyHistogram::execute(TGraph * graph) {
    m_target->cd();
    graph->Write();

    return true;
  }

  bool MonitoringFile::CopyHistogram::execute(TEfficiency * eff) {
    m_target->cd();
    eff->Write();
    return true;
  }

  void MonitoringFile::CopyHistogram::fillMD(const MetaData& md) {
    std::string name(md.name);
    std::string interval(md.interval);
    std::string chain(md.chain);
    std::string merge(md.merge);
    m_metadata->SetBranchAddress("Name", name.data());
    m_metadata->SetBranchAddress("Interval", interval.data());
    m_metadata->SetBranchAddress("TriggerChain", chain.data());
    m_metadata->SetBranchAddress("MergeMethod", merge.data());
    m_metadata->Fill();
  }

  bool MonitoringFile::CopyHistogram::executeMD(TH1 * hist, const MetaData& md) {
    m_target->cd();
    hist->SetDirectory(m_target);
    hist->Write();

    fillMD(md);

    return true;
  }

  bool MonitoringFile::CopyHistogram::executeMD(TGraph * graph, const MetaData& md) {
    m_target->cd();
    graph->Write();

    fillMD(md);

    return true;
  }

  bool MonitoringFile::CopyHistogram::executeMD(TEfficiency * eff, const MetaData& md) {
    m_target->cd();
    eff->Write();
    fillMD(md);
    return true;
  }

  MonitoringFile::GatherStatistics::GatherStatistics(const std::string& dirName)
      : m_dirName(dirName), m_nHist1D(0), m_nHist1DBins(0), m_nGraph(0), m_nGraphPoints(0), m_nHist2D(0), m_nHist2DBins(0) {}

  bool MonitoringFile::GatherStatistics::execute(TH1 * hist) {
    TH2* hist2d = dynamic_cast<TH2*>(hist);
    if (hist2d != 0) {
      ++m_nHist2D;
      m_nHist2DBins += (hist2d->GetNbinsX() * hist2d->GetNbinsY());
      return true;
    }
    ++m_nHist1D;
    m_nHist1DBins += hist->GetNbinsX();
    return true;
  }

  bool MonitoringFile::GatherStatistics::execute(TGraph * graph) {
    ++m_nGraph;
    m_nGraphPoints += graph->GetMaxSize();
    return true;
  }

  bool MonitoringFile::GatherStatistics::execute(TEfficiency * eff) {
    ++m_nEfficiency;

    TH1* h_total = eff->GetCopyPassedHisto();
    TH2* h_total2D = dynamic_cast<TH2*>(h_total);

    if (h_total2D != 0) {
      m_nEfficiencyBins += (h_total2D->GetNbinsX() * h_total2D->GetNbinsY());
      return true;
    } else {
      m_nEfficiencyBins += h_total->GetNbinsX();
      return true;
    }
  }

  MonitoringFile::GatherNames::GatherNames() {}

  bool MonitoringFile::GatherNames::execute(TH1 * hist) {
    m_names.push_back(std::string(hist->GetName()));
    return true;
  }

  bool MonitoringFile::GatherNames::execute(TGraph * graph) {
    m_names.push_back(std::string(graph->GetName()));
    return true;
  }

  bool MonitoringFile::GatherNames::execute(TEfficiency * eff) {
    m_names.push_back(std::string(eff->GetName()));
    return true;
  }

  void MonitoringFile::clearData() {
    dqi::DisableMustClean disabled;

    delete m_file;
    m_file = 0;
    m_fileCompressionLevel = 1;
    m_doTiming = false;
  }

  bool MonitoringFile::dirHasHistogramsInMetadata(TDirectory * dir) {
    dir->cd();

    TKey* mdKey = dir->FindKey("metadata");
    if (mdKey == 0) {
      return false;
    }

    TTree* md = dynamic_cast<TTree*>(mdKey->ReadObj());
    if (md == 0) {
      return false;
    }

    int counter = 0;
    int nEntries = int(md->GetEntries());

    while (counter < nEntries) {
      try {
        md->GetEntry(counter);
      } catch (const std::exception& e) {
        std::cerr << "Exception: \"" << e.what() << "\" in directory \"" << dir->GetName() << "\"\n" << std::flush;
        return false;
      }

      return true;
      ++counter;
    }

    return false;
  }

  void MonitoringFile::loopOnHistograms(HistogramOperation & fcn, TDirectory * dir) {
    TIter next(dir->GetListOfKeys());
    TKey* key;
    while ((key = dynamic_cast<TKey*>(next())) != 0) {
      TObject* obj = key->ReadObj();
      TH1* h(0);
      TGraph* g(0);
      TEfficiency* e(0);
      if ((h = dynamic_cast<TH1*>(obj))) {
        fcn.execute(h);
      } else if ((g = dynamic_cast<TGraph*>(obj))) {
        fcn.execute(g);
      } else if ((e = dynamic_cast<TEfficiency*>(obj))) {
        fcn.execute(e);
      }
      delete obj;
    }
  }

  bool MonitoringFile::loopOnHistogramsInMetadata(HistogramOperation & fcn, TDirectory * dir) {
    dir->cd();
    TKey* mdKey = dir->FindKey("metadata");
    if (mdKey == 0) {
      return false;
    }

    TTree* md = dynamic_cast<TTree*>(mdKey->ReadObj());
    if (md == 0) {
      return false;
    }

    TKey* i_key;

    TTreeReader reader(md);
    TTreeReaderArray<char> i_name(reader, "Name");
    TTreeReaderArray<char> i_interval(reader, "Interval");
    TTreeReaderArray<char> i_chain(reader, "TriggerChain");
    TTreeReaderArray<char> i_merge(reader, "MergeMethod");

    while (reader.Next()) {
      const std::string nameStr(static_cast<char*>(i_name.GetAddress()));
      dir->cd();
      i_key = dir->FindKey(static_cast<char*>(i_name.GetAddress()));
      if (i_key == 0) {
        std::cerr << "MonitoringFile::loopOnHistogramsInMetadata(): "
                  << "No \'" << nameStr << "\' object found\n";
        return false;
      }
      MetaData md(nameStr, static_cast<char*>(i_interval.GetAddress()), static_cast<char*>(i_chain.GetAddress()), static_cast<char*>(i_merge.GetAddress()));
      TObject* obj = i_key->ReadObj();
      TH1* h = dynamic_cast<TH1*>(obj);
      if (h != 0) {
        fcn.executeMD(h, md);
      } else {
        TGraph* g = dynamic_cast<TGraph*>(obj);
        if (g != 0) {
          fcn.executeMD(g, md);
        }
      }
      delete obj;
    }

    delete md;

    return true;
  }

  bool MonitoringFile::setListFromFile(std::vector<std::string> & filelist, const std::string& listFileName) {
    using namespace std;

    filelist.clear();

    ifstream listfile(listFileName.c_str());
    if (!listfile) {
      cerr << "MonitoringFile::setListFromFile(): "
           << "cannot read from file: " << listFileName << "\n";
      return false;
    }

    string line;
    char c;
    string filename;
    while (getline(listfile, line)) {
      istringstream linestream(line);
      while (linestream.get(c)) {
        if (!isspace(c)) {
          // ignore comments
          if (c == '#') {
            break;
          }

          linestream.putback(c);
          linestream >> filename;
          if (!linestream) {
            cerr << "MonitoringFile::setListFromFile(): "
                 << "badly formatted line: " << line << "\n";
            break;
          }

          filelist.push_back(filename);
        }
      }
    }

    return true;
  }

  int MonitoringFile::mergeLBintervals(const std::string& inFilename) {

    std::cout << "Running mergeLBintervals on " << inFilename << std::endl;

    std::unique_ptr<TFile> f(TFile::Open(inFilename.c_str(), "UPDATE"));
    if (!f) {
      std::cout << "ERROR, could not open file " << inFilename << " for update" << std::endl;
      return -1;
    }
    std::string runDirName;
    const std::regex runDirPattern("run_[0-9]*");
    TIter next(f->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
      const char* name = key->GetName();
      if (std::regex_match(name, runDirPattern)) {
        if (runDirName.size() > 0) {
          std::cout << "ERROR More than one run_XXX directory found! Ignoring " << name << std::endl;
        } else
          runDirName = name;
      }
      break;
    }

    TDirectory* runDir = f->GetDirectory(runDirName.c_str());
    const auto mapping = buildLBToIntervalMap(runDir);

    if (s_dbg.getLvl() == VERBOSE) {
      std::cout << "LB directory mapping:" << std::endl;
      for (const auto& i1 : mapping) {
        std::cout << i1.first;
        for (const auto& i2 : i1.second) {
          std::cout << "\t" << i2 << std::endl;
        }
      }
    }

    for (const auto& [outDir, inDIrs] : mapping) {
      int stat=mergeLB_processLBinterval(f.get(), inDIrs, outDir);
      if (stat) return stat;
     }

    f->Close();
    f.reset(TFile::Open(inFilename.c_str(), "UPDATE"));
    runDir = f->GetDirectory(runDirName.c_str());

    std::cout << "merging lowStat_LB dirs into run-dir" << std::endl;
    std::vector<std::string> lowStatDirs;
    for (TObject* oKey : *runDir->GetListOfKeys()) {
      TKey* key = static_cast<TKey*>(oKey);
      const std::string name = key->GetName();
      const std::string classname = key->GetClassName();
      if (classname.starts_with("TDirectory") and name.starts_with("lowStat_LB")) {
        lowStatDirs.push_back(runDirName + "/" + name);
        s_dbg(VERBOSE, "Found input: " + runDirName + "/" + name);
      }
    }

    int stat=mergeLB_processLBinterval(f.get(), lowStatDirs, runDirName);

    f->Close();
    return stat;
  }

  std::map<std::string, std::vector<std::string>> MonitoringFile::buildLBToIntervalMap(TDirectory * runDir) {

    std::map<std::string, std::vector<std::string>> ranges;

    // No recusion here, everything we care out is run_NNNNN/lb_nnnn (and run_NNNNN/lowStat_nn-mm)
    const std::string runDirName = runDir->GetName();
    for (TObject* oKey : *runDir->GetListOfKeys()) {
      TKey* key = static_cast<TKey*>(oKey);
      const std::string name = key->GetName();
      const std::string classname = key->GetClassName();
      if (!classname.starts_with("TDirectory"))
        continue;
      if (name.starts_with("lb_")) {
        unsigned lumiBlock = 0;
        try {
          lumiBlock = std::stol(name.substr(3));
        } catch (std::invalid_argument& e) {
          std::cout << "ERROR, unexpected directory name " << name << ". Can't parse lb number" << std::endl;
          std::cout << e.what() << std::endl;
          continue;
        }
        // Copied from Control/AthenaMonitoringKernel/src/HistogramFiller/OfflineHistogramProvider.h
        const unsigned lbBase = lumiBlock - (((int64_t)lumiBlock - 1) % 20);
        const std::string lbString = runDirName + "/lowStat_LB" + std::to_string(lbBase) + "-" + std::to_string(lbBase + 19);
        ranges[lbString].push_back(runDirName + "/" + name);
      }  // end if lb_NNNN dir
    }  // end loop over directories under run_NNNNN
    return ranges;
  }

  int MonitoringFile::mergeLB_processLBinterval(TFile * file, const std::vector<std::string>& inputDirNames, const std::string& outputDirName) {

    TDirectory* outDir = file->GetDirectory(outputDirName.c_str());
    if (!outDir) {
      outDir = file->mkdir(outputDirName.c_str());
    }
    if (!outDir) {
      std::cout << "ERROR, can't obtain nor create directory " << outputDirName << " in file " << file->GetName() << std::endl;
      return -1;
    }

    histCollection hc(file, true);
    hc.addDirExclusion(m_mergeMatchDirRE);
    hc.addHistExclusion(m_mergeMatchHistoRE);

    for (const std::string& inDirName : inputDirNames) {
      TDirectory* inDir = file->GetDirectory(inDirName.c_str());
      hc.addDirectory(inDir, outputDirName);
    }
    if (hc.size() == 0) {
      std::cout << "mergeLB_processLBinterval: No new objects found for " << outputDirName << std::endl;
    } else {
      hc.write();
    }
    return 0;
  }

  bool MonitoringFile::CheckHistogram(TFile * f, const char* HistoName) {
    std::unique_ptr<TObject> obj(f->Get(HistoName));
    if (!obj.get()) {
      // std::cerr<<"No such histogram \""<< HistoName << "\"\n";
      return false;
    } else
      return true;
  }

  int MonitoringFile::getDebugLevel() {
    return s_dbg.getLvl();
  }
  void MonitoringFile::setDebugLevel(int level) {
    s_dbg.setLvl((debugLevel_t)(level));
  }
  void MonitoringFile::doTiming() {
    m_doTiming = true;
  }

 
  void MonitoringFile::setCheckEquality(bool value) {dqutils::s_checkEquality=value;}
  std::atomic<int> MonitoringFile::m_fileCompressionLevel = 1;
  bool MonitoringFile::m_doTiming = false;
  std::unordered_map<std::string, std::clock_t> MonitoringFile::m_cpuPerHistogram;
 
  std::string MonitoringFile::getPath(TDirectory * dir) {

    std::string path = dir->GetPath();
    if (path.find(':') != std::string::npos)
      path = path.substr(path.rfind(':') + 1);

    return path;
  }

}  // namespace dqutils
