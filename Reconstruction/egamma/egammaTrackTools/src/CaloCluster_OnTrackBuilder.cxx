/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCluster_OnTrackBuilder.h"

#include "xAODEgamma/Egamma.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

#include "CaloTrackingGeometry/ICaloSurfaceBuilder.h"
#include "CaloDetDescr/CaloDepthTool.h"
#include "CaloUtils/CaloLayerCalculator.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloEvent/CaloCellContainer.h"

#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkSurfaces/Surface.h"

#include "TrkCaloCluster_OnTrack/CaloCluster_OnTrack.h"


// CALO-improved re-fit; following has been taken directly from "master":
namespace {
  // cluster E in MeV and absEta returns
  // quick phi variance parametrization
  // sigma^2 where sigma is mrad
  double getPhiVariance(double clusterE, double absEta) {
    // convert from MeV to GeV
    const double EinGeV = clusterE * 1e-3;
    // sigma phi = b/E (+) c
    // E in GeV
    // and (+) sum in quadrature
    // we return variance so
    // sigma^2 = (b*b)/(E*E) + c*c
    //
    if (absEta < 0.1) {
      return (0.14 * 0.14) / (EinGeV * EinGeV) + 0.001 * 0.001;
    }
    if (absEta < 0.6) {
      return (0.15 * 0.15) / (EinGeV * EinGeV) + 0.001 * 0.001;
    }
    if (absEta < 0.8) {
      return (0.19 * 0.19) / (EinGeV * EinGeV) + 0.001 * 0.001;
    }
    if (absEta < 1.15) {
      return (0.26 * 0.26) / (EinGeV * EinGeV) + 0.001 * 0.001;
    }
    if (absEta < 1.37) {
      return (0.36 * 0.36) / (EinGeV * EinGeV) + 0.001 * 0.001;
    }
    if (absEta < 1.52) {
      return (0.52 * 0.52) / (EinGeV * EinGeV) + 0.003 * 0.003;
    }
    if (absEta < 1.81) {
      return (0.46 * 0.46) / (EinGeV * EinGeV) + 0.004 * 0.004;
    }
    if (absEta < 2.01) {
      return (0.35 * 0.35) / (EinGeV * EinGeV) + 0.004 * 0.004;
    }
    if (absEta < 2.37) {
      return (0.38 * 0.38) / (EinGeV * EinGeV) + 0.005 * 0.005;
    }
    return (0.47 * 0.47) / (EinGeV * EinGeV) + 0.006 * 0.006;
  }
}



CaloCluster_OnTrackBuilder::CaloCluster_OnTrackBuilder(const std::string& t,
                                                       const std::string& n,
                                                       const IInterface*  p )
: AthAlgTool(t,n,p),
  m_calosurf("CaloSurfaceBuilder"),
  m_cellContainer(0),
  m_eta(0),
  m_phi(0),
  m_deta(0),
  m_dphi(0),
  m_calo_dd(0),
  m_emid(0),
  m_sam(CaloSampling::EMB2),
  m_subcalo(CaloCell_ID::LAREM),
  m_barrel(0)
{
  declareInterface<ICaloCluster_OnTrackBuilder>(this);
  declareProperty( "CaloSurfaceBuilder",     m_calosurf);
  declareProperty( "InputCellContainerName", m_caloCellContainerName = "AODCellContainer");
  declareProperty( "UseClusterEnergy",       m_useClusterEnergy = true);
  declareProperty( "UseClusterPhi" ,         m_useClusterPhi    = true);
  declareProperty( "UseClusterEta" ,         m_useClusterEta    = true);

  // CALO-improved re-fit; from "master"
  m_eg_resol = std::make_unique<eg_resolution>("run2_R21_v1");
}

