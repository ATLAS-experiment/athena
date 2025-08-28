/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibUtils/LArOFPhasePicker.h"
#include "LArRawConditions/LArOFCComplete.h"
#include "LArRawConditions/LArShapeComplete.h"
#include "LArRawConditions/LArOFCBinComplete.h"
#include "CaloIdentifier/CaloGain.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

LArOFPhasePicker::LArOFPhasePicker(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_inputPhase(nullptr),
  m_onlineID(nullptr),
  m_groupingType(LArConditionsContainerBase::Unknown)
{
  declareProperty("KeyOFC",m_keyOFC="LArOFC_org");
  declareProperty("KeyOFC_new",m_keyOFC_new="LArOFC");
  declareProperty("KeyShape",m_keyShape="LArShape_org");
  declareProperty("KeyShape_new",m_keyShape_new="LArShape");
  declareProperty("KeyPhase",m_keyPhase="");
  declareProperty("GroupingType",m_groupingName="ExtendedSubDetector");
  declareProperty("TimeOffsetCorrection",m_timeOffsetCorr=0);
  declareProperty("DefaultPhase",m_defaultPhase=4);
  declareProperty("doOFC",m_doOFC=true);  
  declareProperty("doShape",m_doShape=true);  
  declareProperty("isSC",m_isSC=false);
}

LArOFPhasePicker::~LArOFPhasePicker()
= default;

StatusCode LArOFPhasePicker::initialize()
{
  if ( m_groupingName == "Unknown" ) {
     m_groupingType = LArConditionsContainerBase::Unknown ;
  } else if ( m_groupingName == "SingleGroup" ) {
     m_groupingType = LArConditionsContainerBase::SingleGroup ;
  } else if ( m_groupingName == "SubDetector" ) {
     m_groupingType = LArConditionsContainerBase::SubDetectorGrouping ;
  } else if ( m_groupingName == "FeedThrough" ) {
     m_groupingType = LArConditionsContainerBase::FeedThroughGrouping ;
  } else if ( m_groupingName == "ExtendedFeedThrough" ) {
     m_groupingType = LArConditionsContainerBase::ExtendedFTGrouping ;
  } else if ( m_groupingName == "ExtendedSubDetector" ) {
     m_groupingType = LArConditionsContainerBase::ExtendedSubDetGrouping ;
  } else if ( m_groupingName == "SuperCells" ) {
    m_groupingType = LArConditionsContainerBase::SuperCells;
  } else {
     ATH_MSG_ERROR( "Grouping type " << m_groupingName << " is not foreseen!" ) ;
     ATH_MSG_ERROR( "Only \"Unknown\", \"SingleGroup\", \"SubDetector\", \"FeedThrough\", \"ExtendedFeedThrough\", \"ExtendedSubDetector\" and \"SuperCells\" are allowed" ) ;
     return StatusCode::FAILURE ;
  }

  StatusCode sc;
  if ( m_isSC ) {
    const LArOnline_SuperCellID* ll;
    sc = detStore()->retrieve(ll, "LArOnline_SuperCellID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineID = static_cast<const LArOnlineID_Base*>(ll);
      ATH_MSG_DEBUG("Found the LArOnlineID helper");
    }
  } else { // m_isSC
    const LArOnlineID* ll;
    sc = detStore()->retrieve(ll, "LArOnlineID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Could not get LArOnlineID helper !" << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      m_onlineID = static_cast<const LArOnlineID_Base*>(ll);
      ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
    }
  }

  ATH_CHECK(m_ofcWKey.initialize(!m_ofcWKey.empty()));
  
  return StatusCode::SUCCESS;
}

StatusCode LArOFPhasePicker::execute()
{
  return StatusCode::SUCCESS;
}



StatusCode LArOFPhasePicker::stop() {

  ATH_MSG_DEBUG(" In stop() ");

  if (!m_keyPhase.empty()) {
    StatusCode sc=detStore()->retrieve(m_inputPhase,m_keyPhase);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Failed to get input OFC phase with key " << m_keyPhase );
      ATH_MSG_ERROR( "Will use default phase !!" );
      m_inputPhase=nullptr;
    } else {
      ATH_MSG_DEBUG( "Got OFC phase with key " << m_keyPhase );
    }
  }
  else {
    ATH_MSG_INFO( "No StoreGate key for OFC bin given. Will use default phase=" << m_defaultPhase );
  }

  if (m_doOFC) {
    ATH_CHECK(pickOFC());
  }

  if (m_doShape) {
    ATH_CHECK(pickShape());
  }
  return StatusCode::SUCCESS;
}


