/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
	Copyright 2010 Henning Meyer

	This file is part of KardioPerfusion.

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

    Diese Datei ist Teil von KardioPerfusion.

    KardioPerfusion ist Freie Software: Sie k�nnen es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
    ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es n�tzlich sein wird, aber
    OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License f�r weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#include "patlakdata.h"
#include <algorithm>

PatlakData::PatlakData(const QwtData &tissueSource, const QwtData &arterySource)
  :m_tissue(tissueSource), m_artery(arterySource), 
  m_tissueBaseline(0), m_arteryBaseline(0),
  m_startIndex(0), m_endIndex(0) {}

double PatlakData::y(size_t i) const {
  return m_tissue.y(i+m_startIndex) / m_artery.y(i+m_startIndex);
}

double PatlakData::x(size_t i) const {
  double lastTissueTime = m_tissue.x(m_startIndex);
  double lastTissueDensity = m_tissue.y(m_startIndex);
  double integral = .0;
  for(unsigned k = m_startIndex+1; k < (m_startIndex+i+1); ++k) {
    double tissueTime = m_tissue.x(k);
    double tissueDensity = m_tissue.y(k);
    integral += (lastTissueDensity + tissueDensity) * .5 * (tissueTime - lastTissueTime);
    lastTissueTime = tissueTime;
    lastTissueDensity = tissueDensity;
  }
  return integral / m_tissue.y(i);
}

size_t PatlakData::size() const  { 
  signed d = m_endIndex+1;
  d-=m_startIndex;
  return std::max(d,1); 
}
