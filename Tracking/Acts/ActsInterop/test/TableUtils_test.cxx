#ifdef NDEBUG
#  undef NDEBUG
#endif
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <algorithm>
#include "ActsInterop/TableUtils.h"

enum EStat : std::size_t {
   k1,
   k2,
   k3,
   kN
};

std::vector<std::string> makeLabels(const std::string &prefix, std::size_t n) {
   std::vector<std::string> labels;
   labels.reserve(n);
   for(std::size_t i=0; i<n; ++i) {
      std::stringstream tmp;
      tmp << prefix << "  " << i;
      labels.push_back(tmp.str());
   }
   return labels;
}

void dumpRef(std::stringstream &out) {
   while (out) {
      std::string aline;
      std::getline(out, aline);
      std::cout << "\"" << aline << "\\n\"" << std::endl;
   }
   out.seekg(0);
   out.clear();
}

bool compare_strings(const std::string &val, const std::string &ref, bool verbose) {
   if (val != ref) {
      std::cout << "size val vs ref: " << val.size() << " =?= " << ref.size() << std::endl;
      std::size_t last_i = std::min(val.size(),ref.size());
      for (std::size_t i=0; i<last_i; ++i) {
         if (ref[i] != val[i]) {
            std::cout << "diff " << std::setw(4) << i << " "
                      << "#" << static_cast<unsigned int>(val[i])
                      << " != #" << static_cast<unsigned int>(ref[i])
                      << " | \""
                      << val[i]
                      << "\" != \"" << ref[i] << "\""
                      << std::endl;
            if (!verbose) break;
         }
      }
      if (verbose) {
         for (std::size_t i=last_i; i<val.size(); ++i) {
            std::cout << "extra in val " << std::setw(4) << i << " #"
                      << static_cast<unsigned int>(val[i])
                      << " | \"" << val[i] << "\""
                      << std::endl;
         }
         for (std::size_t i=last_i; i<ref.size(); ++i) {
            std::cout << "extra in ref " << std::setw(4) << i << " #"
                      << static_cast<unsigned int>(ref[i])
                      << " | \"" << ref[i] << "\""
                      << std::endl;
         }
      }
      return false;
   }
   return true;
}


