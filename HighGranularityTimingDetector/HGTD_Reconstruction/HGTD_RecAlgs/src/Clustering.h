/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @author Laforge, Bertrand <laforge@lpnhe.in2p3.fr>
 * @author Leopold, Alexander <alexander.leopold@cern.ch>
 *
 * @brief Clusters objects in N dimensions by minimizing the resolution of the
 *        final clusters.
 *
 * TODO:
 * - SimultaneousClustering should be able to deal with unknown variables in a
 *   higher dimensional case.
 */

#ifndef CLUSTERINGHELPER_H
#define CLUSTERINGHELPER_H

#include "TMath.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace HGTD {

template <typename T> class Cluster {

public:
  Cluster();

  Cluster(const std::vector<double> &v, const std::vector<double> &v_sigma);

  Cluster(const std::vector<double> &v, const std::vector<double> &v_sigma,
          const T &entry);

  /**
   * @brief Add an object of type T to the Cluster.
   *
   * @param [in] entry Object that should be added to the Cluster.
   *
   */
  void addEntry(const T &entry);

  /**
   * @brief This can be used to combine the subsets stored in two vertices
   * all in once while merging.
   *
   * @param [in] entry_vector Vector of objects (usually extracted from vertex
   * to be merged with).
   *
   */
  void addEntryVector(const std::vector<T> &entry_vector);

  /**
   * @brief Return the objects that are part of the Cluster.
   */
  const std::vector<T>& getEntries() const;

  /**
   * @brief The value of a vertex has to be set manually. This is because
   * we use a weighted time after vertex fusion, so this has to be accounted
   * for (also gives the option to use different methods).
   *
   * @param [in] v Global position of this Cluster.
   * @param [in] v_sigma Global resolution of the position of this vertex.
   *
   */
  void setClusterValue(const std::vector<double> &v,
                       const std::vector<double> &v_sigma);

  /**
   * @brief Return the N-dimensional value of the Cluster.
   */
  std::vector<double> getValues() const;

  /**
   * @brief  Return the N-dimensional resolution of the Cluster.
   */
  const std::vector<double>& getSigmas() const;

  /**
   * @brief Return the number of objects stored in the Cluster.
   */
  int getNEntries() const;

  /**
   * @brief Return true if the Cluster is the result of a merge.
   **/
  bool mergeStatus() const;

  void setMergeStatus(bool status);

  int getMergeIteration() const;

  void setMergeIteration(int iteration);

  bool containsUnknowns() const;

  void setUnknownStatus(bool status);

private:
  int m_debug_level = 0; // TODO set this dynamically
  std::vector<T> m_entries { }; // Stores raw objects that are part of this Cluster
  double m_combined_value { };
  double m_combined_sigma { };
  std::vector<double> m_combined_value_vector { };
  std::vector<double> m_combined_sigma_vector { };
  bool m_was_merged = false;
  int m_merge_iteration = 0;
  bool m_contains_unknowns = false;
};

/////////////////////////////////////////////

enum class ClusterAlgo { Eager, Simultaneous, SimultaneousMean };

/////////////////////////////////////////////

template <typename T> class ClusterCollection {
public:
  void addCluster(const Cluster<T> & vx);
  void doClustering(ClusterAlgo algo);

private:
  double getDistanceBetweenClusters(const Cluster<T> &a, const Cluster<T> &b);

  /**
   * @brief Creates a new Cluster object that is a fusion of the two clusters
   * given in the arguments.
   *
   * @param [in] a A cluster that should be merged.
   * @param [in] b A cluster that should be merged.
   */
  Cluster<T> mergeClusters(const Cluster<T> &a, const Cluster<T> &b);

  Cluster<T> mergeClustersMean(const Cluster<T> &a, const Cluster<T> &b);

  std::pair<Cluster<T>, int> largestClusterInfo();

public:
  Cluster<T> getMaxEntriesCluster();
  int getMaxClusterSize_Info();
  const std::vector<Cluster<T>>& getClusters() const;
  int getNClusters() const;
  /**
   * @brief Set the distance cut. This allows an update and rerunning of the
   * vertexing then no vertex that fulfills the selection criteria is found.
   *
   * @param [in] cut_value Given in units of resolution.
   *
   */
  void updateDistanceCut(double cut_value);
  void setDebugLevel(int debug_level) { m_debug_level = debug_level; }

private:
  int m_debug_level = 0;
  double m_distance_cut = 3.0;
  std::vector<Cluster<T>> m_clusters;
};

