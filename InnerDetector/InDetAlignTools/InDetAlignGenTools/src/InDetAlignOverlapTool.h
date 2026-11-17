/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNTOOLS_OVERLAPTOOL_H
#define INDETALIGNTOOLS_OVERLAPTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "IInDetAlignOverlapTool.h"

#include "IInDetAlignFillTrack.h"
#include "AlignSiModuleList.h"
#include "InDetAlignTrkInfo/AlignSiHit.h"
#include <vector>
//#include "AthContainers/DataVector.h"

/** @class InDetAlignOverlapTool
    @brief The InDetAlignOverlapTool is to select good quality tracks for alignment to 
    build residuals with possible cuts on overlap hits, overlaps in SCT and PIXELs (in Phi and Eta).
*/

class AlignTrk;
class AlignSiModuleList;



class InDetAlignOverlapTool : public extends<AthAlgTool, IInDetAlignOverlapTool> {
 public:
  //standard constructor
  using base_class::base_class;
  virtual ~InDetAlignOverlapTool() ;
  
  virtual StatusCode initialize() ;
  virtual StatusCode finalize  () ;

  /**main method to get an overlap hit **/
  
  int getNumberOverlapPIX( const AlignTrk& ) const ;
  int getNumberOverlapSCT( const AlignTrk& ) const ;
  std::vector<AlignSiHit> getOverlapHit( const AlignTrk& );// const ;


 private:
  std::vector<AlignSiHit>  m_Overlaphits{};
     
  const AlignSiModuleList* p_modlist{};  
  
  
} ;

#endif // INDETALIGNTOOLS_OVERLAPTOOL_H
