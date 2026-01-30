/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSamplesMon/TreeAccessor.h"

#include "LArSamplesMon/Data.h"
#include "LArCafJobs/EventData.h"
#include "LArCafJobs/RunData.h"
#include "LArSamplesMon/FilterParams.h"
#include "LArSamplesMon/DataTweaker.h"
#include "LArSamplesMon/ClassCounts.h"
#include "LArCafJobs/ShapeInfo.h"

#include "TObjString.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TString.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using std::cout;
using std::endl;

using namespace LArSamples;


std::unique_ptr<TreeAccessor> TreeAccessor::open(const TString& fileName)
{
  std::unique_ptr<TFile> file (TFile::Open(fileName));
  if (!file) return nullptr;
  if (!file->IsOpen()) { return nullptr; }
  TTree* cellTree = (TTree*)file->Get("cells");
  if (!cellTree) return nullptr;
  TTree* scTree = (TTree*)file->Get("SC");
  if (!scTree) return nullptr;
  TTree* eventTree = (TTree*)file->Get("events");
  if (!eventTree) return nullptr;
  TTree* runTree = (TTree*)file->Get("runs");
  return std::make_unique<TreeAccessor>(*cellTree, *scTree, *eventTree, runTree, file.release());
}


const CellInfo* TreeAccessor::getCellInfo(unsigned int i) const 
{ 
  const HistoryContainer* cont = historyContainer(i);
  if (!cont || !cont->cellInfo()) return nullptr;
  return new CellInfo(*cont->cellInfo());
}

const CellInfo* TreeAccessor::getSCInfo(unsigned int i) const 
{ 
  const HistoryContainer* cont = historyContainerSC(i);
  if (!cont || !cont->cellInfo()) return nullptr;
  return new CellInfo(*cont->cellInfo());
}


const History* TreeAccessor::getCellHistory(unsigned int i) const 
{ 
  if (i >= cellTree().GetEntries()) return nullptr;
  getCellEntry(i);

  std::vector<const EventData*> eventDatas;
  
  for (unsigned int k = 0; k < currentContainer()->nDataContainers(); k++) {
    const EventData* evtData = eventData(currentContainer()->dataContainer(k)->eventIndex());
    EventData* newEvtData = (evtData ? new EventData(*evtData) : nullptr);
    eventDatas.push_back(newEvtData);
  }
  return (currentContainer()->cellInfo() ? new History(*currentContainer(), eventDatas, i) : nullptr);
}

const History* TreeAccessor::getSCHistory(unsigned int i) const 
{ 
  if (i >= SCTree().GetEntries()) return nullptr;
  getSCEntry(i);

  std::vector<const EventData*> eventDatas;
  
  for (unsigned int k = 0; k < currentContainerSC()->nDataContainers(); k++) {
    const EventData* evtData = eventData(currentContainerSC()->dataContainer(k)->eventIndex());
    EventData* newEvtData = (evtData ? new EventData(*evtData) : nullptr);
    eventDatas.push_back(newEvtData);
  }
  return (currentContainerSC()->cellInfo() ? new History(*currentContainerSC(), eventDatas, i) : nullptr);
}

