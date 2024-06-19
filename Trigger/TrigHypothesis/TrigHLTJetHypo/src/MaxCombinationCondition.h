/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_MAXCOMBINATIONCONDITION_H
#define TRIGHLTJETHYPO_MAXCOMBINATIONCONDITION_H

#include "./ICondition.h"
#include "CombinationsIterator.h"
#include <vector>
#include <memory>

/*
 * The Condition is initialised with an integer value k.
 * isSatisfied() iterates over all n choose k combinations of the n
 * The combination with the max value of a user supplied function
 * is tested with an ICondition object.
 *
 * Taking the first use case as an example:
 * - a hypojet vector of size n is passes to this Condition
 * - combinatorial machinery is used to find the (n choose k) combinations
 * - These combinations are iterated over. std::max_element is used
 *   to find the combination with the highest DIPz defined likelihood. The
 *   comparator used by std::max_element is provided as a template parameter.
 * - The combination with the highest likelihood is passed to the HT Condition
 *   which is the m_accepringCondition attribute of this class.
 *   This Condition passes the event if the HT of the maximising
 *   combination exceeds a threshold value.
 */

template<typename T>
class MaxCombinationCondition: public ICondition {
  
public:
  MaxCombinationCondition(std::size_t k, std::unique_ptr<ICondition> cond, const T& cmp);
  ~MaxCombinationCondition() override = default;

  virtual bool
  isSatisfied(const HypoJetVector&,
	      const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;
        
  virtual unsigned int capacity() const override {return m_k;}

  virtual std::string toString() const override;

private:
  // k as in n choose k. n is given by th number of incoming jets.
  std::size_t m_k;

  // the Condition which decides whether the particular n choose k
  // set  of jets  choosen my the maximising function passes.
  std::unique_ptr<ICondition> m_acceptingCondition;

  T m_cmp;
};

#endif


#include "MaxCombinationCondition.icc"