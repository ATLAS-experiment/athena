/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "TruthUtils/HepMCHelpers.h"


using namespace MC;


const std::array<int,TABLESIZE> MC::triple_charge = {
  +0, -1, +2, -1, +2, -1, +2, -1, +2, +0,
  +0, -3, +0, -3, +0, -3, +0, -3, +0, +0,
  +0, +0, +0, +0, +3, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +3, +0, +0, +3, +6, +0,
  +0, +0, -1, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0
};


//Note: The PDG rules assign the range 51-60 for generic DM, with explicit spins defined for 51-55.
// To keep the rest of this range consistent within ATLAS, 56, 57 and 58 are assigned spins 0, +1/2, +1 (respectively) as seen in arXiv:2504.10597v2
const std::array<int,TABLESIZE> MC::double_spin = {
  +0, +1, +1, +1, +1, +1, +1, +1, +1, +0,
  +0, +1, +1, +1, +1, +1, +1, +1, +1, +0,
  +2, +2, +2, +2, +2, +0, +0, +0, +0, +0,
  +0, +0, +2, +2, +2, +0, +0, +0, +0, +4,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +1, +2, +0, +2, +0, +1, +2, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0,
  +0, +0, +0, +0, +0, +0, +0, +0, +0, +0
};


DecodedPID::DecodedPID(int p)
{
  this->first=p;
  this->second.reserve(10);
  int ap = std::abs(p);
  for(; ap; ap/=10) this->second.push_back( ap%10 );
  std::reverse(this->second.begin(), this->second.end());
}


bool MC::isDiquark(const DecodedPID& p) {
  if ( p.ndigits() == 4 && p(0) >= p(1) && p(1) !=0 && p(2) == 0 && (p.last() == 1 || p.last() == 3)
       && p.max_digit(2,4) < QUARK_LIMIT
       ) return true;
  return false;
}


bool MC::isMeson(const DecodedPID& p) {
  if (p.ndigits() < 3 ) return false;
  if (p.ndigits() == 7 && (p(0) == 1 || p(0) == 2)) return false; // APID don't match SUSY particles
  if (std::abs(p.pid()) == K0S) return true;
  if (std::abs(p.pid()) == K0L) return true;
  if (std::abs(p.pid()) == K0) return true;
  if (p.last() % 2 != 1 ) return false;
  if (p.max_digit(1,3) >= QUARK_LIMIT ) return false; // Ignore pdg_ids which would describe states including fourth generation quarks
  if (p.min_digit(1,3) == 0 ) return false;
  if (*(p.second.rbegin() + 2) < *(p.second.rbegin() + 1) ) return false; // Quark ordering (nq2 >= nq3)
  if (*(p.second.rbegin() + 2) == *(p.second.rbegin() + 1) && p.pid() < 0 ) return false; // Illegal antiparticle check (nq2 == nq3)
  if (p.ndigits() == 3 ) return true;
  if (*(p.second.rbegin() + 3) != 0 ) return false; // Only two quarks! (nq1 == 0)
  if (p.ndigits() == 5 && p(0) == 1 ) return true;
  if (p.ndigits() == 5 && p(0) == 2 && p.last() > 1 ) return true;
  if (p.ndigits() == 5 && p(0) == 3 && p.last() > 1 ) return true;
  if (p.ndigits() == 6 && p.last() % 2 == 1 ) return true;
  if (p.ndigits() == 7 && p(0) == 9 && p(1) == 0 ) return true;

  return false;
}


bool MC::isQuarkonium(const DecodedPID& p) {
  if (!isMeson(p)) return false; //< all quarkonia are mesons
  return (*(p.second.rbegin() + 2) > SQUARK && p.last() > 0 && *(p.second.rbegin() + 1) == *(p.second.rbegin() + 2));
}


