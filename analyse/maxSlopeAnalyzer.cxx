

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
	recalculateData(seg);
}

void MaxSlopeAnalyzer::recalculateGamma(SegmentInfo &seginfo)
{
	recalculateData(seginfo);
	segments->refresh();
}

void MaxSlopeAnalyzer::setGammaEndIndex(int value, const QModelIndexList& indexList)
{
	SegmentInfo &seg = segments->getSegment(indexList[0]);
	seg.setGammaEndIndex(value);
	recalculateData(seg);
}

void MaxSlopeAnalyzer::recalculateData(SegmentInfo &seginfo)
{
	seginfo.recalculateGamma();
	segments->refresh();
}