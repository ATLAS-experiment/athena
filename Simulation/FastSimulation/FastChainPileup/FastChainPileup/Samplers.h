/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Helper for MultiParticleGunPileup
// Olivier Arnaez, started 17/12/15
// adapted from Generators/ParticleGun/python/samplers.py

#include "TLorentzVector.h"
#include "TRandom.h"
#include "TMath.h"
#include <map>
#include <vector>
#include <iostream>
#include <string>

//Base class for all samplers
class Sampler {
 public:
  Sampler() = default;
  explicit Sampler(float val):m_val(val){}
  virtual ~Sampler()= default;
  virtual float shoot() {return m_val;};
  float m_val{};
  TRandom m_random;
};

//A special-case sampler which just returns one value rather than sampling.
class ConstSampler : public Sampler {
 public:
  ConstSampler() =default;
  explicit ConstSampler(float val): Sampler(val) {};
  ~ConstSampler() = default;
  virtual float shoot() { return m_val;};
};

class MomSampler {
 public:
  MomSampler() = default;
  ~MomSampler() = default;
  virtual TLorentzVector shoot() {return m_val;};
  TLorentzVector m_val;
  ConstSampler * m_mass{};
};

//--------------------------------
//Continuous distribution samplers
//--------------------------------

//Uniformly sample in the range [low,high).
class UniformSampler : public Sampler {
 public:
  UniformSampler()= default;
  UniformSampler(float low, float high) {
    assert(low <= high);
    m_low = float(low);
    m_high = float(high);
  };
  ~UniformSampler() {};
  float shoot() {    return Sampler::m_random.Uniform(m_low, m_high); };
  float m_low{}, m_high{1.0};
};

//Uniformly sample in the modulus range (-high,low]+[low,high).
class ModUniformSampler : public UniformSampler {
 public:
  ~ModUniformSampler(){};
   ModUniformSampler(float low, float high) : UniformSampler (low, high){
    assert(low == fabs(low) && high == fabs(high));
    assert(low <= high);
    m_low = float(low);
    m_high = float(high);
  };

  float shoot() {
    m_val = m_random.Uniform(m_low, m_high);
    if (m_random.Uniform() > 0.5)
      m_val *= -1;
    return m_val;
  };
};

//Uniformly sample from a set of disjoint intervals.
class DisjointUniformSampler : public UniformSampler {
  /*    The ranges variable can either be a list of increasing numbers or a
        list of pairs of numbers.

        The former case will be treated as
        defining alternating on/off ranges for sampling, starting with an active
        one (i.e. it's a list of bin edges). The latter way specifically lists
        the 'on' regions only, with their start and end values in the pairs.

        The behaviour is undefined if the numbers are not ordered or overlap --
        i.e. it might work but hasn't been designed that way and might change in
        future. Don't rely on this behaviour!
  */
 public:    
  ~DisjointUniformSampler(){};
  DisjointUniformSampler(const std::vector<float> & ranges) {
    for (unsigned int i=0; i<ranges.size();) {
      std::pair<float,float> p(ranges[i],ranges[i+1]);
      i+=2;
      m_ranges.push_back(p);
    }
    _setRanges();
  };
  DisjointUniformSampler(const std::vector< std::pair<float,float> > & ranges): m_ranges(ranges){    _setRanges();}

  const std::vector< std::pair<float,float> > & _getRanges() { return m_ranges; };

  void _setRanges() {
    for (unsigned int i=0; i<m_ranges.size(); i++) {
      m_totalwidth += m_ranges[i].second - m_ranges[i].first;
    };
    float runningwidth = 0.0;
    m_divisions.push_back(0.0);
    for (unsigned int i=0; i<m_ranges.size(); i++) {
      assert(m_ranges[i].second >= m_ranges[i].first);
      runningwidth += float(m_ranges[i].second - m_ranges[i].first);
      m_divisions.push_back(runningwidth);
    }
    m_totalwidth = runningwidth;
    for (unsigned int i=0; i<m_ranges.size(); i++) 
      m_divisions[i] = float(m_divisions[i]) / float(m_totalwidth);
  };

  float _map_unit_to_val(float x){
    assert(x >= 0 && x <= 1);
    unsigned int idx = -1, rem = 0;
    for (unsigned int i=0; i<m_divisions.size()-1; i++) 
      if (x >= m_divisions[i] and x < m_divisions[i+1]) {
        idx = i;
        rem = x - m_divisions[i];
        break;
      }
    float val = m_ranges[idx].first + m_totalwidth * rem;
    return val;
  };

  float shoot(){
    float rand = m_random.Uniform();
    float val = _map_unit_to_val(rand);
    return val;
  };
 private:
  std::vector< std::pair<float,float> > m_ranges;
  float m_totalwidth{};
  std::vector<float> m_divisions;
};

//Randomly sample from an exponential distribution (i.e. uniformly on a log scale).
class LogSampler : public UniformSampler {
 public:
  LogSampler(float low, float high): UniformSampler (low, high){
    m_low = low;
    m_high = high;
  };
  ~LogSampler(){};

