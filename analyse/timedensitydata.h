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

#ifndef TIMEDENSITYDATA_H
#define TIMEDENSITYDATA_H

#include <qwt_data.h>
#include "segmentationvalues.h"

class TimeDensityData: public QwtData {
public:
    virtual QwtData *copy() const {
        return new TimeDensityData(*this);
    }
	//getter for size, x, y
    virtual size_t size() const {
        return time.size();
    }

    virtual double x(size_t i) const {
        return time[i];
    }

    virtual double y(size_t i) const {
        return values[i].mean;
    }
	// get time and value at index i
    double getTimeAndValues( size_t i, SegmentationValues &v ) const {
		v = values[i];
		return time[i];
    }
	//add sample to the data
    void pushPoint( double t, const SegmentationValues &v) {
		if (time.size() == 0 || t> time[time.size()-1]) {
			time.push_back(t);
			values.push_back(v);
		}
    }

private:
  std::vector< double > time;
  std::vector< SegmentationValues > values;
};


#endif // TIMEDENSITYDATA_H
