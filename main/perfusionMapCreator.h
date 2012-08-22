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
