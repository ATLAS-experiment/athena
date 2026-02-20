/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <fstream> 
#include <map>
#include <memory>
#include <getopt.h>

// ROOT include(s):
#include <TSystem.h>
#include <TError.h>
#include <TFile.h>
#include <TH1F.h>

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#else
#include "POOLRootAccess/TEvent.h"
#endif

#include "AsgTools/AnaToolHandle.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "AthContainers/Accessor.h"
#include "AsgMessaging/MessageCheck.h"
#include "AsgAnalysisInterfaces/IFakeBkgTool.h"
#include "AsgAnalysisInterfaces/ILinearFakeBkgTool.h"
#include "TruthUtils/ParticleConstants.h"


bool successful(bool arg) { return arg; }
bool successful(const StatusCode& arg) { return arg.isSuccess(); }

#define FBT_CHECK1(x, FAIL)                                             \
  do {                                                                  \
    if(!successful(x)) {                                                \
      Error("fbtTestBasics", "failure encounted on l.%d", __LINE__);    \
      return FAIL;							\
    }                                                                   \
  } while(false)
#define FBT_CHECK(x) FBT_CHECK1(x, false)

struct Result
{
  float value = 0, statUp = 0, statDown = 0;
  std::map<CP::SystematicVariation, float> variations;
  bool operator==(const Result&) const;
  void Print() const;
};

#ifdef XAOD_STANDALONE
using Store_t = std::unique_ptr<xAOD::TStore>;
#else
using Store_t = StoreGateSvc*;
#endif

bool allTests(Store_t& store);
bool minimalTest(const std::string& type, const std::vector<std::string>& config, Store_t& store, Result& result);
bool parallelJob(const std::string& type, const std::vector<std::string>& config, Store_t& store, const std::string& saveAs, int nEvents, int eventOffset);
bool readFromROOT(std::vector<std::string>& config);
bool readFromXML(std::vector<std::string>& config);

template<class Interface = CP::IFakeBkgTool> bool setup(asg::AnaToolHandle<Interface>& tool, const std::string& type, const std::vector<std::string>& config, const std::string& progressFile="");
template<class Interface = CP::IFakeBkgTool> bool eventLoop(asg::AnaToolHandle<Interface>& tool, Store_t& store, Result& result, int nEvents, int eventOffset=0);
bool addEventWeight(asg::AnaToolHandle<CP::ILinearFakeBkgTool>& tool, Result& result);
bool addEventWeight(asg::AnaToolHandle<CP::IFakeBkgTool>& tool, Result& result);
template<class Interface = CP::IFakeBkgTool> bool fillResult(asg::AnaToolHandle<Interface>& tool, Result& result);

const std::string selection = ">=1T";
const std::string process = ">=1F[T]";
const bool readCPVariations = true;
const int nEvents = 48;

std::atomic<bool> verbose = false;  // set once in main

int main(int argc, char* argv[])
{
  for(int i=1;i<argc;++i)
    {
      std::string option = argv[i];
      if(option=="-v" || option=="--verbose") verbose = true;
    }

#ifdef XAOD_STANDALONE
  xAOD::Init("fbtTestBasics").ignore();
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  Store_t store = std::make_unique<xAOD::TStore>();
  StatusCode::enableFailure();
#else
  IAppMgrUI* app = POOL::Init();
  POOL::TEvent event(POOL::TEvent::kClassAccess);
  FBT_CHECK1( event.evtStore().retrieve(), 1 );
  Store_t store = event.evtStore().get();
#endif
    
  int returnCode = allTests(store) ? 0 : 1;
    
#ifndef XAOD_STANDALONE
  FBT_CHECK1( app->finalize(), 1 );
#endif
  return returnCode;
}

bool allTests(Store_t& store)
{
  std::vector<std::string> config;
  Result result0, result1, result2, result3, result4, result5;
    
  if(verbose) std::cout <<"\nWill do minimal test with CP::ApplyFakeFactor\n";
  FBT_CHECK( readFromROOT(config) );
  FBT_CHECK( minimalTest("CP::ApplyFakeFactor", config, store, result0) );
    
  if(verbose) std::cout <<"\nWill do minimal test with CP::AsymptMatrixTool\n";
  FBT_CHECK( minimalTest("CP::AsymptMatrixTool", config, store, result1) );
    
  if(verbose) std::cout <<"\nWill do minimal test with CP::LhoodMM_tools\n";
  FBT_CHECK( minimalTest("CP::LhoodMM_tools", config, store, result2) );
    
  if(verbose) std::cout <<"\nWill test loading efficiencies from XML\n";
  FBT_CHECK( readFromXML(config) );
  FBT_CHECK( minimalTest("CP::AsymptMatrixTool", config, store, result3) );
  FBT_CHECK( readFromROOT(config) );
  FBT_CHECK( result1 == result3 );
    
  if(verbose) std::cout <<"\nWill test getEventWeight()\n";
  {
    asg::AnaToolHandle<CP::ILinearFakeBkgTool> tool;
    FBT_CHECK( setup(tool, "CP::AsymptMatrixTool", config) );
    FBT_CHECK( eventLoop(tool, store, result4, nEvents) );
    if(verbose) result3.Print();
  }
  FBT_CHECK( result1 == result4 );
    
  if(verbose) std::cout <<"\nWill test parallel jobs and merging\n";
  FBT_CHECK( parallelJob("CP::LhoodMM_tools", config, store, "fbt_job1.root", 16, 0) );
  FBT_CHECK( parallelJob("CP::LhoodMM_tools", config, store, "fbt_job2.root", 16, 16) );
  FBT_CHECK( parallelJob("CP::LhoodMM_tools", config, store, "fbt_job3.root", 16, 32) );
  FBT_CHECK( gSystem->Exec("hadd -f fbt_merged.root fbt_job*.root") == 0 );
  {
    asg::AnaToolHandle<CP::IFakeBkgTool> tool;
    FBT_CHECK( setup(tool, "CP::LhoodMM_tools", config, "fbt_merged.root") );
    FBT_CHECK( fillResult(tool, result5) );
  }
  FBT_CHECK(result2 == result5 );
    
  return true;
}

