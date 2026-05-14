/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./ICutter.h"
#include <sstream>

namespace GlobalSim {

  class lt: public ICutter {
  public:
    lt(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v < m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: < ";
      return ss.str();
    }
    
  private:
    ulong m_cut;
  };

  class leq: public ICutter {
  public:
    leq(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v <= m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: <= ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };

  class gt: public ICutter {
  public:
    gt(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v > m_cut;  
    };
    
    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: > ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };

  
  class geq: public ICutter {
  public:
    geq(const ulong& c): m_cut(c) {}
    virtual bool cut(const ulong& v) const override {
      return v >= m_cut;  
    };

    virtual std::string to_string() const override {
      std::stringstream ss;
      ss << "cut: " << m_cut << " op: >= ";
      return ss.str();
    }

  private:
    ulong m_cut;
  };

  std::unique_ptr<ICutter> make_cutter(const ulong& cut,
				       const std::string& op) {
    
    auto cutter = std::unique_ptr<ICutter>(nullptr);
    
    if (op == ">"){
      cutter.reset(new gt(cut));
    } else if (op == ">="){
      cutter.reset(new geq(cut));
    } else if (op == "<"){
      cutter.reset(new lt(cut));
    } else if (op == "<="){
      cutter.reset(new leq(cut));
    } else {
      throw std::invalid_argument("unown operator " + op);
    }
    
    return cutter;
  }
  
}
