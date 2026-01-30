/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TABLE_UTILS_H
#define TABLE_UTILS_H

#include <array>
#include <string>
#include <cassert>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <vector>
#include <utility>
#include <limits>

class MsgStream;

// Utility class to wrap a constant variable length array interface over static sized arrays
// This avoid duplication of compiled code for arrays which only differ
// by the number of elements.
// The interface is sufficient to allow using the wrapped arrays in range based for loops,
// and for random access.
namespace TableUtils {
   template <typename T>
   struct Range {
      const T     *m_ptr  = nullptr;
      std::size_t  m_size = 0u;
      std::size_t  m_offset = 1u;

      struct const_iterator {
         const_iterator &operator++() { m_ptr += m_offset; return *this; }
         const T &operator*() const { return *m_ptr; }
         bool operator!=(const const_iterator &other) const { return m_ptr != other.m_ptr; }
         const T *m_ptr;
         std::size_t m_offset;
      };
      const_iterator begin() const {
         return const_iterator{ m_ptr, m_offset };
      }
      const_iterator end() const {
         assert( (m_size % m_offset) == 0 );
         return const_iterator{ m_ptr+m_size, m_offset };
      }
      const T &operator[](std::size_t index) const { assert(index<m_size && m_ptr); return m_ptr[index]; }

      std::size_t size() const { return m_size; }

      operator bool() const { return m_ptr != nullptr && m_size>0; }
      template <typename T_Other>
      bool equalSize(const T_Other &other_range) {
         return m_size == other_range.m_size;
      }
   };

   // Utility class to wrap a constant two dimensional variable length array interface over static sized arrays
   // This avoid duplication of compiled code for arrays which only differ
   // by the number of elements in the two dimensions.
   // The interface is sufficient to allow using the wrapped arrays in range based for loops,
   // and for random access.
   template <typename T>
   struct Range2D {
      const T     *m_ptr     = nullptr;
      std::size_t  m_rows    = 0u;
      std::size_t  m_columns = 0u;
      std::size_t  m_columnOffset = 0u;
      std::size_t  m_firstColumnIndex = 0u;
      std::size_t  m_offset  = 1u;

      struct const_iterator {
         const_iterator &operator++() { m_ptr += m_columnOffset; return *this; }
         Range<T> operator*() const { return Range<T> { m_ptr, m_columns, m_offset }; }
         bool operator!=(const const_iterator &other) const { return m_ptr != other.m_ptr; }
         const T *m_ptr;
         std::size_t m_columns;
         std::size_t m_columnOffset;
         std::size_t m_firstColumnIndex;
         std::size_t m_offset;
      };

      const_iterator begin() const {
         return const_iterator{ m_ptr, m_columns, m_columnOffset, m_firstColumnIndex, m_offset};
      }
      const_iterator end() const {
         return const_iterator{ m_ptr+m_rows * m_columnOffset, m_columns, m_columnOffset, m_firstColumnIndex, m_offset };
      }
      Range<T> operator[](std::size_t index) const {
         assert(index<m_rows && m_ptr);
         return Range<T>{m_ptr + m_columnOffset * index, m_columns, m_offset};
      }

      std::size_t nColumns() const { return m_columns; }
      std::size_t nRows() const    { return m_rows; }

      operator bool() { return m_ptr != nullptr && m_columns>0; }
      template <typename T_Other>
      bool equalSize(const T_Other &other_range) {
         return m_rows == other_range.m_size;
      }
   };