bool minimalTest(const std::string& type, const std::vector<std::string>& config, Store_t& store, Result& result)
{
  asg::AnaToolHandle<CP::IFakeBkgTool> tool;
  FBT_CHECK( setup(tool, type, config) );
  FBT_CHECK( eventLoop(tool, store, result, nEvents) );
  FBT_CHECK( fillResult(tool, result) );
  return true;
}

bool parallelJob(const std::string& type, const std::vector<std::string>& config, Store_t& store, const std::string& saveAs, int nEvents, int eventOffset)
{
  asg::AnaToolHandle<CP::IFakeBkgTool> tool;
  FBT_CHECK( setup(tool, type, config) );
  Result result;
  FBT_CHECK( eventLoop(tool, store, result, nEvents, eventOffset) );
  std::unique_ptr<TFile> f(TFile::Open(saveAs.c_str(), "RECREATE"));
  FBT_CHECK( !!f );
  FBT_CHECK( tool->saveProgress(f->mkdir("fakes")) );
  f->Close();
  if(verbose)
    {
      float value, up, down;
      FBT_CHECK( tool->getTotalYield(value, up, down) );
      std::cout << "Parallel job stored intermediate yield " << value << " +" << up << " -" << down << std::endl;
    }
  return true;
}

static std::atomic<long> instance = 0; /// mostly useful for athena, which will otherwise re-use the previous tool

template<class Interface>
bool setup(asg::AnaToolHandle<Interface>& tool, const std::string& type, const std::vector<std::string>& config, const std::string& progressFile)
{
  tool = asg::AnaToolHandle<Interface>(type + "/FBT" + std::to_string(++instance));
  FBT_CHECK( tool.setProperty("InputFiles", config) );
  FBT_CHECK( tool.setProperty("EnergyUnit", "GeV") );
  FBT_CHECK( tool.setProperty("ConvertWhenMissing", true) );
  FBT_CHECK( tool.setProperty("Selection", selection) );
  FBT_CHECK( tool.setProperty("Process", process) );
  if(!progressFile.empty())
    {
      FBT_CHECK( tool.setProperty("ProgressFileName", progressFile) );
      FBT_CHECK( tool.setProperty("ProgressFileDirectory", "fakes") );
    }
  FBT_CHECK( tool.initialize() );
  return true;
    
}

template<class Interface>
bool eventLoop(asg::AnaToolHandle<Interface>& tool, Store_t& store, Result& result, int nEvents, int eventOffset)
{
  auto eventInfo = std::make_unique<xAOD::EventInfo>();
  auto eventAuxInfo = std::make_unique<xAOD::EventAuxInfo>();
  eventInfo->setStore(eventAuxInfo.get());
  static const SG::Accessor<int> flagAcc("flag");
  flagAcc(*eventInfo) = 1;
  FBT_CHECK( store->record(std::move(eventInfo), "EventInfo") );
  FBT_CHECK( store->record(std::move(eventAuxInfo), "EventInfoAux.") );
 
  xAOD::IParticleContainer particles(SG::VIEW_ELEMENTS);
  auto e = std::make_unique<xAOD::Electron>();
  e->makePrivateStore();
  e->setCharge(1);
  particles.push_back(static_cast<xAOD::IParticle*>(&*e));
  static const SG::Accessor<char> TightAcc("Tight");
  for(int i=eventOffset;i<nEvents+eventOffset;++i)
    {
      e->setPtEtaPhi((1 + (i%3))*1e4, 0., 0.);
      TightAcc(*e) = (i%4)? 0 : 1;
      FBT_CHECK( tool->addEvent(particles) );
      FBT_CHECK( addEventWeight(tool, result) );
    }
	
#ifdef XAOD_STANDALONE
  store->clear();
#else
  FBT_CHECK( store->clearStore(true) );
#endif
  return true;
}