bool MC::isBaryon(const DecodedPID& p) {
  if (p.ndigits() < 4 ) return false;
  if (p.max_digit(1,4) >= QUARK_LIMIT ) return false; // Ignore pdg_ids which would describe states including fourth generation quarks
  if (p.min_digit(1,4) == 0) return false; // Ignore pdg_ids with zero for nq1, nq2, nq3
  if (p.ndigits() == 4 && (p.last() == 2 || p.last() == 4|| p.last() == 6|| p.last() == 8) ) return true;

  if (p.ndigits() == 5 && p(0) == 1 &&  (p.last() == 2 || p.last() == 4) ) return true;
  if (p.ndigits() == 5 && p(0) == 3 &&  (p.last() == 2 || p.last() == 4) ) return true;

  if (p.ndigits() == 6 ) {
    if (p(0) == 1 && p(1) == 0 && p.last() == 2 ) return true;
    if (p(0) == 1 && p(1) == 0 && p.last() == 4 ) return true;
    if (p(0) == 1 && p(1) == 0 && p.last() == 6 ) return true;
    if (p(0) == 1 && p(1) == 1 && p.last() == 2 ) return true;
    if (p(0) == 1 && p(1) == 2 && p.last() == 4 ) return true;

    if (p(0) == 2 && p(1) == 0 && p.last() == 2 ) return true;
    if (p(0) == 2 && p(1) == 0 && p.last() == 4 ) return true;
    if (p(0) == 2 && p(1) == 0 && p.last() == 6 ) return true;
    if (p(0) == 2 && p(1) == 0 && p.last() == 8 ) return true;
    if (p(0) == 2 && p(1) == 1 && p.last() == 2 ) return true;
  }

  if (p.ndigits() == 5 ) {
    if (p(0) == 2 && p.last() == 2 ) return true;
    if (p(0) == 2 && p.last() == 4 ) return true;
    if (p(0) == 2 && p.last() == 6 ) return true;
    if (p(0) == 5 && p.last() == 2 ) return true;
    if (p(0) == 1 && p.last() == 6 ) return true;
    if (p(0) == 4 && p.last() == 2 ) return true;
  }
  return false;
}


bool MC::isTetraquark(const DecodedPID& p) {
  return (p.ndigits() == 9 && p(0) == 1 && p(5) == 0 &&
          p.max_digit(1,3) < QUARK_LIMIT && p.min_digit(1,3) > 0 && // ignore 4th generation quarks for nq3 and nq4
          p.max_digit(4,6) < QUARK_LIMIT && p.min_digit(4,6) > 0 && // ignore 4th generation quarks for nq1 and nq2
          ( p(3) >= p(4) && p(6) >= p(7) ) && ( ( p(3) > p(6) ) || ( p(3) == p(6) && (p(4) >= p(7))))
          );
}

bool MC::isPentaquark(const DecodedPID& p) {
  return (p.ndigits() == 9 && p(0) == 1 &&
          p.max_digit(1,6) < QUARK_LIMIT && p.min_digit(1,6) > 0 && // ignore 4th generation (anti-)quarks
          ( p(3) >= p(4) && p(4) >= p(5) && p(5) >= p(6)) );
}


bool MC::isGenSpecific(const int& p) {
  int ap = std::abs(p);
  if (ap >= 81 && ap <= 100) return true;
  if (ap >= 901 && ap <= 930) return true;
  if (ap >= 998 && ap <= 999) return true;
  if (ap >= 1901 && ap <= 1930) return true;
  if (ap >= 2901 && ap <= 2930) return true;
  if (ap >= 3901 && ap <= 3930) return true;
  return false;
}


bool MC::isGlueball(const DecodedPID& p) {
  if (p.ndigits() > 4) return false; // APID avoid classifying R-Glueballs as SM Glueballs
  return
    ( ( p.ndigits() == 3 && p(0) == COMPOSITEGLUON && p(1) == COMPOSITEGLUON && (p.last() == 1 || p.last() == 5) ) ||
      ( p.ndigits() == 4 && p(0) == COMPOSITEGLUON && p(1) == COMPOSITEGLUON && p(2) == COMPOSITEGLUON &&  (p.last() == 3 || p.last() == 7) )  );
}

bool MC::isTechnicolor(const DecodedPID& p) {
  const auto& pp = (p.ndigits() == 7) ? p.shift(2) : DecodedPID(0);
  return (p.ndigits() == 7 && p(0) == 3 && (p(1) == 0 || p(1) == 1) &&
          (isQuark(pp) || isLepton(pp) || isBoson(pp) || isGlueball(pp) ||
           isDiquark(pp) || isHadron(pp)));
}


bool MC::isExcited(const DecodedPID& p) {
  const auto& pp = (p.ndigits() == 7) ? p.shift(2) : DecodedPID(0);
  return (p.ndigits() == 7 && (p(0) == 4 && p(1) == 0) &&
          (isLepton(pp) || isQuark(pp)));
}


