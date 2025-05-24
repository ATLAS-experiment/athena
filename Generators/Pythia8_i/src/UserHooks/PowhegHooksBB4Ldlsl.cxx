// PowhegHooksBB4L.h
// Rewritten by T. Jezo in 2021.  With various contributions from S. Ferrario
// Ravasio, B. Nachman, P.  Nason and M. Seidel. Inspired by
// ttb_NLO_dec/main-PYTHIA8.f by P. Nason and E. Re and by PowhegHooks.h by R.
// Corke.
//
// Adapted to be Athena and Pythia8_i compliant by Katharina Voß (katharina.voss@cern.ch) 
// following the example from Simone Amoroso in PowhegBB4Ltms.cxx

// # Introduction
//
// This hook is intended for use together with POWHEG-BOX-RES/b_bbar_4l NLO LHE
// events. This also includes events in which one of the W bosons was
// re-decayed hadronically. (Note that LHE format version larger than 3 may not
// require this hook).
//
// The hook inherits from PowhegHooks and as such it indirectly implements the
// following:
//  - doVetoMPIStep
//  - doVetoISREmission
//  - doVetoMPIEmission
// and it overloads:
//  - doVetoFSREmission, which works as follows (if POWHEG:veto==1):
//      - if inResonance==true it vetoes all the emission that is harder than
//      the scale of its parent (anti-)top quark or W^+(-)
//      - if inResonance==false, it calls PowhegHooks::doVetoISREmission
// and it also implements:
//  - doVetoProcessLevel, which is never used for vetoing (i.e. it always
//    returns false). Instead it is used for the calculation of reconance scales
//    using LHE kinematics.
//
// This version of the hooks is only suitable for use with fully compatible
// POWHEG-BOX Les Houches readers (such the one in main-PYTHIA82-lhef but
// not the one in main31.cc.)
//
//
// # Basic use
//
// In order to use this hook you must replace all the declarations and
// constructor calls of PowhegHooks to PowhegHooksBB4L:
//
//   PowhegHooks *powhegHooks; -> PowhegHooksBB4L *powhegHooks;
//   *powhegHooks = new PowhegHooks(); -> *powhegHooks = new PowhegHooksBB4L();
//
// In order to switch it on set POWHEG:veto = 1 and
// POWHEG:bb4l:FSREmission:veto = 1. This will lead to a veto in ISR, FSR and
// MPI steps of pythia as expected using PowhegHooks in all the cases other than
// the case of FSR emission in resonance decay. Within resonance decays
// PowhegHooksBB4L takes over the control.
//
// Furthermore, this hook can also be used standalone without PowhegHooks, i.e.
// only the FSR emission from resonances will be vetoed (the default use in
// 1801.03944 and 1906.09166). In order to do that set
// POWHEG:bb4l:FSREmission:veto = 1 and POWHEG:veto = 0. Note that the this is
// not recommended for comparing against data but because it is easier to
// interpret it is often done in theoretical studies.
//
// Note that this version of the hook relies on the event "radtype" (1 for
// btilde, 2 for remnant) to be set by an external program, such as
// main-PYTHIA82-lhef in the radtype_ common block.
// There also exists a version of this hook in which the event "radtype" is
// read in from the .lhe file using pythia built in functionality. You need
// that version if you want to use this hook with main31.cc.
//
//
// # Expert use
//
// This hook also implements an alternative veto procedure which allows to
// assign a "SCALUP" type of scale to a resonance using the scaleResonance
// method. This is a much simpler veto but it is also clearly inferior as
// compared to the one implemented using the doVetoFSREmission method because
// the definition of the scale of the emission does not match the
// POWHEG-BOX-RES definition. Nevertheless, it can be activated using
// POWHEG:bb4l:ScaleResonance:veto = 1. Additionally one MUST switch off the
// other veto by calling on POWHEG:bb4l:FSREmission:veto = 0.
// 
// The following settings are at the disposal of the user to control the
// behaviour of the hook
//   - On/off switches for the veto:
//      - POWHEG:bb4l:FSREmission:veto
//          on/off switch for the default veto based on doFSREmission
//      - POWHEG:bb4l:ScaleResonance:veto
//          on/off switch for the alternative veto based on scaleResonance (only
//          for expert users)
//   - Important settings:
//      - POWHEG:bb4l:ptMinVeto: MUST be set to the same value as the
//      corresponding flag in POWHEG-BOX-RES
//   - Alternatives for scale calculations
//      - default: emission scale is calculated using POWHEG definitions and in
//      the resonance rest frame
//      - POWHEG:bb4l:FSREmission:vetoDipoleFrame: emission scale is calculated
//      using POWHEG definitions in the dipole frame
//      - POWHEG:bb4l:FSREmission:pTpythiaVeto: emission scale is calculated 
//      using Pythia definitions
//   - Other flags:
//      - POWHEG:bb4l:FSREmission:vetoQED: decides whether or not QED emission
//      off quarks should also be vetoed (not implemented in the case of 
//      the ScaleResonance:veto)
//      - POWHEG:bb4l:DEBUG: enables debug printouts on standard output

