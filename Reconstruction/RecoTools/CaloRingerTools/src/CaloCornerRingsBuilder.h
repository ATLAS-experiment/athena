/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALORINGERTOOLS_CALOCORNERRINGSBUILDER_H
#define CALORINGERTOOLS_CALOCORNERRINGSBUILDER_H

// Base includes:
#include "CaloRingerTools/ICaloRingsBuilder.h"
#include "CaloRingsBuilder.h"

namespace Ringer
{

	class CaloCornerRingsBuilder : public CaloRingsBuilder
	{

	public:
		/// @name CaloCornerRingsBuilder ctors and dtors:
		/// @{
		/**
		 * @brief Default constructor
		 **/
		CaloCornerRingsBuilder(const std::string &type,
							   const std::string &name,
							   const IInterface *parent);

		/**
		 * @brief Destructor
		 **/
		~CaloCornerRingsBuilder();
		/// @}

		/// Tool main methods:
		/// @}
		virtual StatusCode execute(const xAOD::CaloCluster &cluster,
								   ElementLink<xAOD::CaloRingsContainer> &clRings) override;

		/**
		 * @brief execute method for IParticle
		 **/
		virtual StatusCode execute(const xAOD::IParticle &particle,
								   ElementLink<xAOD::CaloRingsContainer> &clRings) override;
		/// @}
	protected:
		using CaloRingsBuilder::buildRingSet;
		//
		/// Tool protected methods:
		/// @{
		/**
		 * @brief main method where the RingSets are built.
		/// @}
		 **/
		virtual StatusCode buildRingSet(
			const xAOD::RingSetConf::RawConf &rawConf,
			const AtlasGeoPoint &seed,
			xAOD::RingSet *rs,
			const unsigned int offset,
			const unsigned int nSubRings);
		/// @}

		/**
		 * @brief Get the seeds for the corner rings (top-left, etc).
		 */
		StatusCode getCornerRingsSeeds(
			const xAOD::RingSetConf::RawConf &rawConf,
			const AtlasGeoPoint &centralSeed,
			std::vector<AtlasGeoPoint> &cornerSeeds);
		/**
		 * @brief Number of cells to shift for corner seeds (top-left, etc).
		 **/
		Gaudi::Property<int> m_cornerShift{
			this,
			"CornerShift",
			3,
			"Number of cells to shift for corner seeds (top-left, etc)"};

	private:
		/// Tool private methods:
		/// @{
		/**
		 * @brief unique execute method for integrating interface code.
		 **/
		template <typename T>
		StatusCode executeTemp(
			const T &input,
			ElementLink<xAOD::CaloRingsContainer> &crEL);
		/// @}
	};

} // namespace Ringer

#endif