bool MC::isRGlueball(const DecodedPID& p) {
  if (p.ndigits() != 7 || p(0) != 1) return false;
  auto pp = p.shift(1);
  return
    ( ( pp.ndigits() == 3 && pp(0) == COMPOSITEGLUON && pp(1) == COMPOSITEGLUON && (pp.last() == 1 || pp.last() == 3) ) ||
      ( pp.ndigits() == 4 && pp(0) == COMPOSITEGLUON && pp(1) == COMPOSITEGLUON && pp(2) == COMPOSITEGLUON && (pp.last() == 1 || pp.last() == 5) )  );
}


bool MC::isRMeson(const DecodedPID& p) {
  if (!(p.ndigits() == 7 && (p(0) == 1 || p(0) == 2))) return false;
  auto pp = p.shift(1);
  return (
          // Handle ~gluino-quark-antiquark states
          (pp.ndigits() == 4 && p(0) == 1 && pp(0) == COMPOSITEGLUON && pp.min_digit(1,3) > 0 && pp.max_digit(1,3) < QUARK_LIMIT && pp(2) <= pp(1) && (pp.last() == 1 || pp.last() == 3)) ||
          // Handle squark-antiquark states (previously called Smeson/mesoninos)
          (pp.ndigits() == 3 && pp.min_digit(1,3) > 0 && pp.max_digit(1,3) < QUARK_LIMIT && pp(1) <= pp(0) && pp.last() == 2)
          );
}


bool MC::hasSquark(const DecodedPID& p, const int& q) {
  auto pp = p.shift(1); return (
                                (isSquark(p) || isRHadron(p))
                                && pp.ndigits() != 2 // skip lepton and boson super-partners by vetoing ndigits==2
                                && pp(0) == q // After shifting, the first digit will always represent the squark in R-Hadron (and squark) PIDs
                                );
}


bool MC::isRBaryon(const DecodedPID& p) {
  if (!(p.ndigits() == 7 && (p(0) == 1 || p(0) == 2))) return false;
  auto pp = p.shift(1);
  return (
          // Handle ~gluino-quark-quark-quark states
          (pp.ndigits() == 5 && p(0) == 1 && pp(0) == COMPOSITEGLUON && pp.min_digit(1,4) > 0 && pp.max_digit(1,4) < QUARK_LIMIT && pp(2) <= pp(1) && pp(3) <= pp(2) && (pp.last() == 2 || pp.last() == 4)) ||
          // Handle squark-quark-quark states (previously called Sbaryons)
          (pp.ndigits() == 4 && pp.min_digit(1,4) > 0 && pp.max_digit(1,4) < QUARK_LIMIT && pp(1) <= pp(0) && pp(2) <= pp(1) && (pp.last() == 1 || pp.last() == 3))
          );
}


bool MC::isDM(const int& p) {
  auto sp = std::abs(p);
  auto value_digits = DecodedPID(p);
  return (sp >= 51 && sp <= 60) || (value_digits.ndigits() == 7 && value_digits(0) == 5 && value_digits(1) == 9) || sp == DARKPHOTON;
}


bool MC::isHiddenValley(const DecodedPID& p) {
  const auto& pp = (p.ndigits() == 7) ? p.shift(2) : DecodedPID(0);
  return (p.ndigits() == 7 && p(0) == 4 && p(1) == 9 &&
          (isQuark(pp) || isLepton(pp) || isBoson(pp) || isGlueball(pp) ||
           isDiquark(pp) || isHadron(pp)));
}


bool MC::isNucleus(const DecodedPID& p){
  if (std::abs(p.pid()) == PROTON) return true;
  if (p.ndigits() != 10) return false;
  // charge should always be less than or equal to baryon number
  // the following line is A >= Z
  const int A = p(8) + 10*p(7) + 100*p(6);
  const int Z = p(5) + 10*p(4) + 100*p(3);
  return ( A >= Z &&  p(0) == 1 &&  p(1) == 0 );
}