StatusCode LArOFPhasePicker::pickOFC() {
  StatusCode sc;
  const LArOFCComplete* inputOFC=nullptr;
  sc=detStore()->retrieve(inputOFC,m_keyOFC);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to get input OFCs with key " << m_keyOFC );
    ATH_MSG_INFO(detStore()->dump());
    return sc;
  }
  
  std::unique_ptr<LArOFCComplete> larOFCComplete=std::make_unique<LArOFCComplete>();
  larOFCComplete->setGroupingType( static_cast<LArConditionsContainerBase::GroupingType>(m_groupingType));
  sc  = larOFCComplete->initialize(); 
  if ( sc.isFailure() ) {
     ATH_MSG_ERROR( "Could not initialize LArOFCComplete data object - exit!" ) ;
     return sc ;
  }
  ATH_MSG_DEBUG("Working on OFC container '"<< m_keyOFC << "' new container will be '" << m_keyOFC_new << "'");

  const LArOFCweightSC *ofW = nullptr;
  if(!m_ofcWKey.empty()) {
     SG::ReadCondHandle<LArOFCweightSC> wHdl{m_ofcWKey};
     ofW=*wHdl;
     if(!ofW){
        ATH_MSG_WARNING("Could not get the OFCb weights asked with key "<<m_ofcWKey.key()<<", no weighting done !!!");
     } else {
        ATH_MSG_DEBUG("Got the OFCb weights with key "<<m_ofcWKey.key());
     }
  }

  int count = 0;
  for(unsigned int gain = CaloGain::LARHIGHGAIN; gain < CaloGain::LARNGAIN; ++gain) {
    LArOFCComplete::ConstConditionsMapIterator it=inputOFC->begin(gain);
    LArOFCComplete::ConstConditionsMapIterator it_e=inputOFC->end(gain);
    for(; it!=it_e;++it){
      LArOFCComplete::ConstReference ofc=(*it);
      const HWIdentifier id = it.channelId() ; 
      const int nPhases=ofc.OFC_aSize();
      if (nPhases==0) {
        ATH_MSG_DEBUG("Got empty OFC object for channel " << m_onlineID->channel_name(id) << " (disconnected?)");
        continue;
      }
      ATH_MSG_VERBOSE("nPhases=" << nPhases);
      count++;
      std::size_t phase=std::min(m_defaultPhase,nPhases-1);

      if(m_inputPhase) {
         const int p = m_inputPhase->bin(id, gain);
        ATH_MSG_DEBUG("OFC picking, gain=" << gain << ", channel "  << m_onlineID->channel_name(id) <<", p=" << p);
         if (p>0 && p<nPhases) phase=p;
       }
      ATH_MSG_DEBUG("OFC picking, gain=" << gain << ", channel "  << m_onlineID->channel_name(id) <<", phase=" << phase);
      ILArOFC::OFCRef_t vOFC_a = ofc.OFC_a(phase);
      ILArOFC::OFCRef_t vOFC_b = ofc.OFC_b(phase);
      const float timeOffset=ofc.timeOffset()+m_timeOffsetCorr;
      //some sanity check on the OFCs
      if ( vOFC_a.size() == 0 || vOFC_b.size() == 0 ) {
        ATH_MSG_WARNING( "OFC not found for gain "<< gain << " channel "  <<  m_onlineID->channel_name(id) );

      }else if ( vOFC_a.size() != vOFC_b.size() ) {
        ATH_MSG_WARNING( "OFC a (" << vOFC_a.size() << ") and b (" << vOFC_b.size() << ") are not the same size for channel " 
                          <<  m_onlineID->channel_name(id) );
        ATH_MSG_WARNING( "Will be not exported !!!" );
      } else { // save in new container
        std::vector<std::vector<float> > OFC_a;
        OFC_a.push_back(vOFC_a.asVector());
        std::vector<std::vector<float> > OFC_b;
        OFC_b.push_back(vOFC_b.asVector());
        if(ofW) { // weight OFCb
           ATH_MSG_DEBUG("weighting channel by "<<ofW->getW(id));
           std::transform(OFC_b[0].begin(), OFC_b[0].end(), OFC_b[0].begin(),
                                std::bind(std::multiplies<float>(), std::placeholders::_1, ofW->getW(id)));
        } else if (m_isSC && m_onlineID->isHECchannel(id)) { // weight HEC OFCb for SC
           ATH_MSG_DEBUG("weighting HEC channel by 1.5");
           std::transform(OFC_b[0].begin(), OFC_b[0].end(), OFC_b[0].begin(),
                                std::bind(std::multiplies<float>(), std::placeholders::_1, 1.5));

        }
        larOFCComplete->set(id,(int)gain,OFC_a,OFC_b,timeOffset,25.); //There is no sensible time-bin width for single-phase OFCs 25 seems to make the most sense...
      }
    }
  }

  ATH_MSG_DEBUG(" Selected OFCs for  " << count << " cells");

  sc = detStore()->record(std::move(larOFCComplete),  m_keyOFC_new);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to record LArOFCComplete object with key " << m_keyOFC_new );
    return sc;
  }
  sc=detStore()->symLink(ClassID_traits<LArOFCComplete>::ID(),m_keyOFC_new,ClassID_traits<ILArOFC>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to sym-link LArOFCComplete object" );
    return sc;
  }
  return StatusCode::SUCCESS;
  
}