std::unique_ptr<TreeAccessor> TreeAccessor::merge(const std::vector<const Accessor*>& accessors,
                                                  const TString& fileName)
{
  cout << "Merging to " << fileName << endl;
  auto newAcc = std::make_unique<TreeAccessor>(fileName);
  unsigned int size = 0;

  int evtIndex = 0, runIndex = 0;
  std::map<std::pair<int, int>, int> evtMap;
  std::map<int, int> runMap;

  cout << "Merging runs" << endl;
  for (const Accessor* accessor : accessors) {
    if (!accessor) {
      cout << "Cannot merge: one of the inputs is null!" << endl;
      return nullptr;
    }
    for (unsigned int i = 0; i < accessor->nRuns(); i++) {
      int run = accessor->runData(i)->run();
      if (runMap.find(run) != runMap.end()) continue;
      runMap[run] = runIndex;
      RunData newRun(*accessor->runData(i));
      newAcc->addRun(&newRun);
      runIndex++;
    }
  }
  
  cout << "Merging events" << endl;
  unsigned int nEventsTotal = 0, iEvt = 0;
  for (const Accessor* accessor : accessors)
    nEventsTotal += accessor->nEvents();  
  for (const Accessor* accessor : accessors) {
    for (unsigned int i = 0; i < accessor->nEvents(); i++) {
      iEvt++;
      if (iEvt % 100000 == 0) cout << "Merging event " << iEvt << "/" << nEventsTotal << endl;
      std::pair<int, int> evtId(accessor->eventData(i)->run(), accessor->eventData(i)->event());
      if (evtMap.find(evtId) != evtMap.end()) continue;
      evtMap[evtId] = evtIndex;
      std::map<int, int>::const_iterator idx = runMap.find(accessor->eventData(i)->run());
      int newRunIndex = (idx == runMap.end() ? -999 : idx->second);
      //cout << "Storing eventData for run " << accessor->eventData(i)->run() << " at index " << newRunIndex << " instead of " << accessor->eventData(i)->runIndex() << endl;
      EventData newEvent(*accessor->eventData(i), newRunIndex);
      newAcc->addEvent(&newEvent);
      evtIndex++;
    }
  } 
  
  for (unsigned int i = 0; i < newAcc->nChannels(); i++) {
    if (i % 10000 == 0) {
      cout << "Merging channel " << i << "/" <<  newAcc->nChannels() << " (current size = " << size << ")" << endl;
      //ClassCounts::printCountsTable();
    }
    std::optional<HistoryContainer> historyContainer;
  CellInfo* info = nullptr;
  for (const Accessor* accessor : accessors) {
      const History* history = accessor->cellHistory(i);
      if (!history || !history->isValid()) continue;
      if (!historyContainer) {
        info = new CellInfo(*history->cellInfo());
        historyContainer.emplace (info);
      }
      for (unsigned int j = 0; j < history->nData(); j++) {
        auto newDC = std::make_unique<DataContainer>(history->data(j)->container());
        std::map<std::pair<int, int>, int>::const_iterator newIndex 
          = evtMap.find(std::make_pair(history->data(j)->run(), history->data(j)->event()));
        if (newIndex == evtMap.end()) std::cout << "Event not found for cell " << i << ", data " << j << ".\n";
        newDC->setEventIndex(newIndex != evtMap.end() ? newIndex->second : -1);
        historyContainer->add(newDC.release());
        if (not info) continue;
        if (!info->shape(history->data(j)->gain())) {
          const ShapeInfo* shape = history->cellInfo()->shape(history->data(j)->gain());
          if (!shape) {
            cout << "Shape not filled for hash = " << i << ", index = " << j << ", gain = " << Data::gainStr(history->data(j)->gain()) << endl;
          }
          info->setShape(history->data(j)->gain(), (shape ? new ShapeInfo(*shape) : nullptr));
        }
      }
    }
    if (historyContainer) size += historyContainer->nDataContainers();
    newAcc->add(&historyContainer.value());
  }

  cout << "Merging done, final size = " << size << endl;
  newAcc->save();
  return newAcc;
}


