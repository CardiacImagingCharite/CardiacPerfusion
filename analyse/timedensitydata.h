/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

    KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License für weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#ifndef TIMEDENSITYDATA_H
#define TIMEDENSITYDATA_H

#include <qwt_data.h>
#include "segmentationvalues.h"
/*! \class TimeDensityData TimeDensityData.h "TimeDensityData.h"
 *  \brief This class represents the time density data.
 */
class TimeDensityData: public QwtData {
public:
	///Copies the actual object.
	/*!
	\return Copy of the object.
	*/
    virtual QwtData *copy() const {
        return new TimeDensityData(*this);
    }
	///Gets the number of elements.
	/*!
	\return Size of the time vector.
	*/
    virtual size_t size() const {
        return time.size();
    }
	///Gets the x value at a specific position.
	/*!
	\param i Defines the desired position.

	\return The x value at the index i.
	*/
    virtual double x(size_t i) const {
        return time[i];
    }
	///Gets the y value at a specific position.
	/*!
	\param i Defines the desired position.

	\return The y value at the index i.
	*/
    virtual double y(size_t i) const {
        return values[i].mean;
    }
	///Gets time and value at a specific positon
	/*!
	\param i Defines the desired position.
	\param v Placeholder for the segmentation values

	\return Time of the index position.
	*/
    double getTimeAndValues( size_t i, SegmentationValues &v ) const {
		v = values[i];
		return time[i];
    }
	///Adds a sample to the data.
	/*!
	\param t The time sample.
	\param v The segmentation values.
	*/
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
