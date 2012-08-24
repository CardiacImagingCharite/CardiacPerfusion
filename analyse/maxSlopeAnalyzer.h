/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

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

#ifndef maxSlopeAnalyzer_H
#define maxSlopeAnalyzer_H

#include "perfusionAnalyzer.h"
#include "segmentinfo.h"
#include <QString>

class MaxSlopeAnalyzer : public PerfusionAnalyzer
{
	public:
		MaxSlopeAnalyzer(QWidget *p);
		~MaxSlopeAnalyzer(); 
		
		void calculatePerfusionResults();

		void setGammaStartIndex(int value, const QModelIndexList& indexList);
		void setGammaEndIndex(int value, const QModelIndexList& indexList);
		void recalculateGamma(SegmentInfo &seginfo);
		void recalculateGamma(const QModelIndexList& indexList);

	private:
		void recalculateData(SegmentInfo &seginfo);

};

#endif