#include "Pythia8/Pythia.h"
#include "UserHooksUtils.h"
#include "Pythia8_i/UserHooksFactory.h"
#if PYTHIA_VERSION_INTEGER >= 8310
// Disable the pythia 8.310 plugin mechanism.
// We use our own mechanism to create these classes, and we get link errors
// if these are used in more than one compilation unit.
# include "Pythia8/Plugins.h"
# undef PYTHIA8_PLUGIN_CLASS
# undef PYTHIA8_PLUGIN_VERSIONS
# define PYTHIA8_PLUGIN_CLASS(BASE, CLASS, PYTHIA, SETTINGS, LOGGER)
# define PYTHIA8_PLUGIN_VERSIONS(...)
#endif

#include "PowhegHooksSetterMethod.cxx"

#include "UserSetting.h"
#include <iostream>
#include <cassert>
#include <stdexcept>

struct{
  int radtype;
} radtype_;


namespace Pythia8 { class PowhegBB4Ldlsl; }
Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::PowhegBB4Ldlsl> powhegBB4LdlslCreator("PowhegBB4Ldlsl");


namespace Pythia8 {
    using namespace std;

    class PowhegBB4Ldlsl : public PowhegHooksSetterMethod {

        public:

            PowhegBB4Ldlsl() 
                : m_debug("Powheg:bb4l:DEBUG", false),
                    m_vetoFSREmission("Powheg:bb4l:FSREmission:veto", true),
                    m_vetoQED("Powheg:bb4l:vetoQED", false),
                    m_topresscale(-1.), m_atopresscale(-1.), 
                    m_wpresscale(-1.),m_wmresscale(-1.),
                    m_nInResonanceFSRveto(0),
                    m_pTmin("Powheg:bb4l:pTminVeto", 0.5),
                    m_vetoProduction("Powheg:veto", 1),
                    m_pTpythiaVeto("Powheg:bb4l:pTpythiaVeto", 0),
                    m_vetoDipoleFrame("Powheg:bb4l:FSREmission:vetoDipoleFrame", 0),
                    m_scaleResonanceVeto("Powheg:bb4l:ScaleResonance:veto", 0.),
                    m_pThardMode("POWHEG:pThard",0)
                {
                std::cout << "**********************************************************" << std::endl;
                std::cout << "*                                                        *" << std::endl;
                std::cout << "*      Applying Powheg BB4L UserHook (PowhegBB4Ldlsl)!    *" << std::endl;
                std::cout << "*       Must run on dedicated Powheg LHE input file      *" << std::endl;
                std::cout << "*           (on your own responsibility)                 *" << std::endl;
                std::cout << "*                                                        *" << std::endl;
                std::cout << "**********************************************************" << std::endl;

                }