std::unique_ptr<TreeAccessor> TreeAccessor::merge(const std::vector<const Accessor*>& accessors,const TString& fileName,const TString& LBFile)
{
  // O.Simard - 01.07.2011
  // Alternative version with LB cleaning.

  bool kBadLB=false;
  std::vector<unsigned int> LBList;
  std::ifstream infile(LBFile.Data());
  std::string line;
  // assume single-line format with coma-separated LBs (from python)
  std::getline(infile,line,'\n');
  TString filter(line.c_str());
  std::unique_ptr<TObjArray> list (filter.Tokenize(", ")); // coma\space delimiters
  if(list->GetEntries() == 0){
    printf("No LB filtering specified, or bad format. Exiting.\n");
    return nullptr;
  }
  
  for(int k = 0; k < list->GetEntries(); k++){
    TObjString* tobs = (TObjString*)(list->At(k));
    LBList.push_back((unsigned int)(tobs->String()).Atoi());
  }
  printf("LB List: %d\n",(int)LBList.size());
  

  // from here it is similar to other functions of this class
  auto newAcc = std::make_unique<TreeAccessor>(fileName);
  unsigned int size = 0;

  int evtIndex = 0, runIndex = 0;
  std::map<std::pair<int, int>, int> evtMap;
  std::map<int, int> runMap;

  cout << "Merging runs" << endl;
  for (const Accessor* accessor : accessors) {
    if (!accessor) {
      cout << "Cannot merge: one of the inputs is null!" << endl;
      return nullptr;
    }
    for (unsigned int i = 0; i < accessor->nRuns(); i++) {
      int run = accessor->runData(i)->run();
      if (runMap.find(run) != runMap.end()) continue;
      runMap[run] = runIndex;
      RunData newRun(*accessor->runData(i));
      newAcc->addRun(&newRun);
      runIndex++;
    }
  }
  
  cout << "Merging events" << endl;
  unsigned int nEventsTotal = 0, iEvt = 0;
  for (const Accessor* accessor : accessors)
    nEventsTotal += accessor->nEvents();  
  for (const Accessor* accessor : accessors) {
    for (unsigned int i = 0; i < accessor->nEvents(); i++) {
      iEvt++;
      if (iEvt % 100000 == 0) cout << "Merging event " << iEvt << "/" << nEventsTotal << endl;

      // ----
      // skip LBs which are found in the list
      kBadLB=false;
      for(unsigned int ilb = 0 ; ilb < LBList.size() ; ilb++){
        if(LBList.at(ilb)==accessor->eventData(i)->lumiBlock()){
	  kBadLB=true;
	  //printf("  == Rejecting Event in LB %4d\n",accessor->eventData(i)->lumiBlock());
	  break;
	}
      }
      if(kBadLB) continue;
      // ----

      std::pair<int, int> evtId(accessor->eventData(i)->run(), accessor->eventData(i)->event());
      if (evtMap.find(evtId) != evtMap.end()) continue;
      evtMap[evtId] = evtIndex;
      std::map<int, int>::const_iterator idx = runMap.find(accessor->eventData(i)->run());
      int newRunIndex = (idx == runMap.end() ? -999 : idx->second);
      EventData newEvent(*accessor->eventData(i), newRunIndex);
      newAcc->addEvent(&newEvent);
      evtIndex++;
    }
  } 
  
  cout << "Merging cells" << endl;
  for (unsigned int i = 0; i < newAcc->nChannels(); i++) {
    if (i % 10000 == 0) {
      cout << "Merging channel " << i << "/" <<  newAcc->nChannels() << " (current size = " << size << ")" << endl;
      //ClassCounts::printCountsTable();
    }
    std::optional<HistoryContainer> historyContainer;
  CellInfo* info = nullptr;
  for (const Accessor* accessor : accessors) {
      const History* history = accessor->cellHistory(i);
      if (!history || !history->isValid()) continue;
      if (!historyContainer) {
        info = new CellInfo(*history->cellInfo());
        historyContainer.emplace (info);
      }
      for (unsigned int j = 0; j < history->nData(); j++) {
        auto newDC = std::make_unique<DataContainer>(history->data(j)->container());
        std::map<std::pair<int, int>, int>::const_iterator newIndex 
          = evtMap.find(std::make_pair(history->data(j)->run(), history->data(j)->event()));
        //if (newIndex == evtMap.end()) cout << "Event not found for cell " << i << ", data " << j << "." << endl;
        newDC->setEventIndex(newIndex != evtMap.end() ? newIndex->second : -1);
        historyContainer->add(newDC.release());
        if (not info) continue;
        if (!info->shape(history->data(j)->gain())) {
         const ShapeInfo* shape = history->cellInfo()->shape(history->data(j)->gain());
         if (!shape) 
           cout << "Shape not filled for hash = " << i << ", index = " << j << ", gain = " << Data::gainStr(history->data(j)->gain()) << endl;
          info->setShape(history->data(j)->gain(), (shape ? new ShapeInfo(*shape) : nullptr));
        }
      }
    }
    if(historyContainer){
      size += historyContainer->nDataContainers();
    }
    newAcc->add(&historyContainer.value());
    //}
  }

  cout << "Merging SC" << endl;
  for (unsigned int i = 0; i < newAcc->nChannelsSC(); i++) {
    if (i % 10000 == 0) {
      cout << "Merging channel " << i << "/" <<  newAcc->nChannelsSC() << " (current size = " << size << ")" << endl;
    }
    std::optional<HistoryContainer> historyContainer;
  CellInfo* info = nullptr;
  for (const Accessor* accessor : accessors) {
      const History* history = accessor->getSCHistory(i);
      if (!history || !history->isValid()) continue;
      if (!historyContainer) {
        info = new CellInfo(*history->cellInfo());
        historyContainer.emplace (info);
      }
      for (unsigned int j = 0; j < history->nData(); j++) {
        auto newDC = std::make_unique<DataContainer>(history->data(j)->container());
        std::map<std::pair<int, int>, int>::const_iterator newIndex 
          = evtMap.find(std::make_pair(history->data(j)->run(), history->data(j)->event()));
        //if (newIndex == evtMap.end()) cout << "Event not found for cell " << i << ", data " << j << "." << endl;
        newDC->setEventIndex(newIndex != evtMap.end() ? newIndex->second : -1);
        historyContainer->add(newDC.release());
        if (not info) continue;
        if (!info->shape(history->data(j)->gain())) {
         const ShapeInfo* shape = history->cellInfo()->shape(history->data(j)->gain());
         if (!shape) 
           cout << "Shape not filled for hash = " << i << ", index = " << j << ", gain = " << Data::gainStr(history->data(j)->gain()) << endl;
          info->setShape(history->data(j)->gain(), (shape ? new ShapeInfo(*shape) : nullptr));
        }
      }
    }
    if(historyContainer){
      size += historyContainer->nDataContainers();
    }
    newAcc->addSC(&historyContainer.value());
    //}
  }

  cout << "Merging done, final size = " << size << endl;
  newAcc->save();
  return newAcc;
}