/////////////////////////////////////////////
/////////////////////////////////////////////
////                CPP                   ///
/////////////////////////////////////////////
/////////////////////////////////////////////

template <class T>
Cluster<T>::Cluster()
    : m_combined_value(-999.), m_combined_sigma(-999.) {}

template <typename T>
Cluster<T>::Cluster(
  const std::vector<double> &v, const std::vector<double> &v_sigma)
  : m_combined_value_vector(v), m_combined_sigma_vector(v_sigma) {

  for (const auto &s : v_sigma) {
    if (s < 0) {
      m_contains_unknowns = true;
    }
  }
}

template <typename T>
Cluster<T>::Cluster(
  const std::vector<double> &v, const std::vector<double> &v_sigma,
  const T &entry)
  : m_combined_value_vector(v), m_combined_sigma_vector(v_sigma) {

  m_entries.push_back(entry);

  for (const auto &s : v_sigma) {
    if (s < 0) {
      m_contains_unknowns = true;
    }
  }
}

template <class T> int Cluster<T>::getNEntries() const {
  return m_entries.size();
}

template <class T> void Cluster<T>::addEntry(const T &entry) {
  m_entries.push_back(entry);
}

template <class T> const std::vector<T>& Cluster<T>::getEntries() const {
  return m_entries;
}

template <class T>
void Cluster<T>::addEntryVector(const std::vector<T> &entry_vector) {
  if (not entry_vector.empty()) {
    m_entries.insert(m_entries.end(), entry_vector.begin(), entry_vector.end());
  }
}

template <class T>
void Cluster<T>::setClusterValue(const std::vector<double> &v,
                                 const std::vector<double> &v_sigma) {
  m_combined_value_vector = v;
  m_combined_sigma_vector = v_sigma;
}

template <class T> std::vector<double> Cluster<T>::getValues() const {
  // return a warning when this value is a default value
  if (m_combined_value_vector.size() == 0) {
    if (m_debug_level > 0) {
      std::cout
          << "Cluster::getTime: ATTENTION, combi values are not initialized!"
          << std::endl;
    }
  }
  return m_combined_value_vector;
}

template <class T> const std::vector<double>& Cluster<T>::getSigmas() const {
  return m_combined_sigma_vector;
}

template <class T> void Cluster<T>::setMergeStatus(bool status) {
  m_was_merged = status;
}

template <class T> bool Cluster<T>::mergeStatus() const { return m_was_merged; }

template <class T> int Cluster<T>::getMergeIteration() const {
  return m_merge_iteration;
}

template <class T> void Cluster<T>::setMergeIteration(int iteration) {
  m_merge_iteration = iteration;
}

template <class T> bool Cluster<T>::containsUnknowns() const {
  return m_contains_unknowns;
}