            ~PowhegBB4Ldlsl()  override { }

            //--- Initialization -------------------------------------------------------
            virtual bool initAfterBeams() override {
                // initialize settings of the parent class
                PowhegHooksSetterMethod::initAfterBeams();
                // initialize settings of this class already initialized in constructor
                return true;
            }

            //--- PROCESS LEVEL HOOK ---------------------------------------------------
            // This hook gets triggered for each event before the shower starts, i.e. at 
            // the LHE level. We use it to calculate the scales of resonances.
            inline virtual bool canVetoProcessLevel() override { return true; }
            inline virtual bool doVetoProcessLevel(Event &e) override {

                // extract the radtype from the event comment
                stringstream ss;
                ss << infoPtr->getEventComments();
                string temp;
                ss >> temp >> radtype_.radtype;
                assert (temp == "#rwgt");

                // we only calculate resonance scales for btilde events (radtype_.radtype==1)
                // remnant events are not vetoed
                if (radtype_.radtype==2) return false;
                // find last top and the last anti-top in the record
                int i_top = -1, i_atop = -1, i_wp = -1, i_wm = -1;
                for (int i = 0; i < e.size(); i++) {
                    if (e[i].id() == 6) i_top = i;
                    if (e[i].id() == -6) i_atop = i;
                    if (e[i].id() == 24) i_wp = i;
                    if (e[i].id() == -24) i_wm = i;
                }
                // if found calculate the resonance scale
                m_topresscale = findresscale(i_top, e);
                // similarly for anti-top
                m_atopresscale = findresscale(i_atop, e);
                // and for W^+ and W^-
                m_wpresscale = findresscale(i_wp, e);
                m_wmresscale = findresscale(i_wm, e);

                // this is from old UserHook -> not needed anymore?
                //   // initialize stuff
                //   doVetoFSRInit();


                // do not veto, ever
                return false;
            }

