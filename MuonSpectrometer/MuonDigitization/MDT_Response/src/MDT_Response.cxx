/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MDT_Response/MDT_Response.h"
#include "CLHEP/Random/RanluxEngine.h"

MDT_Response::MDT_Response() : 
  m_rhit(0.0), 
  m_xhit(0.0), 
  m_pathLength(0.0), 
  m_rtParameters(nullptr),
  m_t0(0.0)
{
  InitTubeParameters();
  InitClusters(200.,0.2);
  InitdEdxTable(); 
}

MDT_Response::MDT_Response(double timewindow, double binsize) : 
  m_rhit(0.0), 
  m_xhit(0.0), 
  m_pathLength(0.0), 
  m_rtParameters(nullptr),
  m_t0(0.0)
{
  InitTubeParameters();
  InitClusters(timewindow,binsize);
  InitdEdxTable(); 
}

MDT_Response::MDT_Response(bool doUpdatedMdtDigi)
  : m_rhit(0.0),
    m_xhit(0.0),
    m_pathLength(0.0),
    m_rtParameters(nullptr),
    m_t0(0.0),
    m_DoUpdatedMdtDigi(doUpdatedMdtDigi)
{
  InitTubeParameters(); 
  InitClusters(200., 0.2);
  InitdEdxTable();
}
			
MDT_Response::~MDT_Response() 
{
  delete[] m_rtParameters; m_rtParameters=nullptr;
}

void MDT_Response::InitClusters(double timewindow, double binsize)
{
  m_binsize = binsize;
  m_timeWindow = timewindow;
  m_offset = (int)(3*m_difSmearing/m_binsize);
  m_bins = (int)(m_timeWindow/m_binsize) + m_offset;
  m_clusters.resize(m_bins);
  m_amplifier.InitResponse(m_bins,binsize);
  //std::cout << "MDT_Response Initialize: new cluster vector with " << m_clusters.size() << " bins " << m_bins << std::endl;
}

void MDT_Response::InitTubeParameters()
{
  // set the default values for central MDT digitization
  m_radius = 14.6275;        // tube radius in atlas
  m_signalSpeed = 300.;   // mm/ns
  m_rtMode = 2;
  m_difSmearing = 10.;
  m_triggerElectron = 20.;
  m_attLength = 30000.;   // mm
  m_clusterDensity = 8.5;           // clusters per mm for 100 GeV muon
  m_integrationWindow = 20.; // ns
  m_amp_adcOffset = 30.;
  m_amp_adcFactor = 90.;
  m_amp_adcFraction = 10.;

  if (m_DoUpdatedMdtDigi) {
    m_attLength = 30000.;   // mm
    m_clusterDensity    = 10.;
    m_integrationWindow = 18.5; // ns
    m_amp_adcOffset     = 35.; 
    m_amp_adcFactor     = 150.;
  }
  
  m_amplifier.SetTriggerElectron(m_triggerElectron);
  m_amplifier.SetIntegrationWindow(m_integrationWindow);
  m_amplifier.SetIntegrationWindowNs(m_integrationWindow);
  m_amplifier.SetAdcOffset(m_amp_adcOffset);
  m_amplifier.SetAdcFactor(m_amp_adcFactor);
  m_amplifier.SetAdcFraction(m_amp_adcFraction);

  InitRt();
    
}