template <class T> void Cluster<T>::setUnknownStatus(bool status) {
  m_contains_unknowns = status;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

template <class T>
void ClusterCollection<T>::updateDistanceCut(double cut_value) {
  m_distance_cut = cut_value;
}

template <typename T> void ClusterCollection<T>::addCluster(const Cluster<T> & vx) {
  m_clusters.push_back(vx);
}

template <typename T>
double ClusterCollection<T>::getDistanceBetweenClusters(const Cluster<T> &a,
                                                        const Cluster<T> &b) {
  std::vector<double> a_values = a.getValues();
  std::vector<double> b_values = b.getValues();
  std::vector<double> a_sigmas = a.getSigmas();
  std::vector<double> b_sigmas = b.getSigmas();

  std::vector<double> distances(a_values.size());
  for (size_t i = 0; i < a_values.size(); i++) {
    double distance_i = 0;
    if (a_sigmas.at(i) >= 0.0 && b_sigmas.at(i) >= 0.0) {
      distance_i = std::abs(a_values.at(i) - b_values.at(i)) /
                   std::hypot(a_sigmas.at(i), b_sigmas.at(i));
    }
    distances.at(i) = distance_i;
  }
  double distance2 = 0.;
  for (double d : distances) {
    distance2 += d * d;
  }
  return std::sqrt(distance2);
}

template <class T>
Cluster<T> ClusterCollection<T>::mergeClusters(const Cluster<T> &a,
                                               const Cluster<T> &b) {
  Cluster<T> merged_cluster;
  merged_cluster.addEntryVector(a.getEntries());
  merged_cluster.addEntryVector(b.getEntries());

  std::vector<double> a_values = a.getValues();
  std::vector<double> b_values = b.getValues();
  std::vector<double> a_sigmas = a.getSigmas();
  std::vector<double> b_sigmas = b.getSigmas();

  std::vector<double> new_cluster_values(a_values.size());
  std::vector<double> new_cluster_sigmas(a_values.size());

  for (size_t i = 0; i < a_values.size(); i++) {
    double value1 = a_values.at(i);
    double value2 = b_values.at(i);
    double var1 = std::pow(a_sigmas.at(i), 2.0);
    double var2 = std::pow(b_sigmas.at(i), 2.0);
    double new_cluster_value =
        (value1 / var1 + value2 / var2) / (1.0 / var1 + 1.0 / var2);
    double new_cluster_sigma = std::sqrt(var1 * var2 / (var1 + var2));
    new_cluster_values.at(i) = new_cluster_value;
    new_cluster_sigmas.at(i) = new_cluster_sigma;
  }
  int new_merge_iteration = a.getMergeIteration() + b.getMergeIteration() + 1;
  merged_cluster.setClusterValue(new_cluster_values, new_cluster_sigmas);
  merged_cluster.setMergeStatus(true);
  merged_cluster.setMergeIteration(new_merge_iteration);
  return merged_cluster;
}

template <class T>
Cluster<T> ClusterCollection<T>::mergeClustersMean(const Cluster<T> &a,
                                                   const Cluster<T> &b) {
  Cluster<T> merged_cluster;
  merged_cluster.addEntryVector(a.getEntries());
  merged_cluster.addEntryVector(b.getEntries());

  std::vector<double> a_values = a.getValues();
  std::vector<double> b_values = b.getValues();
  std::vector<double> a_sigmas = a.getSigmas();
  std::vector<double> b_sigmas = b.getSigmas();

  std::vector<double> new_cluster_values(a_values.size());
  std::vector<double> new_cluster_sigmas(a_values.size());

  for (size_t i = 0; i < a_values.size(); i++) {
    double value1 = a_values.at(i);
    double value2 = b_values.at(i);
    double sigma1 = a_sigmas.at(i);
    double sigma2 = b_sigmas.at(i);
    double new_cluster_value = (value1 + value2) / 2.;
    double new_cluster_sigma = std::hypot(sigma1, sigma2);
    new_cluster_values.at(i) = new_cluster_value;
    new_cluster_sigmas.at(i) = new_cluster_sigma;
  }
  int new_merge_iteration = a.getMergeIteration() + b.getMergeIteration() + 1;
  merged_cluster.setClusterValue(new_cluster_values, new_cluster_sigmas);
  merged_cluster.setMergeStatus(true);
  merged_cluster.setMergeIteration(new_merge_iteration);
  return merged_cluster;
}

template <typename T>
void ClusterCollection<T>::doClustering(ClusterAlgo algo) {
  if (algo == ClusterAlgo::Eager) {
    for (const auto &clust : m_clusters) {
      if (clust.containsUnknowns()) {
        throw std::invalid_argument(
            "[ClusterCollection::doClustering] ERROR "
            "- eager clustering does not allow for unknown values");
      }
    }
  }

  // TODO case where I have 2 vertices in my collection
  if (m_debug_level > 0) {
    std::cout << "ClusterCollection::doTimeClustering" << std::endl;
  }
  double distance = 1.e30; // initial distance value, "far away"

  while (m_clusters.size() > 1) {
    int i0 = 0;
    int j0 = 0;
    if (m_debug_level > 0) {
      std::cout << "using " << m_clusters.size() << " vertices" << std::endl;
    }
    // find the two vertices that are closest to each other
    distance = getDistanceBetweenClusters(m_clusters.at(0), m_clusters.at(1));
    for (size_t i = 0; i < m_clusters.size(); i++) {
      for (size_t j = i + 1; j < m_clusters.size(); j++) {

        if (algo == ClusterAlgo::Simultaneous ||
            algo == ClusterAlgo::SimultaneousMean) {

          if (m_clusters.at(i).mergeStatus() or m_clusters.at(j).mergeStatus()) {
            continue;
          }
        }

        double current_distance =
            getDistanceBetweenClusters(m_clusters.at(i), m_clusters.at(j));
        if (current_distance <= distance) {
          distance = current_distance;
          i0 = i;
          j0 = j;
        }
      } // loop over j
    }   // loop over i
    if (m_debug_level > 0) {
      std::cout << "using vertex " << i0 << " and " << j0 << std::endl;
    }
    // now the closest two vertices are found and will be fused if cut passes
    if (distance < m_distance_cut && i0 != j0) {

      Cluster<T> new_cluster { };

      if (algo == ClusterAlgo::SimultaneousMean) {
        new_cluster = mergeClustersMean(m_clusters.at(i0), m_clusters.at(j0));
      } else {
        new_cluster = mergeClusters(m_clusters.at(i0), m_clusters.at(j0));
      }

      if (m_debug_level > 0) {
        std::cout << "starting to erase" << std::endl;
      }
      m_clusters.erase(m_clusters.begin() + j0);
      if (i0 < j0) {
        m_clusters.erase(m_clusters.begin() + i0);
      } else {
        m_clusters.erase(m_clusters.begin() + (i0 - 1));
      }
      if (m_debug_level > 0) {
        std::cout << "erase done" << std::endl;
      }
      m_clusters.push_back(new_cluster);
      if (m_debug_level > 0) {
        std::cout << "new cluster stored" << std::endl;
      }
    } else {
      if (algo == ClusterAlgo::Eager) {
        break;
      }
      // if there is a cluster that was merged
      if (std::find_if(m_clusters.begin(), m_clusters.end(),
                       [](const Cluster<T> &c) { return c.mergeStatus(); }) !=
          m_clusters.end()) {
        // reset each status to false
        std::for_each(m_clusters.begin(), m_clusters.end(), [](Cluster<T> &c) {
          c.setMergeStatus(false);
        });
      } else {
        // if not, this is the end of the clustering
        break;
      }
    }
  } // while loop
}

template <typename T>
std::pair<Cluster<T>, int> ClusterCollection<T>::largestClusterInfo() {

  int max_n_hits = 0;
  int count = 0;
  Cluster<T> max_n_vertex { };

  for (const auto& vx : m_clusters) {
    int current_n = vx.getNEntries();

    if (current_n > max_n_hits) {
      max_n_hits = current_n;
      max_n_vertex = vx;
      count = 1;
    }
    else if (current_n == max_n_hits) {
      count++;
    }
  }

  return std::make_pair(max_n_vertex, count);
}

template <typename T> Cluster<T> ClusterCollection<T>::getMaxEntriesCluster() {

  const auto [max_cluster, nMaxClusters] = largestClusterInfo();
  // If two or more clusters have the same maximum number of entries, I can't
  // decide, so return default
  if (nMaxClusters > 1) {
    return Cluster<T> { };
  }
  return max_cluster;
}

template <typename T> int ClusterCollection<T>::getMaxClusterSize_Info() {
  // find the vertex with a maximum amount of hits clustered in it
  // and return thisn umber
  const auto [max_cluster, nMaxClusters] = largestClusterInfo();

  return max_cluster.getNEntries();
}

template <typename T>
const std::vector<Cluster<T>>& ClusterCollection<T>::getClusters() const {
  return m_clusters;
}

template <typename T> int ClusterCollection<T>::getNClusters() const {
  return static_cast<int>(m_clusters.size());
}

} // namespace HGTD

#endif // CLUSTERINGHELPER_H
