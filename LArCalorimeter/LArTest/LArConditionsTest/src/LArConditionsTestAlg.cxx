/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LArConditionsTestAlg.cxx
 *
 * @brief This file contains an algorithm for testing lar conditions
 * data access
 *
 * @author RD Schaffer  <R.D.Schaffer@cern.ch>
 * @author Hong Ma      <hma@bnl.gov>
 */

#include "LArConditionsTest/LArConditionsTestAlg.h"

#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "LArRawConditions/LArRampMC.h" 
#include "LArRawConditions/LArConditionsChannelSet.h" 
#include "LArElecCalib/ILArOFC.h" 

#include "StoreGate/ReadCondHandle.h"

#include "AthenaKernel/errorcheck.h"

/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR:
/////////////////////////////////////////////////////////////////////

LArConditionsTestAlg::LArConditionsTestAlg(const std::string& name, ISvcLocator* pSvcLocator) :
	AthAlgorithm(name,pSvcLocator)
{
}

/////////////////////////////////////////////////////////////////////
// DESTRUCTOR:
/////////////////////////////////////////////////////////////////////

LArConditionsTestAlg::~LArConditionsTestAlg()
{  }

/////////////////////////////////////////////////////////////////////
// INITIALIZE:
/////////////////////////////////////////////////////////////////////