std::unique_ptr<TreeAccessor> TreeAccessor::filter(const Accessor& accessor,
                                                   const FilterParams& filterParams,
                                                   const TString& fileName,
                                                   const DataTweaker& tweaker)
{
  FilterList filterList; filterList.add(filterParams, fileName);  
  std::vector<std::unique_ptr<TreeAccessor> > result = filter(accessor, filterList, tweaker);
  return (!result.empty() ? std::move(result[0]) : nullptr);
}

std::vector<std::unique_ptr<TreeAccessor> >
TreeAccessor::filter(const Accessor& accessor,
                     const FilterList& filterList, 
                     const DataTweaker& tweaker)
{
  std::vector<std::unique_ptr<TreeAccessor> > newAccessors;

  if (filterList.size() == 0) {
    cout << "No filter categories specified, done! (?)" << endl;
    return newAccessors;
  }
  
  for (unsigned int f = 0; f < filterList.size(); f++) {
    cout << "Skimming to " << filterList.fileName(f) << endl;
    if (!gSystem->AccessPathName(filterList.fileName(f))) {
      cout << "File already exists, exiting." << endl;
      return newAccessors;
    }
  }
  
  for (unsigned int f = 0; f < filterList.size(); f++)
    newAccessors.push_back(std::make_unique<TreeAccessor>(filterList.fileName(f)));
  std::map<std::pair<unsigned int, unsigned int>, unsigned int> eventIndices;
  std::vector< std::map<unsigned int, unsigned int> > eventsToKeep(filterList.size());
  std::vector< std::map<unsigned int, unsigned int> > runsToKeep(filterList.size());
  
  double nTot = 0, nPass = 0; 
  
  for (unsigned int i = 0; i < accessor.nEvents(); i++) {
    const EventData* eventData = accessor.eventData(i);
    eventIndices[std::pair<unsigned int, unsigned int>(eventData->run(), eventData->event())] = i;
  }  

  for (unsigned int i = 0; i < accessor.nChannels(); i++) {
    if (i % 25000 == 0) {
      cout << "Filtering " << i << "/" <<  accessor.nChannels() 
       << " (passing so far = " << nPass << ", total seen = " << nTot << ")" << endl;
      //ClassCounts::printCountsTable();
    }    
    bool first = true;
    
    const History* history = nullptr;
    for (unsigned int f = 0; f < filterList.size(); f++) {
      history = accessor.pass(i, filterList.filterParams(f));
      if (history) break;
    }
    for (unsigned int f = 0; f < filterList.size(); f++) {
      if (!history || !history->cellInfo() || !filterList.filterParams(f).passCell(*history->cellInfo())) {
        HistoryContainer newHist;
        newAccessors[f]->add(&newHist);
        continue;
      }
      if (first) { nTot += history->nData(); first = false; }
      HistoryContainer newHist(new CellInfo(*history->cellInfo()));
      for (unsigned int k = 0; k < history->nData(); k++) {
        if (!filterList.filterParams(f).passEvent(*history->data(k))) continue;
        const EventData* eventData = history->data(k)->eventData();
        std::map<std::pair<unsigned int, unsigned int>, unsigned int>::const_iterator findIndex = 
          eventIndices.find(std::pair<unsigned int, unsigned int>(eventData->run(), eventData->event()));
        if (findIndex == eventIndices.end()) { 
          cout << "Inconsistent event numbering!!!" << endl; 
          return std::vector<std::unique_ptr<TreeAccessor> >();
        }
        int oldEvtIndex = findIndex->second;
        bool isNewEvt = (eventsToKeep[f].find(oldEvtIndex) == eventsToKeep[f].end());
        unsigned int newEvtIndex = (isNewEvt ? eventsToKeep[f].size() : eventsToKeep[f][oldEvtIndex]);      
        if (isNewEvt) eventsToKeep[f][oldEvtIndex] = newEvtIndex;

        int oldRunIndex = history->data(k)->eventData()->runIndex();
        bool isNewRun = (runsToKeep[f].find(oldRunIndex) == runsToKeep[f].end());
        unsigned int newRunIndex = (isNewRun ? runsToKeep[f].size() : runsToKeep[f][oldRunIndex]);      
        if (isNewRun) runsToKeep[f][oldRunIndex] = newRunIndex;

        Data* newData = tweaker.tweak(*history->data(k), newEvtIndex);
        if (!newData) {
          cout << "Filtering failed on data " << k << " of cell " << i << ", aborting" << endl;
          return std::vector<std::unique_ptr<TreeAccessor> >();
        }
        nPass++;
        newHist.add(newData->dissolve());
      }
      newAccessors[f]->add(&newHist);
    }
  }

  for (unsigned int f = 0; f < filterList.size(); f++) {      
    cout << "Adding runs..." << endl;
    std::vector<unsigned int> runsToKeep_ordered(runsToKeep[f].size());
    for (const auto& runIndex : runsToKeep[f]) 
      runsToKeep_ordered[runIndex.second] = runIndex.first;

    for (unsigned int runIndex : runsToKeep_ordered) {
      RunData newRun(*accessor.runData(runIndex));
      newAccessors[f]->addRun(&newRun);
    }
    cout << "Adding events..." << endl;
    std::vector<unsigned int> eventsToKeep_ordered(eventsToKeep[f].size());
    for (const auto& eventIndex : eventsToKeep[f])
      eventsToKeep_ordered[eventIndex.second] = eventIndex.first;

    for (unsigned int eventIndex : eventsToKeep_ordered) {
      std::map<unsigned int, unsigned int>::const_iterator idx = runsToKeep[f].find(accessor.eventData(eventIndex)->runIndex());
      int newRunIndex = (idx == runsToKeep[f].end() ? 0 : idx->second);
      std::unique_ptr<EventData> newEvent (tweaker.tweak(*accessor.eventData(eventIndex), newRunIndex));
      newAccessors[f]->addEvent(newEvent.get());
    }
  }
  cout << "Filtering done! final size = " << nPass << endl;
  //ClassCounts::printCountsTable();
  for (unsigned int f = 0; f < filterList.size(); f++) {      
    cout << "Saving " << newAccessors[f]->fileName() << endl;    
    newAccessors[f]->save();
  }
  return newAccessors;
}


