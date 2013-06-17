/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie

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

#include "perfusionAnalyzer.h"
#include <QMessageBox>
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <boost/foreach.hpp>
#include "timedensitydata.h"
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <sstream>

#include "itkImageFileWriter.h"

PerfusionAnalyzer::PerfusionAnalyzer(QWidget* p)
{
	m_segments = new SegmentListModel(p);
	m_parent = p;
}

PerfusionAnalyzer::~PerfusionAnalyzer()
{
}

void PerfusionAnalyzer::addImage(CTImageTreeItem *image)
{
	m_images.insert(image);
}

void PerfusionAnalyzer::addSegment(BinaryImageTreeItem *segment)
{
	m_segments->addSegment(segment);
}

void PerfusionAnalyzer::popBackSegment()
{
	m_segments->popBackSegment();
}

SegmentListModel* PerfusionAnalyzer::getSegments()
{
	//calculateTacValues();

	//iterate over the list of segments
/*	BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
		//attach the curves for the actual segment to the plot
		currentSegment.attachSampleCurves(plot);
	}
	*/
	return m_segments;
}



std::string PerfusionAnalyzer::getTacValuesAsString()
{
	std::ostringstream tacValueStream;
	
	tacValueStream << "Times;";

	calculateTacValues();

	BOOST_FOREACH( double t, m_times) {
		tacValueStream << t << ";";
	}

	tacValueStream << "\r\n";

	BOOST_FOREACH( SegmentInfo &currentSegment, *m_segments) {
		//attach the curves for the actual segment to the plot
		TimeDensityData* data = currentSegment.getSampleData();

		tacValueStream << &currentSegment.getName() << ";";

		for( unsigned int i = 0; i < data->size(); i++)
		{
			tacValueStream << data->y(i) << ";";
		}
		tacValueStream << "\r\n";
	}
	return tacValueStream.str();
}

void PerfusionAnalyzer::calculateTacValues()
{
	//if image list or segment list is empty, reject dialog and print warning
	if (!m_images.size() || !m_segments->rowCount()) {
		QMessageBox::warning(m_parent,QObject::tr("Analyse Error"),QObject::tr("Select at least one volume with at least one segment"));
		return;
	}
	//get acquisition time of the first image
	double firstTime = (*m_images.begin())->getTime();
	int imageIndex = 0;
	//create object for segmentation values and set accuracy
	SegmentationValues values; values.m_accuracy = SegmentationValues::SimpleAccuracy;

/*	typedef itk::ImageFileWriter< CTImageTreeItem::ImageType >  WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "test.dcm" );
	*/

	//iterate over all images
	for(ImageSet::const_iterator ii = m_images.begin(); ii != m_images.end(); ++ii) {
		const CTImageTreeItem *ct = *ii;
		//calculate relative time of actual image
		double relTime = ct->getTime() - firstTime;

/*		writer->SetInput( ct->getITKImage() );
		try 
		{
			writer->Update();
		}
		catch( itk::ExceptionObject & excep )
		{
			std::cerr << "Exception catched !" << std::endl;
			std::cerr << excep << std::endl;
		}
*/
		//add relative time to the list of times
		m_times.push_back(relTime);
		//iterate over all segments
		BOOST_FOREACH( SegmentInfo &currentSegment, *m_segments) {
			//add segment to the segmentation values 
			values.m_segment = currentSegment.getSegment();
			//get segmentation values and add the sample to the list of segments
			if (ct->getSegmentationValues( values )) {
				currentSegment.pushSample(relTime, values);
			} else {
				std::cerr << "Analyse Error: Could not apply Segment " << currentSegment.getName().toStdString() << " on image #" << imageIndex << std::endl;
			}
		}

		++imageIndex;
	}
}

//functor for comparing two times
bool PerfusionAnalyzer::CTImageTimeCompareFunctor::operator()(const argT &x, const argT &y) const {
  return (x->getTime() < y->getTime());
}

double PerfusionAnalyzer::getTime(unsigned int index)
{
	if(index < m_times.size())
		return m_times[index];

	return -1;
}

int PerfusionAnalyzer::getImageCount()
{
	return m_images.size();
}