   // Helper method to print wrapped static arrays in table form to an output stream
   // The table will be composed of one column for row labels and one column for the data
   template <class T_Stream, typename T_Counter>
   T_Stream &dumpTable(T_Stream &out,
                       Range<T_Counter>   counter,
                       const Range<std::string>& label,
                       const std::string &label_prefix,
                       const std::size_t column_width,
                       const std::size_t min_label_width,
                       const bool dump_footer,
                       const bool separate_last_row,
                       const unsigned int precision) {
      if (counter && label && counter.equalSize(label)) {
         std::size_t max_size =min_label_width;
         for (const std::string &name : label ) {
            max_size = std::max(max_size, name.size());
         }
         const std::size_t total_size =max_size+3+2*2+column_width;
         std::string line(total_size, '-');
         std::array<std::size_t,3> vertical_line_pos{0u, max_size+3, line.size()-1};
         for (std::size_t pos : vertical_line_pos) {
            line[pos]='|';
         }
         out << line << std::endl;
         std::size_t idx=0;
         std::string empty;
         auto default_precision = out.precision();
         for (const T_Counter &a : counter) {
            if (separate_last_row && idx+1 == label.size()) {
               out << line << std::endl;
            }
            assert( idx < label.size());
            out << "| " << (label_prefix.empty() ? std::left : std::right)
                << std::setw(label_prefix.size()) << ( idx==0 ? label_prefix : empty)
                << std::setw(max_size-label_prefix.size()) << label[idx] << std::right
                << std::setprecision( precision != std::numeric_limits<unsigned int>::max() ? precision : default_precision)
                << " | " << std::setw(column_width) << a << " |" << std::endl;
            ++idx;
         }
         out << std::setprecision(default_precision);
         if (dump_footer) {
            out << line << std::endl;
         }
      }
      return out;
   }

   // Helper method to print wrapped static two dimensional arrays in table form to an output stream
   // The table will be composed of a header showing column labels, one column for row labels and
   // then column for each data item. The array is expected in row major order.
   template <class T_Stream, typename T_Counter>
   T_Stream &dumpTable(T_Stream &out,
                       Range2D<T_Counter>   counter,
                       const Range<std::string>& row_label,
                       const Range<std::string>& column_label,
                       const std::string &top_left_label,
                       const std::string &label_prefix,
                       const std::size_t column_width,
                       const std::size_t min_label_width,
                       const bool dump_header,
                       const bool dump_footer,
                       const bool separate_last_row,
                       const std::vector<unsigned int> &precision) {
      if (counter && row_label && column_label
          && counter.equalSize(row_label)
          && counter.nColumns() == column_label.size()) {
         std::size_t max_size =std::max(top_left_label.size(), static_cast<std::size_t>(min_label_width));
         for (const std::string &name : row_label ) {
            max_size = std::max(max_size, name.size() + label_prefix.size());
         }
         std::size_t the_width = column_width;
         for (const std::string &name : column_label ) {
            the_width = std::max(the_width, name.size());
         }
         std::size_t total_size =max_size+2*2;
         for (std::size_t column_i=0; column_i<column_label.size(); ++column_i) {
            total_size += the_width + 3;
         }
         std::string line(total_size, '-');
         std::size_t pos=0;
         line[pos]='|';
         pos += max_size+3;
         for (std::size_t column_i=0; column_i<column_label.size(); ++column_i) {
            line[pos]='|';
            pos += the_width + 3;
         }
         line[line.size()-1]='|';
         if (dump_header) {
            out << line << std::endl << "| " << std::setw(max_size) << top_left_label << " |" << std::left;
            for (const std::string &header : column_label ) {
               out << " " << std::setw(the_width) << header << " |";
            }
            out << std::right << std::endl;
         }
         out << line << std::endl;
         std::size_t idx=0;
         std::string empty;
         auto default_precision = out.precision();
         for (const Range<T_Counter> &a_row : counter) {
            if (separate_last_row && idx+1 == row_label.size()) {
               out << line << std::endl;
            }
            assert( idx < row_label.size());
            out << "| " << (label_prefix.empty() ? std::left : std::right)
                << std::setw(label_prefix.size()) << ( idx==0 ? label_prefix : empty)
                << std::setw(max_size-label_prefix.size()) << row_label[idx] << std::right << " |";
            unsigned int col_i=0;
            for (const T_Counter &a : a_row) {
               out << " "
                   << std::setprecision( (col_i < precision.size()
                                          && precision[col_i] != std::numeric_limits<unsigned int>::max())
                                         ? precision[col_i]
                                         : default_precision)
                   << std::setw(the_width) << a << " |";
               ++col_i;
            }
            out << std::endl;
            ++idx;
         }
         out << std::setprecision(default_precision);
         if (dump_footer) {
            out << line;
         }
      }
      return out;
   }