StatusCode LArConditionsTestAlg::initialize()
{
    ATH_MSG_DEBUG ( " TestCondObjs flag         = " << m_testCondObjs );
    ATH_MSG_DEBUG ( " ReadCondObjs flag         = " << m_readCondObjs );
    ATH_MSG_DEBUG ( " WriteCondObjs flag        = " << m_writeCondObjs );
    ATH_MSG_DEBUG ( " WriteCorrections flag     = " << m_writeCorrections );
    ATH_MSG_DEBUG ( " ApplyCorrections flag     = " << m_applyCorrections );
    ATH_MSG_DEBUG ( " TestReadDBDirect flag     = " << m_testReadDB );
    ATH_MSG_DEBUG ( " Testbeam flag             = " << m_TB );

    ATH_CHECK( detStore()->retrieve(m_onlineID) );

    const CaloCell_ID* calocell_id = nullptr;
    ATH_CHECK( detStore()->retrieve(calocell_id) );

    ATH_MSG_DEBUG ( "initialize done" );
    return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// EXECUTE:
/////////////////////////////////////////////////////////////////////

StatusCode LArConditionsTestAlg::execute()
{
    ATH_MSG_DEBUG ( " retrieve DataHandle<ILArRamp>  in execute " );

    if(m_testCondObjs){ 

	// create cache
        ATH_CHECK(  testCondObjects() );
// 	StatusCode sc = testDbObjectRead();
// 	if(sc.isFailure()) {
//   	    log << MSG::ERROR << "Failed testDbObjectRead " << endmsg;
//   	    return StatusCode::FAILURE;
//   	}
	
    }
    

//    if(m_testCondObject){ 
// 	std::string key = "LArRamp";
// 	const ILArRamp* ramp = 0 ;
// 	detStore()->retrieve(ramp, key);
// 	if(!ramp) {
// 	    log<< MSG::ERROR<<" Failed to get LArRamp in execute " << endmsg;
// 	    return StatusCode::FAILURE ; 
// 	}
//     } 

    if(m_TB){ 
	const ILArOFC* ofc = nullptr;
	ATH_CHECK( detStore()->retrieve(ofc, "LArOFC") );

	const ILArRamp* ramp = nullptr;
	ATH_CHECK( detStore()->retrieve(ramp, "LArRamp") );
    } 

    return StatusCode::SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// FINALIZE:
// Note that it is NOT NECESSARY to run the finalize of individual
// sub-algorithms.  The framework takes care of it.
/////////////////////////////////////////////////////////////////////

StatusCode LArConditionsTestAlg::finalize()
{
    if(m_testReadDB)  ATH_CHECK(testDbObjectRead());
    return StatusCode::SUCCESS; 
} 


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode
LArConditionsTestAlg::createCompareObjects()
{
    // StatusCode sc;
    ATH_MSG_INFO ("in createCompareObjects()" );

    // Create set of ids, LArRampComplete::LArCondObj

    std::vector<HWIdentifier>::const_iterator chanIt  = m_onlineID->channel_begin();
    std::vector<HWIdentifier>::const_iterator chanEnd = m_onlineID->channel_end();
    int ichan           = -1;
    int icorr           = 0;
    float vramp         = 0;
    int gain            = 0;
    for (; chanIt != chanEnd; ++chanIt, ++ichan) {
	// add channels with downscale factor
	if (ichan % 1000 != 5) continue;

	// Create ramp with 3 vRamp elements
	LArRampPTmp ramp((*chanIt), gain);
	ramp.m_vRamp.push_back(vramp);
	vramp += 1.0;
	ramp.m_vRamp.push_back(vramp);
	vramp += 1.0;
	ramp.m_vRamp.push_back(vramp);
	vramp += 1.0;
	// add to cache
	m_rampCache.push_back(ramp);
	
	// Change gain each time
	gain = (gain == 2) ? 0 : gain + 1;
	
	if (m_writeCorrections || m_applyCorrections) {
	    // Create downscaled corrections
	    ++icorr;
	    if (icorr % 10 != 5) continue;
	    // Just change sign of ramp values
	    for (unsigned int i = 0; i < 3; ++i)ramp.m_vRamp[i] = -ramp.m_vRamp[i];
	    m_rampCorrections.push_back(ramp);
	}
    }
    
    // Print out cache and corrections
    for (unsigned int i = 0; i < m_rampCache.size(); ++i) {
      ATH_MSG_DEBUG ("Cache: chan, gain, ramps "
                     << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 
                     << m_rampCache[i].m_gain << " " 
                     << m_rampCache[i].m_vRamp[0] << " " 
                     << m_rampCache[i].m_vRamp[1] << " " 
                     << m_rampCache[i].m_vRamp[2] << " " );
    }
    for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
      ATH_MSG_DEBUG ("Corrections: chan, gain, ramps "
                     << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                     << m_rampCorrections[i].m_gain << " " 
                     << m_rampCorrections[i].m_vRamp[0] << " " 
                     << m_rampCorrections[i].m_vRamp[1] << " " 
                     << m_rampCorrections[i].m_vRamp[2] << " " );
    }
    
    ATH_MSG_DEBUG ( "End of create comparison objects " );
    return StatusCode::SUCCESS; 
} 

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

inline bool operator == (const LArRampComplete::LArCondObj& r1, const LArRampPTmp& r2) 
{
    // Comparison of two LArRampComplete::LArCondObj objects
    if (r1.m_vRamp.size() != r2.m_vRamp.size()) return (false);
    for (unsigned int i = 0; i < r1.m_vRamp.size(); ++i) {
	if (r1.m_vRamp[i] != r2.m_vRamp[i]) return (false);
    }
    return (true);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

inline bool CorrectionCompare (const LArRampComplete::LArCondObj& r1, const LArRampPTmp& r2) 
{
    // Comparison of two LArRampComplete::LArCondObj objects
    if (r1.m_vRamp.size() != r2.m_vRamp.size()) return (false);
    for (unsigned int i = 0; i < r1.m_vRamp.size(); ++i) {
	if (r1.m_vRamp[i] != -r2.m_vRamp[i]) return (false);
    }
    return (true);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

inline bool operator != (const LArRampComplete::LArCondObj& r1, const LArRampPTmp& r2) 
{
    if(r1 == r2)return (false);
    return (true);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode
LArConditionsTestAlg::testCondObjects()
{
    ATH_MSG_INFO ("in testCondObjects()" );

    static std::atomic<bool> first = true;
    if (!first) {
        ATH_MSG_INFO ("Multiple entries - returning" );
	return StatusCode::SUCCESS; 
    }
    first = false;
    
    typedef LArRampMC::CONTAINER            CONTAINER; 
    typedef CONTAINER::chan_const_iterator  chan_const_iterator;
    typedef CONTAINER::iov_const_iterator   iov_const_iterator;
    
    ATH_CHECK( createCompareObjects() );
    ATH_CHECK( testChannelSet() );

    const LArRampMC* ramps = 0;
    
    // Create SingleGroup
    if (m_readCondObjs) {
	const ILArRamp* iramps = 0;
	ATH_CHECK( detStore()->retrieve(iramps, "/LArCalorimeter/LArTests/LArRampsSingleGroup") );
	ATH_MSG_INFO ( "Retrieved ramps for LArRampsSingleGroup " );
	ramps = dynamic_cast<const LArRampMC*>(iramps);
	if (!ramps) {
            ATH_MSG_ERROR ("Could not dynamic cast ILArRamp to LArRampMC" );
	    return( StatusCode::FAILURE);
	}
    }
    else {
	LArRampMC* ramps_rw = new LArRampMC;
	ramps = ramps_rw;
	ATH_MSG_INFO ( "Created ramps for LArRampsSingleGroup "  );
	ramps_rw->setGroupingType(LArConditionsContainerBase::SingleGroup);
	ATH_CHECK( ramps_rw->initialize() );
    }
    
    ATH_CHECK( testEachCondObject(ramps) );
    ATH_MSG_INFO ( "Succeeded SingleGroup test  " );

    if (!m_readCondObjs) {
	// Save in DetectorStore
        ATH_CHECK( detStore()->record(ramps, "/LArCalorimeter/LArTests/LArRampsSingleGroup") );
	const ILArRamp* iramps = 0;
        ATH_CHECK( detStore()->symLink(ramps, iramps) );

	/// Statistics: total number of conditions 
        if (ramps) {
          ATH_MSG_DEBUG  ( "Total number of conditions objects"
                           << ramps->totalNumberOfConditions() );
        }
	if (!m_writeCondObjs) {
	    // Remove conditions objects if not writing out
	    LArRampMC* ramps_rw = const_cast<LArRampMC*>(ramps);
	    if (!ramps_rw) {
                ATH_MSG_ERROR ( "Could not const cast to LArRampMC " );
		return StatusCode::FAILURE;
	    }
	    ramps_rw->removeConditions();
	    ATH_MSG_DEBUG  ( "Removed conditions objects" );
	}
	ATH_MSG_DEBUG  ( "Total number of conditions objects "
                         << ramps->totalNumberOfConditions() );
	ATH_MSG_DEBUG  ( "Total number of correction objects"
                         << ramps->totalNumberOfCorrections() );
    }
    
    // Create SubDetectorGrouping
    if (m_readCondObjs) {
	const ILArRamp* iramps = 0;
	ATH_CHECK( detStore()->retrieve(iramps, "/LArCalorimeter/LArTests/LArRampsSubDetectorGrouping") );
	ATH_MSG_INFO ( "Retrieved ramps for LArRampsSubDetectorGrouping "  );
	ramps = dynamic_cast<const LArRampMC*>(iramps);
	if (!ramps) {
          ATH_MSG_ERROR ("Could not dynamic cast ILArRamp to LArRampMC" );
          return( StatusCode::FAILURE);
	}
    }
    else {
	LArRampMC* ramps_rw = new LArRampMC;
	ramps = ramps_rw;
	//ramps_rw->setGroupingType(LArConditionsContainerBase::SubDetectorGrouping);
	ATH_CHECK( ramps_rw->initialize() );
    }
    
    ATH_CHECK( testEachCondObject(ramps) );
    ATH_MSG_INFO ( "Succeeded SubDetectorGrouping test  " );

    if (!m_readCondObjs) {
	// Save in DetectorStore
        ATH_CHECK( detStore()->record(ramps, "/LArCalorimeter/LArTests/LArRampsSubDetectorGrouping") );
	const ILArRamp* iramps = 0;
	ATH_CHECK( detStore()->symLink(ramps, iramps) );
	/// Statistics: total number of conditions 
        if (ramps) {
          ATH_MSG_DEBUG  ( "Total number of conditions objects"
                           << ramps->totalNumberOfConditions() );
        }
	if (!m_writeCondObjs) {
	    // Remove conditions objects if not writing out
	    LArRampMC* ramps_rw = const_cast<LArRampMC*>(ramps);
	    if (!ramps_rw) {
                ATH_MSG_ERROR ( "Could not const cast to LArRampMC " );
		return StatusCode::FAILURE;
	    }
	    ramps_rw->removeConditions();
	    ATH_MSG_DEBUG  ( "Removed conditions objects" );
	}
	ATH_MSG_DEBUG  ( "Total number of conditions objects "
                         << ramps->totalNumberOfConditions() );
	ATH_MSG_DEBUG  ( "Total number of correction objects"
                         << ramps->totalNumberOfConditions() );
    }
    
    // Create FeedThroughGrouping
    if (m_readCondObjs) {
	const ILArRamp* iramps = 0;
	ATH_CHECK( detStore()->retrieve(iramps, "/LArCalorimeter/LArTests/LArRampsFeedThroughGrouping") );
	ATH_MSG_INFO ( "Retrieved ramps for LArRampsFeedThroughGrouping " );
	ramps = dynamic_cast<const LArRampMC*>(iramps);
	if (!ramps) {
            ATH_MSG_ERROR ("Could not dynamic cast ILArRamp to LArRampMC" );
	    return( StatusCode::FAILURE);
	}
    }
    else {
	LArRampMC* ramps_rw = new LArRampMC;
	ramps = ramps_rw;
	ramps_rw->setGroupingType(LArConditionsContainerBase::FeedThroughGrouping);
	ATH_CHECK( ramps_rw->initialize() );
    }
    
    ATH_CHECK( testEachCondObject(ramps) );
    ATH_MSG_INFO ( "Succeeded FeedThroughGrouping test  " );

    if (!m_readCondObjs) {
	// Save in DetectorStore
      ATH_CHECK( detStore()->record(ramps, "/LArCalorimeter/LArTests/LArRampsFeedThroughGrouping") );
	const ILArRamp* iramps = 0;
	ATH_CHECK( detStore()->symLink(ramps, iramps) );
	/// Statistics: total number of conditions 
        if (ramps) {
          ATH_MSG_DEBUG  ( "Total number of conditions objects"
                           << ramps->totalNumberOfConditions() );
        }
	if (!m_writeCondObjs) {
	    // Remove conditions objects if not writing out
	    LArRampMC* ramps_rw = const_cast<LArRampMC*>(ramps);
	    if (!ramps_rw) {
                ATH_MSG_ERROR ( "Could not const cast to LArRampMC " );
		return StatusCode::FAILURE;
	    }
	    ramps_rw->removeConditions();
	    ATH_MSG_DEBUG  ( "Removed conditions objects" );
	}
	ATH_MSG_DEBUG  ( "Total number of conditions objects "
                         << ramps->totalNumberOfConditions() );
	ATH_MSG_DEBUG  ( "Total number of correction objects"
                         << ramps->totalNumberOfCorrections() );
    }

    ATH_MSG_DEBUG ( "Statistics for LArRampsFeedThroughGrouping " );
    ATH_MSG_DEBUG ( "Number of channels, iovs "
                    << ramps->chan_size() << " " << ramps->iov_size() );

    iov_const_iterator iovIt  = ramps->iov_begin();
    iov_const_iterator iovEnd = ramps->iov_end  ();
    msg() << MSG::DEBUG << "IOVs found:  ";
    for (; iovIt != iovEnd; ++iovIt) {
      msg() << MSG::DEBUG << (*iovIt) << ", ";
    }
    msg() << MSG::DEBUG << endmsg;
    
    chan_const_iterator chIt  = ramps->chan_begin();
    chan_const_iterator chEnd = ramps->chan_end  ();
    for (; chIt != chEnd; ++chIt) {
      ATH_MSG_DEBUG ( "Channel:  " << (*chIt) 
                      << " number of conditions: " << ramps->conditionsPerChannel((*chIt)) );
    }
    
    for (unsigned int i = 0; i < ramps->nGroups(); ++i) {
      ATH_MSG_DEBUG ( "Group:  " << i 
                      << " number of conditions: " << ramps->conditionsPerGroup(i) );
    }
    ATH_MSG_DEBUG ("");
    
    for (unsigned int i = 0; i < ramps->nGains(); ++i) {
      ATH_MSG_DEBUG ( "Gain:  " << i 
                      << " number of conditions: " << ramps->conditionsPerGain(i) );
    }
    ATH_MSG_DEBUG  ( "Total number of conditions objects "
                     << ramps->totalNumberOfConditions() );
    ATH_MSG_DEBUG  ( "Total number of correction objects "
                     << ramps->totalNumberOfCorrections() );

    
    ATH_MSG_DEBUG ( "End of testCondObjects " );

    return StatusCode::SUCCESS; 
} 

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode
LArConditionsTestAlg::testEachCondObject ATLAS_NOT_THREAD_SAFE (const LArRampMC* ramps)
{
    ATH_MSG_INFO ("in testEachCondObject()" );
    bool error = false;
    
    typedef LArRampMC::CONTAINER         CONTAINER; 
    //typedef CONTAINER::ConstCorrectionIt ConstCorrectionIt;

    // Cast into r/w for tests
    LArRampMC* ramps_rw = const_cast<LArRampMC*>(ramps);
    if (!ramps_rw) {
        ATH_MSG_ERROR ( "Could not const cast to LArRampMC " );
	return StatusCode::FAILURE;
    }

    if (ramps_rw->correctionsApplied())
      ATH_CHECK(ramps_rw->undoCorrections());

    if (!m_readCondObjs) {
	if (m_writeCondObjs) {
	    
	    for (unsigned int i = 0; i < m_rampCache.size(); ++i) {
              ATH_MSG_DEBUG ("setPdata for chan, chan id, gain " << i << " "
                             << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 
                             << m_rampCache[i].m_gain << " " );

		// Must copy LArRampPTmp into a LArRampComplete::LArCondObj
		LArRampComplete::LArCondObj ramp;
		ramp.m_vRamp = m_rampCache[i].m_vRamp;

		ramps_rw->setPdata(m_rampCache[i].m_channelID, 
				   ramp,
				   m_rampCache[i].m_gain);
	    }
	}
	
	ATH_MSG_DEBUG ( "Finished conditions, now write corrections " );

	if (m_writeCorrections) {
	    for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {

              ATH_MSG_DEBUG ("insert corr for chan, chan id, gain " << i << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " );

		// Must copy LArRampPTmp into a LArRampComplete::LArCondObj
		LArRampComplete::LArCondObj ramp;
		ramp.m_vRamp = m_rampCorrections[i].m_vRamp;

		ATH_CHECK( ramps_rw->insertCorrection(m_rampCorrections[i].m_channelID, 
                                                      ramp,
                                                      m_rampCorrections[i].m_gain) );
	    }
	}
    }
    
    ATH_MSG_DEBUG ("Number of channels, iovs "
                   << ramps->chan_size() << " " << ramps->iov_size() );

    CONTAINER::chan_const_iterator   chanIt1  = ramps->chan_begin();
    CONTAINER::chan_const_iterator   endChan1 = ramps->chan_end  ();
    for (unsigned int i = 0; chanIt1 != endChan1; ++chanIt1, ++i) {
	const CONTAINER::Subset* subset = ramps->at(i);
	ATH_MSG_DEBUG ( "Index " << i 
	    << " channel "           << subset->channel() 
	    << " gain "              << subset->gain() 
	    << " groupingType "      << subset->groupingType()
	    << " subsetSize "        << subset->subsetSize()
	    << " correctionVecSize " << subset->correctionVecSize() );
	if ((*chanIt1) != subset->channel()) {
          ATH_MSG_ERROR ( "Channel numbers not the same for MultChanColl and subset: " 
		<< i 
		<< " multchan "           << (*chanIt1)
		<< " subset   "           << subset->channel() );
	    error = true;
	}
    }

    ATH_MSG_DEBUG ("Number of channels, iovs, subsets "
                   << ramps->chan_size() << " " 
                   << ramps->iov_size()  << " "
                   << ramps->size()  << " " );

    ATH_MSG_DEBUG ("Compare LArRampMC with cache " );
    // Now loop over ramps and compare with cache
    for (unsigned int i = 0; i < m_rampCache.size(); ++i) {

	LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCache[i].m_channelID, 
					      m_rampCache[i].m_gain);
	unsigned int coolChannel = ramps->coolChannel(m_rampCache[i].m_channelID, 
						      m_rampCache[i].m_gain);

	if (!rampP.isEmpty()) {
          ATH_MSG_DEBUG ("New  : cool chan, chan id, gain, ramps "
                         << coolChannel << " "
                         << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 	
                         << m_rampCache[i].m_gain << " " 
                         << rampP.m_vRamp[0] << " " 
                         << rampP.m_vRamp[1] << " " 
                         << rampP.m_vRamp[2] << " " );
	}
	else {
          ATH_MSG_DEBUG ("New  : isEmpty " );
	}
	ATH_MSG_DEBUG ("Cache: cool chan, chan id, gain, ramps "
                       << coolChannel << " "
                       << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 	
                       << m_rampCache[i].m_gain << " " 
                       << m_rampCache[i].m_vRamp[0] << " " 
                       << m_rampCache[i].m_vRamp[1] << " " 
                       << m_rampCache[i].m_vRamp[2] << " " 
                       << " Compare = " << (rampP == m_rampCache[i]) );
	if (rampP != m_rampCache[i] && !rampP.isEmpty()) {
            ATH_MSG_ERROR ("LArRampMC and cache NOT equal" );
	    error = true;
	}
    }


    // Now loop over ramps using generic iterator and compare with cache
    ATH_MSG_DEBUG ("Compare LArRampMC with cache using iterator " );
    CONTAINER::ConstConditionsMapIterator rampIt;
    CONTAINER::ConstConditionsMapIterator rampEnd;
    for (unsigned int gain = 0; gain < 3; ++gain) {
	rampIt  = ramps->begin(gain);	    
	rampEnd = ramps->end  (gain);
	for (unsigned int i = 0; i < m_rampCache.size(); ++i) {
	    // cache is not in order for gains, select the current gain
	    if (gain != m_rampCache[i].m_gain) continue; 
	    LArRampComplete::LArCondObj rampP;
	    HWIdentifier rampId;
	    while(rampIt != rampEnd) {
		rampP = *rampIt;
		rampId = rampIt.channelId();
		++rampIt;
		if (!rampP.isEmpty()) break; // break out for first non-empty ramp
	    }
	    unsigned int coolChannel = ramps->coolChannel(m_rampCache[i].m_channelID, 
							  m_rampCache[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New  : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(rampId) << " " 
                             << m_rampCache[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " " );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    ATH_MSG_DEBUG ("Cache: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 	
                           << m_rampCache[i].m_gain << " " 
                           << m_rampCache[i].m_vRamp[0] << " " 
                           << m_rampCache[i].m_vRamp[1] << " " 
                           << m_rampCache[i].m_vRamp[2] << " " 
                           << " Compare = " << (rampP == m_rampCache[i]) );
	    if (rampP != m_rampCache[i] && !rampP.isEmpty()) {
                ATH_MSG_ERROR ("LArRampMC and cache NOT equal" );
		error = true;
	    }
	}
    }
    


    // Now loop over ramps in pieces using the selector on febids to
    // iterate and compare with cache
    ATH_MSG_DEBUG ("Compare LArRampMC with cache using iterator and febid selection " );
    // Loop over cache and divide the febids into three sets, where
    // each set is an array of size 3 for the separate gains
    std::vector<unsigned int> ids1[3];
    std::vector<unsigned int> ids2[3];
    std::vector<unsigned int> ids3[3];
    for (unsigned int i = 0; i < m_rampCache.size(); ++i) {
	if (i < m_rampCache.size()/3) {
	    unsigned int id =  m_onlineID->feb_Id(m_rampCache[i].m_channelID).get_identifier32().get_compact();
	    ids1[m_rampCache[i].m_gain].push_back(id);
	}
	else if (i < 2*m_rampCache.size()/3) {
	    unsigned int id =  m_onlineID->feb_Id(m_rampCache[i].m_channelID).get_identifier32().get_compact();
	    ids2[m_rampCache[i].m_gain].push_back(id);
	}
	else {
	    unsigned int id =  m_onlineID->feb_Id(m_rampCache[i].m_channelID).get_identifier32().get_compact();
	    ids3[m_rampCache[i].m_gain].push_back(id);
	}
    }

    for (unsigned int gain = 0; gain < 3; ++gain) {
	for (unsigned int febSet = 0; febSet < 3; ++febSet) {
	    unsigned int i0   = 0; 
	    unsigned int iend = m_rampCache.size()/3;
	    if (febSet < m_rampCache.size()/3) {
		rampIt  = ramps->begin(gain, ids1[gain]);	    
		msg() << MSG::DEBUG <<"FebID vec 1  : ";
		for (unsigned int i = 0; i < ids1[gain].size(); ++i) {
                  msg() << MSG::DEBUG << m_onlineID->show_to_string(HWIdentifier(ids1[gain][i]))
			<< " ";
		}
		msg() << MSG::DEBUG << endmsg;
	    }
	    else if (febSet < 2*m_rampCache.size()/3) {
		rampIt  = ramps->begin(gain, ids2[gain]);	    
		i0   = m_rampCache.size()/3 + 1;
		iend = 2*m_rampCache.size()/3;
		msg() << MSG::DEBUG <<"FebID vec 2  : ";
		for (unsigned int i = 0; i < ids2[gain].size(); ++i) {
                  msg() << MSG::DEBUG << m_onlineID->show_to_string(HWIdentifier(ids2[gain][i]))
			<< " ";
		}
                msg() << MSG::DEBUG << endmsg;
	    }
	    else {
		rampIt  = ramps->begin(gain, ids3[gain]);	    
		i0   = 2*m_rampCache.size()/3 + 1;
		iend = m_rampCache.size();
		msg() << MSG::DEBUG <<"FebID vec 3  : ";
		for (unsigned int i = 0; i < ids3[gain].size(); ++i) {
                  msg() << MSG::DEBUG << m_onlineID->show_to_string(HWIdentifier(ids3[gain][i]))
			<< " ";
		}
		msg() << MSG::DEBUG << endmsg;
	    }
	    
	    rampEnd = ramps->end  (gain);
	    ATH_MSG_DEBUG ("After ramps->end " );
	    for (unsigned int i = i0; i < iend; ++i) {
		// cache is not in order for gains, select the current gain
		if (gain != m_rampCache[i].m_gain) continue; 
		LArRampComplete::LArCondObj rampP;
		HWIdentifier rampId;

		ATH_MSG_DEBUG ("Looking for "
                               << m_onlineID->show_to_string(m_rampCache[i].m_channelID) );

		// Skip the empty channels
		while(rampIt != rampEnd) {
		    rampP = *rampIt;
		    rampId = rampIt.channelId();
		    ++rampIt;
		    if (!rampP.isEmpty()) break; // break out for first non-empty ramp
		}
		unsigned int coolChannel = ramps->coolChannel(m_rampCache[i].m_channelID, 
							      m_rampCache[i].m_gain);
		if (!rampP.isEmpty()) {
                  ATH_MSG_DEBUG ("New  : cool chan, chan id, gain, ramps "
                                 << coolChannel << " "
                                 << m_onlineID->show_to_string(rampId) << " " 
                                 << m_rampCache[i].m_gain << " " 
                                 << rampP.m_vRamp[0] << " " 
                                 << rampP.m_vRamp[1] << " " 
                                 << rampP.m_vRamp[2] << " "  );
		}
		else {
                  ATH_MSG_DEBUG ("New  : isEmpty " );
		}
		ATH_MSG_DEBUG ("Cache: cool chan, chan id, gain, ramps "
                               << coolChannel << " "
                               << m_onlineID->show_to_string(m_rampCache[i].m_channelID) << " " 	
                               << m_rampCache[i].m_gain << " " 
                               << m_rampCache[i].m_vRamp[0] << " " 
                               << m_rampCache[i].m_vRamp[1] << " " 
                               << m_rampCache[i].m_vRamp[2] << " " 
                               << " Compare = " << (rampP == m_rampCache[i]) );
		if (rampP != m_rampCache[i] && !rampP.isEmpty()) {
                    ATH_MSG_ERROR ("LArRampMC and cache NOT equal" );
		    error = true;
		}
	    }
	}
    }

    ATH_MSG_DEBUG ("Compare LArRampMC with corrections " );

    if (m_applyCorrections) {
	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCorrections[i].m_channelID, 
						  m_rampCorrections[i].m_gain);
	    unsigned int coolChannel = ramps->coolChannel(m_rampCorrections[i].m_channelID, 
							  m_rampCorrections[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New        : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " "  );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    
	    ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (CorrectionCompare(rampP, m_rampCorrections[i])) );
	    if (!CorrectionCompare(rampP, m_rampCorrections[i]) && !rampP.isEmpty()) {
		
                ATH_MSG_ERROR ("Before correction: LArRampMC and correction DO NOT compare - should have opposite signs for rampes" );
		error = true;
	    }
	}
    

	ATH_MSG_DEBUG ("Apply corrections and compare LArRampMC with corrections " );
	ATH_CHECK( ramps_rw->applyCorrections() );
	ATH_MSG_DEBUG ("Corrections applied: " << ramps->correctionsApplied() );

	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCorrections[i].m_channelID, 
						  m_rampCorrections[i].m_gain);
	    unsigned int coolChannel = ramps->coolChannel(m_rampCorrections[i].m_channelID, 
							  m_rampCorrections[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New        : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " "  );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (rampP == m_rampCorrections[i]) );
	    if (rampP != m_rampCorrections[i] && !rampP.isEmpty()) {
                ATH_MSG_ERROR ("After correction: LArRampMC and correction NOT equal" );
		error = true;
	    }
	}

	ATH_MSG_DEBUG ("Undo corrections and compare LArRampMC with corrections " );
	ATH_CHECK( ramps_rw->undoCorrections() );
	ATH_MSG_DEBUG ("Corrections applied: " << ramps->correctionsApplied() );

	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCorrections[i].m_channelID, 
						  m_rampCorrections[i].m_gain);
	    unsigned int coolChannel = ramps->coolChannel(m_rampCorrections[i].m_channelID, 
							  m_rampCorrections[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New        : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " "  );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (CorrectionCompare(rampP, m_rampCorrections[i])) );
	    if (!CorrectionCompare(rampP, m_rampCorrections[i]) && !rampP.isEmpty()) {
		
                ATH_MSG_ERROR ("After undo: LArRampMC and correction DO NOT compare - should have opposite signs for ramps" );
		error = true;
	    }
	}

	ATH_MSG_DEBUG ("2nd Apply corrections and compare LArRampMC with corrections " );
	ATH_CHECK( ramps_rw->applyCorrections() );
	ATH_MSG_DEBUG ("Corrections applied: " << ramps->correctionsApplied() );

	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCorrections[i].m_channelID, 
						  m_rampCorrections[i].m_gain);
	    unsigned int coolChannel = ramps->coolChannel(m_rampCorrections[i].m_channelID, 
							  m_rampCorrections[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New        : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " "  );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (rampP == m_rampCorrections[i]) );
	    if (rampP != m_rampCorrections[i] && !rampP.isEmpty()) {
                ATH_MSG_ERROR ("After correction: LArRampMC and correction NOT equal" );
		error = true;
	    }
	}

	ATH_MSG_DEBUG ("2nd Undo corrections and compare LArRampMC with corrections " );
	ATH_CHECK( ramps_rw->undoCorrections() );
	ATH_MSG_DEBUG ("Corrections applied: " << ramps->correctionsApplied() );

	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    LArRampComplete::LArCondObj rampP           = ramps->get(m_rampCorrections[i].m_channelID, 
						  m_rampCorrections[i].m_gain);
	    unsigned int coolChannel = ramps->coolChannel(m_rampCorrections[i].m_channelID, 
							  m_rampCorrections[i].m_gain);
	    if (!rampP.isEmpty()) {
              ATH_MSG_DEBUG ("New        : cool chan, chan id, gain, ramps "
                             << coolChannel << " "
                             << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                             << m_rampCorrections[i].m_gain << " " 
                             << rampP.m_vRamp[0] << " " 
                             << rampP.m_vRamp[1] << " " 
                             << rampP.m_vRamp[2] << " "  );
	    }
	    else {
              ATH_MSG_DEBUG ("New  : isEmpty " );
	    }
	    ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                           << coolChannel << " "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (CorrectionCompare(rampP, m_rampCorrections[i])) );
	    if (!CorrectionCompare(rampP, m_rampCorrections[i]) && !rampP.isEmpty()) {
 		
                ATH_MSG_ERROR ("After undo: LArRampMC and correction DO NOT compare - should have opposite signs for ramps" );
		error = true;
	    }
	}
    }
    

    /*
    log << MSG::DEBUG <<"Find each correction "
	<< endmsg;

    for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	HWIdentifier id   = m_rampCorrections[i].m_channelID;
	unsigned int gain = m_rampCorrections[i].m_gain;

	ConstCorrectionIt  it    = ramps->findCorrection(id, gain);
	// May not have any corrections
	if (it != ramps->correctionsEnd(gain)) {
		
	    unsigned int coolChannel = ramps->coolChannel(id, gain);
	    HWIdentifier id1((*it).first);
	    LArRampComplete::LArCondObj rampP        = (*it).second;
	    if (id != id1 || rampP != m_rampCorrections[i]) {
		log << MSG::ERROR <<"Correction retrieved with findCorrection does not match: " 
		    << " i = " << i << endmsg;
		error = true;
		log << MSG::DEBUG <<"New        : cool chan, chan id, gain, ramps "
		    << coolChannel << " "
		    << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
		    << m_rampCorrections[i].m_gain << " " 
		    << rampP.m_vRamp[0] << " " 
		    << rampP.m_vRamp[1] << " " 
		    << rampP.m_vRamp[2] << " " 
		    << endmsg;
		log << MSG::DEBUG <<"Corrections: cool chan, chan id, gain, ramps "
		    << coolChannel << " "
		    << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
		    << m_rampCorrections[i].m_gain << " " 
		    << m_rampCorrections[i].m_vRamp[0] << " " 
		    << m_rampCorrections[i].m_vRamp[1] << " " 
		    << m_rampCorrections[i].m_vRamp[2] << " " 
		    << " Compare = " << (rampP == m_rampCorrections[i])
		    << endmsg;
	    }
	}
	else {
	    log << MSG::DEBUG <<"No corrections found "
		<< endmsg;
	}	    
    }
    log << MSG::DEBUG <<"End - Find each correction "
	<< endmsg;


    // Count the number of corrections per gain
    unsigned int gains[3] = {0,0,0};
    for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	unsigned int gain = m_rampCorrections[i].m_gain;
	gains[gain]++;
    }
    for (unsigned int i = 0; i < 3; ++i) {
	if (gains[i] != ramps->correctionsSize(i)) {
	    log << MSG::ERROR <<"Number of corrections not same as number inserted: " 
		<< gains[i] << " " 
		<< ramps->correctionsSize(i) << " gain " << i
		<< endmsg;
	    error = true;
	}

	// Check that each correction is the same using container iterator
	unsigned int nit = 0;
	ConstCorrectionIt  it  = ramps->correctionsBegin(i);
	ConstCorrectionIt  end = ramps->correctionsEnd(i);
	unsigned int icorr = 0;
	for (; it != end && icorr <  m_rampCorrections.size(); ++it, ++nit, ++icorr) {
	    while (m_rampCorrections[icorr].m_gain != i) ++icorr;
	    HWIdentifier id          = m_rampCorrections[icorr].m_channelID;
	    unsigned int gain        = m_rampCorrections[icorr].m_gain;
	    unsigned int coolChannel = ramps->coolChannel(id, gain);
	    HWIdentifier id1((*it).first);
	    LArRampComplete::LArCondObj rampP           = (*it).second;
	    if (id != id1 || rampP != m_rampCorrections[icorr]) {
		log << MSG::ERROR <<"Correction retrieved with iterator does not match: " 
		    << " gain  = " << i 
		    << " icorr = " << icorr
		    << " nit   = " << nit
		    << endmsg;
		error = true;
		log << MSG::DEBUG <<"New        : cool chan, chan id, gain, ramps "
		    << coolChannel << " "
		    << m_onlineID->show_to_string(m_rampCorrections[icorr].m_channelID) << " " 
		    << m_rampCorrections[icorr].m_gain << " " 
		    << rampP.m_vRamp[0] << " " 
		    << rampP.m_vRamp[1] << " " 
		    << rampP.m_vRamp[2] << " " 
		    << endmsg;
		log << MSG::DEBUG <<"Corrections: cool chan, chan id, gain, ramps "
		    << coolChannel << " "
		    << m_onlineID->show_to_string(m_rampCorrections[icorr].m_channelID) << " " 
		    << m_rampCorrections[icorr].m_gain << " " 
		    << m_rampCorrections[icorr].m_vRamp[0] << " " 
		    << m_rampCorrections[icorr].m_vRamp[1] << " " 
		    << m_rampCorrections[icorr].m_vRamp[2] << " " 
		    << " Compare = " << (rampP == m_rampCorrections[icorr])
		    << endmsg;
	    }
	}
    }

    removed.  
    */

    ATH_MSG_DEBUG ("Number of channels, iovs "
                   << ramps->chan_size() << " " << ramps->iov_size() );

    std::set<unsigned int> channelNumbers;
    CONTAINER::chan_const_iterator   chanIt  = ramps->chan_begin();
    CONTAINER::chan_const_iterator   endChan = ramps->chan_end  ();
    for (unsigned int i = 0; chanIt != endChan; ++chanIt, ++i) {
	const CONTAINER::Subset* subset = ramps->at(i);
	ATH_MSG_DEBUG ( "Index " << i 
                        << " channel "           << subset->channel() 
                        << " gain "              << subset->gain() 
                        << " groupingType "      << subset->groupingType()
                        << " subsetSize "        << subset->subsetSize()
                        << " correctionVecSize " << subset->correctionVecSize() );
	if ((*chanIt) != subset->channel()) {
          ATH_MSG_ERROR ( "Channel numbers not the same for MultChanColl and subset: " 
                          << i 
                          << " multchan "           << (*chanIt)
                          << " subset   "           << subset->channel()  );
	    error = true;
	}
	if (!(channelNumbers.insert(subset->channel()).second)) {
            ATH_MSG_ERROR ( "Duplicate channel number - Index " << i 
                            << " channel "           << subset->channel()  );
	    error = true;
	}
    }
    ATH_MSG_DEBUG ( "Channel numbers size " << channelNumbers.size()
                    << " ramps size " << ramps->chan_size() );
    
    if (error) {
        ATH_MSG_ERROR ("Failing check of LArRamp - see above" );
	return (StatusCode::FAILURE);
    }

    ATH_MSG_DEBUG ( "End of testEachCondObject " );
    return StatusCode::SUCCESS; 
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode 
LArConditionsTestAlg::testChannelSet()
{

    typedef LArConditionsChannelSet<LArRampComplete::LArCondObj>    ChanSet;
    typedef ChanSet::ConstChannelIt              ConstChannelIt;

    ATH_MSG_INFO ("in testChannelSet" );

    ChanSet chanSet;
    
    // corrections not always available:
    if (m_writeCorrections || m_applyCorrections) {
	// add corrections to channel set 
	for (unsigned int i = 0; i < m_rampCorrections.size(); ++i) {
	    // Must copy LArRampPTmp into a LArRampComplete::LArCondObj
	    LArRampComplete::LArCondObj ramp;
	    ramp.m_vRamp = m_rampCorrections[i].m_vRamp;
	    chanSet.insert(m_rampCorrections[i].m_channelID.get_identifier32().get_compact(), ramp);
	}
	// Now loop over corrections and check that they agree
	bool error = false;
	if (m_rampCorrections.size() != chanSet.size()) {
            ATH_MSG_ERROR ("Corrections not the same size as channel set: " 
                           << m_rampCorrections.size() << " " << chanSet.size() );
	    return (StatusCode::FAILURE);
	}
	else {
          ATH_MSG_DEBUG ("Sizes OK: "  << chanSet.size() );
	}

	ConstChannelIt it    = chanSet.begin();
	ConstChannelIt itEnd = chanSet.end();

	
	unsigned int i = 0;
	for (; it != itEnd; ++it, ++i) {

	    HWIdentifier id          = m_rampCorrections[i].m_channelID;
	    HWIdentifier id1((*it).first);
	    LArRampComplete::LArCondObj rampP           = (*it).second;
	    if (id != id1 || rampP != m_rampCorrections[i]) {
                ATH_MSG_ERROR ("Correction retrieved with iterator does not match: " 
                               << " i = " << i );
		error = true;
	    }
	    ATH_MSG_DEBUG ("New        : chan id, gain, ramps "
                           << m_onlineID->show_to_string(id1) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << rampP.m_vRamp[0] << " " 
                           << rampP.m_vRamp[1] << " " 
                           << rampP.m_vRamp[2] << " " 
                           );
	    ATH_MSG_DEBUG ("Corrections: chan id, gain, ramps "
                           << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                           << m_rampCorrections[i].m_gain << " " 
                           << m_rampCorrections[i].m_vRamp[0] << " " 
                           << m_rampCorrections[i].m_vRamp[1] << " " 
                           << m_rampCorrections[i].m_vRamp[2] << " " 
                           << " Compare = " << (rampP == m_rampCorrections[i])
                           );
	}
	if (!error) {
          ATH_MSG_DEBUG ("Iteration check OK "  );
	}
	
	i = 0;
	for (; i < m_rampCorrections.size(); ++i) {

	    unsigned int id = m_rampCorrections[i].m_channelID.get_identifier32().get_compact();
	    it = chanSet.find(id);
	    if (it == itEnd) {
                ATH_MSG_ERROR ("Could not find correction: " 
                               << " i = " << i );
		error = true;
		ATH_MSG_DEBUG ("Corrections: cool chan, chan id, gain, ramps "
                               << m_onlineID->show_to_string(m_rampCorrections[i].m_channelID) << " " 
                               << m_rampCorrections[i].m_gain << " " 
                               << m_rampCorrections[i].m_vRamp[0] << " " 
                               << m_rampCorrections[i].m_vRamp[1] << " " 
                               << m_rampCorrections[i].m_vRamp[2] << " " 
                               );
	    }
	}
	if (!error) {
          ATH_MSG_DEBUG ("Find check OK "  );
	}

	if (error) {
            ATH_MSG_ERROR ("Failing check of channel set - see above" );
	    return (StatusCode::FAILURE);
	}
    }

    return StatusCode::SUCCESS; 

}


