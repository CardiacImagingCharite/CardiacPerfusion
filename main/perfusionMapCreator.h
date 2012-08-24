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

#ifndef PerfusionMapCreator_H
#define PerfusionMapCreator_H

#include "segmentinfo.h"
#include "maxslopeanalyzer.h"
#include "ctimagetreemodel.h"
#include "imagedefinitions.h"
#include "itkSmartPointer.h"
#include "realimagetreeitem.h"


class PerfusionMapCreator
{
	public:

	PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, SegmentInfo* artery, int factor);
	~PerfusionMapCreator();

	RealImageTreeItem::ImageType::Pointer getPerfusionMap(CTImageTreeModel* model);
	void setAnalyzer(MaxSlopeAnalyzer* analyzer);
	void setShrinkFactor(int shrinkFactor);
	void setArterySegment(SegmentInfo* artery);

private:

	MaxSlopeAnalyzer*   m_analyzer;
	int					m_shrinkFactor;
	SegmentInfo*		m_arterySegment;
};

#endif