bool MC::hasQuark(const DecodedPID& p, const int& q) {
  if (isQuark(p.pid())) { return (std::abs(p.pid()) == q );}
  if (isMeson(p)) { return *(p.second.rbegin() + 1) == q ||*(p.second.rbegin()+2) ==q;}
  if (isDiquark(p)) { auto i = std::find(p.second.rbegin() + 2,p.second.rbegin()+4,q); return (i!=p.second.rbegin()+4);}
  if (isBaryon(p)) { auto i = std::find(p.second.rbegin() + 1,p.second.rbegin()+4,q); return (i!=p.second.rbegin()+4);}
  if (isTetraquark(p)) { auto i = std::find(p.second.rbegin() + 1,p.second.rbegin()+5,q); return (i!=p.second.rbegin()+5);}
  if (isPentaquark(p)) { auto i = std::find(p.second.rbegin() + 1,p.second.rbegin()+6,q); return (i!=p.second.rbegin()+6);}
  if (isNucleus(p) && std::abs(p.pid()) != PROTON) { return (q == 1 || q == 2 || (q==3 && p(2) > 0));}
  if (isSUSY(p)) { // APID SUSY case
    auto pp = p.shift(1);
    if ( pp.ndigits() == 1 ) { return false; } // Handle squarks
    if ( pp.ndigits() == 3 ) { return (pp(1) == q); } // Handle ~q qbar pairs
    if ( pp.ndigits() == 4 ) { return (pp(1) == q || pp(2) == q); } // Ignore gluinos and squarks
    if ( pp.ndigits() == 5 ) {  return (pp(1) == q || pp(2) == q || pp(3) == q); } // Ignore gluinos and squarks
    if ( pp.ndigits() > 5 ) { pp = pp.shift(1); } // Drop gluinos and squarks
    return hasQuark(pp, q); }
  return false;
}


int MC::baryonNumber3(const DecodedPID& p) {
  if (isQuark(p.pid())) { return (p.pid() > 0) ? 1 : - 1;}
  if (isDiquark(p)) { return (p.pid() > 0) ? 2 : -2; }
  if (isMeson(p) || isTetraquark(p)) { return 0; }
  if (isBaryon(p) || isPentaquark(p)){ return (p.pid() > 0) ? 3 : -3; }
  if (isNucleus(p)) {
    const int result = 3*p(8) + 30*p(7) + 300*p(6);
    return (p.pid() > 0) ? result : -result;
  }
  if (isSUSY(p)) {
    auto pp = p.shift(1);
    if (pp.ndigits() < 3 ) { return baryonNumber3(pp); } // super-partners of fundamental particles
    if (pp(0) == COMPOSITEGLUON) {
      if (pp(1) == COMPOSITEGLUON) { return 0; } // R-Glueballs
      if ( pp.ndigits() == 4 ) { return 0; }  // states with gluino-quark-antiquark
      if ( pp.ndigits() == 5) { return (p.pid() > 0) ? 3 : -3; } // states with gluino-quark-quark-quark
    }
    if (pp.ndigits() == 3) { return 0; } // squark-antiquark
    if (pp.ndigits() == 4) { return (p.pid() > 0) ? 3 : -3; } // states with squark-quark-quark
  }
  return 0;
}


int MC::strangeness(const DecodedPID& p) {
  static constexpr std::array<int,10> is_strange = {
    +0, +0, +0, -1, +0, +0, +0, +0, +0, +0
  };

  if (isNucleus(p) && p.ndigits() == 10) { return (p.pid() > 0) ? -p(2) : p(2); }
  if (isStrange(p.pid())) { return (p.pid() > 0) ? -1 : 1; }
  if (!hasStrange(p) && !hasSquark(p,SQUARK)) { return 0; }
  if (std::abs(p.pid()) == K0) { return (p.pid() > 0) ? 1 : -1; }
  size_t nq = 0;
  int sign = 1;
  int signmult = 1;
  int result=0;
  bool classified = false;
  if (!classified && isMeson(p)) { classified = true; nq = 2; if ((*(p.second.rbegin()+2)) == 2||(*(p.second.rbegin()+2)) == 4 ) { sign=-1;} signmult =-1; }
  if (!classified && isDiquark(p)) {return is_strange.at(p(0))+is_strange.at(p(1)); }
  if (!classified && isBaryon(p)) { classified = true; nq = 3; }
  if (!classified && isTetraquark(p)){ return is_strange.at(p(3)) + is_strange.at(p(4)) - is_strange.at(p(6)) - is_strange.at(p(7)); }
  if (!classified && isPentaquark(p)){ return is_strange.at(p(3)) + is_strange.at(p(4)) + is_strange.at(p(5)) + is_strange.at(p(6)) - is_strange.at(p(7)); }
  if (!classified && isSUSY(p)) {
    nq = 0;
    auto pp = p.shift(1);
    if (pp.ndigits() < 3 ) { return strangeness(pp); } // super-partners of fundamental particles
    if (pp(0) == COMPOSITEGLUON) {
      if (pp(1) == COMPOSITEGLUON) { return 0; } // R-Glueballs
      if ( pp.ndigits() == 4 || pp.ndigits() == 5) {
        pp = pp.shift(1); // Remove gluino
      }
    }
    if (pp.ndigits() == 3) { classified = true; nq = 2; if (p.last()%2==0) {sign = -1;} signmult = -1; } // states with quark-antiquark or squark-antiquark
    if (pp.ndigits() == 4) { classified = true; nq = 3; } // states with quark-quark-quark or squark-quark-quark
  }
  for (auto r = p.second.rbegin() + 1; r != p.second.rbegin() + 1 + nq; ++r) {
    result += is_strange.at(*r)*sign;
    sign*=signmult;
  }
  return p.pid() > 0 ? result : -result;
}


