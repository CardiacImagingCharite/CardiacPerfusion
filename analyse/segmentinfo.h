/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

	This file was part of perfusionkit (Copyright 2010 Henning Meyer)
	and was modified and extended to fit the actual needs. 

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

#ifndef SEGMENTINFO_H
#define SEGMENTINFO_H

#include <boost/shared_ptr.hpp>
#include <qwt_plot_curve.h>
#include <QString>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/max.hpp>


class BinaryImageTreeItem;
class QwtPlotCurve;
class TimeDensityData;
class GammaFitData;
class QwtPlot;
namespace GammaFunctions {
class GammaVariate;
}

struct SegmentationValues;

/*! \class SegmentInfo SegmentInfo.h "SegmentInfo.h"
 *  \brief This class represents the information of a segment.
 */
class SegmentInfo {

public:
	///Constructor
	SegmentInfo(const BinaryImageTreeItem *s);
	///Gets the name of the segment.
	/*!
	\return String with the name.
	*/
	const QString &getName() const;
	///Gets the segment.
	/*!
	\return The segment.
	*/
	inline const BinaryImageTreeItem *getSegment() const {return segment;}
	///Sets the segment.
	/*!
	\param seg The segment to be set.
	*/
	inline void setSegment(BinaryImageTreeItem *seg) {segment = seg;}
	///Gets the artery segment.
	/*!
	\return The artery segment.
	*/
	inline const SegmentInfo *getArterySegment() const {return arterySegment;}
	///Sets the artery segment.
	/*!
	\param seg The artery segment to be set.
	*/
	inline void setArterySegment(const SegmentInfo *seg) {arterySegment = seg;}
	///Attach sample curve to the plot.
	/*!
	\param plot The desired plot to which the curve is to be attached.
	*/
	void attachSampleCurves(QwtPlot *plot);
	///Add a sample to the curve data.
	/*!
	\param time The time of the sample point.
	\param value The value of the sample point.
	*/
	void pushSample(double time, const SegmentationValues &value);

	///Checks if gamma curve is visible.
	/*!
	\return TRUE if curve is visible and FALSE if not.
	*/
	bool isGammaEnabled() const;
	///Sets the visibility of the gamma curve.
	/*!
	\param e (optional) Defines the visibiliy. Default is TRUE.
	*/
	void setEnableGamma(bool e=true);
	///Gets the gamma variate.
	/*!
	\return Gamma variate.
	*/
	GammaFunctions::GammaVariate *getGamma();
	const GammaFunctions::GammaVariate *getGamma() const;
	///Gets the maximum slope of the gamma curve.
	/*!
	\return Maximum slope.
	*/
	double getGammaMaxSlope() const;
	///Gets the maximum of the gamma curve.
	/*!
	\return Maximum of the curve.
	*/
	double getGammaMaximum() const;
	///Gets the center of gravity of the gamma curve.
	/*!
	\return Center of gravity.
	*/
	double getGammaCenterOfGravity() const;
	///Gets the area under the gamma curve.
	/*!
	\return Area under the gamma curve.
	*/
	double getGammaAUC() const;
	///Gets the baseline of the gamma curve.
	/*!
	\return Baseline of the gamma curve.
	*/
	double getGammaBaseline() const;
	///Gets the maximum standard error of the gamma curve.
	/*!
	\return Maximum standard error.
	*/
	double getMaxStandardError() const;
	///Sets the start index of the gamma curve.
	/*!
	\param index Start index.
	*/
	inline void setGammaStartIndex(unsigned index) { gammaStartIndex = index; }
	///Sets the end index of the gamma curve.
	/*!
	\param index End index.
	*/
	inline void setGammaEndIndex(unsigned index)  { gammaEndIndex = index; }
	///Gets the start index of the gamma curve.
	/*!
	\return Start index.
	*/	
	inline unsigned getGammaStartIndex() const { return gammaStartIndex; }
	///Gets the end index of the gamma curve.
	/*!
	\return End index.
	*/
	inline unsigned getGammaEndIndex() const { return gammaEndIndex; }
  
	///Attach patlak to a plot.
	/*!
	\param plot The destination plot.

	\return FALSE if patlak could not be created.
	*/
	bool attachPatlak(QwtPlot *plot);
	///Detach patlak from a plot.
	void detachPatlak();
	///Gets the patlak start index.
	/*!
	\return Patlak start index.
	*/
	unsigned getPatlakStartIndex() const;
	///Gets the patlak end index.
	/*!
	\return Patlak end index.
	*/
	unsigned getPatlakEndIndex() const;
	///Gets the patlak intercept.
	/*!
	\return Patlak intercept.
	*/
	double getPatlakIntercept() const;
	///Gets the patlak slope.
	/*!
	\return Patlak slope.
	*/
	double getPatlakSlope() const;
	///Sets the patlak start index.
	/*!
	\param index Patlak start index.
	*/
	void setPatlakStartIndex(unsigned index);
	///Sets the patlak end index.
	/*!
	\param index Patlak end index.
	*/
	void setPatlakEndIndex(unsigned index);
	///Gets the sample data.
	/*!
	\return The time density data.
	*/
	TimeDensityData *getSampleData();
	const TimeDensityData *getSampleData() const;
	///Recalculates the gamma curve.
	void recalculateGamma();

private:
	bool createPatlak();
	unsigned gammaStartIndex;
	unsigned gammaEndIndex;
	const BinaryImageTreeItem *segment;
	const SegmentInfo *arterySegment;
	QwtPlotCurve sampleCurve;
	QwtPlotCurve gammaCurve;
	QwtPlotCurve patlakCurve;
	QwtPlotCurve patlakRegression;
	bool patlakCreated;
	boost::accumulators::accumulator_set<
	double,
	boost::accumulators::stats<
		boost::accumulators::tag::max> > standardErrorAccumulator;
  
};

#endif // SEGMENTINFO_H