   // Helper struct to wrap data that should be dumped in table from to an output stream
   template <typename T>
   struct StatTable {
      Range<T>           m_counter;
      Range<std::string> m_label;
      StatTable &columnWidth(std::size_t value) { m_columnWidth=value; return *this;}
      StatTable &minLabelWidth(std::size_t value) { m_minLabelWidth=value; return *this;}
      StatTable &dumpHeader(bool value=true) { m_dumpHeader=value; return *this;}
      StatTable &dumpFooter(bool value=true) { m_dumpFooter=value; return *this;}
      StatTable &separateLastRow(bool value=true) { m_separateLastRow=value; return *this;}
      StatTable &labelPrefix(const std::string& value) { m_labelPrefix=value; return *this;}
      StatTable &precision(unsigned int precision) { m_precision=precision; return *this;}

      std::string        m_labelPrefix {};
      std::size_t        m_columnWidth=12;
      std::size_t        m_minLabelWidth=0;
      unsigned int       m_precision = std::numeric_limits<unsigned int>::max();
      bool               m_dumpHeader=true;
      bool               m_dumpFooter=true;
      bool               m_separateLastRow=false;
   };
   // Helper struct to wrap two dimensional data that should be dumped in table from to an output stream
   template <typename T>
   struct MultiColumnTable {
      Range2D<T>         m_counter;
      Range<std::string> m_rowLabel;
      Range<std::string> m_columnLabel;
      std::string        m_topLeftLable;
      MultiColumnTable &columnWidth(std::size_t value) { m_columnWidth=value; return *this;}
      MultiColumnTable &minLabelWidth(std::size_t value) { m_minLabelWidth=value; return *this;}
      MultiColumnTable &dumpHeader(bool value=true) { m_dumpHeader=value; return *this;}
      MultiColumnTable &dumpFooter(bool value=true) { m_dumpFooter=value; return *this;}
      MultiColumnTable &separateLastRow(bool value=true) { m_separateLastRow=value; return *this;}
      MultiColumnTable &labelPrefix(const std::string& value) { m_labelPrefix=value; return *this;}
      MultiColumnTable &precision(std::vector<unsigned int> &&precision) { m_precision=std::move(precision); return *this;}

      std::string        m_labelPrefix {};
      std::size_t        m_columnWidth=12;
      std::size_t        m_minLabelWidth=0;
      std::vector<unsigned int> m_precision{};
      bool               m_dumpHeader=true;
      bool               m_dumpFooter=true;
      bool               m_separateLastRow=false;
   };

   template <typename T_index, class T_string>
   std::vector<std::string> makeLabelVector(T_index n_entries,
                                            std::initializer_list<std::pair<T_index, T_string> > a_list)
   {
      std::vector<std::string> labels;
      labels.resize( n_entries );
      if (a_list.size() != n_entries) {
         throw std::logic_error("Expected number of entries and elements in the initializer lists do not match.");
      }
      for ( auto elm : a_list) {
         labels.at(elm.first) = std::move(elm.second);
      }
      return labels;
   }

   template<class T_Collection>
   std::size_t maxLabelWidth( const T_Collection &col) {
      std::size_t max_width=0u;
      for (const auto &elm : col ) {
         max_width = std::max( max_width, elm.size());
      }
      return max_width;
   }


