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