/*
 * Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration.
 */

#include "BunchCrossingAverageCondAlg.h"
#include "CoolKernel/IObject.h"
#include "CoralBase/Blob.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include <charconv>
#include <cstdint>
#include <bitset>
#include "CxxUtils/get_unaligned.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/AttributeListException.h"


namespace {
   // helper to set timestamp based IOV with infinit range
  EventIDBase infiniteIOVBegin() {
     return EventIDBase( 0, // run,
                        EventIDBase::UNDEFEVT,  // event
                        0, // seconds
                        0, // ns
                        0 // LB
                        );
  }

  EventIDBase infiniteIOVEend() {
     return EventIDBase( std::numeric_limits<int>::max() - 1, // run
                         EventIDBase::UNDEFEVT,  // event
                         std::numeric_limits<int>::max() - 1, // seconds
                         std::numeric_limits<int>::max() - 1, // ns
                         std::numeric_limits<int>::max() - 1  // LB
                         );
  }

}

StatusCode BunchCrossingAverageCondAlg::initialize() {


  ATH_CHECK( m_fillParamsFolderKey.initialize( m_mode == 0 || m_mode == 1 ) );
  ATH_CHECK( m_lumiCondDataKey.initialize( m_mode == 3 ) );
  ATH_CHECK( m_outputKey.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode BunchCrossingAverageCondAlg::execute (const EventContext& ctx) const {
  SG::WriteCondHandle<BunchCrossingAverageCondData> writeHdl(m_outputKey, ctx);
  if (writeHdl.isValid()) {

    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }
  // make sure that the output IOV has a valid timestamp, otherwise the conditions
  // data cannot be added to the "mixed" conditions data container. A mixed container
  // is needed when the conditions depends on e.g. the LuminosityCondData
  EventIDRange infinite_range(infiniteIOVBegin(),infiniteIOVEend());
  writeHdl.addDependency(infinite_range);

  //Output object & range:
  auto bccd=std::make_unique<BunchCrossingAverageCondData>();


  if (m_mode == 2) { // use trigger bunch groups
    // To be filled
  }

  if (m_mode == 3) { // use luminosity data
    // To be filled
  }

  if (m_mode == 0 || m_mode == 1) { // use FILLPARAMS (data) or /Digitization/Parameters (MC)
    


    SG::ReadCondHandle<CondAttrListCollection> fillParamsHdl (m_fillParamsFolderKey, ctx);
    writeHdl.addDependency(fillParamsHdl);

    
    const CondAttrListCollection* attrList_link=*fillParamsHdl;
    const coral::AttributeList& attrList = attrList_link->attributeList (m_BPTX);
    

    if (attrList.size() == 0 || attrList["Valid"].isNull()) {
      ATH_MSG_DEBUG ("Can't find information for channel " << m_BPTX);
      return StatusCode::SUCCESS;
    }

    const coral::AttributeList& attrList_fBCT = attrList_link->attributeList (m_fBCT);

    if (attrList_fBCT.size() == 0 || attrList_fBCT["Valid"].isNull()) {
      ATH_MSG_DEBUG ("Can't find information for channel " << m_fBCT);
      return StatusCode::SUCCESS;
    }

    const coral::AttributeList& attrList_DCCT = attrList_link->attributeList (m_DCCT);

    if (attrList_DCCT.size() == 0 || attrList_DCCT["Valid"].isNull()) {
      ATH_MSG_DEBUG ("Can't find information for channel " << m_DCCT);
      return StatusCode::SUCCESS;
    }

    const coral::AttributeList& attrList_DCCT24 = attrList_link->attributeList (m_DCCT24);

    if (attrList_DCCT24.size() == 0 || attrList_DCCT24["Valid"].isNull()) {
      ATH_MSG_DEBUG ("Can't find information for channel " << m_DCCT24);
      return StatusCode::SUCCESS;
    }

    // const auto& thisevt = ctx.eventID();

    if (m_mode == 1) {
      ATH_MSG_INFO("Assuming MC case");
      ATH_MSG_INFO("Got AttributeList with size " << attrList.size());
      ATH_MSG_INFO("Got AttributeList fBCT with size " << attrList_fBCT.size());
      ATH_MSG_INFO("Got AttributeList DCCT with size " << attrList_DCCT.size());
      ATH_MSG_INFO("Got AttributeList DCCT24 with size " << attrList_DCCT24.size());

    }
    else { // mode == 0, Data-case
  
      float nb1 = attrList["Beam1Intensity"].data<cool::Float>();
      float nb2 = attrList["Beam2Intensity"].data<cool::Float>();
      float nb1_fBCT = attrList_fBCT["Beam1Intensity"].data<cool::Float>();
      float nb2_fBCT = attrList_fBCT["Beam2Intensity"].data<cool::Float>();
      float nb1_DCCT = attrList_DCCT["Beam1Intensity"].data<cool::Float>();
      float nb2_DCCT = attrList_DCCT["Beam2Intensity"].data<cool::Float>();
      float nb1_DCCT24 = attrList_DCCT24["Beam1Intensity"].data<cool::Float>();
      float nb2_DCCT24 = attrList_DCCT24["Beam2Intensity"].data<cool::Float>();

      float nb1All = attrList["Beam1IntensityAll"].data<cool::Float>();
      float nb2All = attrList["Beam2IntensityAll"].data<cool::Float>();
      float nb1All_fBCT = attrList_fBCT["Beam1IntensityAll"].data<cool::Float>();
      float nb2All_fBCT = attrList_fBCT["Beam2IntensityAll"].data<cool::Float>();
      float nb1All_DCCT = attrList_DCCT["Beam1IntensityAll"].data<cool::Float>();
      float nb2All_DCCT = attrList_DCCT["Beam2IntensityAll"].data<cool::Float>();
      float nb1All_DCCT24 = attrList_DCCT24["Beam1IntensityAll"].data<cool::Float>();
      float nb2All_DCCT24 = attrList_DCCT24["Beam2IntensityAll"].data<cool::Float>();

      float enb1 = attrList["Beam1IntensityStd"].data<cool::Float>();
      float enb2 = attrList["Beam2IntensityStd"].data<cool::Float>();
      float enb1_fBCT = attrList_fBCT["Beam1IntensityStd"].data<cool::Float>();
      float enb2_fBCT = attrList_fBCT["Beam2IntensityStd"].data<cool::Float>();
      float enb1_DCCT = attrList_DCCT["Beam1IntensityStd"].data<cool::Float>();
      float enb2_DCCT = attrList_DCCT["Beam2IntensityStd"].data<cool::Float>();
      float enb1_DCCT24 = attrList_DCCT24["Beam1IntensityStd"].data<cool::Float>();
      float enb2_DCCT24 = attrList_DCCT24["Beam2IntensityStd"].data<cool::Float>();

      float enb1All = attrList["Beam1IntensityAllStd"].data<cool::Float>();
      float enb2All = attrList["Beam2IntensityAllStd"].data<cool::Float>();
      float enb1All_fBCT = attrList_fBCT["Beam1IntensityAllStd"].data<cool::Float>();
      float enb2All_fBCT = attrList_fBCT["Beam2IntensityAllStd"].data<cool::Float>();
      float enb1All_DCCT = attrList_DCCT["Beam1IntensityAllStd"].data<cool::Float>();
      float enb2All_DCCT = attrList_DCCT["Beam2IntensityAllStd"].data<cool::Float>();
      float enb1All_DCCT24 = attrList_DCCT24["Beam1IntensityAllStd"].data<cool::Float>();
      float enb2All_DCCT24 = attrList_DCCT24["Beam2IntensityAllStd"].data<cool::Float>();

      unsigned long long RunLB = attrList["RunLB"].data<unsigned long long >();

      int run = RunLB>>32;
      int lb = RunLB & 0xffffffff;

      ATH_MSG_DEBUG( "Beam1Intensity: " << nb1 );
      ATH_MSG_DEBUG( "Beam2Intensity: " << nb2 );
      ATH_MSG_DEBUG( "Beam1Intensity (fBCT): " << nb1_fBCT );
      ATH_MSG_DEBUG( "Beam2Intensity (fBCT): " << nb2_fBCT );
      ATH_MSG_DEBUG( "Beam1Intensity (DCCT): " << nb1_DCCT );
      ATH_MSG_DEBUG( "Beam2Intensity (DCCT): " << nb2_DCCT );
      ATH_MSG_DEBUG( "Beam1Intensity (DCCT24): " << nb1_DCCT24 );
      ATH_MSG_DEBUG( "Beam2Intensity (DCCT24): " << nb2_DCCT24 );
      ATH_MSG_DEBUG( "Run: " << run );
      ATH_MSG_DEBUG( "LB: " << lb );

      if (m_isRun1) {   //Assume run1 layout as explained at https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CoolOnlineData
        ATH_MSG_DEBUG("Assuming run 1 database");
        
      }
      else { 
        bccd->SetRunLB(RunLB);

        bccd->SetBeam1Intensity(nb1,0);
        bccd->SetBeam2Intensity(nb2,0);
        bccd->SetBeam1Intensity(nb1_fBCT,1);
        bccd->SetBeam2Intensity(nb2_fBCT,1);
        bccd->SetBeam1Intensity(nb1_DCCT,2);
        bccd->SetBeam2Intensity(nb2_DCCT,2);
        bccd->SetBeam1Intensity(nb1_DCCT24,3);
        bccd->SetBeam2Intensity(nb2_DCCT24,3);

        bccd->SetBeam1IntensityAll(nb1All,0);
        bccd->SetBeam2IntensityAll(nb2All,0);
        bccd->SetBeam1IntensityAll(nb1All_fBCT,1);
        bccd->SetBeam2IntensityAll(nb2All_fBCT,1);
        bccd->SetBeam1IntensityAll(nb1All_DCCT,2);
        bccd->SetBeam2IntensityAll(nb2All_DCCT,2);
        bccd->SetBeam1IntensityAll(nb1All_DCCT24,3);
        bccd->SetBeam2IntensityAll(nb2All_DCCT24,3);
        
        bccd->SetBeam1IntensitySTD(enb1,0);
        bccd->SetBeam2IntensitySTD(enb2,0);
        bccd->SetBeam1IntensitySTD(enb1_fBCT,1);
        bccd->SetBeam2IntensitySTD(enb2_fBCT,1);
        bccd->SetBeam1IntensitySTD(enb1_DCCT,2);
        bccd->SetBeam2IntensitySTD(enb2_DCCT,2);
        bccd->SetBeam1IntensitySTD(enb1_DCCT24,3);
        bccd->SetBeam2IntensitySTD(enb2_DCCT24,3);

        bccd->SetBeam1IntensityAllSTD(enb1All,0);
        bccd->SetBeam2IntensityAllSTD(enb2All,0);
        bccd->SetBeam1IntensityAllSTD(enb1All_fBCT,1);
        bccd->SetBeam2IntensityAllSTD(enb2All_fBCT,1);
        bccd->SetBeam1IntensityAllSTD(enb1All_DCCT,2);
        bccd->SetBeam2IntensityAllSTD(enb2All_DCCT,2);
        bccd->SetBeam1IntensityAllSTD(enb1All_DCCT24,3);
        bccd->SetBeam2IntensityAllSTD(enb2All_DCCT24,3);
        


      }//end else run2 
      //Filled bcid-bitsets, now extract trains
      // bccd->m_trains=findTrains(bccd->m_luminous, m_maxBunchSpacing,m_minBunchesPerTrain);
    }//end else is data
  }

  ATH_CHECK( writeHdl.record (std::move (bccd)) );
  return StatusCode::SUCCESS;
}


/**
 * This helper function is used to convert a string of type
 * "[0.0, 0.0, 1.0, 1.0, 1.0]" into a vector of floats. As it happens, the
 * digitization stores the bunch pattern in such a fancy style...
 *
 * @param pattern The pattern extracted from the MC file metadata
 * @returns The "decoded" bunch pattern
 */
std::vector<float> BunchCrossingAverageCondAlg::tokenize( const std::string& pattern ) const {
  ATH_MSG_VERBOSE("Input to tokenize: " << pattern);
  
  std::vector< float > result;
  const char* c= pattern.data();
  const char* cEnd= c + pattern.size();
  while(c<cEnd) {
    //This loop swallows actually any string containing numbers
    //separated by non-numbers
    char* end;
    float f=std::strtof(c,&end);
    if (c==end) {//Can't convert, try next
      c++;
    }
    else { //got a value
      result.push_back(f);
      c=end;
    }
  }//end while loop
  return result;
}
