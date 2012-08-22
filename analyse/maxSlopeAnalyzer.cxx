

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