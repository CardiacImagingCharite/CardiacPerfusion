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

#include "linearregressiondata.h"
#include "linearregression.h"

double LinearRegressionData::y(size_t i) const {
  const_cast<LinearRegressionData*>(this)->update();
  return m * x(i) + n;
}

double LinearRegressionData::getIntercept() const { 
  const_cast<LinearRegressionData*>(this)->update();
  return n; 
}
double LinearRegressionData::getSlope() const { 
  const_cast<LinearRegressionData*>(this)->update();
  return m; 
}

double LinearRegressionData::x(size_t i) const {
  if (i==0) return source.x(0);
  else return source.x(source.size()-1);
}
size_t LinearRegressionData::size() const {
  if (source.size()>1) return 2;
  else return 0;
}

void LinearRegressionData::update(void) {
  bool recalc = false;
  if (xc.size() != source.size()) {
    recalc = true;
    xc.resize( source.size() );
    yc.resize( source.size() );
  }
  double t;
  for(unsigned i=0; i < source.size(); ++i) {
    t = source.x(i);
    if (xc[i] != t) { xc[i] = t; recalc = true; }
    t = source.y(i);
    if (yc[i] != t) { yc[i] = t; recalc = true; }
  }
  if (recalc) LinearRegression(xc.begin(),xc.end(),yc.begin(),n,m);
}