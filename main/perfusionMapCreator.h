#ifndef PerfusionMapCreator_H
#define PerfusionMapCreator_H

#include "segmentinfo.h"
#include "maxslopeanalyzer.h"
#include "ctimagetreemodel.h"
#include "imagedefinitions.h"

class PerfusionMapCreator
{
	public:

	PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, const SegmentInfo* artery, int factor);
	~PerfusionMapCreator();

	RealImageType* getPerfusionMap(CTImageTreeModel* model);
	void setAnalyzer(MaxSlopeAnalyzer* analyzer);
	void setShrinkFactor(int shrinkFactor);
	void setArterySegment(const SegmentInfo* artery);

private:

	MaxSlopeAnalyzer*   m_analyzer;
	int					m_shrinkFactor;
	const SegmentInfo*	m_arterySegment;
};

#endif