  float shoot(){
    float rand = m_random.Uniform();
    float logval = rand * TMath::Log(m_high) + (1 - rand) * TMath::Log(m_low);
    float val = TMath::Exp(logval);
    return val;
  };
};

//Randomly sample from a 1D Gaussian distribution.
class GaussianSampler : public UniformSampler {
 public:
  ~GaussianSampler(){};
  GaussianSampler(float mean, float sigma): UniformSampler (0, 1){
    m_mean = mean;
    m_sigma = sigma;
  }

  float shoot() { return m_random.Gaus(m_mean, m_sigma);};
 private:
  float m_mean{},m_sigma{};
};


//Sequentially sample from a list of values, returning to the beginning once exhausted.
class CyclicSeqSampler : public Sampler {
 public:
  ~CyclicSeqSampler() {};
  CyclicSeqSampler(const CyclicSeqSampler & orig) : Sampler(), m_sequence(orig.m_sequence) , m_index(0){};
  CyclicSeqSampler(std::string s) {
    size_t pos = 0;
    std::string token;
    std::cout << "Initializing CyclicSeqSampler..." << std::endl;
    while ((pos = s.find(',')) != std::string::npos) {
      token = s.substr(0, pos);
      m_sequence.push_back(std::stoi(token));
      s.erase(0, pos + 1);
      std::cout << "  adding " << m_sequence[m_sequence.size()-1] << " from " << token.c_str() << std::endl;
    }
    m_index = 0;
  };
  float shoot() {
    m_index = (m_index + 1) % m_sequence.size();
    std::cout << "CyclicSeqSampler returning " << m_sequence[m_index] << std::endl;
    return m_sequence[m_index];
  };
 private:
  std::vector<int> m_sequence;
  int m_index{};
};

// Convenience function for sampler-making from Python literals
/*
Sampler mksampler(x){
  
//    Automatically cast the provided object to a sampler type. This is used
//    extensively inside the particle and position samplers, so that the user
//    can pass in a primitive type like a number or list and it will be
//    treated as if the more verbose sampler constructors had been called.
//
//    Behaviour:
//     - if x can be called, i.e. x() is valid, we just return x;
//     - a Python list (square brackets) will be converted to a continuous
//       UniformSampler or DisjointUniformSampler;
//     - TODO: a Python tuple (round brackets/parentheses) will be treated
//       as a discrete CyclicSeqSampler;
//     - TODO: a Python set (curly brackets/braces) will be treated
//       as a discrete RandomSeqSampler;
//     - otherwise a ConstSampler will be created from x, so that x is
//       returned when the sampler is called.
  
    if hasattr(x, "__call__"):
        return x
    elif type(x) is list:
        # NB: disjoint ranges can be given as nested lists, e.g. [(1,2), (4,5)]
        if len(x) == 2 and type(x[0]) in (int,float) and type(x[1]) in (int,float):
            #print "MKSAMPLER: Casting %s to UniformSampler" % str(x)
            return UniformSampler(*x)
        elif len(x) > 2 or (len(x) > 0 and type(x[0]) not in (int,float)):
            #print "MKSAMPLER: Casting %s to DisjointUniformSampler" % str(x)
            return DisjointUniformSampler(x)
        if len(x) < 2:
            raise Exception("Supplied list could not be converted to a continuous sampler")
    elif type(x) is tuple:
        #print "MKSAMPLER: Casting %s to CyclicSeqSampler" % str(x)
        return CyclicSeqSampler(*x)
    elif type(x) is set:
        #print "MKSAMPLER: Casting %s to RandomSeqSampler" % str(x)
        return RandomSeqSampler(*x)
    else:
        #print "MKSAMPLER: Casting %s to ConstSampler" % str(x)
        return ConstSampler(x)
*/


// Beam-spot (origin vertex) sampling
//Sampler of position 3-vectors, for modelling a beamspot.
class PosSampler {
 public:
  ~PosSampler() = default;
  PosSampler() = default;
  PosSampler(float x, float y, float z, float t=0):
    m_x(ConstSampler(x)), m_y(ConstSampler(y)), m_z(ConstSampler(z)), m_t(ConstSampler(t)){
    //nop
    }

  TLorentzVector shoot(){
    float x = m_x.shoot();
    float y = m_y.shoot();
    float z = m_z.shoot();
    float t = m_t.shoot();
    return TLorentzVector(x, y, z, t);
  }
 private:
  Sampler m_x{0.f}, m_y{0.f}, m_z{0.f}, m_t{0.f};
};

// Momentum sampling
//A momentum sampler which just returns a null vector with the given mass.
class NullMomSampler : public MomSampler{
 public:
  ~NullMomSampler() {};
  explicit NullMomSampler(float mass=0.0) { m_mass = new ConstSampler(mass);};

