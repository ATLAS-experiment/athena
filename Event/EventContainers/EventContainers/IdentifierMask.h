/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EVENTCONTAINERS_IDENTIFIERMASK_H
#define EVENTCONTAINERS_IDENTIFIERMASK_H

#include <vector>
#include <cstdint>
#include <bit>
#include <cassert>

namespace EventContainers {

/**
 * @brief A class for a dynamic bit mask specialised for fast, ordered iteration using the forEachSetBit method
 */
class IdentifierMask {
public:
    /// Bits per storage unit (64)
    static constexpr size_t BitsPerWord = 64;
    /// Mask for the bit index (63)
    static constexpr size_t Mask = BitsPerWord - 1;
    /// Mask shift value for this word size
    static constexpr size_t MaskShift = std::countr_one(Mask);

    explicit IdentifierMask(size_t maxHash) 
    //Since each uint64_t in the vector can hold 64 bits,
    //we need to divide the total number of required bits (maxHash) by 64
    //to find the number of elements needed in the vector
        : m_bits((maxHash + Mask) / BitsPerWord, 0), m_maxHash(maxHash) {}

    /// Mark a hash as present
    void set(size_t hash)  {
        assert (hash < m_maxHash); //change to a contract in c++26
        // hash >> 6: Efficiently divides by 64 to find the vector index
        // hash & Mask: Efficiently computes hash % 64 to find the bit position within the word
        m_bits[hash >> MaskShift] |= (1ULL << (hash & Mask));
    }

    /// Check if a hash is present
    bool test(size_t hash) const  {
        assert (hash < m_maxHash); //change to a contract in c++26
        return (m_bits[hash >> MaskShift] >> (hash & Mask)) & 1ULL;
    }
    
    /// Remove a hash from the mask (Set bit to 0)
    void unset(size_t hash)  {
        assert (hash < m_maxHash); //change to a contract in c++26
        m_bits[hash >> MaskShift] &= ~(1ULL << (hash & Mask));
    }

    /// Reset all bits to 0
    void clear()  {
        std::fill(m_bits.begin(), m_bits.end(), 0);
    }

    /// Execute a lambda for every set bit
    /// The lambda should accept a size_t index
    template<typename Func>
    void forEachSetBit(Func&& f) const {
        for (size_t block_idx = 0; block_idx < m_bits.size(); ++block_idx) {
            uint64_t word = m_bits[block_idx];
            
            // Skip empty blocks of 64 bits entirely
            while (word != 0) {
                // Returns the number of trailing zeros, which is the index of the first '1' bit
                // countr_zero uses the TZCNT or BSF instruction (1 clock cycle)
                int bit_idx = std::countr_zero(word);

                // (block_idx << 6): Multiply block index by 64 to get the base bit position
                // Then add the bit_idx within that block to get the absolute hash value
                // Execute user logic
                f((block_idx << MaskShift) + bit_idx);
                
                // Brian Kernighan’s Algorithm: Clears the least significant bit set to 1.
                // This allows the 'while' loop to jump directly to the next set bit.
                // Suggested by Google Gemini and verified by maskTest.cxx
                word &= (word - 1);
            }
        }
    }

    /// Return the total size of the mask
    size_t size() const  { return m_maxHash; }
    
    /// Return a count of the bits set to 1
    size_t count() const  {
        size_t total = 0;
        for (uint64_t word : m_bits) total += std::popcount(word);
        return total;
    }

private:
    std::vector<uint64_t> m_bits;
    size_t m_maxHash;
};

} // namespace EventContainers
#endif
