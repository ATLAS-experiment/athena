/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamples::Container
   @brief storage of the time histories of all the cells
*/

#ifndef LArSamples_TreeAccessor_H
#define LArSamples_TreeAccessor_H

#include "LArSamplesMon/Accessor.h"
#include "LArCafJobs/PersistentAccessor.h"
#include "LArCafJobs/CellInfo.h"
#include "LArSamplesMon/History.h"
#include "TString.h"
#include "TTree.h"
#include "LArSamplesMon/Chi2Calc.h"
#include "LArSamplesMon/FilterList.h"

class TFile;

namespace LArSamples {
  
  class Data;
  class FilterParams;
  class DataTweaker;
  class EventData;
  class RunData;

  class ATLAS_NOT_THREAD_SAFE TreeAccessor : public Accessor, public PersistentAccessor {
  
    public:
   
      /** @brief Constructor  */
      TreeAccessor(TTree& cellTree, TTree& scTree, TTree& eventTree, TTree* runTree, TFile* file) 
       : PersistentAccessor(cellTree, scTree, eventTree, runTree, file) { }

      TreeAccessor(const TString& fileName) : PersistentAccessor(fileName) { }
      
      static std::unique_ptr<TreeAccessor> open(const TString& fileName);
      
      virtual ~TreeAccessor() { resetCache(); }
                
      static std::unique_ptr<TreeAccessor> merge(const std::vector<const Accessor*>& accessors,const TString& fileName = "");
      static std::unique_ptr<TreeAccessor> merge(const std::vector<const Accessor*>& accessors,const TString& fileName,const TString& LBFile);
      static std::unique_ptr<TreeAccessor>
        filter(const Accessor& accessor,
               const FilterParams& filterParams,
               const TString& fileName, const DataTweaker& tweaker);
      
      static std::vector<std::unique_ptr<TreeAccessor> >
        filter(const Accessor& accessor,
               const FilterList& filterList, const DataTweaker& tweaker);
                                               
      friend class Interface;

      static std::unique_ptr<TreeAccessor> makeTemplate(const Accessor& accessor, const TString& fileName);
      
      virtual bool writeToFile(const TString& fileName) const override;

      virtual unsigned int historySize(unsigned int i) const override { return PersistentAccessor::historySize(i); }
      virtual unsigned int historySizeSC(unsigned int i) const override { return PersistentAccessor::historySizeSC(i); }

      virtual unsigned int nEvents() const override { return PersistentAccessor::nEvents(); }
      virtual const EventData* eventData(unsigned int i) const override { return PersistentAccessor::eventData(i); }

      virtual unsigned int nRuns() const override { return PersistentAccessor::nRuns(); }
      virtual const RunData* runData(unsigned int i) const override { return PersistentAccessor::runData(i); }

      void add(HistoryContainer* cont) { PersistentAccessor::add(cont); resetCache(); }

      
      virtual std::unique_ptr<const History> getCellHistory(unsigned int i) const override;
      virtual std::unique_ptr<const History> getSCHistory(unsigned int i) const override;
      virtual std::unique_ptr<const CellInfo> getCellInfo(unsigned int i) const override;
      std::unique_ptr<const CellInfo> getSCInfo(unsigned int i) const;
      
  };
}
  
#endif
