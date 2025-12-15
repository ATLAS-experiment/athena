/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 04/2024
* Description: ITkPix* chip layout template. This aims for a contiguously stored
*              array of arbitrary type, representing the individual pixels in
*              ITkPix* chips with a 'physically' motivated (col, row) access
*/

#ifndef ITKPIXLAYOUT_H
#define ITKPIXLAYOUT_H

#include <array>
#include <cstdint>

template<class T> class ITkPixLayout{

    public:

        ITkPixLayout(){};

        T& operator()(const uint16_t col, const uint16_t row){

            //Columns are stored after one another
            return m_pixels[ col * 384 + row ];

        }

        T operator()(const uint16_t col, const uint16_t row) const {

            //Columns are stored after one another
            return m_pixels[ col * 384 + row ];

        }

        int nHits() const {

            //count hits
            int hits = 0;
            for (const auto& pixel : m_pixels) if (pixel) hits++;
            return hits;

        }

    private:

        //All chips will always have 400*384 pixels
        std::array<T, 153600> m_pixels = {};

};

#endif