StatusCode LArOFPhasePicker::pickShape() 
{
  StatusCode sc;

  const LArShapeComplete* inputShape=nullptr;
  sc=detStore()->retrieve(inputShape,m_keyShape);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to get input Shapes with key " << m_keyShape );
    return sc;
  }
  
  std::unique_ptr<LArShapeComplete> larShapeComplete=std::make_unique<LArShapeComplete>();
  larShapeComplete->setGroupingType( static_cast<LArConditionsContainerBase::GroupingType>(m_groupingType));
  sc  = larShapeComplete->initialize(); 
  if ( sc.isFailure() ) {
     ATH_MSG_ERROR( "Could not initialize LArShapeComplete data object - exit!" ) ;
     return sc ;
  }

  ATH_MSG_DEBUG("Working on Shape container '"<< m_keyShape << "' new container will be '" << m_keyShape_new << "'");
  int count = 0;
  for(unsigned int gain = CaloGain::LARHIGHGAIN; gain < CaloGain::LARNGAIN; ++gain) {
    LArShapeComplete::ConstConditionsMapIterator it=inputShape->begin(gain);
    LArShapeComplete::ConstConditionsMapIterator it_e=inputShape->end(gain);
    for(; it!=it_e;++it){
      
      LArShapeComplete::ConstReference shape=(*it);
      const HWIdentifier id = it.channelId() ; 
      const int nPhases=shape.shapeSize();
      if (nPhases==0) {
        ATH_MSG_DEBUG("Got empty Shape object for channel " <<  m_onlineID->channel_name(id) << " (disconnected?)");
        continue;
      }
      count++;

      std::size_t phase=std::min(m_defaultPhase,nPhases-1);
      if(m_inputPhase) {
         const int p = m_inputPhase->bin(id, gain);
         if (p>0 && p<nPhases) phase=p;
       }

      ATH_MSG_VERBOSE("Shape picking, gain=" << gain << ", channel "  << m_onlineID->channel_name(id) << ", phase=" << phase);
      ILArShape::ShapeRef_t vShape = shape.shape(phase);
      ILArShape::ShapeRef_t vShapeDer = shape.shapeDer(phase);
      const float timeOffset=shape.timeOffset();
      //some sanity check on the Shapes
      if ( vShape.size() == 0 || vShapeDer.size() == 0 ) {
        ATH_MSG_WARNING( "Shape not found for gain "<< gain << " channel " <<  m_onlineID->channel_name(id) );
      } else if ( vShape.size() != vShapeDer.size() ) {
        ATH_MSG_WARNING( "Shape a (" << vShape.size() << ") and b (" << vShapeDer.size() << ") are not the same size for channel" 
                          <<  m_onlineID->channel_name(id) );
        ATH_MSG_WARNING( "Will be not exported !!!" );
      } else { // save in new container
        std::vector<std::vector<float> > shapeDer;
        std::vector<std::vector<float> > shapeAmpl;
        shapeAmpl.push_back(vShape.asVector());
        shapeDer.push_back(vShapeDer.asVector());        
        larShapeComplete->set(id,(int)gain,shapeAmpl,shapeDer,timeOffset,25.); //There is no sensible time-bin width for single-phase OFCs 25 seems to make the most sense...
      }
    }
  }

  ATH_MSG_DEBUG(" Selected shapes for "  <<  count << " cells");

  sc = detStore()->record(std::move(larShapeComplete),  m_keyShape_new);
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to record LArShapeComplete object with key " << m_keyShape_new );
    return sc;
  }
  sc=detStore()->symLink(ClassID_traits<LArShapeComplete>::ID(),m_keyShape_new,ClassID_traits<ILArShape>::ID());
  if (sc.isFailure()) {
    ATH_MSG_ERROR( "Failed to sym-link LArShapeComplete object" );
    return sc;
  }

  return StatusCode::SUCCESS;
  
}

