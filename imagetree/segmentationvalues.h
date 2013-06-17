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

#ifndef SEGMENTATIONVALUES_H
#define SEGMENTATIONVALUES_H

#include <boost/serialization/access.hpp>
#include "itkvtktreeitem.h"
#include "imagedefinitions.h"


/*!
	A Structure, which contains necessary information of a segment.
*/
struct SegmentationValues {
	/*!
		An enumeration for the accuracy.
	*/
	enum Accuracy {
		SimpleAccuracy,					/**<Simple accuracy*/
		PreventDoubleSamplingAccuracy,	/**<Accuracy, which prevents double sampling*/
		InterpolatedAccuracy			/**<Interpolated accuracy*/
	};
	//pointer to the segment
	const ITKVTKTreeItem<BinaryImageType> *m_segment;
	//some statistics of the segment and the used accuracy
	long unsigned m_mtime;
	double m_mean;
	double m_stddev;
	int m_min;
	int m_max;
	int m_sampleCount;
	Accuracy m_accuracy;
private:
	//boost serialization class
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

#endif // SEGMENTATIONVALUES_H