   constexpr inline std::size_t categoryStride([[maybe_unused]] const std::size_t categories,
                                               [[maybe_unused]] const std::size_t sub_categories,
                                               [[maybe_unused]] const std::size_t n_counter) {
      return 1;
   }
   constexpr inline std::size_t subCategoryStride([[maybe_unused]] const std::size_t categories,
                                                  [[maybe_unused]] const std::size_t sub_categories,
                                                  [[maybe_unused]] const std::size_t n_counter) {
      return (categories+1) * n_counter;
   }
   constexpr inline std::size_t counterStride([[maybe_unused]] const std::size_t categories,
                                              [[maybe_unused]] const std::size_t sub_categories,
                                              [[maybe_unused]] const std::size_t n_counter) {
      return (categories+1);
   }


   // change order of input statistics counter and compute projections
   // - order: change from array[category*category_stride+sub_category][counter]
   //                 to   array[sub_category*sub_category_stride+counter*counter_stride+category]
   // - projections: sum numbers in direction of category and sub_category direction, respectively.
   // - dimension of the resulting vector: (categories+1) * (sub_categories+1) * N
   template< typename T_Output, typename T_Input, const std::size_t N>
   std::vector<T_Output> createCounterArrayWithProjections( const std::size_t categories,
                                                            const std::size_t sub_categories,
                                                            const std::vector< std::array<T_Input, N> > &input_counts) {
      if (categories*sub_categories!= input_counts.size()) {
         std::stringstream msg;
         msg << "Category dimensions (" << categories << " * " << sub_categories << "="
             << (categories * sub_categories) << ") and input counter container size "
             << input_counts.size() << " do not match.";
         throw std::logic_error(msg.str());
      }
      std::vector<T_Output> output_counts;
      output_counts.resize((categories+1) * (sub_categories+1) * N);
      const std::size_t sub_category_stride = subCategoryStride(categories, sub_categories, N);
      const std::size_t counter_stride      = counterStride(categories, sub_categories, N);
      const std::size_t category_stride     = categoryStride(categories, sub_categories, N);
      // project seeds
      for (std::size_t sub_category_i=0;
           sub_category_i < sub_categories;
           ++sub_category_i) {
         for (std::size_t category_i=0; category_i<categories; ++category_i) {
            std::size_t src_idx       = category_i     * sub_categories      + sub_category_i;
            std::size_t dest_idx_base = sub_category_i * sub_category_stride + 0 * counter_stride;
            std::size_t dest_idx_project_categories_base = dest_idx_base;
            dest_idx_base += category_i * category_stride;
            dest_idx_project_categories_base += categories * category_stride;

            for (std::size_t counter_i=0; counter_i<N; ++counter_i) {
               std::size_t dest_idx=dest_idx_base + counter_i * counter_stride;
               assert( src_idx < input_counts.size() && counter_i < input_counts[src_idx].size());
               assert( dest_idx < output_counts.size());
               output_counts[dest_idx] = input_counts[src_idx][counter_i];
               assert( dest_idx_project_categories_base + counter_i * counter_stride < output_counts.size());
               output_counts[dest_idx_project_categories_base + counter_i * counter_stride] += output_counts[dest_idx];
            }
         }
      }
      // project eta bins
      for (std::size_t category_i=0; category_i<=categories; ++category_i) {
         for (std::size_t counter_i=0; counter_i<N; ++counter_i) {
            std::size_t dest_idx_base  = 0 * sub_category_stride + counter_i * counter_stride + category_i;
            std::size_t dest_idx_project_sub_categories = sub_categories * sub_category_stride + dest_idx_base;
            assert( dest_idx_project_sub_categories < output_counts.size() );
            for (std::size_t sub_category_i=0;
                 sub_category_i<sub_categories;
                 ++sub_category_i) {
               std::size_t sub_category_idx = dest_idx_base + sub_category_i * sub_category_stride;
               assert( sub_category_idx < output_counts.size() );
               output_counts[dest_idx_project_sub_categories] += output_counts[sub_category_idx];
            }
         }
      }
      return output_counts;
   }

