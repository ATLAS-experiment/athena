/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigCompositeUtils_ChainNameParser_h
#define TrigCompositeUtils_ChainNameParser_h

#include <string>
#include <vector>
#include <set>
#include <iterator>
#include "xAODBase/ObjectType.h"

namespace ChainNameParser {
    /// Struct containing information on each leg of a chain
    struct LegInfo
    {
        /// The name of the leg
        std::string legName() const;
        /// The type of xAOD IParticle produced by this signature if relevant
        xAODType::ObjectType type() const;
        /// The multiplicity of the leg (number of objects returned by the leg)
        std::size_t multiplicity{};
        /// The HLT signature responsible for creating the object
        std::string signature{};
        /// The threshold on the object
        int threshold{-1};
        /// All the parts of the leg
        std::vector<std::string> legParts;
    };

    /**
     * @brief Iterate over the legs of a chain
     */
    class LegInfoIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = LegInfo;
        using reference = const value_type &;
        using pointer = const value_type *;
        using difference_type = std::ptrdiff_t;

        /// Default constructor creates a past-the-end iterator
        LegInfoIterator() = default;

        /**
         * @brief Create an iterator from the beginning of a chain name
         * @param chain The full chain name
         */
        LegInfoIterator(const std::string &chain);

        /// Check (in)equality against another iterator
        bool operator==(const LegInfoIterator &other) const;
        bool operator!=(const LegInfoIterator &other) const;

        /**
         * @brief Dereference the iterator
         *
         * Dereferencing a past-the-end iterator returns an invalid LegInfo object
         */
        reference operator *() const;
        pointer operator->() const;

        /// pre-increment operator
        LegInfoIterator &operator++();
        /// post-increment operator
        LegInfoIterator operator++(int);

        /// Whether the iterator is exhausted
        bool exhausted() const;
    private:
        std::string::const_iterator m_itr;
        std::string::const_iterator m_end;
        LegInfo m_current;
        std::size_t m_peekMultiplicity{0};
        std::string m_peekSignature{""};
        int m_peekThreshold{-1};
        bool advance();
    };

    /// Helper class that provides access to information about individual legs
    class HLTChainInfo {
    public:
        using const_iterator = LegInfoIterator;

        HLTChainInfo(const std::string &chain) : m_chain(chain) {}

        /// The chain
        const std::string &chain() const { return m_chain; }

        /// Iterator to the start of the leg infos
        const_iterator begin() const { return LegInfoIterator(m_chain); }
        /// Iterator to the end of the leg infos
        const_iterator end() const { return LegInfoIterator(); }
        /// Read the L1 item from the chain. Returns the empty string if the L1 item isn't in the chain name
        std::string l1Item() const;

    private:
        std::string m_chain;
    }; //> end class HLTChainInfo

    /// A list of all signature names
    const std::vector<std::string> &allSignatures();

    /// A list of all post-fix qualifiers which may come immediately after the "XSigY" pattern, e.g. the noL1 in "2mu10noL1"
    const std::vector<std::string> &allSignaturePostfixQualifiers();

    /// A set of identifiers which are actually single leg, even if followed by other identifiers
    const std::set<std::string> &singleLegIdentifiers();

    /*
     * @brief The regex pattern to match the part at the start of each leg
     *
     * The pattern has three capture groups, the first contains the multiplicity which is either
     * a digit or empty (implying a multiplicity of 1). The second is the signature and the third
     * is the threshold. The threshold will usually be an integer but can also be empty.
     */
    std::string legHeadPattern();

   /*
    * returns multiplicities of the chain legs given the chain name.
    * e.g. for: HLT_2g10_loose_mu20 it would be [ 2 1 ]
    */
    std::vector<int> multiplicities(const std::string& chain);
    /*
    * returns signatures of the chain given the name
    * e.g. for: HLT_2g10_loose_mu20 it would be [ "g" "mu" ]
    * It matches the the multiplicities returned by the above method
    */
    std::vector<std::string> signatures(const std::string& chain);
    
}

#endif // 
