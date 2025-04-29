/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#ifndef ACTSUTILS_STAT_H
#define ACTSUTILS_STAT_H 1
#include <cmath>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

namespace ActsUtils {
/// @brief Simple class to gather statistics : min, max, mean, rms
class Stat {
public:
   /// @bruef Gather a new value
   /// will update min, max and the sums to compute mean and rms
   void add(double val) {
      ++m_n;
      m_sum += val;
      m_sum2 += val*val;
      m_min=std::min(m_min,val);
      m_max=std::max(m_max,val);
   }
   unsigned int n() const { return m_n; }
   double min() const { return m_min; }
   double max() const { return m_max; }
   double mean() const { return m_n>0 ? m_sum/m_n : 0.; }
   double rms2() const { return m_n>1 ? (m_sum2 - m_sum *m_sum/m_n)/(m_n-1) : 0.; }
   double rms() const { return std::sqrt( rms2() ); }

   /// Set statistics to zero
   void reset() {
      m_n=0;
      m_sum=0.;
      m_sum2=0.;
      m_min=std::numeric_limits<double>::max();
      m_max=-std::numeric_limits<double>::max();
   }

   /// @breif Add the statistics gathered in the Stat object b
   Stat &operator +=(const Stat &b) {
      m_n += b.m_n;
      m_sum += b.m_sum;
      m_sum2 += b.m_sum2;
      m_min = std::min(m_min, b.m_min);
      m_max = std::max(m_max, b.m_max);
      return *this;
   }

   unsigned int m_n=0;
   double m_sum=0.;
   double m_sum2=0.;
   double m_min=std::numeric_limits<double>::max();
   double m_max=-std::numeric_limits<double>::max();
};

/// @brief Dump the given statistics object to the given output stream
template <class T_Stream>
inline void dumpStat(T_Stream &out, const Stat &stat) {
   if (stat.n() > 1) {
   out << std::setw(14) << stat.min() << " < "
       << std::setw(14) << stat.mean() << " +- " << std::setw(14) <<  stat.rms() << " < "
       << std::setw(14) << stat.max()
       << " / " << std::setw(9) << stat.n();
   }
   else {
      out << std::setw(14*4+9+3*3+4) << stat.mean();
   }
}

inline std::ostream &operator<<(std::ostream &out, const Stat &stat) {
   dumpStat(out, stat);
   return out;
}

/// @brief Extend Stat helper by an equidistant binned histogram
class StatHist : public Stat {
public:
   /// @brief The default constructor will disable histogramming
   StatHist() = default;

   /// @brief Set up class to also fill a histogram
   /// @param n_bins number of bins without over and underflow
   /// @param the value at the lower edge of the first bin
   /// @param the value at the upper edge of the last bin
   StatHist(unsigned int n_bins, float xmin, float xmax)
   {
      setBinning(n_bins,xmin,xmax);
   }

   /// Define histogramm bins and enable histogramming.
   /// @param n_bins number of bins without over and underflow
   /// @param the value at the lower edge of the first bin
   /// @param the value at the upper edge of the last bin
   void setBinning(unsigned int n_bins, float xmin, float xmax)
   {
      m_xmin=xmin;
      m_scale = ( n_bins / (xmax-xmin) );
      m_xmin -= 1./m_scale;
      m_histogram.resize(n_bins+2,0u);
   }

   //// @brief Create a clone but reset the histogram and all the statistics counter
   StatHist createEmptyClone() {
      StatHist tmp;
      tmp.m_histogram.resize( m_histogram.size());
      tmp.m_xmin = m_xmin;
      tmp.m_scale = m_scale;
      return tmp;
   }

   /// @brief Gather statistics and fill the histogram if not disabled.
   void add(double val) {
      Stat::add(val);
      if (!m_histogram.empty()) {
         unsigned int bin = std::min( static_cast<unsigned int>(m_histogram.size()-1),
                                      static_cast<unsigned int>( std::max(0.,(val - m_xmin)*m_scale)) );
         ++m_histogram.at(bin) ;
      }
   }

   /// Set histogram contents and statistics to zero
   void reset() {
      Stat::reset();
      for (unsigned int &bin : m_histogram) {
         bin = 0u;
      }
   }

   /// @brief Add the statistucs and histogrammed data fro the given object.
   StatHist &operator +=(const StatHist &b) {
      Stat::operator+=(b);
      if (m_histogram.size() == b.m_histogram.size()) {
         for (unsigned int bin_i=0; bin_i< m_histogram.size(); ++bin_i) {
            m_histogram[bin_i] += b.m_histogram[bin_i];
         }
      }
      return *this;
   }

   /// @brief Get the lower edge of the given bin
   /// @param i the bin (0: underflow; n+1 overflow)
   double lowerEdge(unsigned int i) const {
      return m_xmin + i/m_scale;
   }

   /// @brief Create a string showing the contents of the histogram
   /// The string
   std::string histogramToString() const {
      std::stringstream msg;
      if (m_histogram.size()>2) {
         const unsigned int max_val = *std::max_element(m_histogram.begin(), m_histogram.end());
         const double bin_width = 1. / m_scale;
         const unsigned int w = max_val > 0 ? static_cast<unsigned int>(log(1.*max_val) / log(10.))+1 : 1;
         const unsigned int wtitle = std::max(10u, w);
         msg << (m_xmin+bin_width) << " .. " << ((m_histogram.size()-2)/m_scale + m_xmin+bin_width) << " : "
             << std::setw(wtitle) << "lower edge"  << " |";
         for (unsigned int i=1; i<m_histogram.size()-1; ++i) {
            msg << " " << std::setw(w) << lowerEdge(i);
         }
         msg << " | " << std::endl;
         msg << (m_xmin+bin_width) << " .. " << ((m_histogram.size()-2)/m_scale + m_xmin+bin_width) << " : "

            << std::setw(wtitle) << m_histogram[0] << " |";
         for (unsigned int i=1; i<m_histogram.size()-1; ++i) {
            msg << " " << std::setw(w) << m_histogram[i];
         }
         msg << " | " << std::setw(w) << m_histogram.back();
      }
      return msg.str();
   }

   double m_xmin{};
   double m_scale{1.0};
   std::vector<unsigned int> m_histogram;
};
}
#endif
