/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/

#ifndef FPGATrackSimRAWTOLOGICALHITSTOOL_H
#define FPGATrackSimRAWTOLOGICALHITSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include <vector>


// Forward declaration
class IFPGATrackSimMappingSvc;
class FPGATrackSimEventInputHeader;
class FPGATrackSimLogicalEventInputHeader;


class FPGATrackSimRawToLogicalHitsTool : public AthAlgTool
{
 public:

  FPGATrackSimRawToLogicalHitsTool(const std::string&, const std::string&, const IInterface*);
  virtual  ~FPGATrackSimRawToLogicalHitsTool() = default;
  virtual StatusCode initialize() override;

  StatusCode convert(unsigned stage, const FPGATrackSimEventInputHeader& header, 
                                      FPGATrackSimLogicalEventInputHeader& logicheader);
  StatusCode getUnmapped(std::vector<FPGATrackSimHit>& missing_hits);
  const FPGATrackSimPlaneMap* getPlaneMap_1st(int sliceNum);


private:
 
  // JO configuration
  ServiceHandle<IFPGATrackSimMappingSvc>     m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
  ServiceHandle<IFPGATrackSimEventSelectionSvc>  m_EvtSel     {this, "FPGATrackSimEventSelectionSvc", ""};
  IntegerProperty                       m_saveOptional {this, "SaveOptional", 2, "flag to enable the truth/offline tracking save =0 no optional saved, =1 saved in region, =2 save all "};
  IntegerArrayProperty                  m_towersToMap  {this, "TowersToMap", {}, "Which Towers to map, goes from 0 to 96!"};

  // internal members
  std::vector<int> m_towers;
  std::vector<FPGATrackSimHit> m_missing_hits;// vector to save hits not mapped, debugging only
  std::vector<int> m_missing_hit_codes; // for histograms used in debugging
  const FPGATrackSimPlaneMap* m_pmap = nullptr;

};

#endif // FPGATrackSimRAWTOLOGICALHITSTOOL_H
