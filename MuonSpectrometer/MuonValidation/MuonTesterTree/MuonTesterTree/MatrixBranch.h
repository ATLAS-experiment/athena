/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MATRIXBRANCH_H
#define MUONTESTER_MATRIXBRANCH_H
#include <MuonTesterTree/VectorBranch.h>

class TTree;

namespace MuonVal {
class MuonTesterTree;
template <class T> class MatrixBranch : public VectorBranch<std::vector<T>> {
public:
    /// Standard constructor
    MatrixBranch(TTree* tree, const std::string& name);
    MatrixBranch(MuonTesterTree& tree, const std::string& name);

    /// Constructor with default value extending the array
    MatrixBranch(TTree* tree, const std::string& b_name, const T& def);
    MatrixBranch(MuonTesterTree& tree, const std::string& b_name, const T& def);


    virtual ~MatrixBranch() = default;



    /// Returns the i-th element of the outer vector
    using VectorBranch<std::vector<T>>::get;
    using VectorBranch<std::vector<T>>::operator[];

    /// Returns the j-th element of the i-th inner vector
    inline T& get(size_t i, size_t j);

    /// Push back the element into the i-th inner vector
    inline void push_back(size_t i, const T& value);

    /// Size of the outer vector
    inline size_t nrows() const;
    /// Size of the i-th inner vector
    inline size_t ncols(size_t row) const;

    /// Get the default value to fill the empty fields in the vector
    inline const T& getDefault() const;
    void setDefault(const T& def);

private:
    T m_default{};
};
}
#include <MuonTesterTree/MatrixBranch.icc>
#endif