StatusCode LArConditionsTestAlg::testDbObjectRead()
{

    typedef LArRampMC::CONTAINER  CONTAINER; 
    typedef CONTAINER::Subset     Subset;

    const LArRampMC* ramp = 0 ;
    ATH_CHECK( detStore()->retrieve(ramp, "LArRamp") );
  
    ATH_MSG_DEBUG ( " Found LArRampMC, key LArRamp." );

    // Print out channels
    ATH_MSG_DEBUG ( " Number of channels " << ramp->chan_size() );

    // Print out first 10 elements of each gain for subset
    CONTAINER::chan_const_iterator   chanIt  = ramp->chan_begin();
    CONTAINER::chan_const_iterator   endChan = ramp->chan_end  ();
    for (unsigned int i = 0; chanIt != endChan; ++chanIt, ++i) {
	unsigned int coolChan = *chanIt;
	const Subset* subset = ramp->at(i);

	ATH_MSG_DEBUG ( " Channel " << coolChan << " "
                        << " Subset size " << subset->subsetSize() 
                        << " gain, channel, grouping type " << subset->gain() << " "
                        << MSG::hex << subset->channel() << " "  << MSG::dec
                        << subset->groupingType() << " "
                        );

	Subset::ConstSubsetIt   first = subset->subsetBegin();
	Subset::ConstSubsetIt   last  = subset->subsetEnd();
	//for (int i = 0; i < 10 && first != last; ++i, ++first) {
	for (; first != last; ++first) {

	    // select non-zero subsets
	    if ((*first).second.size()) {

              ATH_MSG_DEBUG ( " FEB id " 
                              << m_onlineID->show_to_string(HWIdentifier((*first).first)) << " " 
                              );
		for (unsigned int k = 0; k < 5; ++k) {
                    msg() << MSG::DEBUG << " vramp " ;
//			<< m_onlineID->show_to_string((*first).second[k].m_channelID) << " " 
//			<< (*first).second[k].m_gain << " ";
		    for (unsigned int j = 0; j < (*first).second[k].m_vRamp.size(); ++j) {
                      msg() << MSG::DEBUG << (*first).second[k].m_vRamp[j] << " ";
		    }
		    msg() << MSG::DEBUG << endmsg;
		}

	    }

	}
    }
    

    /*

    // Print out first 10 elements of each gain for corrections
    for (unsigned int gain = 0; gain < 3; ++gain) {
	log << MSG::DEBUG << " Gain, size " 
	    << gain << " "
	    << ramp->correctionsSize(gain) << endmsg;
	CONTAINER::ConstCorrectionIt  first = ramp->correctionsBegin(gain);
	CONTAINER::ConstCorrectionIt  last  = ramp->correctionsEnd(gain);
	for (int i = 0; i < 10 && first != last; ++i, ++first) {
	    log << MSG::DEBUG << " id, vramp " 
		<< m_onlineID->show_to_string(HWIdentifier((*first).first)) << " ";
//		<< m_onlineID->show_to_string((*first).second.m_channelID) << " " 
//		<< (*first).second.m_gain << " ";
	    for (unsigned int j = 0; j < (*first).second.m_vRamp.size(); ++j) {
		log << MSG::DEBUG << (*first).second.m_vRamp[j] << " ";
	    }
	    log << MSG::DEBUG << endmsg;
	}
    }

    */

    return StatusCode::SUCCESS; 

}