int MC::numberOfLambdas(const DecodedPID& p) {
  if (std::abs(p.pid()) == LAMBDA0) { return  (p.pid() > 0) ? 1 : -1; }
  if (isNucleus(p) && p.ndigits() == 10) { return (p.pid() > 0) ? p(2) : -p(2); }
  return 0;
}


int MC::numberOfProtons(const DecodedPID& p) {
  if (std::abs(p.pid()) == PROTON) { return  (p.pid() > 0) ? 1 : -1; }
  if (isNucleus(p)) {
    const int result = p(5) + 10*p(4) + 100*p(3);
    return (p.pid() > 0) ? result : -result;
  }
  return 0;
}


bool MC::isBSM(const DecodedPID& p) {
  if (p.pid() == GRAVITON || std::abs(p.pid()) == MAVTOP || p.pid() == DARKPHOTON) return true;
  if (std::abs(p.pid()) > 16 && std::abs(p.pid()) < 19) return true;
  if (std::abs(p.pid()) > 31 && std::abs(p.pid()) < 39) return true;
  if (std::abs(p.pid()) > 39 && std::abs(p.pid()) < 81) return true;
  if (std::abs(p.pid()) > 6 && std::abs(p.pid()) < 9) return true;
  if (isSUSY(p)) return true;
  if (isNeutrinoRH(p.pid())) return true;
  if (isGenericMultichargedParticle(p)) return true;
  if (isTechnicolor(p)) return true;
  if (isExcited(p)) return true;
  if (isKK(p)) return true;
  if (isHiddenValley(p)) return true;
  if (isMonopole(p)) return true;
  if (isDM(p.pid())) return true;
  return false;
}


bool MC::isBSM(const int& p) {
  if (p == GRAVITON || std::abs(p) == MAVTOP || p == DARKPHOTON) return true;
  if (std::abs(p) > 16 && std::abs(p) < 19) return true;
  if (std::abs(p) > 31 && std::abs(p) < 38) return true;
  if (std::abs(p) > 39 && std::abs(p) < 81) return true;
  if (std::abs(p) > 6 && std::abs(p) < 9) return true;
  return isBSM(DecodedPID(p));
}


bool MC::isValid(const DecodedPID& p) {
  return p.pid() !=0 && ( isQuark(p) || isLepton(p) || isBoson(p) || isGlueball(p) ||
                         isTrajectory(p.pid()) || isGenSpecific(p.pid()) || isDiquark(p) ||
                         isBSM(p) || isHadron(p) || isNucleus(p) || isGeantino(p.pid()) ||
                         isPythia8Specific(p) );
}


bool MC::isValid(const int& p) {
  if (!p) return false;
  if (std::abs(p) < 42) return true;
  if (isGenSpecific(p)) return true;
  return isValid(DecodedPID(p));
}


int MC::leadingQuark(const DecodedPID& p) {
  if (isQuark(p.pid())) { return std::abs(p.pid());}
  if (isMeson(p)) { return p.max_digit(1,3);}
  if (isDiquark(p)) { return p.max_digit(2,4);}
  if (isBaryon(p)) { return p.max_digit(1,4);}
  if (isTetraquark(p)) { return p.max_digit(1,5);}
  if (isPentaquark(p)) { return p.max_digit(1,6);}
  if (isSUSY(p)) { // APID SUSY case
    auto pp = p.shift(1);
    if ( pp.ndigits() == 1 ) { return 0; } // Handle squarks
    if ( pp.ndigits() == 3 ) { pp = DecodedPID(pp(1)); } // Handle ~q qbar pairs
    if ( pp.ndigits()  > 3 ) { pp = pp.shift(1); } // Drop gluinos and squarks
    return leadingQuark(pp); }
  return 0;
}


bool MC::isCCbarMeson(const DecodedPID& p) {
  return leadingQuark(p) == CQUARK && isMeson(p) &&
    (*(p.second.rbegin()+2)) == CQUARK &&
    (*(p.second.rbegin()+1)) == CQUARK;
}


