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
  m_gammaStartIndex(0), m_gammaEndIndex(0), m_segment(s), m_arterySegment(NULL),
  m_sampleCurve(s->getName()), m_gammaCurve(s->getName()+ QObject::tr(" Gamma Fit")),
  m_patlakCurve(s->getName()), m_patlakRegression(s->getName() + QObject::tr(" linear Regression")),
  m_patlakCreated(false) {
    
	//get color of the segment
    QColor color = s->getColor();
	//set color to the curve
    m_sampleCurve.setPen(QPen(color));
    m_sampleCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    
	QwtSymbol symbol; //class for drawing symbols
    symbol.setStyle( QwtSymbol::Ellipse );
    symbol.setSize(8);
    symbol.setPen(QPen(color));
    symbol.setBrush(QBrush(color.darker(130)));
	//set the symbol to the curve
    m_sampleCurve.setSymbol( symbol );
	//initialize curve data 
    m_sampleCurve.setData( TimeDensityData() );      
    
	//create pen for gamma curve with dotted line
    QPen pen(color);
    pen.setStyle(Qt::DotLine);
	//set pen to gamma curve
    m_gammaCurve.setPen(pen);
    m_gammaCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    //initialize gamma curve data and hide it
	m_gammaCurve.setData( GammaFitData() );
	m_gammaCurve.setVisible(false);

	//set style of the patlak curve
    m_patlakCurve.setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_patlakCurve.setStyle(QwtPlotCurve::NoCurve);
    symbol.setStyle( QwtSymbol::XCross );
    symbol.setSize(8);
    symbol.setPen(QPen(color));
    symbol.setBrush(QBrush(color.darker(130)));
    m_patlakCurve.setSymbol( symbol );
    m_patlakRegression.setRenderHint(QwtPlotItem::RenderAntialiased, true);
}

bool SegmentInfo::createPatlak() {
	if (m_patlakCreated) 
		return true;
	if (!isGammaEnabled() || m_arterySegment==NULL || !m_arterySegment->isGammaEnabled())
		return false;
	PatlakData pd(m_sampleCurve.data(), m_arterySegment->m_sampleCurve.data());
	pd.setTissueBaseline(getGamma()->getBaseline());
	pd.setArteryBaseline(m_arterySegment->getGamma()->getBaseline());
	m_patlakCurve.setData( pd );
	LinearRegressionData patlakRegressionData( m_patlakCurve.data() );
	m_patlakRegression.setData(patlakRegressionData);
	m_patlakCreated = true;
	return true;
}

unsigned SegmentInfo::getPatlakStartIndex() const {
	if (m_patlakCreated) {
		return dynamic_cast<const PatlakData&>(m_patlakCurve.data()).getStartIndex();
	} else 
		return 0;
}

unsigned SegmentInfo::getPatlakEndIndex() const {
	if (m_patlakCreated) {
		return dynamic_cast<const PatlakData&>(m_patlakCurve.data()).getEndIndex();
	} else 
		return 0;
}

void SegmentInfo::setPatlakStartIndex(unsigned index) {
	if (m_patlakCreated) {
		dynamic_cast<PatlakData&>(m_patlakCurve.data()).setStartIndex(index);
	}
}

void SegmentInfo::setPatlakEndIndex(unsigned index) {
	if (m_patlakCreated) {
		dynamic_cast<PatlakData&>(m_patlakCurve.data()).setEndIndex(index);
	}
}

double SegmentInfo::getPatlakIntercept() const {
	if (m_patlakCreated) {
		return dynamic_cast<const LinearRegressionData&>(m_patlakRegression.data()).getIntercept();
	} else
		return std::numeric_limits< double >::quiet_NaN();
}

double SegmentInfo::getPatlakSlope() const {
	if (m_patlakCreated) {
		return dynamic_cast<const LinearRegressionData&>(m_patlakRegression.data()).getSlope();
	} else
		return std::numeric_limits< double >::quiet_NaN();
}

double SegmentInfo::getMaxStandardError() const {
	return boost::accumulators::max( m_standardErrorAccumulator );
}

//add sample to the curve data
void SegmentInfo::pushSample(double time, const SegmentationValues &values) {
	//cast curve data and add a point to the curve data
	dynamic_cast<TimeDensityData&>(m_sampleCurve.data()).pushPoint(time, values);
	dynamic_cast<GammaFitData&>(m_gammaCurve.data()).includeTime(time);
	double standardError = values.m_stddev / std::sqrt( static_cast<double>(values.m_sampleCount) );
	m_standardErrorAccumulator( standardError );
}

//attach sample and gamma curve to the plot
void SegmentInfo::attachSampleCurves(QwtPlot *plot) {
	m_sampleCurve.attach(plot);
	m_gammaCurve.attach(plot);
}

//attach patlak to the plot
bool SegmentInfo::attachPatlak(QwtPlot *plot) {
	if (!createPatlak()) 
		return false;
	m_patlakCurve.attach(plot);
	m_patlakRegression.attach(plot);
	return true;
}

//detach patlak 
void SegmentInfo::detachPatlak() {
	m_patlakCurve.detach();
	m_patlakRegression.detach();
}

//getter and setter
bool SegmentInfo::isGammaEnabled() const {
	return m_gammaCurve.isVisible();
}

void SegmentInfo::setEnableGamma(bool e) {
	m_gammaCurve.setVisible(e);
}

GammaFunctions::GammaVariate *SegmentInfo::getGamma() {
	return &(dynamic_cast<GammaFitData&>(m_gammaCurve.data()).getGammaVariate());
}

const GammaFunctions::GammaVariate *SegmentInfo::getGamma() const {
	return &(dynamic_cast<const GammaFitData&>(m_gammaCurve.data()).getGammaVariate());
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
	const QwtData &tdd = m_sampleCurve.data();
	for(unsigned i = m_gammaStartIndex; i <= m_gammaEndIndex; ++i) {
		if (i < tdd.size()) 
			gamma->addSample(tdd.x(i), tdd.y(i));
	}
	gamma->findFromSamples();
}

const QString &SegmentInfo::getName() const {
	if (m_segment!= NULL)
		return m_segment->getName();
	const static QString emptyString;
	return emptyString;
}

TimeDensityData *SegmentInfo::getSampleData() {
	return &(dynamic_cast<TimeDensityData&>(m_sampleCurve.data())); 
}

const TimeDensityData *SegmentInfo::getSampleData() const {
	return &(dynamic_cast<const TimeDensityData&>(m_sampleCurve.data())); 
}