   using SummandDefinition = std::pair<std::size_t, int>;
   using RatioDefinition = std::pair< std::vector< SummandDefinition >,
                                      std::vector< SummandDefinition > >;

   // helper to create the sum definitions for the ratio of single counters
   // this will lead to the computation of the simple ratio defined by counter[numerator] / counter[denominatpr]
   template <typename T>
   inline RatioDefinition
   defineSimpleRatio(T numerator, T denominator) {
      return std::make_pair( std::vector< SummandDefinition > { std::make_pair(static_cast<std::size_t>(numerator),1)},
                             std::vector< SummandDefinition > { std::make_pair(static_cast<std::size_t>(denominator),1)});
   }

   // helper to create a named ratio definition for a ratio of two single counters
   template <typename T>
   inline std::tuple< std::string, RatioDefinition >
   defineSimpleRatio(std::string &&name, T numerator, T denominator) {
      return std::make_pair( std::move(name), defineSimpleRatio(numerator, denominator) );
   }


   // helper to create a single summand definition to be used in a ratio definition
   // @param counter_idx the index of a counter
   // @param multiplier a multiplier to be applied to the value of the referenced counter e.g. +1 or -1.
   // @return the definition of a single summand to be appended to a vector
   template <typename T>
   inline SummandDefinition defineSummand(T counter_idx, int multiplier) {
      return std::make_pair(static_cast<std::size_t>(counter_idx), multiplier) ;
   }

   // compute the sum : sum_j stat[ eta_bin][stat_j][seed] * multiplier_j
   // where the index stat_j is the first element of the pair, and the second element is multiplier_j
   std::size_t computeSum( const std::vector< SummandDefinition >  &sum_def,
                           std::size_t eta_offset,
                           std::size_t row_stride,
                           std::size_t seed_i,
                           const std::vector<std::size_t> &stat);

   inline float computeRatio(std::size_t numerator, std::size_t denominator) {
      return denominator!=0 ? static_cast<float>(numerator/static_cast<double>(denominator)) : 0.f;
   }

   // compute the ratio of two sums;
   // @param ratio_def vectors which define summands for the numerator and denominator
   // The first element of the pair defines the numberator, which is computed by summing the referenced
   // counters  (first element of the inner pair is the counter index) after multiplying them by the
   // multiplier (second element of  the inner pair). The second elment defines the denominator  of
   // the ratio.
   inline float computeRatio( const RatioDefinition &ratio_def,
                         std::size_t eta_offset,
                         std::size_t row_stride,
                         std::size_t seed_i,
                         const std::vector<std::size_t> &stat) {
      std::size_t numerator=computeSum(ratio_def.first, eta_offset, row_stride, seed_i, stat);
      std::size_t denominator=!ratio_def.second.empty()
         ? computeSum(ratio_def.second, eta_offset, row_stride, seed_i, stat)
         : 1;
      return computeRatio(numerator,denominator);
   }


   // helper function to define a named ratio
   inline std::tuple< std::string, RatioDefinition> makeRatioDefinition(std::string &&name,
                                                                        std::vector< SummandDefinition >&&numerator,
                                                                        std::vector< SummandDefinition >&&denominator) {
      return std::make_tuple(std::move(name),
                             std::make_pair(std::move(numerator),
                                            std::move(denominator)));
   }

