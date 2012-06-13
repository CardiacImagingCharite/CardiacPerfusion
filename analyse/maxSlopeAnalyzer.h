
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