void MDT_Response::InitdEdxTable()
{

	// initialize number of clusters per mm vs gamma table	

	double gammaFactor[] = {0.10110E+01,0.10133E+01,0.10162E+01,0.10196E+01,0.10237E+01,0.10287E+01,0.10348E+01,0.10422E+01,0.10511E+01,
	0.10619E+01,0.10750E+01,0.10909E+01,0.11101E+01,0.11334E+01,0.11616E+01,0.11957E+01,0.12371E+01,0.12873E+01,					   
	0.13481E+01,0.14217E+01,0.15109E+01,0.16190E+01,0.17499E+01,0.19085E+01,0.21007E+01,0.23335E+01,0.26156E+01,					   
	0.29573E+01,0.33714E+01,0.38730E+01,0.44807E+01,0.52170E+01,0.61090E+01,0.71897E+01,0.84989E+01,0.10085E+02,
	0.12007E+02,0.14335E+02,0.17156E+02,0.20573E+02,0.24714E+02,0.29730E+02,0.35807E+02,0.43170E+02,0.52090E+02,
	0.62897E+02,0.75989E+02,0.91852E+02,0.11107E+03,0.13435E+03,0.16256E+03,0.19673E+03,0.23814E+03,0.28830E+03,
	0.34907E+03,0.42270E+03,0.51190E+03,0.61997E+03,0.75089E+03,0.90952E+03,0.11017E+04,0.13345E+04,0.16166E+04,
	0.19583E+04,0.23724E+04,0.28740E+04,0.34817E+04,0.42180E+04,0.51100E+04,0.61907E+04,0.74999E+04,0.90862E+04,
	0.11008E+05,0.13336E+05,0.16157E+05,0.19574E+05,0.23715E+05,0.28731E+05,0.34808E+05,0.42171E+05,0.51091E+05,
	0.61898E+05,0.74990E+05,0.90853E+05,0.11007E+06,0.13335E+06,0.16156E+06,0.19574E+06,0.23714E+06,0.28730E+06,
	0.34807E+06,0.42170E+06,0.51090E+06,0.61897E+06,0.74990E+06,0.90852E+06};

  double numberOfClustersPerCm[] = {720.157,611.293,518.910,440.579,374.222,318.055,270.554,230.415,196.530,167.949,143.867,123.598,106.560,
	92.259,80.277,70.258,61.904,54.960,49.215,44.488,40.626,37.503,35.009,33.054,31.558,30.455,29.686,29.200,
	28.954,28.908,29.030,29.288,29.658,30.119,30.653,31.245,31.885,32.563,33.274,34.014,34.781,35.593,36.502,
	37.371,38.065,38.606,39.024,39.347,39.600,39.800,39.960,40.090,40.197,40.285,40.359,40.419,40.469,40.509,
	40.540,40.564,40.582,40.596,40.606,40.614,40.620,40.624,40.627,40.629,40.631,40.632,40.633,40.634,40.635,
	40.635,40.635,40.635,40.635,40.636,40.636,40.636,40.636,40.636,40.636,40.636,40.636,40.636,40.636,40.636,
	40.636,40.636,40.636,40.636,40.636,40.636,40.636,40.636};

  // only used when flag DoUpdatedMdtDigi option enabled
	double numberOfClustersPerMm[] = {188.7333, 160.5230, 135.5958, 115.2599, 98.1318, 83.4168, 70.8906, 60.3481, 51.4957, 44.0192, 37.6769, 
  32.36505, 27.9281, 24.1556, 21.0378, 18.4179, 16.2422, 14.4153, 12.9175, 11.6626, 10.6435, 9.8294, 9.1667, 8.6551, 8.2926, 7.9775, 
  7.7862, 7.656, 7.5869, 7.5707, 7.6145, 7.6727, 7.7725, 7.8923, 8.0268, 8.2013, 8.3624, 8.5358, 8.7236, 8.9275, 9.1214, 9.2668, 9.3918, 9.5144, 
  9.6032, 9.6733, 9.7272, 9.7535, 9.7837, 9.8438, 9.8365, 9.8620, 9.8892, 9.9119, 9.9063, 9.9235, 9.9267, 9.9175, 9.9299, 9.9409, 9.9269, 9.9437, 
  9.9463, 9.9532, 9.9531, 9.9576, 9.9635, 9.9623, 9.9214, 9.9636, 9.9377, 9.9508, 9.9591, 9.9694, 9.937, 9.9606, 9.9611, 9.9481, 9.9492, 9.9469,
  9.9401, 9.9232, 9.9566, 9.9532, 9.9514, 9.9550, 9.9714, 9.9504, 9.9726, 9.9381, 9.9259, 9.9633, 9.9455, 9.9560, 9.9693, 9.9516};	
	
	for( int ik = 0; ik < 96; ++ik ) {
		m_gammaFactorVec.push_back( gammaFactor[ik] );
    m_numberOfClustersPerCmVec.push_back( numberOfClustersPerCm[ik] );
		m_numberOfClustersPerMmVec.push_back( numberOfClustersPerMm[ik] );
        }
}


