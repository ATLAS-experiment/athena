// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef FPGATrackSimGenScanArray_H
#define FPGATrackSimGenScanArray_H

/**
 * @file FPGATrackSimGenScanArray.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Iterable Multidimensional Array Class for FPGATrackSimGenScanTool
 *
 * Declarations in this file:
 *    - template <typename T> class FPGATrackSimGenScanArray
 *
 * Description of purpose/function:
 *    - In order to efficiently navigate and iterate over the 5d bins in FPGATrackSimGenScanTool,
 *      a dedicated array class has been developed. Multidimensional arrays in stl don't
 *      appear until c++23 at the earliest and have not been evaluated for appropriateness.
 *    - This array can have an arbitrary number of dimensions set by an std:vector (dims) with the size
 *      of the number of demensions and the size in each dimension set in the constructor or later
 *      by the "setsize" method.
 *    - The implemenatation uses an std::vector to avoid an direct memory management. The requried
 *      size is calcuated from the m_dims vector.
 *    - For accessing elements, it converts a index std::vector<unsigned> of length Ndim (idx) into a single
 *      int index into the 1d std::vector.
 *    - It has a standard iterator so you can iterate with "for (auto &bin : myArray)" where bin
 *      would be an iterator with two accessors bin.idx() gives the size N-dim std:vector<unsigned>
 *      which is N-dim index the iterator is on and bin.data() which is a reference to the
 *      content of the array for that index
 **/


#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

#include "CxxUtils/checker_macros.h"

template <typename T>
class FPGATrackSimGenScanArray
{
public:
    // constructors
    FPGATrackSimGenScanArray() = default;
    FPGATrackSimGenScanArray(const std::vector<unsigned int> &dims, const T &initval)
    {
        setsize(dims, initval);
    }

    // resize the array (need to provide reference to initialize elements to)
    void setsize(const std::vector<unsigned int> &dims, const T &initval)
    {
        m_dims = dims;

        // Caclulate the number of entries and a "step"-size needed for translating
        // N-dim indices into a 1-d index
        m_entries = 1;
        for (auto &dim : dims)
        {
            m_step.push_back(m_entries);
            m_entries *= dim;
        }

        // Allocate 1-d array
        m_data.resize(m_entries, initval);
    }

    // full size of the array (i.e. product of the size in each dimension)
    unsigned int size() const { return m_entries; }

    // access to internal 1-d std::vector as a const so that you can
    // use various std::count and std::for_each on the elements
    // note you can change the vector because it's const, but you can
    // change the elements
    const std::vector<T>& flatdata() const {return m_data;}

    // get the array with the size in each dimension of the
    const std::vector<unsigned int> &dims() const { return m_dims; }

    // look up content by idx specified as a std::vector<unsigned int>
    const T &operator[](const std::vector<unsigned int> &idx) const
    {
        if(idx.size() != m_step.size()) {
            throw std::runtime_error("FPGATrackSimGenScanArray: index size does not match array dimensions");
        }

        // Translate N-dim index to 1-d index
        unsigned int offset = 0;
        for (unsigned int i = 0; i < m_step.size(); i++)
        {
            if (idx.at(i) >= m_dims.at(i))
            {
                throw std::runtime_error("FPGATrackSimGenScanArray: index out of range i=" + std::to_string(i) +
                     " idx[i]=" + std::to_string(idx.at(i)) +
                     " dims[i]=" + std::to_string(m_dims.at(i)) + "\n");
            }
            offset += idx.at(i) * m_step.at(i);
        }

        // Return data from 1-d vector
        return m_data.at(offset);
    }

    // convient to also be able to use a signed integer vector
    const T &operator[](const std::vector<int> &idx) const
    {
        std::vector<unsigned int> idx_unsigned;
        for (auto &d : idx)
            idx_unsigned.push_back(d);
        return (*this)[idx_unsigned];
    }

    // make non-const version of the operator[] 
    // just call the const version and cast away the constness
    // of the return value
    T& operator[](const std::vector<int> &idx) {
      T& retv ATLAS_THREAD_SAFE = const_cast<T&>(std::as_const(*this)[idx]);
      return retv;
    }
    T& operator[](const std::vector<unsigned> &idx) {
      T &retv ATLAS_THREAD_SAFE = const_cast<T &>(std::as_const(*this)[idx]);
      return retv;
    }

    // Iterator implemenation
    struct Iterator
    {
      using value_type = FPGATrackSimGenScanArray<T>::Iterator;
      using iterator_category = std::forward_iterator_tag;
      using container_type = FPGATrackSimGenScanArray<T>;
      using difference_type = void;
      using pointer = FPGATrackSimGenScanArray<T>::Iterator*;
      using reference = FPGATrackSimGenScanArray<T>::Iterator &;

      // Construtctor
      Iterator(const std::vector<unsigned int> &idx,
               FPGATrackSimGenScanArray<T> &itrdata)
          : m_idx(idx), m_itrdata(itrdata) {
        if (m_idx.size() != m_itrdata.dims().size()) {
          throw std::runtime_error(
              "FPGATrackSimGenScanArray::Interator array size mismatch in "
              "constructor");
        }
        }

        Iterator &operator*()  { return *this; }
        Iterator *operator->() { return this; }

        // Prefix increment
        Iterator &operator++()
        {
            ++m_idx.at(0);
            for (unsigned int i = 0; i < m_idx.size() - 1; i++)
            {
                if (m_idx.at(i) >= m_itrdata.dims().at(i))
                {
                    ++m_idx.at(i + 1);
                    m_idx.at(i) = 0;
                }
            }
            return *this;
        }

        const std::vector<unsigned int> &idx() const { return m_idx; }
        T &data() { return m_itrdata[m_idx]; }

        // Postfix increment
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator &a, const Iterator &b) { return a.m_idx == b.m_idx; };
        friend bool operator!=(const Iterator &a, const Iterator &b)
        {
            return a.m_idx != b.m_idx;
        };

    private:
        // current state of the iterator
        std::vector<unsigned int> m_idx;

        // reference back to the array being iterated over
        FPGATrackSimGenScanArray<T> &m_itrdata;
    };

    // ussual std iterator meanings of begin, end, and size
    Iterator begin() { return Iterator(std::vector<unsigned int>(m_dims.size(), 0), *this); }
    Iterator end() 
    {
        std::vector<unsigned int> retv;
        for (auto &d : m_dims)
            retv.push_back(d - 1);
        return ++Iterator(retv, *this);
    }

private:
    unsigned int m_entries{};
    std::vector<unsigned int> m_dims{};
    std::vector<unsigned int> m_step{};
    std::vector<T> m_data{};
};

#endif // FPGATrackSimGenScanArray_H