std::unique_ptr<TreeAccessor> TreeAccessor::makeTemplate(const Accessor& accessor, const TString& fileName)
{
  auto newAccessor = std::make_unique<TreeAccessor>(fileName);
  
  std::vector<short> samples(5, 0);
  std::vector<float> autoCorrs(4, 0);
  
  RunData dummyRun(0);
  newAccessor->addRun(&dummyRun);
  
  EventData dummyEvent(0, 0, 0, 0);
  newAccessor->addEvent(&dummyEvent);

  for (unsigned int i = 0; i < accessor.nChannels(); i++) {
    if (i % 25000 == 0)
      cout << "Templating " << i << "/" <<  accessor.nChannels() << endl;
    const History* history = accessor.cellHistory(i);
    if (!history || !history->cellInfo()) {
      HistoryContainer newHist;
      newAccessor->add(&newHist);
      continue;
    }
    HistoryContainer newHist(new CellInfo(*history->cellInfo()));
    auto dataContainer = std::make_unique<DataContainer>(CaloGain::LARHIGHGAIN, samples, 0, 0, 0, 0, autoCorrs);
    newHist.add(dataContainer.release());
    newAccessor->add(&newHist);
  }

  newAccessor->save();  
  return newAccessor;
}
      
bool TreeAccessor::writeToFile(const TString& fileName) const
{
  TFile newFile(fileName, "RECREATE");
  if (!newFile.IsOpen()) return false;
  
  cellTree().Write();
  eventTree().Write();

  return true;
}