int main(int argc, char **argv) {
   bool verbose=(argc>1 && atoi(argv[1])>0);
   bool all=(argc>1 && atoi(argv[1])==2);

   // create test data
   // 2 categories with 4 sub-categories each, each containing 3 counters
   // content are powers of 2
   std::vector< std::array<unsigned int, kN> >  in_stat;
   std::size_t n_categories = 2;
   std::size_t n_sub_categories = 4;
   in_stat.resize(n_categories * n_sub_categories, std::array<unsigned int, kN>{} );
   for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
      for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
         for (std::size_t stat_i=0; stat_i<kN; ++stat_i) {
            in_stat.at(cat_i * n_sub_categories + sub_cat_i).at(stat_i)
               = (1<<stat_i)
               * (1<<(cat_i*n_sub_categories*kN))
               * (1<<(sub_cat_i*kN));
         }
      }
   }
   std::swap(in_stat.at( 0 * n_sub_categories + 3).at(1),  in_stat.at(1 * n_sub_categories + 3).at(0) );
   for (std::size_t sub_i=3; sub_i-->0; ) {
      std::swap(in_stat.at( 1 * n_sub_categories + sub_i).at(0),  in_stat.at((1) * n_sub_categories + (sub_i+1)).at(0) );
   }
   std::swap(in_stat.at( 0 * n_sub_categories + 3).at(2),  in_stat.at(1 * n_sub_categories + 3).at(1) );
   for (std::size_t sub_i=3; sub_i-->0; ) {
      std::swap(in_stat.at( 1 * n_sub_categories + sub_i).at(1),  in_stat.at((1) * n_sub_categories + (sub_i+1)).at(1) );
   }
   std::swap(in_stat.at( 0 * n_sub_categories + 2).at(0),  in_stat.at(0 * n_sub_categories + 1).at(2) );
   std::swap(in_stat.at( 0 * n_sub_categories + 0).at(0),  in_stat.at(0 * n_sub_categories + 0).at(1) );


   // dump test data
   if (verbose)  {
      std::cout << "test data : " << std::endl;
      for (std::size_t stat_i=0; stat_i<kN; ++stat_i) {
         std::vector<unsigned int> sum_sub;
         sum_sub.resize( n_categories , 0u);
         for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
            std::cout << std::setw(3) << stat_i << std::setw(3) << sub_cat_i << " :";
            unsigned int sum=0u;
            for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
               std::cout << " " << std::setw(12) << in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(stat_i);
               sum_sub.at(cat_i) += in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(stat_i);
               sum += in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(stat_i);
            }
            std::cout << " | " << std::setw(12) << sum << std::endl;
         }
         std::cout << std::setw(3) << stat_i << std::setw(3) << "" << " :";
         unsigned int sum=0u;
         for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
            std::cout << " " << std::setw(12) << sum_sub.at(cat_i);
            sum += sum_sub.at(cat_i);
         }
         std::cout << " | " << std::setw(12) << sum << std::endl;
         std::cout << std::endl;
      }
      std::cout << "ratios : " << std::endl;
      std::cout << " (aij[2] - aij[1] ) / aij[0] " << std::endl;
      for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
         std::cout << std::setw(3) << "" << std::setw(3) << sub_cat_i << " :";
         for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
            std::cout << " " << std::setw(12) << ( (  in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(2)
                                                    - 1.*in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(1))
                                                    / in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(0) );
         }
         std::cout << std::endl;
      }
      std::cout << " (aij[2]) / aij[1] " << std::endl;
      for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
         std::cout << std::setw(3) << "" << std::setw(3) << sub_cat_i << " :";
         for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
            std::cout << " " << std::setw(12) << ( (  1.*in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(2))
                                                    / in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(1) );
         }
         std::cout << std::endl;
      }
      std::cout << " (aij[1]) / aij[0] " << std::endl;
      for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
         std::cout << std::setw(3) << "" << std::setw(3) << sub_cat_i << " :";
         for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
            std::cout << " " << std::setw(12) << ( (  1.*in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(1))
                                                    / in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(0) );
         }
         std::cout << std::endl;
      }
      std::cout << " (aij[2]) / aij[0] " << std::endl;
      for (std::size_t sub_cat_i=0; sub_cat_i<n_sub_categories; ++sub_cat_i) {
         std::cout << std::setw(3) << "" << std::setw(3) << sub_cat_i << " :";
         for (std::size_t cat_i=0; cat_i<n_categories; ++cat_i) {
            std::cout << " " << std::setw(12) << ( (  1.*in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(2))
                                                    / in_stat.at(cat_i*n_sub_categories+sub_cat_i).at(0) );
         }
         std::cout << std::endl;
      }
   }

   // create labels for the counter
   std::vector<std::string> stat_labels = TableUtils::makeLabelVector(kN,{
         std::make_pair(k1, "Bit 0"),
         std::make_pair(k2, "Bit 1"),
         std::make_pair(k3, "Bit 2")
      });
   // create labels for the categories and sub-categories and add an extra for the projects in
   // category and sub-category directions, respectively
   std::vector<std::string> category_labels = makeLabels("cat ", n_categories);
   category_labels.push_back("cat sum");
   std::vector<std::string> sub_category_labels = makeLabels("sub ", n_sub_categories);
   sub_category_labels.push_back("sub sum");

   // create projections in category and sub-category directions, respectively
   std::vector<std::size_t> stat = TableUtils::createCounterArrayWithProjections<std::size_t>(n_categories,
                                                                                              n_sub_categories,
                                                                                              in_stat);

   // get the strides to iterate over the counter, and sub categories
   std::size_t stat_stride=TableUtils::counterStride(n_categories,
                                                      n_sub_categories,
                                                      static_cast<std::size_t>(kN));
   std::size_t sub_cat_stride=TableUtils::subCategoryStride(n_categories,
                                                             n_sub_categories,
                                                             static_cast<std::size_t>(kN));

   // get the maximum lable width of stat and sub_category
   std::size_t max_label_width = TableUtils::maxLabelWidth(stat_labels) + TableUtils::maxLabelWidth(sub_category_labels) +1;

   // -- dump tables showing the counter split by category and sub-category and the total in category and sub-category
   // -- direction
   std::stringstream table_out;
   table_out << std::setprecision(5);
   try {
     for (std::size_t stat_i=0; stat_i<kN; ++stat_i) {
       std::size_t dest_idx_offset = stat_i * stat_stride;
       table_out << makeTable(stat, dest_idx_offset, sub_cat_stride,
                              sub_category_labels,
                              category_labels)
         .columnWidth(10)
         // only dump the footer for the last eta bin i.e. total
         .dumpHeader(stat_i==0)
         .dumpFooter(stat_i+1 == kN)
         .separateLastRow(true) // separate the sum of all eta bins
         .minLabelWidth(max_label_width)
         .labelPrefix(stat_labels.at(stat_i)+" ");
     }
   }
   catch (const std::exception& e) {
     std::cerr << e.what() << "\n";
     return 1;
   }

   // expected output
   std::string ref =
      "|---------------|------------|------------|------------|\n"
      "|               | cat   0    | cat   1    | cat sum    |\n"
      "|---------------|------------|------------|------------|\n"
      "| Bit 0 sub   0 |          2 |       1024 |       1026 |\n"
      "|       sub   1 |          8 |       4096 |       4104 |\n"
      "|       sub   2 |         32 |      32768 |      32800 |\n"
      "|       sub   3 |        512 |     262144 |     262656 |\n"
      "|---------------|------------|------------|------------|\n"
      "|       sub sum |        554 |     300032 |     300586 |\n"
      "|---------------|------------|------------|------------|\n"
      "| Bit 1 sub   0 |          1 |       2048 |       2049 |\n"
      "|       sub   1 |         16 |       8192 |       8208 |\n"
      "|       sub   2 |        128 |      65536 |      65664 |\n"
      "|       sub   3 |    2097152 |     524288 |    2621440 |\n"
      "|---------------|------------|------------|------------|\n"
      "|       sub sum |    2097297 |     600064 |    2697361 |\n"
      "|---------------|------------|------------|------------|\n"
      "| Bit 2 sub   0 |          4 |      16384 |      16388 |\n"
      "|       sub   1 |         64 |     131072 |     131136 |\n"
      "|       sub   2 |        256 |    1048576 |    1048832 |\n"
      "|       sub   3 |    4194304 |    8388608 |   12582912 |\n"
      "|---------------|------------|------------|------------|\n"
      "|       sub sum |    4194628 |    9584640 |   13779268 |\n"
      "|---------------|------------|------------|------------|";

   if (verbose) {
      std::cout << "counter detail" << std::endl;
      std::cout << table_out.str() << std::endl;
   }

   if (!compare_strings(table_out.str(), ref, !all & verbose)) {
      dumpRef(table_out);
   }
   if (!all) { assert( table_out.str() == ref ); }

   // --- only dump a table of the totals in sub-category direction
   // compute index of the total
   std::size_t dest_idx_offset = n_sub_categories * sub_cat_stride;
   table_out.str("");
   try {
     table_out << makeTable(stat, dest_idx_offset,stat_stride,
                            stat_labels,
                            category_labels,
                            sub_category_labels.at(n_sub_categories))
                .columnWidth(10)
                // only dump the footer for the last eta bin i.e. total
                .dumpFooter(true);
   }
   catch (const std::exception& e) {
     std::cerr << e.what() << "\n";
     return 1;
   }
   if (verbose) {
      std::cout << "counter summary" << std::endl;
      std::cout << table_out.str() << std::endl;
   }

   ref=
      "|---------|------------|------------|------------|\n"
      "| sub sum | cat   0    | cat   1    | cat sum    |\n"
      "|---------|------------|------------|------------|\n"
      "| Bit 0   |        554 |     300032 |     300586 |\n"
      "| Bit 1   |    2097297 |     600064 |    2697361 |\n"
      "| Bit 2   |    4194628 |    9584640 |   13779268 |\n"
      "|---------|------------|------------|------------|";

   if (!compare_strings(table_out.str(), ref,!all & verbose)) {
      dumpRef(table_out);
   }
   if (!all) { assert( table_out.str() == ref ); }

   // --- ratios
   auto [ratio_labels, ratio_def] = TableUtils::splitRatioDefinitionsAndLabels( {
         TableUtils::makeRatioDefinition("3-2 / 1",
                                         std::vector< TableUtils::SummandDefinition > {
                                            TableUtils::defineSummand(k3,      1),
                                            TableUtils::defineSummand(k2,      -1),
                                         },   // failed seeds i.e. seeds which are not duplicates but did not produce a track
                                         std::vector< TableUtils::SummandDefinition >{ TableUtils::defineSummand(k1,1) }),
         TableUtils::defineSimpleRatio("3/2",          k3, k2),
         TableUtils::defineSimpleRatio("1/2",          k2, k1),
         TableUtils::defineSimpleRatio("3/1",          k3,k1)
      });

   std::vector<float> ratio = TableUtils::computeRatios(ratio_def,
                                                        n_categories+1,     //have to add 1 for the projections
                                                        n_sub_categories+1,
                                                        stat);

   // the extra columns and rows for the projections are _not_ added internally
   std::size_t ratio_stride=TableUtils::ratioStride(n_categories+1,
                                                     n_sub_categories+1,
                                                     ratio_def);
   std::size_t ratio_sub_stride=TableUtils::subCategoryStride(n_categories+1,
                                                               n_sub_categories+1,
                                                               ratio_def);
   max_label_width = TableUtils::maxLabelWidth(ratio_labels) + TableUtils::maxLabelWidth(sub_category_labels) + 1;
   // -- dump one table per ratio splitted by category and sub-category, and ratios of the projections in the two
   // -- directions
   table_out.str("");
   try {
     for (std::size_t ratio_i=0; ratio_i<ratio_labels.size(); ++ratio_i) {
       table_out << makeTable(ratio,
                              ratio_i*ratio_stride,
                              ratio_sub_stride,
                              sub_category_labels,
                              category_labels)
         .columnWidth(10)
         // only dump the footer for the last eta bin i.e. total
         .dumpHeader(ratio_i==0)
         .dumpFooter(ratio_i+1==ratio_labels.size())
         .separateLastRow(true) // separate the sum of las
         .minLabelWidth(max_label_width)
         .labelPrefix(ratio_labels.at(ratio_i)+" ");
     }
   }
   catch (const std::exception& e) {
     std::cerr << e.what() << "\n";
     return 1;
   }
   if (verbose) {
      std::cout << "ratios detail" << std::endl;
      std::cout << table_out.str() << std::endl;
   }
   ref=
      "|-----------------|------------|------------|------------|\n"
      "|                 | cat   0    | cat   1    | cat sum    |\n"
      "|-----------------|------------|------------|------------|\n"
      "| 3-2 / 1 sub   0 |        1.5 |         14 |     13.976 |\n"
      "|         sub   1 |          6 |         30 |     29.953 |\n"
      "|         sub   2 |          4 |         30 |     29.975 |\n"
      "|         sub   3 |       4096 |         30 |     37.926 |\n"
      "|-----------------|------------|------------|------------|\n"
      "|         sub sum |     3785.8 |     29.945 |     36.868 |\n"
      "|-----------------|------------|------------|------------|\n"
      "| 3/2     sub   0 |          4 |          8 |      7.998 |\n"
      "|         sub   1 |          4 |         16 |     15.977 |\n"
      "|         sub   2 |          2 |         16 |     15.973 |\n"
      "|         sub   3 |          2 |         16 |        4.8 |\n"
      "|-----------------|------------|------------|------------|\n"
      "|         sub sum |          2 |     15.973 |     5.1084 |\n"
      "|-----------------|------------|------------|------------|\n"
      "| 1/2     sub   0 |        0.5 |          2 |     1.9971 |\n"
      "|         sub   1 |          2 |          2 |          2 |\n"
      "|         sub   2 |          4 |          2 |      2.002 |\n"
      "|         sub   3 |       4096 |          2 |     9.9805 |\n"
      "|-----------------|------------|------------|------------|\n"
      "|         sub sum |     3785.7 |          2 |     8.9737 |\n"
      "|-----------------|------------|------------|------------|\n"
      "| 3/1     sub   0 |          2 |         16 |     15.973 |\n"
      "|         sub   1 |          8 |         32 |     31.953 |\n"
      "|         sub   2 |          8 |         32 |     31.977 |\n"
      "|         sub   3 |       8192 |         32 |     47.906 |\n"
      "|-----------------|------------|------------|------------|\n"
      "|         sub sum |     7571.5 |     31.945 |     45.841 |\n"
      "|-----------------|------------|------------|------------|";

   if (!compare_strings(table_out.str(), ref, !all & verbose)) {
      dumpRef(table_out);
   }
   if (!all) {  assert( table_out.str() == ref ); }

   return 0;
}
