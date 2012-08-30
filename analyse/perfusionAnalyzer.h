/*
    Copyright 2012 Christian Freye

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

#ifndef PERFUSIONANALYZER_H
#define PERFUSIONANALYZER_H

#include <set>
#include "segmentlistmodel.h"
#include <QWidget>

class CTImageTreeItem;

/*! \class PerfusionAnalyzer PerfusionAnalyzer.h "PerfusionAnalyzer.h"
 *  \brief This abstract class defines the basics for a perfusion analyzer.
 */
class PerfusionAnalyzer
{
	public:
		///Constructor
		PerfusionAnalyzer(QWidget* p);
		///Destructor
		~PerfusionAnalyzer();

		///Add an image to the dialog.
		/*!
		\param image The image, that is to be added.
		*/
		void addImage(CTImageTreeItem *image);
		///Add a segment to the dialog.
		/*!
		\param segment The segment, that is to be added. 
		*/
		void addSegment(BinaryImageTreeItem *segment);
		///Gets the model of the segmentlist
		/*!
		\return Segemnts
		*/
		SegmentListModel* getSegments();
		///Calculates the time attenuation curves
		void calculateTacValues();
		///Gets the values of the TAC
		/*!
		\return Comma separated String containing the tac values. 
				The segments are separated by "\r\n" and the values by ";".
		*/
		std::string getTacValuesAsString();
		///Abstract method for calculating the perfusion results.
		virtual void calculatePerfusionResults() = 0;
		///Gets the time at a specific index
		/*!
		\return Time as a double. 
		*/
		double getTime(int index);
		int getImageCount();

	protected:

		///A structure that represents a functor for comparing two times
		struct CTImageTimeCompareFunctor {
			typedef CTImageTreeItem * argT;
			bool operator()(const argT &x, const argT &y) const;
		};
		//definitions for an imageSet and a double vector
		typedef std::set<CTImageTreeItem *, CTImageTimeCompareFunctor> ImageSet;
		typedef std::vector< double > DoubleVector;
  
		SegmentListModel *segments;
		DoubleVector times;
		ImageSet images;

		QWidget* parent;
};


#endif