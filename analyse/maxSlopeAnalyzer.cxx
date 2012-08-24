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

#include "maxSlopeAnalyzer.h"

MaxSlopeAnalyzer::MaxSlopeAnalyzer(QWidget *p) : PerfusionAnalyzer(p)
{

}

MaxSlopeAnalyzer::~MaxSlopeAnalyzer()
{

}

void MaxSlopeAnalyzer::calculatePerfusionResults()
{

}

void MaxSlopeAnalyzer::setGammaStartIndex(int value, const QModelIndexList& indexList)
{

	SegmentInfo &seg = segments->getSegment(indexList[0]);
	seg.setGammaStartIndex(value);
}

void MaxSlopeAnalyzer::recalculateGamma(SegmentInfo &seginfo)
{
	recalculateData(seginfo);
	//segments->refresh();
}

void MaxSlopeAnalyzer::recalculateGamma(const QModelIndexList& indexList)
{
	SegmentInfo &seg = segments->getSegment(indexList[0]);
	recalculateData(seg);
}

void MaxSlopeAnalyzer::setGammaEndIndex(int value, const QModelIndexList& indexList)
{
	SegmentInfo &seg = segments->getSegment(indexList[0]);
	seg.setGammaEndIndex(value);
}

void MaxSlopeAnalyzer::recalculateData(SegmentInfo &seginfo)
{
	seginfo.recalculateGamma();
	segments->refresh();
}