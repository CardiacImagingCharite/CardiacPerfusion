
#ifndef MMID4ANALYZER_H
#define MMID4ANALYZER_H

#include "perfusionAnalyzer.h"
#include <QWidget>

/*! \class MMID4Analyzer MMID4Analyzer.h "MMID4Analyzer.h"
 *  \brief This class is a specific implementation of a perfusion analyzer.
			It implements the MMID4 model. For further information to that model please
			take a look at http://www.physiome.org/jsim/models/webmodel/NSR/MMID4/
 */
class MMID4Analyzer : public PerfusionAnalyzer
{
	public:
	///Constructor
	MMID4Analyzer(QWidget *p);
	///Destructor
	~MMID4Analyzer();
	///Implementation of the calculation for perfusion. 
	/*!
	
	*/
	virtual void calculatePerfusionResults();

};

#endif