   // helper function to split a list of named ratio definitions into a vector of labels and ratio definitions
   inline std::tuple<std::vector<std::string>, std::vector<RatioDefinition> >
   splitRatioDefinitionsAndLabels(std::initializer_list<std::tuple<std::string,  RatioDefinition> > a_ratio_list)
   {
        std::tuple< std::vector<std::string>, std::vector<RatioDefinition> > splitted;
        std::get<0>(splitted).reserve( a_ratio_list.size() );
        for ( auto a_ratio : a_ratio_list) {
            std::get<0>(splitted).emplace_back( std::move(std::get<0>(a_ratio)) );
        }
        std::get<1>(splitted).reserve( a_ratio_list.size() );
        for ( auto a_ratio : a_ratio_list) {
            std::get<1>(splitted).emplace_back( std::move(std::get<1>(a_ratio)) );
        }
        return splitted;
   }


   constexpr inline std::size_t categoryStride([[maybe_unused]] const std::size_t categories,
                                               [[maybe_unused]] const std::size_t sub_categories,
                                               [[maybe_unused]] const std::vector<RatioDefinition> &ratio_def) {
      return 1;
   }
   inline std::size_t subCategoryStride([[maybe_unused]] const std::size_t categories,
                                        [[maybe_unused]] const std::size_t sub_categories,
                                        [[maybe_unused]] const std::vector<RatioDefinition> &ratio_def) {
      return (categories) * ratio_def.size();
   }
   constexpr inline std::size_t ratioStride([[maybe_unused]] const std::size_t categories,
                                            [[maybe_unused]] const std::size_t sub_categories,
                                            [[maybe_unused]] const std::vector<RatioDefinition>  &ratio_def) {
      return (categories);
   }


   std::vector<float> computeRatios(const std::vector<RatioDefinition> &ratio_def,
                                    const std::size_t categories,
                                    const std::size_t sub_categories,
                                    const std::vector< std::size_t> &counter);

   inline std::string makeBinLabel(const std::string &variable_name,
                                   const std::vector<float> &bins,
                                   std::size_t bin_i,
                                   bool abs_value=false,
                                   int precision=1) {
     std::stringstream range_label;
     range_label << std::fixed << std::setprecision(precision);
     if (bin_i==bins.size()+1) {
         range_label << " All " << variable_name;
     }
     else {
         if (bin_i==0) {
            std::stringstream value_str;
            value_str << std::fixed << std::setprecision(precision) << 0.;
            range_label << std::setw(4) <<  (abs_value ? value_str.str().c_str() : "-inf") << "-";
         }
         else {
           range_label << std::setw(4) << bins.at(bin_i-1) <<"-";
         }
         if (bin_i>=bins.size()) {
           range_label << std::setw(4) << "+inf";
         }
         else {
           range_label << std::setw(4) << bins.at(bin_i);
         }
     }
     return range_label.str();
   }

   inline std::string makeEtaBinLabel(const std::vector<float> &eta_bins,
                                   std::size_t eta_bin_i,
                                   bool abs_eta=false) {
      return TableUtils::makeBinLabel("eta",eta_bins, eta_bin_i,abs_eta, 1);
   }
}
// Helper method to wrap data that should be dumped in table form to an output stream
// Usage:   out << makeTable( array, labels);
template <typename T, std::size_t N>
TableUtils::StatTable<T> makeTable(const std::array<T, N>           &counter,
                                   const std::array<std::string, N> &label) {
   return TableUtils::StatTable<T> {
      TableUtils::Range<T>           {counter.data(), counter.size()},
      TableUtils::Range<std::string> {label.data(),   label.size()  }
   };
}

// Helper method to wrap two dimensional data that should be dumped in table form to an output stream
template <typename T, std::size_t Nrows, std::size_t Ncolumns>
TableUtils::MultiColumnTable<T> makeTable(const std::array<std::array<T, Ncolumns>, Nrows> &counter,
                                          const std::array<std::string, Nrows>    &row_label,
                                          const std::array<std::string, Ncolumns> &column_label,
                                          const std::string &top_left_label="") {
   return TableUtils::MultiColumnTable<T> {
      TableUtils::Range2D<T>         {!counter.empty() ? counter[0].data() : nullptr,
                                      counter.size(), column_label.size(),
                                      !counter.empty() ? static_cast<std::size_t>(&counter[1][0] - &counter[0][0]) : 0u,
                                      0u,  // index of first column
                                      1u}, // offset between columns
      TableUtils::Range<std::string> {row_label.data(),    row_label.size(),   1u},
      TableUtils::Range<std::string> {column_label.data(), column_label.size(),1u},
      top_left_label
   };
}

