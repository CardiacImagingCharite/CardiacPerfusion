/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

#include "segmentinfo.h"
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include "gammafitdata.h"
#include "gammaVariate.h"
#include "timedensitydata.h"
#include "patlakdata.h"
#include "linearregressiondata.h"
#include "binaryimagetreeitem.h"
#include <QObject>

SegmentInfo::SegmentInfo(const BinaryImageTreeItem *s):
  gammaStartIndex(0), gammaEndIndex(0), segment(s), arterySegment(NULL),
  sampleCurve(s->getName()), gammaCurve(s->getName()+ QObject::tr(" Gamma Fit")),
  patlakCurve(s->getName()), patlakRegression(s->getName() + QObject::tr(" linear Regression")),
  patlakCreated(false) {
    
	//get color of the segment
    QColor color = s->getColor();
	//set color to the curve
    sampleCurve.setPen(QPen(color));
    sampleCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    
	QwtSymbol symbol; //class for drawing symbols
    symbol.setStyle( QwtSymbol::Ellipse );
    symbol.setSize(8);
    symbol.setPen(QPen(color));
    symbol.setBrush(QBrush(color.darker(130)));
	//set the symbol to the curve
    sampleCurve.setSymbol( symbol );
	//initialize curve data 
    sampleCurve.setData( TimeDensityData() );      
    
	//create pen for gamma curve with dotted line
    QPen pen(color);
    pen.setStyle(Qt::DotLine);
	//set pen to gamma curve
    gammaCurve.setPen(pen);
    gammaCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    //initialize gamma curve data and hide it
	gammaCurve.setData( GammaFitData() );
	gammaCurve.setVisible(false);

	//set style of the patlak curve
    patlakCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    patlakCurve.setStyle(QwtPlotCurve::NoCurve);
    symbol.setStyle( QwtSymbol::XCross );
    symbol.setSize(8);
    symbol.setPen(QPen(color));
    symbol.setBrush(QBrush(color.darker(130)));
    patlakCurve.setSymbol( symbol );
    patlakRegression.setRenderHint(QwtPlotItem::RenderAntialiased, true);
}

bool SegmentInfo::createPatlak() {
	if (patlakCreated) 
		return true;
	if (!isGammaEnabled() || arterySegment==NULL || !arterySegment->isGammaEnabled())
		return false;
	PatlakData pd(sampleCurve.data(), arterySegment->sampleCurve.data());
	pd.setTissueBaseline(getGamma()->getBaseline());
	pd.setArteryBaseline(arterySegment->getGamma()->getBaseline());
	patlakCurve.setData( pd );
	LinearRegressionData patlakRegressionData( patlakCurve.data() );
	patlakRegression.setData(patlakRegressionData);
	patlakCreated = true;
	return true;
}

unsigned SegmentInfo::getPatlakStartIndex() const {
	if (patlakCreated) {
		return dynamic_cast<const PatlakData&>(patlakCurve.data()).getStartIndex();
	} else 
		return 0;
}

unsigned SegmentInfo::getPatlakEndIndex() const {
	if (patlakCreated) {
		return dynamic_cast<const PatlakData&>(patlakCurve.data()).getEndIndex();
	} else 
		return 0;
}

void SegmentInfo::setPatlakStartIndex(unsigned index) {
	if (patlakCreated) {
		dynamic_cast<PatlakData&>(patlakCurve.data()).setStartIndex(index);
	}
}

void SegmentInfo::setPatlakEndIndex(unsigned index) {
	if (patlakCreated) {
		dynamic_cast<PatlakData&>(patlakCurve.data()).setEndIndex(index);
	}
}

double SegmentInfo::getPatlakIntercept() const {
	if (patlakCreated) {
		return dynamic_cast<const LinearRegressionData&>(patlakRegression.data()).getIntercept();
	} else
		return std::numeric_limits< double >::quiet_NaN();
}

double SegmentInfo::getPatlakSlope() const {
	if (patlakCreated) {
		return dynamic_cast<const LinearRegressionData&>(patlakRegression.data()).getSlope();
	} else
		return std::numeric_limits< double >::quiet_NaN();
}

double SegmentInfo::getMaxStandardError() const {
	return boost::accumulators::max( standardErrorAccumulator );
}

//add sample to the curve data
void SegmentInfo::pushSample(double time, const SegmentationValues &values) {
	//cast curve data and add a point to the curve data
	dynamic_cast<TimeDensityData&>(sampleCurve.data()).pushPoint(time, values);
	dynamic_cast<GammaFitData&>(gammaCurve.data()).includeTime(time);
	double standardError = values.stddev / std::sqrt( static_cast<double>(values.sampleCount) );
	standardErrorAccumulator( standardError );
}

//attach sample and gamma curve to the plot
void SegmentInfo::attachSampleCurves(QwtPlot *plot) {
	sampleCurve.attach(plot);
	gammaCurve.attach(plot);
}

//attach patlak to the plot
bool SegmentInfo::attachPatlak(QwtPlot *plot) {
	if (!createPatlak()) 
		return false;
	patlakCurve.attach(plot);
	patlakRegression.attach(plot);
	return true;
}

//detach patlak 
void SegmentInfo::detachPatlak() {
	patlakCurve.detach();
	patlakRegression.detach();
}

//getter and setter
bool SegmentInfo::isGammaEnabled() const {
	return gammaCurve.isVisible();
}

void SegmentInfo::setEnableGamma(bool e) {
	gammaCurve.setVisible(e);
}

GammaFunctions::GammaVariate *SegmentInfo::getGamma() {
	return &(dynamic_cast<GammaFitData&>(gammaCurve.data()).getGammaVariate());
}

const GammaFunctions::GammaVariate *SegmentInfo::getGamma() const {
	return &(dynamic_cast<const GammaFitData&>(gammaCurve.data()).getGammaVariate());
}

double SegmentInfo::getGammaMaxSlope() const {
	return getGamma()->getMaxSlope();
}

double SegmentInfo::getGammaMaximum() const {
	return getGamma()->getMaximum();
}

double SegmentInfo::getGammaCenterOfGravity() const {
	return getGamma()->getCenterOfGravity();
}

double SegmentInfo::getGammaAUC() const {
	return getGamma()->getAUC();
}

double SegmentInfo::getGammaBaseline() const {
	return getGamma()->getBaseline();
}

void SegmentInfo::recalculateGamma() {
	GammaFunctions::GammaVariate * gamma = getGamma();
	gamma->clearSamples();
	const QwtData &tdd = sampleCurve.data();
	for(unsigned i = gammaStartIndex; i <= gammaEndIndex; ++i) {
		if (i < tdd.size()) 
			gamma->addSample(tdd.x(i), tdd.y(i));
	}
	gamma->findFromSamples();
}

const QString &SegmentInfo::getName() const {
	if (segment!= NULL)
		return segment->getName();
	const static QString emptyString;
	return emptyString;
}

TimeDensityData *SegmentInfo::getSampleData() {
	return &(dynamic_cast<TimeDensityData&>(sampleCurve.data())); 
}

const TimeDensityData *SegmentInfo::getSampleData() const {
	return &(dynamic_cast<const TimeDensityData&>(sampleCurve.data())); 
}
