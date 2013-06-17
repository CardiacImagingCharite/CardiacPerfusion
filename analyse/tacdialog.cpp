/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
	Copyright 2010 Henning Meyer

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

#include "tacdialog.h"
#include <QMessageBox>
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <boost/foreach.hpp>
#include "gammaVariate.h"
#include "timedensitydata.h"
#include "gammafitdata.h"
#include <qwt_symbol.h>
#include <qwt_legend.h>

#include <fstream>
#include <iomanip>

//Constructor of the dialog
TacDialog::TacDialog(QWidget * parent, Qt::WindowFlags f) : QDialog( parent, f )
	,m_segments(this)
{
	setupUi( this );
	//configure the plot
	plot->setTitle(tr("Time Density Curves"));
	plot->setAxisTitle(QwtPlot::xBottom, tr("Time [s]"));
	plot->setAxisTitle(QwtPlot::yLeft, tr("Density [HU]"));
	//plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
	plot->setAxisScale(2,0,20);
	plot->setAxisScale(0,0,500);

}

TacDialog::~TacDialog() {

}

//add image to the image list
void TacDialog::addImage(CTImageTreeItem *image) {
	m_images.insert(image);
//  sliderStart->setRange(0, images.size()-1 );
//  sliderEnd->setRange(0, images.size()-1 );
}

//add segment to the segment list
void TacDialog::addSegment(BinaryImageTreeItem *segment) {
	m_segments.addSegment(segment);
}

//execute the dialog
int TacDialog::exec(void){

	fstream tacFile;
	tacFile.open("test.tac", ios::out);

	//if image list or segment list is empty, reject dialog and print warning
	if (!m_images.size() || !m_segments.rowCount()) {
		QMessageBox::warning(this,tr("Analyse Error"),tr("Select at least one volume with at least one segment"));
		return QDialog::Rejected;
	}
	//get acquisition time of the first image
	double firstTime = (*m_images.begin())->getTime();
	int imageIndex = 0;
	//create object for segmentation values and set accuracy
	SegmentationValues values; values.m_accuracy = SegmentationValues::SimpleAccuracy;
	//iterate over all images
	for(ImageSet::const_iterator ii = m_images.begin(); ii != m_images.end(); ++ii) {
		const CTImageTreeItem *ct = *ii;
		//calculate relative time of actual image
		double relTime = ct->getTime() - firstTime;

		tacFile << std::setprecision(2) << relTime;

		//add relative time to the list of times
		m_times.push_back(relTime);
		//iterate over all segments
		BOOST_FOREACH( SegmentInfo &currentSegment, m_segments) {
			//add segment to the segmentation values 
			values.m_segment = currentSegment.getSegment();
			//get segmentation values and add the sample to the list of segments
			if (ct->getSegmentationValues( values )) {
				currentSegment.pushSample(relTime, values);

				tacFile << "\t" << values.m_mean;
			} else {
				std::cerr << "Analyse Error: Could not apply Segment " << currentSegment.getName().toStdString() << " on image #" << imageIndex << std::endl;
			}
		}

		tacFile << std::endl;

		++imageIndex;
	}
	//iterate over the list of segments
	BOOST_FOREACH( SegmentInfo &currentSegment, m_segments) {
		//attach the curves for the actual segment to the plot
		currentSegment.attachSampleCurves(plot);
	}
	//  sliderClearanceStart->setMaximum(images.size()-1);
	//  sliderClearanceEnd->setMaximum(images.size()-1);
	//plot->replot();

	tacFile.close();
	
	return QDialog::exec();
}

void TacDialog::show()
{
	
	fstream tacFile;
	tacFile.open("test.tac", ios::out);

	//if image list or segment list is empty, reject dialog and print warning
	if (!m_images.size() || !m_segments.rowCount()) {
		QMessageBox::warning(this,tr("Analyse Error"),tr("Select at least one volume with at least one segment"));
		return;
		
	}
	//get acquisition time of the first image
	double firstTime = (*m_images.begin())->getTime();
	int imageIndex = 0;
	//create object for segmentation values and set accuracy
	SegmentationValues values; values.m_accuracy = SegmentationValues::SimpleAccuracy;
	//iterate over all images
	for(ImageSet::const_iterator ii = m_images.begin(); ii != m_images.end(); ++ii) {
		const CTImageTreeItem *ct = *ii;
		//calculate relative time of actual image
		double relTime = ct->getTime() - firstTime;

		tacFile << std::setprecision(2) << relTime;

		//add relative time to the list of times
		m_times.push_back(relTime);
		//iterate over all segments
		BOOST_FOREACH( SegmentInfo &currentSegment, m_segments) {
			//add segment to the segmentation values 
			values.m_segment = currentSegment.getSegment();
			//get segmentation values and add the sample to the list of segments
			if (ct->getSegmentationValues( values )) {
				currentSegment.pushSample(relTime, values);

				tacFile << "\t" << values.m_mean;
			} else {
				std::cerr << "Analyse Error: Could not apply Segment " << currentSegment.getName().toStdString() << " on image #" << imageIndex << std::endl;
			}
		}

		tacFile << std::endl;

		++imageIndex;
	}
	//iterate over the list of segments
	BOOST_FOREACH( SegmentInfo &currentSegment, m_segments) {
		//attach the curves for the actual segment to the plot
		currentSegment.attachSampleCurves(plot);
	}
	//  sliderClearanceStart->setMaximum(images.size()-1);
	//  sliderClearanceEnd->setMaximum(images.size()-1);
	//plot->replot();

	tacFile.close();
	
	QDialog::show();
}

//functor for comparing two times
bool TacDialog::CTImageTimeCompareFunctor::operator()(const argT &x, const argT &y) const {
  return (x->getTime() < y->getTime());
}