bool MC::isBBbarMeson(const DecodedPID& p)
{
  return leadingQuark(p) == BQUARK && isMeson(p) &&
    (*(p.second.rbegin()+2)) == BQUARK &&
    (*(p.second.rbegin()+1)) == BQUARK;
}


bool MC::isWeaklyDecayingBHadron(const int& p) {
  const int pid = std::abs(p);
  return ( pid == 511   || // B0
           pid == 521   || // B+
           pid == 531   || // B_s0
           pid == 541   || // B_c+
           pid == 5122  || // Lambda_b0
           pid == 5132  || // Xi_b-
           pid == 5232  || // Xi_b0
           pid == 5112  || // Sigma_b-
           pid == 5212  || // Sigma_b0
           pid == 5222  || // Sigma_b+
           pid == 5332  || // Omega_b-
           pid == 5142  || // Xi_bc0
           pid == 5242  || // Xi_bc+
           pid == 5412  || // Xi'_bc0
           pid == 5422  || // Xi'_bc+
           pid == 5342  || // Omega_bc0
           pid == 5432  || // Omega'_bc0
           pid == 5442  || // Omega_bcc+
           pid == 5512  || // Xi_bb-
           pid == 5522  || // Xi_bb0
           pid == 5532  || // Omega_bb-
           pid == 5542  ); // Omega_bbc0
}


bool MC::isWeaklyDecayingCHadron(const int& p) {
  const int pid = std::abs(p);
  return ( pid == 411   || // D+
           pid == 421   || // D0
           pid == 431   || // Ds+
           pid == 4122  || // Lambda_c+
           pid == 4132  || // Xi_c0
           pid == 4232  || // Xi_c+
           pid == 4212  || // Xi_c0
           pid == 4332  || // Omega_c0
           pid == 4412  || // Xi_cc+
           pid == 4422  || // Xi_cc++
           pid == 4432  ); // Omega_cc+
}


