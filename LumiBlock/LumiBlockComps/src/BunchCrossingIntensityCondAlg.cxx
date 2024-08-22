/*
 * Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration.
 */

#include "BunchCrossingIntensityCondAlg.h"
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

StatusCode BunchCrossingIntensityCondAlg::initialize() {


  if (m_mode == 2) {
    ATH_CHECK( m_trigConfigSvc.retrieve() );
  }

  ATH_CHECK( m_fillParamsFolderKey.initialize( m_mode == 0 || m_mode == 1 ) );
  ATH_CHECK( m_lumiCondDataKey.initialize( m_mode == 3 ) );
  ATH_CHECK( m_outputKey.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode BunchCrossingIntensityCondAlg::execute (const EventContext& ctx) const {

  SG::WriteCondHandle<BunchCrossingIntensityCondData> writeHdl(m_outputKey, ctx);
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
  auto bccd=std::make_unique<BunchCrossingIntensityCondData>();

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

    // const auto& thisevt = ctx.eventID();

    if (m_mode == 1) {
      ATH_MSG_INFO("Assuming MC case");
      ATH_MSG_INFO("Got AttributeList with size " << attrList.size());
      ATH_MSG_INFO("Got AttributeList fBCT with size " << attrList_fBCT.size());
      
    }
    else { // mode == 0, Data-case

      float nb1 = attrList["B1BunchAverage"].data<cool::Float>();
      float nb2 = attrList["B2BunchAverage"].data<cool::Float>();
      float nb1_fBCT = attrList_fBCT["B1BunchAverage"].data<cool::Float>();
      float nb2_fBCT = attrList_fBCT["B2BunchAverage"].data<cool::Float>();

      unsigned long long RunLB = attrList["RunLB"].data<unsigned long long >();

      int run = RunLB>>32;
      int lb = RunLB & 0xffffffff;
      ATH_MSG_DEBUG( "Beam1Intensity: " << nb1 );
      ATH_MSG_DEBUG( "Beam2Intensity: " << nb2 );
      ATH_MSG_DEBUG( "Run: " << run );
      ATH_MSG_DEBUG( "LB: " << lb );

      const coral::Blob& blob1 = attrList["B1BunchIntensities"].data<coral::Blob>();
      const coral::Blob& blob2 = attrList["B2BunchIntensities"].data<coral::Blob>();
      const coral::Blob& blob1_fBCT = attrList_fBCT["B1BunchIntensities"].data<coral::Blob>();
      const coral::Blob& blob2_fBCT = attrList_fBCT["B2BunchIntensities"].data<coral::Blob>();


      if (m_isRun1) {   //Assume run1 layout as explained at https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CoolOnlineData
        ATH_MSG_DEBUG("Assuming run 1 database");

      }
      else { 

        bccd->SetBeam1IntensityAll(nb1,0);
        bccd->SetBeam2IntensityAll(nb2,0);
        bccd->SetBeam1IntensityAll(nb1_fBCT,1);
        bccd->SetBeam2IntensityAll(nb2_fBCT,1);
        bccd->SetRunLB(RunLB);

        //=========== BPTX===================
        //Blob1
        // Hardcode the Run2 BLOB decoding (should use CoolLumiUtilities...)
        const uint8_t* ATH_RESTRICT pchar1 = static_cast<const uint8_t*>(blob1.startingAddress()); // First byte holds storage size and mode
        ++pchar1;  // Points to next char after header
        unsigned int nbcids = BunchCrossingIntensityCondData::m_MAX_BCID;

        std::vector<float> instBeam1 (nbcids);
        for (unsigned int i=0; i<nbcids; i++) {
          // Can't use assignment directly because source may be misaligned.
          instBeam1[i] = CxxUtils::get_unaligned_float (pchar1);
          
        }

        // Blob2
        // Hardcode the Run2 BLOB decoding (should use CoolLumiUtilities...)
        const uint8_t* ATH_RESTRICT pchar2 = static_cast<const uint8_t*>(blob2.startingAddress()); // First byte holds storage size and mode
        ++pchar2;  // Points to next char after header
        std::vector<float> instBeam2 (nbcids);
        for (unsigned int i=0; i<nbcids; i++) {
          // Can't use assignment directly because source may be misaligned.
          instBeam2[i] = CxxUtils::get_unaligned_float (pchar2);
          
        }

        int num_bc = 0;
        for (unsigned int i=0; i<nbcids; i++) {
          if(instBeam1[i]!=0 && instBeam2[i] !=0)num_bc++;
        }

        bccd->setBeam1IntensityPerBCIDVector(std::move(instBeam1),0);
        bccd->setBeam2IntensityPerBCIDVector(std::move(instBeam2),0);
        //


        //=========== fBCT ===================
        //Blob1
        // Hardcode the Run2 BLOB decoding (should use CoolLumiUtilities...)
        const uint8_t* ATH_RESTRICT pchar1_fBCT = static_cast<const uint8_t*>(blob1_fBCT.startingAddress()); // First byte holds storage size and mode
        ++pchar1_fBCT;  // Points to next char after header
        unsigned int nbcids_fBCT = BunchCrossingIntensityCondData::m_MAX_BCID;

        std::vector<float> instBeam1_fBCT (nbcids_fBCT);
        for (unsigned int i=0; i<nbcids_fBCT; i++) {
          // Can't use assignment directly because source may be misaligned.
          instBeam1_fBCT[i] = CxxUtils::get_unaligned_float (pchar1_fBCT);
          
        }

        // Blob2
        // Hardcode the Run2 BLOB decoding (should use CoolLumiUtilities...)
        const uint8_t* ATH_RESTRICT pchar2_fBCT = static_cast<const uint8_t*>(blob2_fBCT.startingAddress()); // First byte holds storage size and mode
        ++pchar2_fBCT;  // Points to next char after header
        std::vector<float> instBeam2_fBCT (nbcids_fBCT);
        for (unsigned int i=0; i<nbcids_fBCT; i++) {
          // Can't use assignment directly because source may be misaligned.
          instBeam2_fBCT[i] = CxxUtils::get_unaligned_float (pchar2_fBCT);
          
        }
        int num_bc_fBCT = 0;
        for (unsigned int i=0; i<nbcids_fBCT; i++) {
          if(instBeam1_fBCT[i]!=0 && instBeam2_fBCT[i] !=0)num_bc_fBCT++;
        }

        bccd->setBeam1IntensityPerBCIDVector(std::move(instBeam1_fBCT),1);
        bccd->setBeam2IntensityPerBCIDVector(std::move(instBeam2_fBCT),1);


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
std::vector<float> BunchCrossingIntensityCondAlg::tokenize( const std::string& pattern ) const {
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
