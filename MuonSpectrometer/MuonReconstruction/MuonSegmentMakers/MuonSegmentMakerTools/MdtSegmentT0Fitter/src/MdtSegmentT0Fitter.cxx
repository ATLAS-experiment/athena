/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtSegmentT0Fitter/MdtSegmentT0Fitter.h"

#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadCondHandle.h"

#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/IRtResolution.h"
#include "MdtCalibData/MdtRtRelation.h"

#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonPrepRawData/MdtPrepData.h"

#include "Minuit2/Minuit2Minimizer.h"
#include "Math/Functor.h"
#include "TMath.h"
#include <functional>

#include <iostream>
#include <fstream>
#include <atomic>
#include <mutex>

namespace {
  // number of fit parameters
  constexpr unsigned int NUMPAR=3;
 
  // time corresponding to r=15 mm for internal rt
  //constexpr double TUBE_TIME = 757.22;

  //constexpr double MAX_DRIFT= 855;

   // garbage time value to return when radius isn't wihin rt range
   constexpr double R2TSPURIOUS = 50000;
   
   constexpr int WEAK_TOPO_T0ERROR = 10;
   
   constexpr int STRONG_TOPO_T0ERROR = 50;

   struct HitCoords {
        public:
            HitCoords(const double z_coord, const double t_coord, 
                      const double y_coord, const double w_coord, 
                      const double r_coord, const MuonCalib::IRtRelation * rt_rel):
            z(z_coord),
            t(t_coord),
            y(y_coord),
            w(w_coord),
            r(r_coord),
            rt(rt_rel){}
        /// Z coordinate
        double z{0.};
        /// Drift time
        double t{0.};
        /// Y coordinate
        double y{0.};
        /// Inverse error
        double w{0.};
        /// Drift radius
        double r{0.};
        /// Drift error
        double dr{0.};
        /// Rt relation function
        const MuonCalib::IRtRelation *rt{nullptr};
        /// Flag whether the hit is rejected
        bool rejected{false};
  };
  template <typename T> constexpr T sq(const T a) {return a * a;}
  
  class FunctionToMinimize : public ROOT::Math::IMultiGenFunction {
    public:      
      explicit FunctionToMinimize(const int used) :m_used{used} {}
     
      double DoEval(const double* xx) const override {
        const double ang = xx[0];
        const double b = xx[1];
        const double t0 = xx[2];
        
        const double cosin = std::cos(ang);
        const double sinus = std::sin(ang);
        
        double fval = 0.;
        // Add t0 constraint
        if (m_t0Error == WEAK_TOPO_T0ERROR ) {
         fval += xx[2]*xx[2]/(1.0 *m_t0Error*m_t0Error);
        }        
        for(int i=0;i<m_used;++i) {
          const double t = m_data[i].t - t0;
          const double z = m_data[i].z;
          const double y = m_data[i].y;
          const double w = m_data[i].w;
          const double dist = std::abs(b*cosin + z*sinus - y*cosin); // same thing as fabs(a*z - y + b)/sqrt(1. + a*a);
          const double uppercut = m_data[i].rt->tUpper();
          const double lowercut = m_data[i].rt->tLower();
                   
          // Penalty for t<lowercut and t >uppercut
          if (t> uppercut ) { // too large
            fval += sq(t-uppercut)*0.1;
          } else if (t < lowercut) {// too small
            fval += sq(t-lowercut)*0.1;
          }
          const double r = t< lowercut ?  m_data[i].rt->radius(lowercut) : t > uppercut ? m_data[i].rt->radius(uppercut) :  m_data[i].rt->radius(t);
          fval += sq(dist - r)*w;
        }
        
        return fval;
      }
      ROOT::Math::IBaseFunctionMultiDim* Clone() const override {return new FunctionToMinimize(m_used);}
      unsigned int NDim() const override {return 3;}
      void setT0Error(const int t0Error){m_t0Error=t0Error;}
      void addCoords(HitCoords coord){
        m_data.emplace_back(coord);
      }
    private:
      std::vector<HitCoords> m_data{};
      int m_used{0};
      int m_t0Error{-1};
  };
  
