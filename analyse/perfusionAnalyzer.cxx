
#include "perfusionAnalyzer.h"
#include <QMessageBox>
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <boost/foreach.hpp>
#include "timedensitydata.h"
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <sstream>

PerfusionAnalyzer::PerfusionAnalyzer(QWidget* p):
	segments(p)
{
	parent = p;
	//configure the plot
	plot->setTitle(QObject::tr("Time Density Curves"));
	plot->setAxisTitle(QwtPlot::xBottom, QObject::tr("Time [s]"));
	plot->setAxisTitle(QwtPlot::yLeft, QObject::tr("Density [HU]"));
}

PerfusionAnalyzer::~PerfusionAnalyzer()
{
}

void PerfusionAnalyzer::addImage(CTImageTreeItem *image)
{
	images.insert(image);
}

void PerfusionAnalyzer::addSegment(BinaryImageTreeItem *segment)
{
	segments.addSegment(segment);
}

QwtPlot* PerfusionAnalyzer::getTacPlot()
{
	calculateTacValues();

	//iterate over the list of segments
	BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
		//attach the curves for the actual segment to the plot
		currentSegment.attachSampleCurves(plot);
	}
	return plot;
}

std::string PerfusionAnalyzer::getTacValues()
{
	std::ostringstream tacValueStream;
	
	tacValueStream << "Times;";

	calculateTacValues();

	BOOST_FOREACH( double t, times) {
		tacValueStream << t << ";";
	}

	tacValueStream << "\r\n";

	BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
		//attach the curves for the actual segment to the plot
		TimeDensityData* data = currentSegment.getSampleData();
		for(int i = 0; i < data->size(); i++)
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
	if (!images.size() || !segments.rowCount()) {
		QMessageBox::warning(parent,QObject::tr("Analyse Error"),QObject::tr("Select at least one volume with at least one segment"));
		return;
	}
	//get acquisition time of the first image
	double firstTime = (*images.begin())->getTime();
	int imageIndex = 0;
	//create object for segmentation values and set accuracy
	SegmentationValues values; values.accuracy = SegmentationValues::SimpleAccuracy;
	//iterate over all images
	for(ImageSet::const_iterator ii = images.begin(); ii != images.end(); ++ii) {
		const CTImageTreeItem *ct = *ii;
		//calculate relative time of actual image
		double relTime = ct->getTime() - firstTime;

		//add relative time to the list of times
		times.push_back(relTime);
		//iterate over all segments
		BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
			//add segment to the segmentation values 
			values.segment = currentSegment.getSegment();
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