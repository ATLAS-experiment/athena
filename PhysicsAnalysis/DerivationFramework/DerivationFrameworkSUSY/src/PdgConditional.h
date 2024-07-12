/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DerivationFramework_PdgConditional_H
#define DerivationFramework_PdgConditional_H

#include <variant>
#include <functional>


/**
 * @brief Class holding an int or some function of an int for use in categorisation of
 * pdg codes.
 */
namespace DerivationFramework {
class PdgConditional{
public:
    //Default c'tor just for testing, *may change*
    PdgConditional();
    //Constructor taking integer pdg code for comparison with a value in operator ==
    PdgConditional(int pdgIntCode);
    //Constructor taking unsigned int for comparison with abs(value) in operator ==
    PdgConditional(unsigned equalInt);
    //Constructor taking a fully custom binary predicate for use in operator ==
    PdgConditional(std::function<bool(int)> b);
    //Uses either v, abs(v) or f(v) for comparison with a held value
    bool operator==(int v) const;
  private:
    //variant member determines behaviour: raw compare, abs compare or use the provided function
    const std::variant<int, unsigned, std::function<bool(int)>> m_condition;
    //determines whether to use std::abs(v) before comparing with held value
    const bool m_abs;
  };

}
#endif