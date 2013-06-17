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

#include "linearregressiondata.h"
#include "linearregression.h"

double LinearRegressionData::y(size_t i) const {
  const_cast<LinearRegressionData*>(this)->update();
  return m_m * x(i) + m_n;
}

double LinearRegressionData::getIntercept() const { 
  const_cast<LinearRegressionData*>(this)->update();
  return m_n; 
}
double LinearRegressionData::getSlope() const { 
  const_cast<LinearRegressionData*>(this)->update();
  return m_m; 
}

double LinearRegressionData::x(size_t i) const {
  if (i==0) return m_source.x(0);
  else return m_source.x(m_source.size()-1);
}
size_t LinearRegressionData::size() const {
  if (m_source.size()>1) return 2;
  else return 0;
}

void LinearRegressionData::update(void) {
  bool recalc = false;
  if (m_xc.size() != m_source.size()) {
    recalc = true;
    m_xc.resize( m_source.size() );
    m_yc.resize( m_source.size() );
  }
  double t;
  for(unsigned i=0; i < m_source.size(); ++i) {
    t = m_source.x(i);
    if (m_xc[i] != t) { m_xc[i] = t; recalc = true; }
    t = m_source.y(i);
    if (m_yc[i] != t) { m_yc[i] = t; recalc = true; }
  }
  if (recalc) LinearRegression(m_xc.begin(),m_xc.end(),m_yc.begin(),m_n,m_m);
}