  TLorentzVector shoot() {
    return TLorentzVector(0, 0, 0, m_mass->shoot());
  }
};



//Create a 4-momentum vector from pt, eta, m and phi distributions/samplers.
class PtEtaMPhiSampler : public MomSampler{
 public:
  ~PtEtaMPhiSampler() { delete m_pt; delete m_eta; delete m_phi;};
  PtEtaMPhiSampler(const PtEtaMPhiSampler & other ) = delete;
  PtEtaMPhiSampler & operator =(const PtEtaMPhiSampler & other) = delete;
  PtEtaMPhiSampler(float ptmin, float ptmax, float etamin, float etamax, float mass=0.0, float phimin=0, float phimax=2.*TMath::Pi()){
    if (ptmin==ptmax)
      m_pt = new ConstSampler(ptmin);
    else
      m_pt = new UniformSampler(ptmin,ptmax);
    if (etamin==etamax)
      m_eta = new ConstSampler(etamin);
    else
      m_eta = new UniformSampler(etamin,etamax);
    m_mass = new ConstSampler(mass);
    if (phimin==phimax)
      m_phi = new ConstSampler(phimin);
    else
      m_phi = new UniformSampler(phimin,phimax);
  };

  TLorentzVector shoot() {
    float        eta = m_eta->shoot();
    float        pt = m_pt->shoot();
    float        phi = m_phi->shoot();
    float        m = m_mass->shoot();
    TLorentzVector tlv; tlv.SetPtEtaPhiM(pt,eta,phi,m);
    return tlv;
  };

 private:
  Sampler * m_pt{}, * m_eta{}, * m_phi{};
};
	  
// Combined samplers returning a particle configuration

// A particle object for use as a return value from the particle samplers
class SampledParticle {
 public:
  ~SampledParticle(){};
  SampledParticle(int pid=0, TLorentzVector mom=TLorentzVector(0,0,0,0), TLorentzVector pos= TLorentzVector(0,0,0,0)) :
   m_pid (pid), m_mom(mom), m_pos(pos), m_mass(0.f){
    //        Constructor/initializer: PID is the (int) PDG particle ID code
    //        of this particle, mom is its momentum 4-vector, and pos is
    //        the vertex 4-position (both as ROOT.TLorentzVector, in MeV).
  }
  int m_pid{};
  TLorentzVector m_mom, m_pos;
  float m_mass{};
};

//    A simple N-independent-particle sampler.
class ParticleSampler {
 public:
  ~ParticleSampler() = default;
  ParticleSampler(Sampler * pid, MomSampler * mom, int n=1):m_mom(mom), m_pos(PosSampler(0,0,0)), m_pid(pid), m_n( ConstSampler(n)),  m_mass_override(true) {
    // A default dictionary of particle masses (in MeV)
    m_massdict[22  ] =     0.0; // photon
    m_massdict[11  ] =     0.5; // electron
    m_massdict[12  ] =     0.0; // nu_e
    m_massdict[13  ] =   105.7; // muon
    m_massdict[14  ] =     0.0; // nu_mu
    m_massdict[15  ] =  1777.8; // tau
    m_massdict[16  ] =     0.0; // nu_tau
    m_massdict[2212] =   938.0; // proton
    m_massdict[2112] =   940.0; // neutron
    m_massdict[111 ] =   135.0; // pi0
    m_massdict[211 ] =   140.0; // pi+-
    m_massdict[221 ] =   547.0; // eta
    m_massdict[321 ] =   494.0; // K+-
    m_massdict[311 ] =   598.0; // K0
  };

  //Return a vector of sampled particles
  std::vector<SampledParticle> shoot() {
    int numparticles = m_n.shoot();
    std::cout << "ParticleSampler throwing " << numparticles << " particles" << std::endl;
    std::vector<SampledParticle> rtn;
    for (int i=0; i<numparticles ; i++){
      //Sample the particle ID and create a particle
      int pid = m_pid->shoot();
      std::cout << "  shot pid=" << pid << std::endl;
      SampledParticle p = SampledParticle(pid);
      // Pass mass info to the v4 sampler and set same generated mass
      if (m_mass_override && m_massdict.find(abs(pid))!=m_massdict.end()){
	float m = m_massdict[abs(pid)];
	m_mom->m_mass = new ConstSampler(m);
	p.m_mass = m;
      }
      // Sample momentum and vertex positions into the particle
      p.m_mom = m_mom->shoot();
      p.m_pos = m_pos.shoot();
      std::cout << "  (" << p.m_mom.Eta() << ", " << p.m_mom.Phi() << ", " << p.m_mom.E() << ", " << p.m_mom.M() << ")" << std::endl;
      // Add particle to output list
      rtn.push_back(p);
    }
    return rtn;
  }
 private:
  MomSampler * m_mom{};
  PosSampler m_pos{};
  Sampler * m_pid{};
  ConstSampler m_n{};
  bool m_mass_override{};
  std::map<unsigned int,float> m_massdict;
};