// Helper method to wrap two dimensional data that should be dumped in table form to an output stream
template <typename T>
TableUtils::MultiColumnTable<T> makeTable(const std::vector<T> &counter,
                                          std::size_t start_idx,
                                          std::size_t row_stride,
                                          const std::vector<std::string>    &row_label,
                                          const std::vector<std::string> &column_label,
                                          const std::string &top_left_label="") {
   if (start_idx + (row_label.size()-1) * row_stride >= counter.size() || row_stride < column_label.size()) {
      std::stringstream msg;
      msg << "Counter dimension and label dimensions (" << row_label.size() << " * " << column_label.size()
          << ") do not match: [" << start_idx << ", "
          <<  start_idx << " + " << (row_label.size()-1) << " * " << row_stride << " = "
          << (start_idx + (row_label.size()-1) * row_stride)
          << " !< " << counter.size();
      msg << " [row_labels:";
      for (const std::string &label : row_label) {
         msg << " " << label;
      }
      msg << "; column_labels:";
      for (const std::string &label : column_label) {
         msg << " " << label;
      }
      msg << "]";
      throw std::logic_error(msg.str());
   }
   return TableUtils::MultiColumnTable<T> {
      TableUtils::Range2D<T>         {!counter.empty() ? &counter[start_idx] : nullptr,
                                      row_label.size(),      // n-rows
                                      column_label.size(),   // n-columns
                                      row_stride,            // offset between rows
                                      0u,                    // first column index
                                      1u},                   // offset between columns
      TableUtils::Range<std::string> {row_label.data(),    row_label.size() },
      TableUtils::Range<std::string> {column_label.data(), column_label.size()},
      top_left_label
   };
}

template <typename T, std::size_t N>
TableUtils::MultiColumnTable<T> makeTable(const std::vector<std::array<T,N> > &counter,
                                          std::size_t start_row_idx,
                                          std::size_t row_stride,
                                          std::size_t start_column_idx,
                                          std::size_t column_stride,
                                          const std::vector<std::string>    &row_label,
                                          const std::vector<std::string> &column_label,
                                          const std::string &top_left_label="") {
   if (start_row_idx + (row_label.size()-1) * row_stride >= counter.size()*N
       || start_column_idx + (column_label.size()-1) * column_stride >= counter.size()*N
       || (row_stride*row_label.size()>column_stride && column_stride*column_label.size()>row_stride) ) {
      std::stringstream msg;
      msg << "Counter dimension and label dimensions (" << row_label.size() << " * " << column_label.size()
          << ") do not match: [" << start_row_idx << ", "
          <<  start_row_idx << " + " << (row_label.size()-1) << " * " << row_stride << " = "
          << (start_row_idx + (row_label.size()-1) * row_stride)
          << " or "
          <<  start_column_idx << " + " << (column_label.size()-1) << " * " << column_stride << " = "
          << (start_column_idx + (column_label.size()-1) * column_stride)
          << " !< " << counter.size()
          << std::endl
          << (start_row_idx + (row_label.size()-1) * row_stride) << "  >= " << (counter.size()*N)
          << " || " << (start_column_idx + (column_label.size()-1) * column_stride) << "  >= " << (counter.size()*N)
          << " || ( " << (row_stride*row_label.size()) << " > " << column_stride << " &&  " << (column_stride*column_label.size()) << " > " << (row_stride)
          << ")";
      msg << " [row_labels:";
      for (const std::string &label : row_label) {
         msg << " " << label;
      }
      msg << "; column_labels:";
      for (const std::string &label : column_label) {
         msg << " " << label;
      }
      msg << "]";
      throw std::logic_error(msg.str());
   }
   return TableUtils::MultiColumnTable<T> {
      TableUtils::Range2D<T>         {!counter.empty() && N>0 ? counter[start_row_idx].data() : nullptr,
                                      row_label.size(),      // n-rows
                                      column_label.size(),   // n-columns
                                      row_stride,            // offset between rows
                                      start_column_idx,                    // first column index
                                      column_stride},                   // offset between columns
      TableUtils::Range<std::string> {row_label.data(),    row_label.size() },
      TableUtils::Range<std::string> {column_label.data(), column_label.size()},
      top_left_label};
}


