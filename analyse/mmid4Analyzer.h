
#ifndef MMID4ANALYZER_H
#define MMID4ANALYZER_H

#include "perfusionAnalyzer.h"
#include <QWidget>

class MMID4Analyzer : public PerfusionAnalyzer
{
	public:

	MMID4Analyzer(QWidget *p);
	~MMID4Analyzer();

	virtual void calculatePerfusionResults();

};

#endif