int MC::charge3(const DecodedPID& p) {
  auto ap = std::abs(p.pid());
  if (ap < TABLESIZE ) return p.pid() > 0 ? triple_charge.at(ap) : -triple_charge.at(ap);
  if (ap == K0) return 0;
  if (ap == GEANTINO0) return 0;
  if (ap == GEANTINOPLUS) return p.pid() > 0 ? 3 : -3;
  if (ap == MAVTOP) return p.pid() > 0 ? 2 : -2;
  size_t nq = 0;
  int sign = 1;
  int signmult = 1;
  int result=0;
  bool classified = false;
  if (!classified && isMeson(p)) { classified = true; nq = 2; if ((*(p.second.rbegin()+2)) == 2||(*(p.second.rbegin()+2)) == 4 ) { sign=-1;} signmult =-1; }
  if (!classified && isDiquark(p)) {return triple_charge.at(p(0))+triple_charge.at(p(1)); }
  if (!classified && isBaryon(p)) { classified = true; nq = 3; }
  if (!classified && isTetraquark(p)){ return triple_charge.at(p(3)) + triple_charge.at(p(4)) - triple_charge.at(p(6)) - triple_charge.at(p(7)); }
  if (!classified && isPentaquark(p)){ return triple_charge.at(p(3)) + triple_charge.at(p(4)) + triple_charge.at(p(5)) + triple_charge.at(p(6)) - triple_charge.at(p(7)); }
  if (!classified && isNucleus(p)) { return 3*numberOfProtons(p);}
  if (!classified && isSUSY(p)) {
    nq = 0;
    auto pp = p.shift(1);
    if (pp.ndigits() < 3 ) { return charge3(pp); } // super-partners of fundamental particles
    if (pp(0) == COMPOSITEGLUON) {
      if (pp(1) == COMPOSITEGLUON) { return 0; } // R-Glueballs
      if ( pp.ndigits() == 4 || pp.ndigits() == 5) {
        pp = pp.shift(1); // Remove gluino
      }
    }
    if (pp.ndigits() == 3) { classified = true; nq = 2; if (p.last()%2==0) {sign = -1;} signmult = -1; } // states with squark-antiquark or quark-anti-quark
    if (pp.ndigits() == 4) { classified = true; nq = 3; } // states with squark-quark-quark or quark-quark-quark
  }
  if (!classified && isHiddenValley(p)) { // Hidden Valley particles
    auto pp = p.shift(2);
    if (!classified && isMeson(pp)) { classified = true; nq = 2; if ((*(pp.second.rbegin()+2)) == 2||(*(pp.second.rbegin()+2)) == 4 ) { sign=-1;} signmult =-1; }
    if (!classified && isDiquark(pp)) {return triple_charge.at(pp(0))+triple_charge.at(pp(1)); }
    if (!classified && isBaryon(pp)) { classified = true; nq = 3; }

  }
  if (!classified && isExcited(p)) { //Excited/composite leptons/quarks
    auto pp = p.shift(2);
    auto ap = std::abs(pp.pid());
    if (ap < TABLESIZE ) return pp.pid() > 0 ? triple_charge.at(ap) : -triple_charge.at(ap);
  }
  if (!classified && isKK(p)) { // Kaluza-Klein particles
    auto pp = p.shift(2);
    auto ap = std::abs(pp.pid());
    if (ap < TABLESIZE ) return pp.pid() > 0 ? triple_charge.at(ap) : -triple_charge.at(ap);

  }
  if (!classified && isDM(p.pid())) { //Dark Matter Particles
    if (p.ndigits() == 7){ // Determining the charges for the more elaborate, 7-digit DM codes
      auto pp = p.shift(3); // The first two digits indicate the particle is DM, the third indicates left/right-handedness (see 11(j))
      auto ap = std::abs(pp.pid());
      if (ap < TABLESIZE ) return pp.pid() > 0 ? triple_charge.at(ap) : -triple_charge.at(ap);
    }else if (std::abs(p.pid()) < TABLESIZE) return p.pid() > 0 ? triple_charge.at(ap) : -triple_charge.at(ap);  // Just to make sure the correct charge is returned for DM 51-60
  }
  if (!classified && isMonopole(p)) {
    ///Codes 411nq1nq2 nq3 0  are then used when the magnetic and electrical charge sign agree and 412nq1nq2 nq3 0
    /// when they disagree, with the overall sign of the particle set by the magnetic charge.
    result = 3*(p(3)*100 + p(4)*10 + p(5));
    return ( (p.pid() > 0 && p(2) == 1) ||  (p.pid() < 0 && p(2) == 2) ) ? result : -result;
  }
  if (!classified && isGenericMultichargedParticle(p)) {
    double abs_charge = 0.0;
    if (p(0) == 1) abs_charge = p(3)*100. + p(4)*10. + p(5)*1 + p(6)*0.1; // multi-charged particle PDG ID is +/-100XXXY0, where the charge is XXX.Y
    if (p(0) == 2) abs_charge = (p(3)*10. + p(4))/(p(5)*10.0 + p(6)); // multi-charged particle PDG ID is +/-200XXYY0, where the charge is XX/YY
    int abs_threecharge = static_cast<int>(std::round(abs_charge * 3.)); // the multi-charged particles might have a fractional charge that's not a multiple of 1/3, in that case round to the closest multiple of 1/3 for charge3 and threecharge
    return p.pid() > 0 ? abs_threecharge : -1 * abs_threecharge;
  }
  for (auto r = p.second.rbegin() + 1; r != p.second.rbegin() + 1 + nq; ++r) {
    result += triple_charge.at(*r)*sign;
    sign*=signmult;
  }
  return p.pid() > 0 ? result : -result;
}


int MC::charge3(const int& p) {
  int ap = std::abs(p);
  if (ap < TABLESIZE) return p > 0 ? triple_charge.at(ap):-triple_charge.at(ap);
  return charge3(DecodedPID(p));
}


double MC::fractionalCharge(const DecodedPID& p) {
  if(!isGenericMultichargedParticle(p)) return 1.0*charge3(p)/3.0; // this method is written for multi-charged particles, still make sure other cases are handled properly
  double abs_charge = 0;
  if (p(0) == 1) abs_charge = p(3)*100. + p(4)*10. + p(5)*1 + p(6)*0.1; // multi-charged particle PDG ID is +/-100XXXY0, where the charge is XXX.Y
  if (p(0) == 2) abs_charge = (p(3)*10. + p(4))/(p(5)*10.0 + p(6)); // multi-charged particle PDG ID is +/-200XXYY0, where the charge is XX/YY
  return p.pid() > 0 ? abs_charge : -1 * abs_charge;
}


bool MC::isEMInteracting(const int& p) {
  return isPhoton(p) || isZ(p) || p == ZPRIME || p == ZDBLPRIME ||
    std::abs(charge(p))>std::numeric_limits<double>::epsilon() || isMonopole(p);
}