// Helper method to wrap two dimensional data that should be dumped in table form to an output stream
template <typename T>
TableUtils::MultiColumnTable<T> makeTable(const std::vector<T> &counter,
                                          const std::vector<std::string> &row_label,
                                          const std::vector<std::string> &column_label,
                                          const std::string &top_left_label="") {
   return makeTable(counter, 0u, column_label.size(), row_label, column_label, top_left_label);
}


// convenience method to dump wrapped arrays in table form to a MsgStream
// Usage:   msg(MSG::INFO) << makeTable( array, labels);
//          ATH_MSG_INFO( makeTable( array, labels) );
template <typename T>
inline MsgStream &operator<<(MsgStream &out,
                             const TableUtils::StatTable<T> &stat)
{
   return dumpTable(out,
                    stat.m_counter,
                    stat.m_label,
                    stat.m_labelPrefix,
                    stat.m_columnWidth,
                    stat.m_minLabelWidth,
                    stat.m_dumpFooter,
                    stat.m_separateLastRow,
                    stat.m_precision);
}

// convenience method to dump wrapped two dimensional arrays in table form to a MsgStream
// Usage:   msg(MSG::INFO) << makeTable( array2d, row_labels, column_labels);
//          ATH_MSG_INFO( makeTable( array2d, row_labels, column_labels) );
template <typename T>
inline MsgStream &operator<<(MsgStream &out,
                             const TableUtils::MultiColumnTable<T> &stat)
{
   return dumpTable(out,
                    stat.m_counter,
                    stat.m_rowLabel,
                    stat.m_columnLabel,
                    stat.m_topLeftLable,
                    stat.m_labelPrefix,
                    stat.m_columnWidth,
                    stat.m_minLabelWidth,
                    stat.m_dumpHeader,
                    stat.m_dumpFooter,
                    stat.m_separateLastRow,
                    stat.m_precision);
}


// convenience method to dump wrapped two dimensional arrays in table form to a std output stream
// Usage:   out << makeTable( array2d, row_labels, column_labels);
template <typename T>
inline std::ostream &operator<<(std::ostream &out,
                                const TableUtils::StatTable<T> &stat)
{
   return dumpTable(out,
                    stat.m_counter,
                    stat.m_label,
                    stat.m_labelPrefix,
                    stat.m_columnWidth,
                    stat.m_minLabelWidth,
                    stat.m_dumpFooter,
                    stat.m_separateLastRow,
                    stat.m_precision);
}

// convenience method to dump wrapped two dimensional arrays in table form to a std output stream
// Usage:   out << makeTable( array2d, row_labels, column_labels);
template <typename T>
inline std::ostream &operator<<(std::ostream &out,
                                const TableUtils::MultiColumnTable<T> &stat)
{
   return dumpTable(out,
                    stat.m_counter,
                    stat.m_rowLabel,
                    stat.m_columnLabel,
                    stat.m_topLeftLable,
                    stat.m_labelPrefix,
                    stat.m_columnWidth,
                    stat.m_minLabelWidth,
                    stat.m_dumpHeader,
                    stat.m_dumpFooter,
                    stat.m_separateLastRow,
                    stat.m_precision);
}

#endif
