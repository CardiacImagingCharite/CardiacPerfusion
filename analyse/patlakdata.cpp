/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

	This file was part of perfusionkit (Copyright 2010 Henning Meyer)
	and was modified and extended to fit the actual needs. 

    KardioPerfusion is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KardioPerfusion is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KardioPerfusion.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "patlakdata.h"
#include <algorithm>

PatlakData::PatlakData(const QwtData &tissueSource, const QwtData &arterySource)
  :tissue(tissueSource), artery(arterySource), 
  tissueBaseline(0), arteryBaseline(0),
  startIndex(0), endIndex(0) {}

double PatlakData::y(size_t i) const {
  return tissue.y(i+startIndex) / artery.y(i+startIndex);
}

double PatlakData::x(size_t i) const {
  double lastTissueTime = tissue.x(startIndex);
  double lastTissueDensity = tissue.y(startIndex);
  double integral = .0;
  for(unsigned k = startIndex+1; k < (startIndex+i+1); ++k) {
    double tissueTime = tissue.x(k);
    double tissueDensity = tissue.y(k);
    integral += (lastTissueDensity + tissueDensity) * .5 * (tissueTime - lastTissueTime);
    lastTissueTime = tissueTime;
    lastTissueDensity = tissueDensity;
  }
  return integral / tissue.y(i);
}

size_t PatlakData::size() const  { 
  signed d = endIndex+1;
  d-=startIndex;
  return std::max(d,1); 
}
