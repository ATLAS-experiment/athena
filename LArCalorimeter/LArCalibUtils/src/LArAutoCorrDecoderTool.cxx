/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibUtils/LArAutoCorrDecoderTool.h"
#include "AthenaKernel/errorcheck.h"

LArAutoCorrDecoderTool::~LArAutoCorrDecoderTool() = default;;

StatusCode LArAutoCorrDecoderTool::initialize() 
{
  ATH_MSG_DEBUG("LArAutoCorrDecoderTool initialize() begin");
  
  if ( m_isSC ) {
    const LArOnline_SuperCellID* ll;
    ATH_CHECK( detStore()->retrieve(ll, "LArOnline_SuperCellID") );
    m_onlineID = static_cast<const LArOnlineID_Base*>(ll);
    ATH_MSG_DEBUG("Found the LArOnlineID helper");
    
  } else { // m_isSC
    const LArOnlineID* ll;
    ATH_CHECK( detStore()->retrieve(ll, "LArOnlineID") );
    m_onlineID = static_cast<const LArOnlineID_Base*>(ll);
    ATH_MSG_DEBUG(" Found the LArOnlineID helper. ");
  }


  if (!m_isSC && m_alwaysHighGain)
    ATH_MSG_INFO( "Will always return HIGH gain autocorrelation matrix for EM calo, MEDIUM for HEC and FCAL" );

  ATH_MSG_DEBUG("LArAutoCorrDecoderTool initialize() end");
  return StatusCode::SUCCESS;
}


const Eigen::MatrixXd LArAutoCorrDecoderTool::AutoCorr( const HWIdentifier&  CellID, int gain, unsigned nSamples=5 ) const
{
  if (m_decodemode==1u)
    return ACPhysics(CellID,gain,nSamples);
  else
    return ACDiagonal(CellID,gain,nSamples);
}


const Eigen::MatrixXd LArAutoCorrDecoderTool::ACDiagonal( const HWIdentifier&  CellID, int gain, unsigned nSamples=5 ) const {

  if (!m_isSC && m_alwaysHighGain) {
    if (m_onlineID->isFCALchannel(CellID) ||m_onlineID->isHECchannel(CellID))
      gain=1;
    else
      gain=0;
  }

  Eigen::MatrixXd AutoCorrMatrix=Eigen::MatrixXd::Zero(nSamples,nSamples);

  const ILArAutoCorr* autoCorr=nullptr;
  detStore()->retrieve(autoCorr,m_keyAutoCorr).ignore();
  

  if ( autoCorr ) { // LArAutoCorrComplete is loaded in DetStore

    ILArAutoCorr::AutoCorrRef_t dbcorr = autoCorr->autoCorr(CellID,gain);

    if ( dbcorr.size()== 0 ) { // empty AutoCorr for given channel
      ATH_MSG_WARNING( "Empty AutoCorr vector for channel " <<  m_onlineID->channel_name(CellID) << " in Gain = " << gain);
      nSamples=0;
    }
    else if (dbcorr.size() < nSamples-1 ) {
      ATH_MSG_WARNING( "Not enough samples in AutoCorr vector for channel " <<  m_onlineID->channel_name(CellID) << " in Gain = " << gain);
      nSamples=1+dbcorr.size(); //The remaining values of the eigen matrix are left to 0.0
    } 
  
    // fill diagonal matrix with vector 
    for (unsigned i=0;i<nSamples;i++) {
      AutoCorrMatrix(i,i)= 1 ;
      for (unsigned j=i+1;j<nSamples;j++) {
	AutoCorrMatrix(i,j) = AutoCorrMatrix(j,i) = dbcorr[j-i-1];      
      }
    }
  }//else if m_autoCorr
  else { // no LArAutoCorrComplete loaded in DetStore (e.g. DB problem) :-(
    ATH_MSG_WARNING( "No valid AutoCorr object loaded from DetStore" );
  }

  ATH_MSG_DEBUG("AutoCorr Diagonal matrix for channel " <<  m_onlineID->channel_name(CellID) 
		<< " in Gain = " << gain
		<< ":\n" << AutoCorrMatrix);

  return AutoCorrMatrix;
  
}

const Eigen::MatrixXd LArAutoCorrDecoderTool::ACPhysics( const HWIdentifier&  CellID, int gain, unsigned nSamples=5 ) const {


  if (!m_isSC && m_alwaysHighGain) {
    if (m_onlineID->isFCALchannel(CellID) ||m_onlineID->isHECchannel(CellID))
      gain=1;
    else
      gain=0;
  }

  Eigen::MatrixXd AutoCorrMatrix=Eigen::MatrixXd::Identity(nSamples,nSamples);

  const ILArAutoCorr* autoCorr=nullptr;
  detStore()->retrieve(autoCorr,m_keyAutoCorr).ignore();
  
  if ( autoCorr ) { // LArAutoCorrComplete is loaded in DetStore

    ILArAutoCorr::AutoCorrRef_t corrdb = autoCorr->autoCorr(CellID,gain);
   
    if ( corrdb.size()== 0 ) { // empty AutoCorr for given channel
      ATH_MSG_WARNING( "Empty AutoCorr vector for channel " << m_onlineID->channel_name(CellID) << " in Gain = " << gain);
      nSamples=0; //return all-zero matrix
    }
    else  if ( corrdb.size() < nSamples*(nSamples+1)/2 ) {
      ATH_MSG_WARNING( "Not enough samples in AutoCorr vector for channel " <<  m_onlineID->channel_name(CellID) 
			<< "in Gain = " << gain << " for AC Physics mode");
      nSamples=0;//return all-zero matrix 
    } 

    // Corr size could be bigger, then it's asked now, need remapping:
    const unsigned int nsamples_AC = (-1+((int)(sqrt(1+8*corrdb.size()))))/2;
    unsigned int k=0;
    for (unsigned i=0;i<nSamples;i++) {
      for (unsigned j=i;j<nSamples;j++,k++) {
	if (i<=j) {
	  AutoCorrMatrix(i,j) = AutoCorrMatrix(j,i)= corrdb[k];
	}
      }
      k+=nsamples_AC-nSamples;
    }
  } //end if m_autoCorr 
  else { // no LArAutoCorrComplete loaded in DetStore (e.g. DB problem) :-(
    ATH_MSG_WARNING( "No valid AutoCorr object loaded from DetStore" );
  }
   
  ATH_MSG_DEBUG("AutoCorr Physics matrix for channel " <<  m_onlineID->channel_name(CellID) 
		<< " in Gain = " << gain
		<< ":\n" << AutoCorrMatrix);
  return AutoCorrMatrix;
  
}