   /***********************************************************************************/
  /// RT function from Craig Blocker
  /// ok for now, possibly replace with actual RT function used to calibrate run

  //constexpr double T2R_A[] = {1.184169e-1, 3.32382e-2, 4.179808e-4, -5.012896e-6, 2.61497e-8, -7.800677e-11, 1.407393e-13, -1.516193e-16, 8.967997e-20, -2.238627e-23};
  //constexpr double RCORR_A[] = {234.3413, -5.803375, 5.061677e-2, -1.994959e-4, 4.017433e-7, -3.975037e-10, 1.522393e-13};

  

  double r2t_ext(const MuonCalib::IRtRelation* rtrel, double r) {
    double ta = rtrel->tLower();
    double tb = rtrel->tUpper();
    if(r<rtrel->radius(ta) ) {
      return -1*R2TSPURIOUS;
    } else if(r>rtrel->radius(tb)) {
      return R2TSPURIOUS;
    }

    int itr = 0;
    while (ta <= tb) {
      double tm  = (ta + tb) / 2;  // compute mid point.
      double rtm = rtrel->radius(tm);
      if(std::abs(rtm - r) < 0.001 ) {
        return tm;
      }
      else if (r > rtm) {
        ta = tm;  // repeat search in top half.
      }
      else if (r < rtm ) {
        tb = tm; // repeat search in bottom half.
      }

      itr++;
      if(itr>50) return -1;
    }
    return -1;    // failed to find key
  }
  int sign(double a) {
    return a > 0 ? 1 : a < 0 ? -1 : 0;
  }
}

namespace TrkDriftCircleMath {

  MdtSegmentT0Fitter::MdtSegmentT0Fitter(const std::string& ty,const std::string& na,const IInterface* pa)
  : AthAlgTool(ty,na,pa),
    DCSLFitter() {
    declareInterface <IDCSLFitProvider> (this);
  }