            //--- FSR EMISSION LEVEL HOOK ----------------------------------------------
            // This hook gets triggered everytime the parton shower attempts to attach
            // a FSR emission.
            inline virtual bool canVetoFSREmission() override { return m_vetoFSREmission(settingsPtr) || m_vetoProduction(settingsPtr); }
            inline virtual bool doVetoFSREmission(int sizeOld, const Event &e, int iSys, bool inResonance) override {

                // FSR VETO INSIDE THE RESONANCE (if it is switched on)
                if (inResonance && m_vetoFSREmission(settingsPtr)) {

                    // get the participants of the splitting: the recoiler, the radiator and the emitted 
                    int iRecAft = e.size() - 1;
                    int iEmt = e.size() - 2;
                    int iRadAft = e.size() - 3;
                    int iRadBef = e[iEmt].mother1();

                    // find the resonance the radiator originates from
                    int iRes = e[iRadBef].mother1();
                    while ( iRes > 0 && (std::abs(e[iRes].id()) !=6 && std::abs(e[iRes].id()) != 24) ) {
                        iRes = e[iRes].mother1();
                    }
                    if (iRes == 0) {
                        #if PYTHIA_VERSION_INTEGER >= 8310
                            loggerPtr->ERROR_MSG("Warning in PowhegHooksBB4L::doVetoFSREmission: emission in resonance not from the top quark or from the W boson, not vetoing");
                        #else
                            infoPtr->errorMsg("Warning in PowhegHooksBB4L::doVetoFSREmission: emission in resonance not from the top quark or from the W boson, not vetoing");
                        #endif
                        return doVetoFSR(false);
                    }
                    int iResId = e[iRes].id();

                    // calculate the scale of the emission
                    double scale;
                    //using pythia pT definition ...
                    if(m_pTpythiaVeto(settingsPtr))
                    scale = pTpythia(e, iRadAft, iEmt, iRecAft);
                    //.. or using POWHEG pT definition
                    else{
                        Vec4 pr(e[iRadAft].p()), pe(e[iEmt].p()), pres(e[iRes].p()), prec(e[iRecAft].p()), psystem;
                        // The computation of the POWHEG pT can be done in the top rest frame or in the diple one.
                        // pdipole = pemt +prec +prad (after the emission)
                            // For the first emission off the top resonance pdipole = pw +pb (before the emission) = ptop
                        if(m_vetoDipoleFrame(settingsPtr))
                            psystem = pr+pe+prec;
                        else
                            psystem = pres;
        
                        // gluon splitting into two partons
                        if (e[iRadBef].id() == 21)
                                scale = gSplittingScale(psystem, pr, pe);
                        // quark emitting a gluon (or a photon)
                        else if (std::abs(e[iRadBef].id()) <= 5 && ((e[iEmt].id() == 21) && ! m_vetoQED(settingsPtr)) )
                                scale = qSplittingScale(psystem, pr, pe);
                        // other stuff (which we should not veto)
                        else {
                                scale = 0;
                        }
                    }
                    
                    // compare the current splitting scale to the correct resonance scale
                    if (iResId == 6) {
                        if ( m_debug(settingsPtr) && scale > m_topresscale)
                            cout << iResId << ": " << e[iRadBef].id() << " > " << e[iRadAft].id() << " + " << e[iEmt].id() << "; " << scale << endl;
                        return doVetoFSR(scale > m_topresscale);
                    }
                    else if (iResId == -6){
                        if ( m_debug(settingsPtr) && scale > m_atopresscale)
                            cout << iResId << ": " << e[iRadBef].id() << " > " << e[iRadAft].id() << " + " << e[iEmt].id() << "; " << scale << endl;
                        return doVetoFSR(scale > m_atopresscale);
                    }
                    else if (iResId == 24) {
                        if ( m_debug(settingsPtr) && scale > m_wpresscale)
                            cout << iResId << ": " << e[iRadBef].id() << " > " << e[iRadAft].id() << " + " << e[iEmt].id() << "; " << scale << endl;
                        return doVetoFSR(scale > m_wpresscale);
                    }
                    else if (iResId == -24){
                        if ( m_debug(settingsPtr) && scale > m_wmresscale)
                            cout << iResId << ": " << e[iRadBef].id() << " > " << e[iRadAft].id() << " + " << e[iEmt].id() << "; " << scale << endl;
                        return doVetoFSR(scale > m_wmresscale);
                    }
                    else {
                        #if PYTHIA_VERSION_INTEGER >= 8310
                            loggerPtr->ERROR_MSG("Error in PowhegHooksBB4L::doVetoFSREmission: unimplemented case");
                        #else
                            infoPtr->errorMsg("Error in PowhegHooksBB4L::doVetoFSREmission: unimplemented case");
                        #endif
                        exit(-1);
                    }
                }
                // FSR VETO THE PRODUCTION PROCESS, i.e. OUTSIDE RESONANCE (if it is switched on)
                else if(!inResonance && m_vetoProduction(settingsPtr)){
                        return PowhegHooksSetterMethod::doVetoFSREmission(sizeOld, e, iSys, inResonance);
                }
                // OTHERWISE DON'T VETO
                else {
                    return false;
                }
            }

            inline bool doVetoFSR(bool condition)  {
                if (radtype_.radtype==2) return false;
                if (condition) {
                    m_nInResonanceFSRveto++;
                    return true;
                }
                return false;
            }

            //--- SCALE RESONANCE HOOK -------------------------------------------------
            // called before each resonance decay shower
            inline virtual bool canSetResonanceScale() override { return m_scaleResonanceVeto(settingsPtr); }
            // if the resonance is the (anti)top or W+/W- set the scale to:
            // - if radtype=2 (remnant): resonance virtuality 
            // - if radtype=1 (btilde): 
            //    - (a)topresscale/wp(m)resscale for tops and Ws
            //    - a large number otherwise
            // if is not the top, set it to a big number
            inline virtual double scaleResonance(int iRes, const Event &e) override {
                if(radtype_.radtype == 2)
                    return sqrt(e[iRes].m2Calc());
                else {
                    if (e[iRes].id() == 6)
                        return m_topresscale;
                    else if (e[iRes].id() == -6)
                        return m_atopresscale;
                    else if (e[iRes].id() == 24)
                        return m_wpresscale;
                    else if (e[iRes].id() == -24)
                        return m_wmresscale;
                    else
                        return 1e30;
                }
            }

