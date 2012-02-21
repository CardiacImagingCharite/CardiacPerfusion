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

#ifndef LINEARREGRESSIONDATA_H
#define LINEARREGRESSIONDATA_H
#include <vector>

#include <qwt_data.h>

class LinearRegressionData : public QwtData
{
  public:
    LinearRegressionData( const QwtData &source_ ):source(source_), m(0), n(0) { update();}
    virtual double y(size_t i) const;
    virtual double x(size_t i) const;
    virtual size_t size() const;
    virtual QwtData* copy() const { return new LinearRegressionData( source ); }
    void update(void);
    double getIntercept() const;
    double getSlope() const;
  private:
    const QwtData &source;
    double m, n;
    std::vector< double > xc,yc;
};
#endif // LINEARREGRESSIONDATA_H
