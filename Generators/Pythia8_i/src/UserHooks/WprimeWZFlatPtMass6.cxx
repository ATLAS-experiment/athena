/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#include "Pythia8_i/UserHooksFactory.h"
#include "Pythia8/PhaseSpace.h"
#include "UserHooksUtils.h"
#include "UserSetting.h"

namespace Pythia8{
  class WprimeWZFlatPtMass6;
}

namespace Pythia8 {
  
  class WprimeWZFlatPtMass6 : public UserHooks {
    
  public:
    
    // Constructor.
    WprimeWZFlatPtMass6(){}
    
    // Destructor.
    ~WprimeWZFlatPtMass6(){}
    
    // Allow process cross section to be modified...
    virtual bool canModifySigma() {return true;}
    
    // ...which gives access to the event at the trial level, before selection.
    virtual double multiplySigmaBy(const SigmaProcess* sigmaProcessPtr, 
                                   const PhaseSpace* phaseSpacePtr, 
                                   bool /* inEvent */) {
      // All events should be 2 -> 1, but kill them if not.
      if (sigmaProcessPtr->nFinal() != 1) return 0.; 

      // Weight cross section with BW propagator, i.e. to remove it.
      // (inEvent = false for initialization).
      // No inEvent criteria, want weight both for cross section 
      // and MC generation.

      int idRes   = sigmaProcessPtr->resonanceA();
      double mRes = particleDataPtr->m0(idRes);
      double wRes = particleDataPtr->mWidth(idRes);
      double m2Res   = mRes*mRes;
      double gamMRat = wRes/mRes;
      double sHat = phaseSpacePtr->sHat();
      double weightBW = m2Res*m2Res + sHat*sHat*(1 + gamMRat*gamMRat) - 2.*sHat*m2Res;      
      double rH = sqrt(sHat);
      
      return (m_flatpT(settingsPtr)) ? weightBW * pTWeight(rH) : weightBW * breitWignerDenom(rH/settingsPtr->parm("Beams:eCM"));
    }
    //bool canVetoResonanceDecays() { return true; }
    bool canVetoProcessLevel() { return true; }

    //bool doVetoResonanceDecays(Event& process) {
    bool doVetoProcessLevel(Event& process) {

      const int mode = m_flatMass(settingsPtr);
      //subEvent(process,true);
      //omitResonanceDecays(process,true);
      //process = workEvent;
      //process.list();

      double apT = 1.0;
      //std::cout<<"before changes"<<std::endl;
      //process.list();   
      bool isVetoed = false;
      if(mode == 1){
	for (int i = 1; i < process.size(); ++i) {
	  // Select vector bosons
	  Particle& v = process[i];
	  
	  if (v.idAbs() != 34) continue;
	  
	  // Find W/Z daughter particles
	  Particle& d_W = process[v.daughter1()];
	  Particle& d_Z = process[v.daughter2()];
	  if(d_W.idAbs() !=24 ){
	    d_W = process[v.daughter2()];
	    d_Z = process[v.daughter1()];
	  }
	  Vec4 pv_W = d_W.p(); 
	  Vec4 pv_Z = d_Z.p(); 
	  Vec4 pv_W_orig = d_W.p(); 
	  Vec4 pv_Z_orig = d_Z.p(); 
          double pTW = sqrt(pow(pv_W.px(),2) + pow(pv_W.py(),2));
	  double mW = sqrt(pow(pv_W.e(),2) - pow(pv_W.px(),2) - pow(pv_W.py(),2) - pow(pv_W.pz(),2)); 
	  double mZ = sqrt(pow(pv_Z.e(),2) - pow(pv_Z.px(),2) - pow(pv_Z.py(),2) - pow(pv_Z.pz(),2)); 
	  double mWflat = getFlatmW(mW);

	  if((pow(mWflat,2) - pow(mW,2))/pow(pTW,2) < 1.0){                                                                                                                
	    apT = sqrt(1 - (pow(mWflat,2) - pow(mW,2))/pow(pTW,2));      
	  }else{
	    return true;///veto the event since pT not large enough.
	  }

	  ///rescale W boson px,py and mass components
	  pv_W.px(apT*pv_W.px());                                                                                                                                                                  
	  pv_W.py(apT*pv_W.py());                                                                                                                                                                  
	  d_W.p(pv_W);                                                                                                                                                                              
	  d_W.m(mWflat);

	  ///rescale accordingly Z boson px,py and mass components
	  pv_Z.px(apT*pv_Z.px());                                                                                                                                                                  
	  pv_Z.py(apT*pv_Z.py());                                                                                                                                                                   
	  d_Z.p(pv_Z);                                                                                                                                                                             
	  d_Z.m(sqrt(pow(pv_Z.e(),2) - pow(pv_Z.px(),2) - pow(pv_Z.py(),2) - pow(pv_Z.pz(),2)));
	  double mZref = d_Z.m();
	  
          if (mZref > 600.0) {
		isVetoed = true;
	  }

	  if (mWflat > 700.0){
		isVetoed = true;
	  }

          //rescale components of the W and Z boson daughters.
	  Particle& d1_W = process[d_W.daughter1()];                                                                                                                                         
	  Particle& d2_W = process[d_W.daughter2()];   
	  RescaleDaughters(d1_W,d2_W,pv_W,pv_W_orig,mW,mWflat);
 
	  Particle& d1_Z = process[d_Z.daughter1()];                                                                                                                                         
	  Particle& d2_Z = process[d_Z.daughter2()];   
          RescaleDaughters(d1_Z,d2_Z,pv_Z,pv_Z_orig,mZ,mZref);
	  
	}//loop over particles
	//std::cout<<"after changes"<<std::endl;
	//process.list();
   
      }//mode == 1
      return isVetoed;
    }

