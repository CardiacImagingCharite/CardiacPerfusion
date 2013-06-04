/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie

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

#ifndef maxSlopeAnalyzer_H
#define maxSlopeAnalyzer_H

#include "perfusionAnalyzer.h"
#include "segmentinfo.h"
#include <QString>
/*! \class MaxSlopeAnalyzer MaxSlopeAnalyzer.h "MaxSlopeAnalyzer.h"
 *  \brief This class represents an maximum slope analyzer. It inherits the PerfusionAnalyzer-class.
 */
class MaxSlopeAnalyzer : public PerfusionAnalyzer
{
	public:
		///Constructor
		MaxSlopeAnalyzer(QWidget *p);
		//Destructor
		~MaxSlopeAnalyzer(); 
		///Calculates perfusion results.
		void calculatePerfusionResults();
		///Sets the gamma start index.
		void setGammaStartIndex(int value, const QModelIndexList& indexList);
		///Sets the gamma end index.
		void setGammaEndIndex(int value, const QModelIndexList& indexList);
		///recalculates the gamma variate.
		void recalculateGamma(SegmentInfo &seginfo);
		///recalculates the gamma variate.
		void recalculateGamma(const QModelIndexList& indexList);

	private:
		void recalculateData(SegmentInfo &seginfo);

};

#endif