int MC::spin2(const DecodedPID& p) {
  if (isSUSY(p)) {
    auto pp = p.shift(1);
    auto ap = std::abs(pp.pid());
    if (ap < TABLESIZE ) { return std::abs(double_spin.at(ap)-1); } // sparticles (0->1, 1 -> 0,  2->1,  4->3)
    return p.last()-1; // R-Hadrons (p.last() == 2J +1)
  }
  if (isHiddenValley(p)) { //Hidden Valley spins
    auto pp = p.shift(2);
    if (isHadron(pp)) { return pp.last()-1; } // Hadrons (p.last == 2J+1 - special cases handled above)
  }
  if (isKK(p)) { // Kaluza-Klein spins
    auto pp = p.shift(2);
    auto ap = std::abs(pp.pid());
    if (ap < TABLESIZE ) { return double_spin.at(ap); } // fundamental particles
  }
  if (isDM(std::abs(p.pid()))) { //DM spins
    if (p.ndigits() == 7) { // Determining the spins for the more elaborate, 7-digit DM codes
      auto pp = p.shift(3); // The first two digits indicate the particle is DM, the third indicates left/right-handedness (see 11(j))
      auto ap = std::abs(pp.pid());
      if (ap < TABLESIZE) { return double_spin.at(ap); } // fundamental particles
    }else if (std::abs(p.pid()) < TABLESIZE) { // Just to make sure the correct spin is returned for DM 51-60
      return std::abs(double_spin.at(std::abs(p.pid())));
    }
  }
  auto ap = std::abs(p.pid());
  if (ap == K0S) { return 0; }
  if (ap == K0L) { return 0; }
  if (ap == MAVTOP) { return 1; } // TODO check this
  if (ap == DARKPHOTON) { return 2; } // TODO check this
  if (ap < TABLESIZE ) { return double_spin.at(ap); } // fundamental particles
  if (isHadron(p)) { return p.last()-1; } // Hadrons (p.last == 2J+1 - special cases handled above)
  if (isMonopole(p)) { return 0; } // PDG 11i - For now no spin information is provided. Also matches the definition in the G4Extensions/Monopole package.
  if (isGenericMultichargedParticle(p)) { return 0; } // APID Matches the definition in the G4Extensions/Monopole package.
  if (isNucleus(p)) { return 1; }  // TODO need to explicitly deal with nuclei
  return p.last() > 0 ? 1 : 0; //  Anything else - best guess
}


std::vector<int> MC::containedQuarks(const int& p) {
  auto pp = DecodedPID(p);
  std::vector<int> quarks;
  if (isQuark(pp.pid())) { quarks.push_back(std::abs(pp.pid())); }
  else if (isDiquark(pp)) { quarks.push_back(pp(0)); quarks.push_back(pp(1)); }
  else if (isMeson(pp)) { quarks.push_back(*(pp.second.rbegin() + 1)); quarks.push_back(*(pp.second.rbegin()+2)); }
  else if (isBaryon(pp)) { for (size_t digit = 1; digit < 4; ++digit) { quarks.push_back(*(pp.second.rbegin() + digit)); } }
  else if (isTetraquark(pp)) { for (size_t digit = 1; digit < 5; ++digit) { quarks.push_back(*(pp.second.rbegin() + digit)); } }
  else if (isPentaquark(pp)) { for (size_t digit = 1; digit < 6; ++digit) { quarks.push_back(*(pp.second.rbegin() + digit)); } }
  else if (isNucleus(pp)) { const int A = std::abs(baryonNumber3(pp)/3); const int Z = std::abs(numberOfProtons(pp)); const int L = std::abs(numberOfLambdas(pp));
    const int n_uquarks = A + Z; const int n_dquarks = 2*A - Z - L; const int n_squarks = L;
    quarks.reserve(3*A); quarks.insert(quarks.end(), n_dquarks, 1); quarks.insert(quarks.end(), n_uquarks, 2); quarks.insert(quarks.end(), n_squarks, 3); }
  else if (isSUSY(pp)) { // APID SUSY case
    pp = pp.shift(1);
    if ( pp.ndigits() > 1 ) { // skip squarks
      if ( pp.ndigits() == 3 ) { pp = DecodedPID(pp(1)); } // Handle ~q qbar pairs
      if ( pp.ndigits()  > 3 ) { pp = pp.shift(1); } // Drop gluinos and squarks
      return containedQuarks(pp.pid());
    }
  }
  return quarks;
}


bool MC::isStrongInteracting(const int& p) {
  // APID: Glueballs and R-Hadrons are also strong-interacting
  return isGluon(p) || isQuark(p) || isDiquark(p) || isGlueball(p) ||
    isLeptoQuark(p) || isHadron(p) || isRHadron(p);
}