bool addEventWeight(asg::AnaToolHandle<CP::ILinearFakeBkgTool>& tool, Result& result)
{
  FBT_CHECK( tool->applySystematicVariation({}) );
  float y;
  FBT_CHECK( tool->getEventWeight(y, selection, process) );
  result.value += y;
  result.statUp = sqrt(pow(result.statUp, 2) + y*y);
  result.statDown = sqrt(pow(result.statDown, 2) + y*y);
  for(auto& sysvar : tool->affectingSystematics())
    {
      FBT_CHECK( tool->applySystematicVariation({sysvar}) );
      FBT_CHECK( tool->getEventWeight(y, selection, process) );
      result.variations[sysvar] += y;
    }
  return true;
}

bool addEventWeight(asg::AnaToolHandle<CP::IFakeBkgTool>&, Result&)
{
  return true;
}

template<class Interface>
bool fillResult(asg::AnaToolHandle<Interface>& tool, Result& result)
{
  result.variations.clear();
  FBT_CHECK( tool->getTotalYield(result.value, result.statUp, result.statDown) );
  if(readCPVariations)
    {
      for(auto& sysvar : tool->affectingSystematics())
        {
	  FBT_CHECK( tool->applySystematicVariation({sysvar}) );
	  float unused;
	  FBT_CHECK( tool->getTotalYield(result.variations[sysvar], unused, unused) );
        }
    }
  if(verbose) result.Print();
  return true;
}

bool Result::operator==(const Result& rhs) const
{
  auto compare = [&](auto x1, auto x2) -> bool
    {
      bool closeEnough = std::fabs(x1 - x2) <  std::max(1e-5, 1e-3 * std::min(std::fabs(x1), std::fabs(x2)));;
      if(!closeEnough)
        {
	  std::cout << "ERROR: found different values for the equality test: " << x1 << " vs " << x2 << std::endl;
        }
      return closeEnough; 
    };
  FBT_CHECK( compare(value, rhs.value) ); 
  FBT_CHECK( compare(statUp, rhs.statUp) ); 
  FBT_CHECK( compare(statDown, rhs.statDown) );
  FBT_CHECK( variations.size() == rhs.variations.size() );
  for(auto& kv : variations)
    {
      auto itr = rhs.variations.find(kv.first);
      FBT_CHECK( itr != rhs.variations.end() );
      FBT_CHECK( compare(kv.second, itr->second) );
    }
  return true;
}

void Result::Print() const
{
  std::cout << "Result: total yield = " << value << " +" << statUp << " -" << statDown << std::endl;
  for(auto& kv : variations)
    {
      std::cout << "   variation " << kv.first.name() << " = " << kv.second << std::endl;
    }
}

bool readFromROOT(std::vector<std::string>& config)
{
  config.clear();
  config.emplace_back("fbt_efficiencies.root");
  TH1D hElFake("FakeEfficiency_el_pt","FakeEfficiency", 1, 10., 100.);
  hElFake.SetBinContent(1, 0.05);
  hElFake.SetBinError(1, 0.01);
  TH1D hMuFake("FakeEfficiency_mu_pt","FakeEfficiency", 1, 10., 100.);
  hMuFake.SetBinContent(1, 0.15);
  hMuFake.SetBinError(1, 0.032);
  TH1D hElReal("RealEfficiency_el_pt","RealEfficiency", 1, 10., 100.);
  hElReal.SetBinContent(1, 0.90);
  hElReal.SetBinError(1, 0.01);
  TH1D hMuReal("RealEfficiency_mu_pt","RealEfficiency", 1, 10., 100.);
  hMuReal.SetBinContent(1, 0.95);
  hMuReal.SetBinError(1, 0.01);
  std::unique_ptr<TFile> f(TFile::Open(config.back().c_str(), "RECREATE"));
  FBT_CHECK( !!f );
  f->cd();
  hElFake.Write();
  hElReal.Write();
  hMuFake.Write();
  hMuReal.Write();
  f->Close();
  return true;
}

bool readFromXML(std::vector<std::string>& config)
{
  config.clear();
  config.emplace_back("fbt_efficiencies.xml");
  std::ofstream out(config.back().c_str(), std::ios_base::out);
  FBT_CHECK( out.is_open() );
  /// note: the declarations must be placed in the "good" order,
  /// otherwise the definitions of the SystematicVariations will be different than those obtained with readFromROOT()
  out << "<efficiencies>\n";
  out << "<param type=\"int\" level=\"event\"> flag </param>\n";
  out << "<electron type=\"fake-efficiency\" input=\"central-value\" stat=\"per-bin\" >\n\t<bin flag=\"1\">\n 0.05 +- 0.01 (stat) </bin>\n</electron>\n";
  out << "<electron type=\"real-efficiency\" input=\"central-value\" stat=\"global\" >\n\t<bin flag=\"1\">\n 0.90 +- 0.01 (stat) </bin>\n</electron>\n";
  out << "<muon type=\"fake-efficiency\" input=\"central-value\" stat=\"per-bin\" >\n\t<bin flag=\"1\">\n 0.15 +- 0.032 (stat) </bin>\n</muon>\n";
  out << "<muon type=\"real-efficiency\" input=\"central-value\" stat=\"global\" >\n\t<bin flag=\"1\">\n 0.95 +- 0.01 (stat) </bin>\n</muon>\n";
  out << "</efficiencies>\n";
  out.close();
  return true;
}