            //--- Internal helper functions --------------------------------------------
            // Calculates the scale of the hardest emission from within the resonance system
            // translated by Markus Seidel modified by Tomas Jezo
            inline double findresscale( const int iRes, const Event& event) {

                // return large scale if the resonance position is ill defined
                if (iRes < 0) return 1e30;

                // get number of resonance decay products
                int nDau = event[iRes].daughterList().size();

                // iRes is not decayed, return high scale equivalent to
                // unrestricted shower
                if (nDau == 0) {
                    return 1e30;
                }
                // iRes did not radiate, this means that POWHEG pt scale has
                // evolved all the way down to pTmin
                else if (nDau < 3) {
                    return m_pTmin(settingsPtr);
                }            
                // iRes is a (anti-)top quark
                else if (std::abs(event[iRes].id()) == 6) {
                    // find top daughters
                    int idw = -1, idb = -1, idg = -1;
                    for (int i = 0; i < nDau; i++) {
                        int iDau = event[iRes].daughterList()[i];
                        if (std::abs(event[iDau].id()) == 24) idw = iDau;
                        if (std::abs(event[iDau].id()) ==  5) idb = iDau;
                        if (std::abs(event[iDau].id()) == 21) idg = iDau;
                    }

                    // Get daughter 4-vectors in resonance frame
                    Vec4 pw(event[idw].p());
                    pw.bstback(event[iRes].p());
                    Vec4 pb(event[idb].p());
                    pb.bstback(event[iRes].p());
                    Vec4 pg(event[idg].p());
                    pg.bstback(event[iRes].p());

                    // Calculate scale and return it
                    return sqrt(2*pg*pb*pg.e()/pb.e());
                }
                // iRes is a W+(-) boson
                else if (std::abs(event[iRes].id()) == 24) {
                    // Find W daughters
                    int idq = -1, ida = -1, idg = -1;
                    for (int i = 0; i < nDau; i++) {
                      int iDau = event[iRes].daughterList()[i];
                      if      (event[iDau].id() == 21) idg = iDau;
                      else if (event[iDau].id()  >  0) idq = iDau;
                      else if (event[iDau].id()  <  0) ida = iDau;
                    }
                    if (idq<0 or ida <0){
                      throw std::out_of_range("idq or ida out of range in PowhegHooksBB4Ldlsl.cxx");
                    }
                    // Get daughter 4-vectors in resonance frame
                    Vec4 pq(event[idq].p());
                    pq.bstback(event[iRes].p());
                    Vec4 pa(event[ida].p());
                    pa.bstback(event[iRes].p());
                    Vec4 pg(event[idg].p());
                    pg.bstback(event[iRes].p());
                    
                    // Calculate scale
                    Vec4 pw = pq + pa + pg;
                    double q2 = pw*pw;
                    double csi = 2*pg.e()/sqrt(q2);
                    double yq = 1 - pg*pq/(pg.e()*pq.e());
                    double ya = 1 - pg*pa/(pg.e()*pa.e());
                    // and return it 
                    return sqrt(min(1-yq,1-ya)*pow2(csi)*q2/2);
                }
                // in any other case just return a high scale equivalent to
                // unrestricted shower
                return 1e30;
            }