void MDT_Response::InitRt(){
  //std::cout << "Setting rt-relation" << std::endl;
  if(m_rtParameters) delete m_rtParameters;
  switch(m_rtMode){
  case 0:    /* linear rt */
    m_rtNpar = 1;
    m_rtParameters = new double[m_rtNpar];
    m_rtParameters[0] = 700./m_radius;
    break;
  case 1:    /* used for work presented in muonsw nov 2003 */
    m_rtNpar = 4;
    m_rtParameters = new double[m_rtNpar];
    m_rtParameters[0] = 0.;
    m_rtParameters[1] = 16.4;
    m_rtParameters[2] = 0.43;
    m_rtParameters[3] = 0.102;
    break;
  case 2: // garfield rt-relation
    m_rtNpar = 10;
    m_rtParameters = new double[m_rtNpar];
    m_rtParameters[0] = -1.79390e-01;
    m_rtParameters[1] = 2.04417e+01;
    m_rtParameters[2] = 4.96576e+00;
    m_rtParameters[3] = -2.59002e+00;
    m_rtParameters[4] = 5.45522e-01;
    m_rtParameters[5] = -4.66626e-02;
    m_rtParameters[6] = 5.66214e-04;
    m_rtParameters[7] = 1.89740e-04;
    m_rtParameters[8] = -1.30073e-05;
    m_rtParameters[9] = 2.68995e-07;
    break;
  case 3: // output rt-relation
    m_rtNpar = 10;
    m_rtParameters = new double[m_rtNpar];
    m_rtParameters[0] =  2.31708e+01;
    m_rtParameters[1] =  4.55828;
    m_rtParameters[2] =  1.41878e+01; 
    m_rtParameters[3] = -5.99392; 
    m_rtParameters[4] =  1.34392; 
    m_rtParameters[5] = -1.65248e-01; 
    m_rtParameters[6] =  1.15960e-02; 
    m_rtParameters[7] = -4.29922e-04; 
    m_rtParameters[8] =  6.14201e-06; 
    m_rtParameters[9] = 2.00429e-08; 
    break;
  default:
    std::cout << "MDT_Response:  Wrong rt-mode" << std::endl;
    m_rtNpar = 0;
    m_rtParameters = nullptr;
    break;
  }
//   std::cout << "MDT_Response: RT-relation -> Polynomial" << std::endl;
//   for(unsigned int i=0;i<m_rtNpar;++i)
//     std::cout << "    Par[" << i << "] = " << m_rtParameters[i] << std::endl;
}

void MDT_Response::DoStepping(CLHEP::HepRandomEngine *rndmEngine)
{
  //double propDelay = PropagationDelay(m_xhit);
  double damping = DampingFactor(m_xhit);
  double r2(m_rhit*m_rhit);
  m_t0 = RtoT(m_rhit);
   
  for(int i=0;i<2;++i){        // splite track into to segments
    double cl(0.);             // current position along track
    while(cl < m_pathLength){   
      cl += DoStep(rndmEngine);    // Do step along track
      double r = sqrt(cl*cl + r2); // calculate corresponding r
      int q = GenerateQ(rndmEngine);// generate cluster size
      double t = RtoT(r);     
      // check if value lies within window
      if(t-m_t0 > m_bins*m_binsize + 3.*SigmaDiffusion(r) ) {
	break;
      }
      double tc = t + Diffusion(r, rndmEngine) - m_t0;
      int bin( (int)(tc/m_binsize+m_offset) );
	
      // fill value into corresponding bin
      if(bin < (int)m_clusters.size() && bin >=0 ){
	m_clusters[bin] += q*damping;
      }else if(bin < 0) {
	std::cout << "out of range " << tc << " bin " << bin << std::endl;
      }
    }
  }
}