  private:

    double getFlatmW(double mW){

      double mMin = 10.;
      double mMax = 1000.;
      
      double mWt = ((mMax - mMin)/log(mMax/mMin))*log(mW/mMin)+mMin;                                                                                                                          
      return mWt;
    }

    void RescaleDaughters(Particle& d1_W,Particle& d2_W,Vec4 pv_W,Vec4 pv_W_orig,double mW,double mWflat){

	  Vec4 pd1_W = d1_W.p();                                                                                                                                                                   
	  Vec4 pd2_W = d2_W.p();                                                                                                                                                                   
          ///boost to the W boson rest-frame
	  pd1_W.bstback(pv_W_orig);                                                                                                                                                                 
	  pd2_W.bstback(pv_W_orig);  

	  double m1_W_orig = sqrt(pow(pd1_W.e(),2) - pow(pd1_W.px(),2) - pow(pd1_W.py(),2) - pow(pd1_W.pz(),2));                                                                                   
	  double m2_W_orig = sqrt(pow(pd2_W.e(),2) - pow(pd2_W.px(),2) - pow(pd2_W.py(),2) - pow(pd2_W.pz(),2));
	  double mWchild = sqrt(pow(pd1_W.e()+pd2_W.e(),2)-pow(pd1_W.px()+pd2_W.px(),2)-pow(pd1_W.py()+pd2_W.py(),2)-pow(pd1_W.pz()+pd2_W.pz(),2));

	  double r_W_orig = 1 - (pow(m1_W_orig,2) + pow(m2_W_orig,2))/pow(mW,2);                                                                                                                   
	  double r_W_flat = 1 - (pow(m1_W_orig,2) + pow(m2_W_orig,2))/pow(mWflat,2);                                                                                                                 
	  double mR_W = (mWflat*r_W_flat)/(mWchild*r_W_orig); 
  
          //rescale daughters to ensure invariant mass equal to the new W boson mass
	  pd1_W.px(mR_W*pd1_W.px());
	  pd1_W.py(mR_W*pd1_W.py());
	  pd1_W.pz(mR_W*pd1_W.pz());
	  pd1_W.e(sqrt(pow(pd1_W.px(),2)+pow(pd1_W.py(),2)+pow(pd1_W.pz(),2)+pow(m1_W_orig,2)));
	  
	  pd2_W.px(mR_W*pd2_W.px());
	  pd2_W.py(mR_W*pd2_W.py());
	  pd2_W.pz(mR_W*pd2_W.pz());
	  pd2_W.e(sqrt(pow(pd2_W.px(),2)+pow(pd2_W.py(),2)+pow(pd2_W.pz(),2)+pow(m2_W_orig,2)));
	  
          ///boost back to the lab frame using edited pvW
	  pd1_W.bst(pv_W); d1_W.p(pd1_W);                                                                                                                                                             
	  pd2_W.bst(pv_W); d2_W.p(pd2_W); 
	  double m1_W = sqrt(pow(pd1_W.e(),2) - pow(pd1_W.px(),2) - pow(pd1_W.py(),2) - pow(pd1_W.pz(),2));
	  double m2_W = sqrt(pow(pd2_W.e(),2) - pow(pd2_W.px(),2) - pow(pd2_W.py(),2) - pow(pd2_W.pz(),2));

          //rescale mass of W boson daughters
	  d1_W.m(m1_W);
	  d2_W.m(m2_W);
    }
    double breitWignerDenom(double mFrac){
            
      if(mFrac < 0.025) return breitWignerDenom(0.025);
      if(mFrac > 0.6) return breitWignerDenom(0.6);
      
      if(mFrac < 0.0425) return 1e-12/(-1.293+1.098e+2*mFrac-2.800e+3*mFrac*mFrac+2.345e+4*mFrac*mFrac*mFrac);
      if(mFrac < 0.073) return 1.248e-12*(exp(1.158+18.34*mFrac));
      
      return 5.733e-10*pow(mFrac,-3.798-0.6555*log(mFrac))/pow(1.427-mFrac,30.017);
    }
    
    double pTWeight(double rH){
      
      double pe0 = 9.705/2000.;
      double pe1 = -1.27668e-03;
      
      double weightHighpT =1./(exp(pe0+pe1*rH));
      
      double p0 = 0.00405295;
      double p1 = -1.15389e-06;
      double p2 =  -8.83305e-10;
      double p3 =  1.02983e-12;
      double p4 = -3.64486e-16;
      double p5 = 6.05783e-20;
      double p6 = -4.74988e-24;
      double p7 = 1.40627e-28;
      double weightFinal = (p0+(p1*rH)+(p2*pow(rH,2))+(p3*pow(rH,3))+(p4*pow(rH,4))+(p5*pow(rH,5))+(p6*pow(rH,6))+(p7*pow(rH,7)));
      
      if(rH < 400.) weightFinal *= 0.5;
      
      return weightHighpT * weightFinal;
    }

    // This switch says whether to use the old style flattening of the Breit Wigner, or additionally flatten the PT spectrum.
    // Off by default, for consistency with old production jobs    
    Pythia8_UserHooks::UserSetting<int> m_flatpT = Pythia8_UserHooks::UserSetting<int>("WprimeWZFlatPtMass6:FlatPt", 0);  
    Pythia8_UserHooks::UserSetting<int> m_flatMass = Pythia8_UserHooks::UserSetting<int>("WprimeWZFlatPtMass6:FlatMass",0);  
       
  };  

  Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::WprimeWZFlatPtMass6> WprimeWZFlatPtMass6Creator("WprimeWZFlatPtMass6");

} // end namespace Pythia8


