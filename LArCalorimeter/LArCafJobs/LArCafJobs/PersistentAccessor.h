/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamples::Container
   @brief storage of the time histories of all the cells
*/

#ifndef LArSamples_PersistentAccessor_H
#define LArSamples_PersistentAccessor_H

#include "LArCafJobs/CellInfo.h"
#include "LArCafJobs/HistoryContainer.h"
#include "LArCafJobs/EventData.h"
#include "LArCafJobs/RunData.h"
#include "TString.h"
#include "TTree.h"

class TFile;

namespace LArSamples {
  
  class ATLAS_NOT_THREAD_SAFE PersistentAccessor {
  
    public:
   
      /** @brief Constructor  */
      PersistentAccessor(TTree& cellTree, TTree& SCTree, TTree& eventTree, TTree* runTree, TFile* file);
      PersistentAccessor(const TString& fileName);

      PersistentAccessor (const PersistentAccessor&);
      PersistentAccessor& operator= (const PersistentAccessor&);

      static PersistentAccessor* open(const TString& fileName);

      virtual ~PersistentAccessor();
                
      unsigned int historySize(unsigned int i) const;
      unsigned int historySizeSC(unsigned int i) const;

      bool save() const;

      const TTree& cellTree()  const { return *m_cellTree; }
      const TTree& SCTree()  const { return *m_SCTree; }
      const TTree& eventTree() const { return *m_eventTree; }
      const TTree& runTree()   const { return *m_runTree; }

      TFile* file() const { return m_file; }
      TString fileName() const;

      const HistoryContainer* historyContainer(unsigned int i) const;
      const HistoryContainer* historyContainerSC(unsigned int i) const;

      unsigned int nEvents() const { return m_eventTree->GetEntries(); }
      const EventData* eventData(unsigned int i) const;

      unsigned int nRuns() const { return m_runTree->GetEntries(); }
      const RunData* runData(unsigned int i) const;

      void add(HistoryContainer* cont);
      void addSC(HistoryContainer* cont);
      void addEvent(EventData* eventData);
      void addRun(RunData* runData);

      HistoryContainer* currentContainer() const { return m_historyCont; }
      HistoryContainer* currentContainerSC() const { return m_historyContSC; }

      static PersistentAccessor* merge(const std::vector<const PersistentAccessor*>& accessors, const TString& fileName);
      static PersistentAccessor* merge(const std::vector<TString>& inputFiles, const TString& fileName);

      int getCellEntry(unsigned int i) const { return m_cellTree->GetEntry(i); }
      int getSCEntry(unsigned int i) const { return m_SCTree->GetEntry(i); }

    private:
      
      TTree* m_cellTree, *m_SCTree, *m_eventTree, *m_runTree;
      mutable TFile* m_file;
      mutable HistoryContainer* m_historyCont;
      mutable HistoryContainer* m_historyContSC;
      mutable EventData* m_eventData;
      mutable RunData* m_runData;
      mutable std::map<unsigned int, const RunData*> m_runCache;
  };
}
  
#endif