            // The following routine will match daughters of particle `e[iparticle]` to an expected pattern specified via the list of expected particle PDG ID's `ids`,
            // id wildcard is specified as 0 if match is obtained, the positions and the momenta of these particles are returned in vectors `positions` and `momenta`
            // respectively
            // if exitOnExtraLegs==true, it will exit if the decay has more particles than expected, but not less
            inline bool match_decay(int iparticle, const Event &e, const vector<int> &ids, vector<int> &positions, vector<Vec4> &momenta, bool exitOnExtraLegs = true){
                // compare sizes
                if (e[iparticle].daughterList().size() != ids.size()) {
                    if (exitOnExtraLegs && e[iparticle].daughterList().size() > ids.size()) {
                        cout << "extra leg" << endl;
                        exit(-1);
                    }
                    return false; 
                }
                // compare content
                for (long unsigned int i = 0; i < e[iparticle].daughterList().size(); i++) {
                    int di = e[iparticle].daughterList()[i];
                    if (ids[i] != 0 && e[di].id() != ids[i]) 
                        return false;
                }
                // reset the positions and momenta vectors (because they may be reused)
                positions.clear();
                momenta.clear();
                // construct the array of momenta
                for (long unsigned int i = 0; i < e[iparticle].daughterList().size(); i++) {
                    int di = e[iparticle].daughterList()[i];
                    positions.push_back(di);
                    momenta.push_back(e[di].p());
                }
                return true;
            }

            inline double qSplittingScale(Vec4 pt, Vec4 p1, Vec4 p2){
                p1.bstback(pt);
                p2.bstback(pt);
                return sqrt( 2*p1*p2*p2.e()/p1.e() );
            }

            inline double gSplittingScale(Vec4 pt, Vec4 p1, Vec4 p2){
                p1.bstback(pt);
                p2.bstback(pt);     
                return sqrt( 2*p1*p2*p1.e()*p2.e()/(pow(p1.e()+p2.e(),2)) );
            }

            // Routines to calculate the pT (according to pTdefMode) in a FS splitting:
            // i (radiator before) -> j (emitted after) k (radiator after)
            // For the Pythia pT definition, a recoiler (after) must be specified.
            // (INSPIRED BY pythia8F77_31.cc double pTpythia)
            inline double pTpythia(const Event &e, int RadAfterBranch, int EmtAfterBranch,
                        int RecAfterBranch)
            {

                // Convenient shorthands for later
                Vec4 radVec = e[RadAfterBranch].p();
                Vec4 emtVec = e[EmtAfterBranch].p();
                Vec4 recVec = e[RecAfterBranch].p();
                int  radID  = e[RadAfterBranch].id();

                // Calculate virtuality of splitting
                Vec4 Q(radVec + emtVec);
                double Qsq = Q.m2Calc();

                // Mass term of radiator
                double m2Rad = (std::abs(radID) >= 4 && std::abs(radID) < 7) ?
                pow2(particleDataPtr->m0(radID)) : 0.;

                // z values for FSR 
                double z, pTnow;
                // Construct 2 -> 3 variables
                Vec4 sum = radVec + recVec + emtVec;
                double m2Dip = sum.m2Calc();

                double x1 = 2. * (sum * radVec) / m2Dip;
                double x3 = 2. * (sum * emtVec) / m2Dip;
                z     = x1 / (x1 + x3);
                pTnow = z * (1. - z);


                // Virtuality
                pTnow *= (Qsq - m2Rad);

                if (pTnow < 0.) {
                    cout << "Warning: pTpythia was negative" << endl;
                    return -1.;
                }
                else
                        return(sqrt(pTnow));
            }

            // Functions to return statistics about the veto
            inline int getNInResonanceFSRVeto() { return m_nInResonanceFSRveto; }


              //--------------------------------------------------------------------------

            // Extraction of pThard based on the incoming event.
            // Assume that all the final-state particles are in a continuous block
            // at the end of the event and the final entry is the POWHEG emission.
            // If there is no POWHEG emission, then pThard is set to SCALUP.