void MDT_Response::DoStepping(double ParticleCharge,double ParticleGamma, CLHEP::HepRandomEngine *rndmEngine)
{
// extract  Cluster Density for the given value of particle gamma from the vector gammaFactorVec()
	double correctedClusterDensity;
	int km=0;
	int kmm=0;
	if(ParticleGamma<0.90852E+06){
		while(ParticleGamma>m_gammaFactorVec.at(km)){
			++km;
		}
		int mm=km-1;	
		if(km!=0){
		    double deltagamma=fabs(m_gammaFactorVec.at(km)-m_gammaFactorVec.at(mm))/2.;
			kmm=km-1;
			if(ParticleGamma > (m_gammaFactorVec.at(mm)+deltagamma)){
				kmm=km;
			}
		}
    if (m_DoUpdatedMdtDigi) {
      correctedClusterDensity = m_numberOfClustersPerMmVec.at(kmm);
    }
    else {
      correctedClusterDensity=(8.5/40.636)*(m_numberOfClustersPerCmVec.at(kmm));
    }
		
	}else{
		correctedClusterDensity = m_clusterDensity; // response plateaus: default to value for 100 GeV muon;
	}
	
  //double propDelay = PropagationDelay(m_xhit);
  double damping = DampingFactor(m_xhit);
  double r2(m_rhit*m_rhit);
  m_t0 = RtoT(m_rhit);
  
  for(int i=0;i<2;++i){        // splite track into to segments
    double cl(0.);             // current position along track
    while(cl < m_pathLength){ 

    if(fabs(ParticleCharge)!=1.){ 	 
      cl += m_clusterDensity/(correctedClusterDensity*pow(ParticleCharge,2))*DoStep(rndmEngine);                // Do step along track
	}else{  
      cl += m_clusterDensity/(correctedClusterDensity)*DoStep(rndmEngine);                // Do step along track
	}  
	  
      double r = sqrt(cl*cl + r2); // calculate corresponding r
      int q = GenerateQ(rndmEngine);         // generate cluster size
      double t = RtoT(r);     
      // check if value lies within window
      if(t-m_t0 > m_bins*m_binsize + 3.*SigmaDiffusion(r) ) {
	break;
      }
      double tc = t + Diffusion(r, rndmEngine) - m_t0;
      int bin( (int)(tc/m_binsize+m_offset) );
	
      // fill value into corresponding bin
      if(bin < (int)m_clusters.size() && bin >=0 ){
	m_clusters[bin] += q*damping;
      }else if(bin < 0) {
	std::cout << "out of range " << tc << " bin " << bin << std::endl;
      }
    }
  }
}





void MDT_Response::Reset()
{
  clusterVec::iterator cit = m_clusters.begin();
  for( ; cit != m_clusters.end() ; ++cit)
    *cit = 0;
  m_amplifier.Reset();
} 
  
bool MDT_Response::GetSignal(CLHEP::HepRandomEngine *rndmEngine)
{
  // generate and propagate clusters
  DoStepping(rndmEngine);

  // get amplifier response and see if it passed threshold
  bool hasSignal = m_amplifier.AddClusters(m_clusters);  
  return hasSignal;
}


bool MDT_Response::GetSignal(double ParticleCharge,double ParticleGamma,CLHEP::HepRandomEngine *rndmEngine) 
{
  // generate and propagate clusters
  DoStepping(ParticleCharge,ParticleGamma,rndmEngine); 

  // get amplifier response and see if it passed threshold
  bool hasSignal = m_amplifier.AddClusters(m_clusters);  
  return hasSignal;
}