//--------------------------------------------------------------------------------------------
CaloCluster_OnTrackBuilder::~CaloCluster_OnTrackBuilder() {}
//--------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------
StatusCode CaloCluster_OnTrackBuilder::initialize() {
//--------------------------------------------------------------------------------------------

  ATH_MSG_INFO("Initializing CaloCluster_OnTrackBuilder");
  ATH_MSG_INFO("UseClusterEnergy = " << m_useClusterEnergy);
  ATH_MSG_INFO("UseClusterEta    = " << m_useClusterEta);
  ATH_MSG_INFO("UseClusterPhi    = " << m_useClusterPhi);

  // Retrieve the updator CaloSurfaceBuilder
  if ( m_calosurf.retrieve().isFailure() ){
    ATH_MSG_FATAL ( "Unable to retrieve the instance " << m_calosurf.name() << "... Exiting!" );
    return StatusCode::FAILURE;
  }
  
  // retrieve all helpers from det store
  m_calo_dd = CaloDetDescrManager::instance();

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
StatusCode CaloCluster_OnTrackBuilder::finalize(){ return StatusCode::SUCCESS; }
//--------------------------------------------------------------------------------------------






//--------------------------------------------------------------------------------------------
  Trk::CaloCluster_OnTrack* CaloCluster_OnTrackBuilder::buildClusterOnTrack( const xAOD::Egamma* eg, int charge )
//--------------------------------------------------------------------------------------------
{
  return buildClusterOnTrack( eg->caloCluster(), charge );
}



//--------------------------------------------------------------------------------------------
  Trk::CaloCluster_OnTrack* CaloCluster_OnTrackBuilder::buildClusterOnTrack( const xAOD::CaloCluster* cluster, int charge ) 
//--------------------------------------------------------------------------------------------
{
  ATH_MSG_DEBUG("Building Trk::CaloCluster_OnTrack");
  
  if(!m_useClusterPhi && !m_useClusterEta && !m_useClusterEnergy){
    ATH_MSG_WARNING("CaloCluster_OnTrackBuilder is configured incorrectly");  
    return 0;  
  }
  
  if(!cluster) return 0;
  const Trk::Surface* surface = getCaloSurface( cluster );
  
  if(!surface) return 0;
  
  const Trk::LocalParameters*  lp =getClusterLocalParameters( cluster, surface, charge );

  if (!lp){
    delete surface;
    return 0;
  }
     
  const  Amg::MatrixX *em  =getClusterErrorMatrix( cluster, surface, charge );
  
  if (!em){
    delete surface;
    delete lp;
    return 0;
  }
  
  Trk::CaloCluster_OnTrack* ccot = new  Trk::CaloCluster_OnTrack( *lp, *em, *surface );
  delete em;
  delete surface;
  delete lp;

  if(ccot) {
    ATH_MSG_DEBUG("Successful build of Trk::CaloCluster_OnTrack");
    // std::cout << *ccot << std::endl;
  }

  return ccot;
}



//--------------------------------------------------------------------------------------------
const Trk::Surface*   CaloCluster_OnTrackBuilder::getCaloSurface( const xAOD::CaloCluster* cluster ) 
//--------------------------------------------------------------------------------------------
{
 
  const Trk::Surface* destinationSurface = 0;
  
  // Determine if we want to extrapolate to the barrel or endcap.  If in the crack choose the 
  // detector with largest amount of energy in the second sampling layer 
  if ( xAOD::EgammaHelpers::isBarrel( cluster ) )
  {
    destinationSurface = m_calosurf->CreateUserSurface (CaloCell_ID::EMB2, 0. , cluster->eta() );
  } else{ 
    destinationSurface = m_calosurf->CreateUserSurface (CaloCell_ID::EME2, 0. , cluster->eta() );
  }
  return destinationSurface;
}


//--------------------------------------------------------------------------------------------
const Trk::LocalParameters*   CaloCluster_OnTrackBuilder::getClusterLocalParameters( const xAOD::CaloCluster* cluster, 
                                                                                     const Trk::Surface* surf,
                                                                                     int charge) const
//--------------------------------------------------------------------------------------------
{
 
  Amg::Vector3D  surfRefPoint = surf->globalReferencePoint();
  //std::cout << "REFPOINT " << "[r,phi,z] = [ " << surfRefPoint.perp() << ", " << surfRefPoint.phi() << ", " << surfRefPoint.z() << " ]" <<std::endl; 
  
  double eta = cluster->eta();
  double theta = 2*atan(exp(-eta)); //  -log(tan(theta/2));
  double tantheta = tan(theta);
  double phi = cluster->phi();
  
  // CALO-improved re-fit; from "master"
  double clusterQoverE = cluster->e() !=0 ? (double)charge/cluster->e() : 0;

  //std::cout << "   Cluster Energy        "<< cluster->calE() << std::endl;  

/*
  double correction(0);
  double phis = CalculatePhis( cluster );
  int inteta = abs(eta*10);
  //int intphis = phis*2e3-25; 
  if (phis < 0 || phis > 74){
    //intphis = 0; 
  }
  if (inteta > 24) inteta = 24;
  
  phi += correction * 1e-3 * charge;
*/

  Trk::LocalParameters* newLocalParameters(0);
 
  if ( xAOD::EgammaHelpers::isBarrel( cluster ) ){
    //Two corindate in a cyclinder are 
    //Trk::locRPhi = 0 (ie phi)
    //Trk::locZ    = 1(ie z)
    double r = surfRefPoint.perp() ;
    double z = tantheta == 0 ? 0. : r/tantheta;      
    Trk::DefinedParameter locRPhi( r * phi  ,  Trk::locRPhi  ) ;
    Trk::DefinedParameter locZ   ( z        ,  Trk::locZ     ) ;
    Trk::DefinedParameter qOverP   ( clusterQoverE   ,  Trk::qOverP     ) ;
    std::vector<Trk::DefinedParameter> defPar ;
    if(m_useClusterPhi)defPar.push_back( locRPhi ) ;
    if(m_useClusterEta)defPar.push_back( locZ ) ;
    if(m_useClusterEnergy)defPar.push_back( qOverP );
    newLocalParameters = new Trk::LocalParameters( defPar ) ;
  } else{ 
    //Local paramters of a disk are
    //Trk::locR   = 0
    //Trk::locPhi = 1
    double z = surfRefPoint.z();
    double r = z*tantheta; 
    Trk::DefinedParameter locR  ( r   ,  Trk::locR    ) ;
    Trk::DefinedParameter locPhi( phi ,  Trk::locPhi  ) ;
    Trk::DefinedParameter qOverP   ( clusterQoverE   ,  Trk::qOverP     ) ;
    std::vector<Trk::DefinedParameter> defPar ;
    if(m_useClusterEta)defPar.push_back( locR ) ;
    if(m_useClusterPhi)defPar.push_back( locPhi ) ;
    if(m_useClusterEnergy)defPar.push_back( qOverP );

    newLocalParameters = new Trk::LocalParameters( defPar ) ;
  }
  
  return newLocalParameters;

}


//--------------------------------------------------------------------------------------------
const  Amg::MatrixX*   CaloCluster_OnTrackBuilder::getClusterErrorMatrix( const xAOD::CaloCluster* cluster,
                                                                          const Trk::Surface* surf,
                                                                          int ) const
//--------------------------------------------------------------------------------------------
{

  
/*
  double eta = cluster->eta();
  int inteta = abs(eta*10);
  if (inteta > 24) inteta = 24;
  double phis = CalculatePhis( cluster );
  int intphis = phis*2e3 ;
  intphis -= 25;
  if (phis < 0 || phis > 74){
    intphis = 0; 
  }
*/

  // CALO-improved re-fit; from "master"
  const double clusterE   = cluster->e();
  const double clusterEta = cluster->eta();

  // variance in phi from calorimeter phi resolution
  double phivariance_prep = getPhiVariance(clusterE, std::abs(clusterEta));
  if (phivariance_prep < 1e-5) {
    // Avoid going too small for  very high E
    phivariance_prep = 1e-5;
  }

  // q over p variance from sigmaE/E (calo energy resolution)
  const double sigmaP_over_P = m_eg_resol->getResolution(0, // electron
                                                         clusterE,
                                                         clusterEta,
                                                         2 // 90% quantile
  );
  const double qOverP = 1. / clusterE;
  const double qOverP_variance_prep = (qOverP * qOverP) * (sigmaP_over_P * sigmaP_over_P);

  // Variance in Z
  // sigma ~ 20 mm large error
  // As currently we do not want to rely
  // on the eta side of the cluster.
  constexpr double zvariance_prep = 400;

  const double phivariance     = m_useClusterPhi    ? phivariance_prep     : -1;
  const double zvariance       = m_useClusterEta    ? zvariance_prep       : -1;
  const double qOverP_variance = m_useClusterEnergy ? qOverP_variance_prep : -1;

  int matrixSize = static_cast<int>(m_useClusterPhi) +
                   static_cast<int>(m_useClusterEta) +
                   static_cast<int>(m_useClusterEnergy);

  Amg::MatrixX covMatrix(matrixSize, matrixSize);
  covMatrix.setZero();
  
  if ( xAOD::EgammaHelpers::isBarrel( cluster ) ){
    //Two corindate in a cyclinder are 
    //Trk::locRPhi = 0 (ie phi)
    //Trk::locZ    = 1(ie z)   
    Amg::Vector3D surfRefPoint = surf->globalReferencePoint();
    double r2 = pow(surfRefPoint.perp(), 2);
    
    int indexCount(0);

    if(m_useClusterPhi){ 
      covMatrix( indexCount, indexCount ) = phivariance * r2 ;
      ++indexCount;
    }
    if(m_useClusterEta){
      covMatrix( indexCount, indexCount ) = zvariance ;
      ++indexCount;
    }
    if(m_useClusterEnergy){
      covMatrix( indexCount, indexCount ) = qOverP_variance ;
      ++indexCount;
    }
  } else{ 
    //Local paramters of a disk are
    //Trk::locR   = 0
    //Trk::locPhi = 1
    
    int indexCount(0);
     
    if(m_useClusterEta){
      covMatrix( indexCount, indexCount ) = zvariance ;
      ++indexCount;
    }
    if(m_useClusterPhi){
      covMatrix( indexCount, indexCount ) = phivariance ;
      ++indexCount;
    }
    if(m_useClusterEnergy){
      covMatrix( indexCount, indexCount ) = qOverP_variance ;
      ++indexCount;
    }

  }


  const Amg::MatrixX *result= new Amg::MatrixX(covMatrix);
  
  return result;
}


//--------------------------------------------------------------------------------------------
double CaloCluster_OnTrackBuilder::getClusterPhiError( const xAOD::CaloCluster* cluster ) const 
//--------------------------------------------------------------------------------------------
{

  /** Error on theta = C(eta) mrad/sqrt(Energy) */
  /** NOTE: Eta dependence of C  will be implemented later. C=10mrad based on EG? note*/ 
  /** Note these should be take from EMError details*/
  
  double clusterEnergy =  cluster->calE()*1e-3;
   
  /** Error on phi = C(eta) mrad/sqrt(Energy) */
  double error  = electronPhiResolution( cluster->eta() , clusterEnergy ); 
  return  error * 1.1;


}

//--------------------------------------------------------------------------------------------
double CaloCluster_OnTrackBuilder::electronPhiResolution(double eta, double energy) const
//--------------------------------------------------------------------------------------------
{

  eta = fabs( eta );

  return  electronPhiResoA( eta ) + electronPhiResoB( eta ) / energy;

}

//--------------------------------------------------------------------------------------------
double CaloCluster_OnTrackBuilder::electronPhiResoA(double eta) const
//--------------------------------------------------------------------------------------------
{

  if ( eta < 0.30 )
    return 0.000191492 ;
  
  else if ( eta < 0.60) 
    return 9.35047e-05 + 0.000392766 * eta;
  
  else if ( eta < 0.80) 
    return 0.000327201;   
    
  else if ( eta < 1.05)
    return 0.000141755;
  
  else if ( eta < 1.35)
    return (-1.07475  + 1.15372*eta)*1e-3;

  else if ( eta < 1.55)
    return (-15.2133 + 11.2163*eta)*1e-3 ;
  
  else if ( eta < 1.85)
    return 0.00128452 - 0.00053016 * eta;

  else if ( eta < 2.30)
    return -0.000665622 + 0.00052136 * eta;

  else
    return 0.000327754;

}

//--------------------------------------------------------------------------------------------
double CaloCluster_OnTrackBuilder::electronPhiResoB(double eta) const
//--------------------------------------------------------------------------------------------
{

  if ( eta < 0.65 )
    return 0.0285262  + 0.00985529 * eta;
  
  else if ( eta < 1.04 )
    return -0.0690774 + 0.166424   * eta; 
  
  else if ( eta < 1.25 )
    return 0.0769113  + 0.0149434  * eta;   
  
  else if ( eta < 1.55 )
    return -0.407594  + 0.393218   * eta;     
  
  else if ( eta < 1.95 )
    return 0.415602   - 0.172824   * eta;     
  
  else if ( eta < 2.05 )
    return 0.0840844;     
  
  else if ( eta < 2.40 )
    return 0.187563   - 0.0472463  * eta;
    
  else 
    return 0.0693652;
  
}


// =====================================================================
bool CaloCluster_OnTrackBuilder::FindPosition(const xAOD::CaloCluster* cluster) const
{
  //
  // From the original (eta,phi) position, find the location
  // (sampling, barrel/end-cap, granularity)
  // For this we use the tool egammaEnergyAllSamples
  // which uses the CaloCluster method inBarrel() and inEndcap()
  // but also, in case close to the crack region where both 
  // boolean can be true, the energy reconstructed in the sampling
  //

  // eta max and averaged eta 
  

  
  m_eta = cluster->etaSample(m_sam);
  m_phi = cluster->phiSample(m_sam);
  if ((m_eta==0. && m_phi==0.) || fabs(m_eta)>100) return false;

  int sampling_or_module;

  // granularity in (eta,phi) in the pre sampler
  // CaloCellList needs both enums: subCalo and CaloSample
  m_calo_dd->decode_sample(m_subcalo, m_barrel, sampling_or_module, 
         (CaloCell_ID::CaloSample) m_sam);

  // Get the corresponding grannularities : needs to know where you are
  //                  the easiest is to look for the CaloDetDescrElement
  const CaloDetDescrElement* dde;
  dde = m_calo_dd->get_element(m_subcalo,sampling_or_module,m_barrel,m_eta,m_phi);
  // if object does not exist then return
  if (!dde) return false;

  // local granularity
  m_deta = dde->deta();
  m_dphi = dde->dphi();



  return true;
}



// =====================================================================
double  CaloCluster_OnTrackBuilder::CalculatePhis(const xAOD::CaloCluster* cluster) const
// =====================================================================
{
  if (!cluster) return 9999.;
  m_sam = xAOD::EgammaHelpers::isBarrel(cluster) ? CaloSampling::EMB2 : CaloSampling::EME2;
  
  // From the original (eta,phi) position, find the location
  // (sampling, barrel/end-cap, granularity)
  if (!FindPosition(cluster)) return 0.;

  if (evtStore()->retrieve( m_cellContainer, m_caloCellContainerName ).isFailure())
    return 0.;
  
  CaloLayerCalculator calc;
  if (calc.fill(m_cellContainer,cluster->etaSample(m_sam),cluster->phiSample(m_sam),
                7*m_deta,7*m_dphi,m_sam).isFailure() )
     ATH_MSG_WARNING("CaloLayerCalculator failed fill ");
  
  double etamax = calc.etarmax();
  double phimax = calc.phirmax();

  if (calc.fill(m_cellContainer,etamax,phimax,3.*m_deta,7.*m_dphi,m_sam).isFailure())
    ATH_MSG_WARNING("CaloLayerCalculator failed fill ");
   
  return calc.phis(); 
}
