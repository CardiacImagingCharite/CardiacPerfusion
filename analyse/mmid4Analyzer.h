/*
    Copyright 2012 Christian Freye

	This file is part of KardioPerfusion.

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

    Diese Datei ist Teil von KardioPerfusion.

    KardioPerfusion ist Freie Software: Sie k�nnen es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
    ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es n�tzlich sein wird, aber
    OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License f�r weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
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