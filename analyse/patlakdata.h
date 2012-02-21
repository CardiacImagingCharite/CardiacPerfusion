/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PATLAKDATA_H
#define PATLAKDATA_H

#include <qwt_data.h>


class PatlakData : public QwtData
{
  public:
    PatlakData( const QwtData &tissueSource, const QwtData &arterySource );
    virtual double y(size_t i) const;
    virtual double x(size_t i) const;
    virtual size_t size() const;
    virtual QwtData* copy() const { return new PatlakData( *this ); }
    inline void setTissueBaseline(double tb) { tissueBaseline = tb; }
    inline void setArteryBaseline(double ab) { arteryBaseline = ab; }
    inline size_t getStartIndex() const { return startIndex; }
    inline size_t getEndIndex() const { return endIndex; }
    inline void setStartIndex(size_t index) { startIndex = std::min(index,std::min(tissue.size(), artery.size())-1); }
    inline void setEndIndex(size_t index) { endIndex = std::min(index,std::min(tissue.size(), artery.size())-1);  }
  private:
    const QwtData &tissue;
    const QwtData &artery;
    double tissueBaseline, arteryBaseline;
    size_t startIndex, endIndex;
};

#endif // PATLAKDATA_H
