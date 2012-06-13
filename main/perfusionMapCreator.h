#ifndef PerfusionMapCreator_H
#define PerfusionMapCreator_H

#include "segmentinfo.h"
#include "perfusionAnalyzer.h"
#include "ctimagetreemodel.h"
#include "imagedefinitions.h"

class PerfusionMapCreator
{
	public:

	PerfusionMapCreator(PerfusionAnalyzer* analyzer, int factor);
	~PerfusionMapCreator();

	RealImageType* getPerfusionMap(CTImageTreeModel* model);
	void setAnalyzer(PerfusionAnalyzer* analyzer);
	void setShrinkFactor(int shrinkFactor);

private:

	PerfusionAnalyzer*  m_analyzer;
	int					m_shrinkFactor;
};

#endif