            inline bool canVetoMPIStep() override { return true; }
            inline int  numberVetoMPIStep() override { return 1; }
            inline bool doVetoMPIStep(int nMPI, const Event &e) override {
                // let the original PowhegHook intialise all necessary variables
                // and set shower starting scale for pThard=0 mode
                // consequence: always need to set nFinal = -1 in job options!
                PowhegHooksSetterMethod::doVetoMPIStep(nMPI,e);
                 // only reset pThard value if using pThardMode =1 or = 2 
                if (m_pThardMode(settingsPtr) == 1 || m_pThardMode(settingsPtr) == 2){
                    // Extra check on nMPI
                    if (nMPI > 1) return false;
                    int count = 0;
                    //check if there is a top and an antitop in the event
                    bool top_exists = false;
                    bool antitop_exists = false;
                    for (int i = e.size() - 1; i > 0; i--) {
                    if (e[i].isFinal()) {
                        count++;
                        //check if there is a top and an antitop in the event
                        if (e[i].id() == 6) top_exists = true;
                        if (e[i].id() == -6) antitop_exists = true;
                    } else break;
                    }

                    //adjust nFinal depending on the number of tops 
                    int nCurrentFinal = -1;
                    // if top and antitop exist, then pp -> t tbar (+ pwhg emissions) -> WbWb (+X) 
                    if (top_exists && antitop_exists) nCurrentFinal = 2;
                    // if only top or antitop exist, then pp -> t W b (+ pwhg emissions) -> WbWb (+X)
                    else if (top_exists || antitop_exists) nCurrentFinal = 3;
                    else{
                        std::cout << "Error: neither top nor anti-top found - something went wrong" << std::endl;
                        exit(1);
                    }
                    // other cases not considered currently
                    if (count != nCurrentFinal && count != nCurrentFinal + 1) {
                        cout << "Error: wrong number of final state particles in event: count " << count << " nCurrentFinal: "<< nCurrentFinal << endl;
                        for (int i = e.size() - 1; i > 0; i--) {
                            if (e[i].isFinal()) { std:: cout << "FS particle " << i << " PID: " << e[i].id() << std::endl;
                            }
                        }
                        exit(1);
                    }
                    // Flag if POWHEG radiation present and index
                    bool isEmt = (count == nCurrentFinal) ? false : true;
                    int iEmt  = (isEmt) ? e.size() - 1 : -1;

                    double pThard = 0;
                    if (m_pThardMode(settingsPtr) == 1) {
                        pThard = PowhegHooksSetterMethod::pTcalc(e, -1, iEmt, -1, -1, -1);
                    // If pThardMode is 2, then the pT of all final-state partons is checked
                    // against all other incoming and outgoing partons, with the minimal value
                    // taken.
                    } else if (m_pThardMode(settingsPtr) == 2) {
                        pThard = PowhegHooksSetterMethod::pTcalc(e, -1, -1, -1, -1, -1);
                    } else {
                        std::cout << "Error: m_pThardMode neither 1 or 2 - something went wrong" << std::endl;
                        exit(1);
                    }

                    // set pThard value in base class UserHook
                    PowhegHooksSetterMethod::setpThard(pThard);
                }

                // Do not veto the event
                return false;
            }

            //--------------------------------------------------------------------------

        private:
            Pythia8_UserHooks::UserSetting<bool> m_debug;
            Pythia8_UserHooks::UserSetting<bool> m_vetoFSREmission, m_vetoQED;
            double m_topresscale,  m_atopresscale, m_wpresscale, m_wmresscale;
            unsigned long int m_nInResonanceFSRveto;
            Pythia8_UserHooks::UserSetting<double> m_pTmin;
            Pythia8_UserHooks::UserSetting<int> m_vetoProduction, m_pTpythiaVeto, m_vetoDipoleFrame;
            Pythia8_UserHooks::UserSetting<double>  m_scaleResonanceVeto;
            Pythia8_UserHooks::UserSetting<int> m_pThardMode;

    };

    //==========================================================================

} // end namespace Pythia8


