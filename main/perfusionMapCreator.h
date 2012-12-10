/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie

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

    KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License für weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#ifndef PerfusionMapCreator_H
#define PerfusionMapCreator_H

#include "segmentinfo.h"
#include "maxslopeanalyzer.h"
#include "ctimagetreemodel.h"
#include "imagedefinitions.h"
#include "itkSmartPointer.h"
#include "realimagetreeitem.h"

/*! \class PerfusionMapCreator PerfusionMapCreator.h "PerfusionMapCreator.h"
 *  \brief This class creates an perfusion map.
 */
class PerfusionMapCreator
{
	public:
	///Constructor
	PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, SegmentInfo* artery, int factor);
	///Destructor
	~PerfusionMapCreator();

	///calculates the perfusion map.
	/*!
	\param model The used image model.

	\return Returns an image filled with perfusion values.
	*/
	RealImageTreeItem::ImageType::Pointer calculatePerfusionMap(CTImageTreeModel* model);
	///Setter for the used analyzer. 
	/*!
	\param analyzer The Analyzer
	*/
	void setAnalyzer(MaxSlopeAnalyzer* analyzer);
	///Sets the shrink factor.
	/*!
	\param shrinkFactor The factor for shrinking the image before processing. 
	*/
	void setShrinkFactor(int shrinkFactor);
	///Sets the artery segment.
	/*!
	\param artery The artery segment, which is needed to calculate the perfusion values. 
	*/
	void setArterySegment(SegmentInfo* artery);

private:

	MaxSlopeAnalyzer*   m_analyzer;
	int					m_shrinkFactor;
	SegmentInfo*		m_arterySegment;
};

#endif