  StatusCode MdtSegmentT0Fitter::initialize() {
    ATH_CHECK(m_calibDbKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode MdtSegmentT0Fitter::finalize() {

    double scaleFactor = m_ntotalCalls != 0 ? 1./(double)m_ntotalCalls : 1.;

    ATH_MSG_INFO( "Summarizing fitter statistics " << "\n"
                  << " Total fits       " << std::setw(10) << m_ntotalCalls << "   " << scaleFactor*m_ntotalCalls << "\n"
                  << " hits > 2         " << std::setw(10) << m_npassedNHits << "   " << scaleFactor*m_npassedNHits << "\n"
                  << " hit consis.      " << std::setw(10) << m_npassedSelectionConsistency << "   " << scaleFactor*m_npassedSelectionConsistency << "\n"
                  << " sel. hits > 2    " << std::setw(10) << m_npassedNSelectedHits << "   " << scaleFactor*m_npassedNSelectedHits << "\n"
                  << " Hits > min hits  " << std::setw(10) << m_npassedMinHits << "   " << scaleFactor*m_npassedMinHits << "\n"
                  << " Passed Fit       " << std::setw(10) << m_npassedMinuitFit << "   " << scaleFactor*m_npassedMinuitFit  );
    return StatusCode::SUCCESS;
  }

  bool MdtSegmentT0Fitter::fit( Segment& result, const Line& line, const DCOnTrackVec& dcs, const HitSelection& selection, double t0Seed ) const {
    ++m_ntotalCalls;
    const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};
    ATH_MSG_DEBUG("New seg: ");
    const EventContext& ctx{Gaudi::Hive::currentContext()};
    SG::ReadCondHandle<MuonCalib::MdtCalibDataContainer> calibData{m_calibDbKey, ctx};
    if (!calibData.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve Mdt calibration object "<<m_calibDbKey.fullKey());
      return false;
    }
    std::array<const MuonCalib::MdtRtRelation*, 2> rtRelations{};
    {
      unsigned int nRel{0};
      for (unsigned int i = 0; i < dcs.size() ; ++i) {        
        const Identifier id = dcs[i].rot()->identify();
        const int mlIdx = id_helper.multilayer(id) -1;
        if (rtRelations[mlIdx]) continue;
        rtRelations[mlIdx] = calibData->getCalibData(id, msgStream())->rtRelation.get();
        ++nRel;
        if (nRel == 2) break;
      }
    }
    const DCOnTrackVec& dcs_keep = dcs;

    unsigned int N = dcs_keep.size();

    result.setT0Shift(-99999,-99999);

    if(N<2) {
      return false;
    }
    ++m_npassedNHits;
    if( selection.size() != N ) {
      ATH_MSG_ERROR("MdtSegmentT0Fitter.cxx:fit with t0 <bad HitSelection>");
      return false;
    }
    ++m_npassedSelectionConsistency;
    int used=0;
    for(unsigned int i=0;i<N;++i){
      if( selection[i] == 0 ) ++used;
    }
    if(used < 2){
      ATH_MSG_DEBUG("TOO FEW HITS SELECTED");
      return false;
    }
    ++m_npassedNSelectedHits;
    
    if(used < m_minHits) {
      ATH_MSG_DEBUG("FEWER THAN Minimum HITS N " << m_minHits << " total hits " <<N<<" used " << used);

      //
      //     Copy driftcircles and reset the drift radii as they might have been overwritten
      //     after a succesfull t0 fit
      //

      DCOnTrackVec dcs_new;
      dcs_new.reserve(dcs.size());
      
      double chi2p = 0.;    
      int n_elements = dcs.size();
      for(int i=0; i< n_elements; ++i ){
          const DriftCircle* ds  = & dcs[i];
          if(std::abs(ds->r()-ds->rot()->driftRadius())>m_dRTol) ATH_MSG_DEBUG("Different radii on dc " << ds->r() << " rot " << ds->rot()->driftRadius());
          
          DriftCircle dc_keep(ds->position(), ds->rot()->driftRadius(), ds->dr(), ds->drPrecise(), ds->driftState(), ds->id(), ds->index(),ds->rot() );
          DCOnTrack dc_new(dc_keep, 0., 0.);
          
          dc_new.state(dcs[i].state());
          dcs_new.push_back( dc_new );
          if( selection[i] == 0 ){
            double t = ds->rot()->driftTime();
            const unsigned int mlIdx = id_helper.multilayer(ds->rot()->identify()) - 1;
            const MuonCalib::MdtRtRelation *rtInfo = rtRelations[mlIdx];
            
            double tUp = rtInfo->rt()->tUpper();
            double tLow = rtInfo->rt()->tLower();
            
            if(t<tLow) chi2p += sq(t-tLow)*0.1;
            else if(t>tUp) chi2p += sq(t-tUp)*0.1;
            }
      }
      
      if(chi2p>0) ATH_MSG_DEBUG("NO Minuit Fit TOO few hits Chi2 penalty " << chi2p);
      
      bool oldrefit = DCSLFitter::fit( result, line, dcs_new, selection );
      
      chi2p += result.chi2();
      // add chi2 penalty for too large or too small driftTimes  t < 0 or t> t upper
      result.set(chi2p, result.ndof(),  result.dtheta(),  result.dy0());
      int iok = 0;
      if(oldrefit) iok = 1;
      ATH_MSG_DEBUG(" chi2 total " << result.chi2() << " angle " << result.line().phi() << " y0 " << result.line().y0()  << " nhits "<< selection.size() << " refit ok " << iok);
      return oldrefit;
    
    }
    
    ATH_MSG_DEBUG("FITTING FOR T0 N "<<N<<" used " << used);
    

    ++m_npassedMinHits;

   
    ATH_MSG_DEBUG(" in  MdtSegmentT0Fitter::fit with N dcs "<< N << " hit selection size " <<  selection.size());
    ATH_MSG_DEBUG("in fit "<<result.hasT0Shift()<< " " <<result.t0Shift());
    

    double Zc{0.}, Yc{0.}, S{0.}, Sz{0.}, Sy{0};

    std::vector<HitCoords> hits{};
    hits.reserve(N);
    FunctionToMinimize minFunct(used);

    {
      unsigned int ii{0};
      for(const DCOnTrack& keep_me : dcs_keep ){
        const Muon::MdtDriftCircleOnTrack *roto = keep_me.rot();
        const unsigned int mlIdx = id_helper.multilayer(roto->identify()) - 1;
        const MuonCalib::MdtRtRelation *rtInfo = rtRelations[mlIdx];

        const double newerror = m_scaleErrors ? keep_me.drPrecise() : keep_me.dr();
        const double w = newerror >0. ? sq(1./newerror) : 0.;
        hits.emplace_back(keep_me.x(), roto->driftTime(), keep_me.y(), w, std::abs(roto->driftRadius()), rtInfo->rt());
        HitCoords& coords = hits.back();
        coords.dr = keep_me.dr();
        coords.rejected = selection[ii];
        ATH_MSG_DEBUG("DC:  (" << coords.y << "," << coords.z << ") R = " << coords.r << " W " << coords.w 
                               <<" t " <<coords.t<< " id: "<<keep_me.id()<<" sel " <<coords.rejected);
        if (!coords.rejected) {          
           S += coords.w;
           Sz+= coords.z* coords.w;
           Sy+= coords.y * coords.w;
        }
        ++ii;
      }
    }
    
    /// Normalize the mean positions
    const double inv_S = 1. / S;
    Zc = Sz*inv_S;
    Yc = Sy*inv_S;

    ATH_MSG_DEBUG("Yc " << Yc << " Zc " << Zc);

    /// go to coordinates centered at the average of the hits
    for(HitCoords& coords : hits) {
      coords.y -= Yc;
      coords.z -= Zc;
    }

    int selcount{0};
  
    // replicate for the case where the external rt is used...
    // each hit has an rt function with some range...we want to fit such that
    // tlower_i < ti - t0 < tupper_i
    double min_tlower{std::numeric_limits<float>::max()}, max_tupper{-std::numeric_limits<float>::max()};

    double t0seed=0; // the average t0 of the hit
    double st0 = 0; // the std deviation of the hit t0s
    double min_t0 = 1e10; // the smallest t0 seen
  
    for(HitCoords& coords : hits) {
      if(coords.rejected) continue;
      
      double r2tval = r2t_ext(coords.rt, coords.r) ;
      const double tl = coords.rt->tLower();
      const double th = coords.rt->tUpper();
      const double tee0 = coords.t - r2tval;

      min_tlower = std::min(min_tlower, coords.t - tl);
      max_tupper = std::max(max_tupper, coords.t - th);
      
      
      ATH_MSG_DEBUG(" z "<<coords.z <<" y "<<coords.y<<" r "<<coords.r
                  <<" t "<<coords.t<<" t0 "<<tee0<<" tLower "<<tl<<" tUpper "<<th);
      t0seed += tee0;
      st0 += sq(tee0);
      if(tee0 < min_t0 && std::abs(r2tval) < R2TSPURIOUS) min_t0 = tee0;

      minFunct.addCoords(coords);

      selcount++;
      
    }
    t0seed /= selcount;
    st0 = st0/selcount - t0seed*t0seed;
    st0 = st0 > 0. ? std::sqrt(st0) : 0.;

    ATH_MSG_DEBUG(" t0seed "<<t0seed<<" sigma "<<st0<< " min_t0 "<<min_t0);

    // ************************* seed the parameters
    const double theta = line.phi();
    double cosin = std::cos(theta);
    double sinus = std::sin(theta);

    if ( sinus < 0.0 ) {
      sinus = -sinus;
      cosin = -cosin;
    } else if ( sinus == 0.0 && cosin < 0.0 ) {
      cosin = -cosin;
    }
    
    ATH_MSG_DEBUG("before fit theta "<<theta<<" sinus "<<sinus<< " cosin "<< cosin);

    double d = line.y0() + Zc*sinus-Yc*cosin;

    
    ATH_MSG_DEBUG(" line x y "<<line.position().x()<<" "<<line.position().y());
    ATH_MSG_DEBUG(" Zc Yc "<< Zc <<" "<<Yc);
    ATH_MSG_DEBUG(" line x0 y0 "<<line.x0()<<" "<<line.y0());
    ATH_MSG_DEBUG(" hit shift " << -Zc*sinus+Yc*cosin);
    
// Calculate signed radii

    int nml1p{0}, nml2p{0}, nml1n{0}, nml2n{0};
    int ii{-1};
    for(const DCOnTrack& keep_me : dcs_keep){
      ++ii;
      const HitCoords& coords = hits[ii];
      if(coords.rejected) continue;
      const double sdist = d*cosin + coords.z*sinus - coords.y*cosin; // same thing as |a*z - y + b|/sqrt(1. + a*a);
      nml1p+=(keep_me.id().ml()==0&&sdist > 0);
      nml1n+=(keep_me.id().ml()==0&&sdist < 0);
      nml2p+=(keep_me.id().ml()==1&&sdist > 0);
      nml2n+=(keep_me.id().ml()==1&&sdist < 0);
    }

// Define t0 constraint in Minuit
    int t0Error = STRONG_TOPO_T0ERROR;
    if (nml1p+nml2p < 2 || nml1n+nml2n < 2) t0Error = WEAK_TOPO_T0ERROR;

    minFunct.setT0Error(t0Error);

// Reject topologies where in one of the Multilayers no +- combination is present
    if((nml1p<1||nml1n<1)&&(nml2p<1||nml2n<1)&&m_rejectWeakTopologies) {
       ATH_MSG_DEBUG("Combination rejected for positive radii ML1 " <<  nml1p << " ML2 " <<  nml2p << " negative radii ML1 " << nml1n << " ML " << nml2n << " used hits " << used << " t0 Error " << t0Error);
      DCOnTrackVec::const_iterator it = dcs.begin();
      DCOnTrackVec::const_iterator it_end = dcs.end();
      double chi2p = 0.;
      DCOnTrackVec dcs_new;
      dcs_new.reserve(dcs.size());
      for(int i=0; it!=it_end; ++it, ++i ){
	      const DriftCircle* ds  = & dcs[i];
        if(std::abs(ds->r()-ds->rot()->driftRadius())>m_dRTol) ATH_MSG_DEBUG("Different radii on dc " << ds->r() << " rot " << ds->rot()->driftRadius());
        DriftCircle dc_keep(ds->position(), ds->rot()->driftRadius(), ds->dr(), ds->drPrecise(), ds->driftState(), ds->id(), ds->index(),ds->rot() );
        DCOnTrack dc_new(dc_keep, 0., 0.);
        dc_new.state(dcs[i].state());
        dcs_new.push_back( std::move(dc_new) );
        if( selection[i] == 0 ){
          double t = ds->rot()->driftTime();
          const unsigned int mlIdx = id_helper.multilayer(ds->rot()->identify()) - 1;
          const MuonCalib::MdtRtRelation *rtInfo = rtRelations[mlIdx];
          double tUp = rtInfo->rt()->tUpper();
          double tLow = rtInfo->rt()->tLower();
          if(t<tLow) chi2p += sq(t-tLow)*0.1;
          if(t>tUp) chi2p += sq(t-tUp)*0.1;
        }
      }
      if(chi2p>0) ATH_MSG_DEBUG(" Rejected weak topology Chi2 penalty " << chi2p);
      bool oldrefit = DCSLFitter::fit( result, line, dcs_new, selection );
      chi2p += result.chi2();
// add chi2 penalty for too large or too small driftTimes  t < 0 or t> t upper
      result.set( chi2p, result.ndof(),  result.dtheta(),  result.dy0() );
      return oldrefit;
    }  // end rejection of weak topologies

    ATH_MSG_DEBUG("positive radii ML1 " <<  nml1p << " ML2 " <<  nml2p << " negative radii ML1 " << nml1n << " ML " << nml2n << " used hits " << used << " t0 Error " << t0Error);

    constexpr std::array<Double_t,3> step{0.01 , 0.01 , 0.1 };
    // starting point
    std::array<Double_t,3> variable{theta,d,0};
    // if t0Seed value from outside use this
    if(t0Seed > -999.) variable[2] = t0Seed;

    ROOT::Minuit2::Minuit2Minimizer minimum("algoName");
    minimum.SetMaxFunctionCalls(10000);
    minimum.SetTolerance(0.001);
    minimum.SetPrintLevel(-1);
    if(msgLvl(MSG::VERBOSE)) minimum.SetPrintLevel(1);

    if (m_floatDir){
      minimum.SetVariable(0,"a",variable[0], step[0]);
      minimum.SetVariable(1,"b",variable[1], step[1]);
    } else {
      minimum.SetFixedVariable(0,"a", variable[0]);
      minimum.SetFixedVariable(1,"b", variable[1]);
    }
    
    minimum.SetVariable(2,"t0",variable[2], step[2]);
    
    minimum.SetFunction(minFunct);

    // do the minimization
    if ( !minimum.Minimize() ) {
      ATH_MSG_DEBUG("MinuitMinimizer::Minimize() fit did not converge");
      return false;
    }

    const double *results = minimum.X();
    const double *errors = minimum.Errors();
    ATH_MSG_DEBUG("Minimum: f(" << results[0] << "+-" << errors[0] << "," << results[1]<< "+-" << errors[1]<< "," << results[2] << "+-" << errors[2]<< "): " << minimum.MinValue());

    ++m_npassedMinuitFit;

    // Get the fit values
    double aret=results[0];
    double aErr=errors[0];
    double dtheta = aErr;
    double tana = std::tan(aret); // tangent of angle
    double ang = aret;  // between zero and pi
    cosin = std::cos(ang);
    sinus = std::sin(ang);
    if ( sinus < 0.0 ) {
      sinus = -sinus;
      cosin = -cosin;
    } else if ( sinus == 0.0 && cosin < 0.0 ) {
      cosin = -cosin;
    }
    ang = std::atan2(sinus, cosin);
    double b=results[1];
    double bErr=errors[1];
    double t0=results[2];
    double t0Err=errors[2];
    double dy0 = cosin * bErr - b * sinus * aErr;

    const double del_t = std::abs(hits[0].rt->radius((t0+t0Err)) - hits[0].rt->radius(t0)) ;

    
    ATH_MSG_DEBUG("____________FINAL VALUES________________" );
    ATH_MSG_DEBUG("Values: a "<<tana<<" d "<<b * cosin <<" t0 "<<t0);
    ATH_MSG_DEBUG("Errors: a "<<aErr<<" b "<<dy0 <<" t0 "<<t0Err);
    
    d = b * cosin;
    if(msg().level() <=MSG::DEBUG) {
      msg() << MSG::DEBUG <<"COVAR  ";
      for(int it1=0; it1<3; it1++) {
        for(int it2=0; it2<3; it2++) {
          msg() << MSG::DEBUG <<minimum.CovMatrix(it1,it2)<<" ";
        }
        msg() << MSG::DEBUG << endmsg;
      }
    }

    result.dcs().clear();
    result.clusters().clear();
    result.emptyTubes().clear();

     ATH_MSG_DEBUG("after fit theta "<<ang<<" sinus "<<sinus<< " cosin "<< cosin);

    double chi2 = 0;
    unsigned int nhits(0);
    
    // calculate predicted hit positions from track parameters
  
    ATH_MSG_DEBUG("------NEW HITS------");
    int i{-1};
    for(const HitCoords& coords : hits){
      ++i;
      const DCOnTrack& keep_me{dcs_keep[i]};
      const double uppercut = coords.rt->tUpper();
      const double lowercut = coords.rt->tLower();
      
      double rad = coords.rt->radius(coords.t-t0);
      if(coords.t-t0<lowercut) rad = coords.rt->radius(lowercut);
      if(coords.t-t0>uppercut) rad = coords.rt->radius(uppercut);
      if (coords.w==0) {
        ATH_MSG_WARNING("coords.w==0, continuing");
        continue;
      }
      double drad = 1.0/std::sqrt(coords.w) ;

      double yl = (coords.y -  tana*coords.z - b);
      
      ATH_MSG_DEBUG("i "<<i<<" ");
      

      double dth = -(sinus*coords.y + cosin*coords.z)*dtheta;
      double residuals = std::abs(yl)/std::sqrt(1+tana*tana) - rad;
      
      ATH_MSG_DEBUG(" dth "<<dth<<" dy0 "<<dy0<<" del_t "<<del_t);
      

      double errorResiduals = std::hypot(dth, dy0, del_t);

      // derivatives of the residual 'R'
      std::array<double,3> deriv{};
      // del R/del theta
      double dd = coords.z * sinus + b *cosin - coords.y * cosin;
      deriv[0] = sign(dd) * (coords.z * cosin - b * sinus + coords.y * sinus);
      // del R / del b
      deriv[1] = sign(dd) * cosin ;
      // del R / del t0

      deriv[2] = -1* coords.rt->driftvelocity(coords.t-t0);

      double covsq=0;
      for(int rr=0; rr<3; rr++) {
        for(int cc=0; cc<3; cc++) {
          covsq += deriv[rr]*minimum.CovMatrix(rr,cc)* deriv[cc];
        }
      }
      ATH_MSG_DEBUG(" covsquared " << covsq);
      if( covsq < 0. && msgLvl(MSG::DEBUG)){
        for(int rr=0; rr<3; rr++) {
            for(int cc=0; cc<3; cc++) {
                double dot = deriv[rr]*minimum.CovMatrix(rr,cc)* deriv[cc];
                ATH_MSG_DEBUG(" adding term " << dot << " dev1 " << deriv[rr] << " cov " << minimum.CovMatrix(rr,cc) << " dev2 " << deriv[cc]);
            }
        }
      }
      
      covsq = covsq > 0. ? std::sqrt(covsq) : 0.;
      const DriftCircle* ds  = & keep_me;
      if (m_propagateErrors) drad = coords.dr;
      
      DriftCircle dc_newrad(keep_me.position(), rad, drad, ds->driftState(), keep_me.id(), keep_me.index(),ds->rot() );
      DCOnTrack dc_new(dc_newrad, residuals, covsq);
      dc_new.state(keep_me.state());

      ATH_MSG_DEBUG("T0 Segment hit res "<<residuals<<" eres "<<errorResiduals<<" covsq "<<covsq<<" ri " << coords.r<<" ro "<<rad<<" drad "<<drad << " sel "<<selection[i]<< " inv error " << coords.w);

      if(!coords.rejected) {
        ++nhits;
        if (!m_propagateErrors) {
          chi2 += sq(residuals)*coords.w;
        } else {
          chi2 += sq(residuals)/sq(drad);
        }
        ATH_MSG_DEBUG("T0 Segment hit res "<<residuals<<" eres "<<errorResiduals<<" covsq "<<covsq<<" ri " << coords.r<<" radius after t0 "<<rad<<" radius error "<< drad <<  " original error " << coords.dr);
// Put chi2 penalty for drift times outside window
        if (coords.t-t0> uppercut ) { // too large
	          chi2  += sq(coords.t-t0-uppercut)*0.1;
        }else if (coords.t-t0 < lowercut ) {// too small
	          chi2 += sq(coords.t-t0-lowercut)*0.1;
        }
      }
      result.dcs().push_back( dc_new );
    }

    double oldshift = result.t0Shift();
    ATH_MSG_DEBUG("end fit old "<<oldshift<< " new " <<t0);
    // Final Minuit Fit result
    if(nhits==NUMPAR) {
      nhits++;
      chi2 += 1.;
    }
    result.set( chi2, nhits-NUMPAR, dtheta, -1.*dy0 );
    result.line().set( LocVec2D( Zc - sinus*d, Yc + cosin*d ), ang );
    if(t0==0.) t0=0.00001;
    result.setT0Shift(t0,t0Err);
   
    ATH_MSG_DEBUG("Minuit Fit complete: Chi2 " << chi2 << " angle " << result.line().phi() << " nhits "<< nhits  << " t0result " << t0);
    ATH_MSG_DEBUG("Minuit Fit complete: Chi2 " << chi2 << " angle " << result.line().phi() << " nhits "<<nhits<<" numpar "<<NUMPAR << " per dof " << chi2/(nhits-NUMPAR));
    ATH_MSG_DEBUG("Fit complete: Chi2 " << chi2 <<" nhits "<<nhits<<" numpar "<<NUMPAR << " per dof " << chi2/(nhits-NUMPAR)<<(chi2/(nhits-NUMPAR) > 5 ? " NOT ":" ")<< "GOOD");
    ATH_MSG_DEBUG("chi2 "<<chi2<<" per dof "<<chi2/(nhits-NUMPAR));
    
    return true;
  }

}
