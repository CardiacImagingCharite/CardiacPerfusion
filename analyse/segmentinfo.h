/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
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


class SegmentInfo {
  public:
  SegmentInfo(const BinaryImageTreeItem *s);
  const QString &getName() const;
  inline const BinaryImageTreeItem *getSegment() const {return segment;}
  inline void setSegment(BinaryImageTreeItem *seg) {segment = seg;}
  inline const SegmentInfo *getArterySegment() const {return arterySegment;}
  inline void setArterySegment(const SegmentInfo *seg) {arterySegment = seg;}
  void attachSampleCurves(QwtPlot *plot);
  void pushSample(double time, const SegmentationValues &value);

  bool isGammaEnabled() const;
  void setEnableGamma(bool e=true);
  GammaFunctions::GammaVariate *getGamma();
  const GammaFunctions::GammaVariate *getGamma() const;
  double getGammaMaxSlope() const;
  double getGammaMaximum() const;
  double getGammaCenterOfGravity() const;
  double getGammaAUC() const;
  double getGammaBaseline() const;
  double getMaxStandardError() const;
  inline void setGammaStartIndex(unsigned index) { gammaStartIndex = index; }
  inline void setGammaEndIndex(unsigned index)  { gammaEndIndex = index; }
  inline unsigned getGammaStartIndex() const { return gammaStartIndex; }
  inline unsigned getGammaEndIndex() const { return gammaEndIndex; }
  
  bool attachPatlak(QwtPlot *plot);
  void detachPatlak();
  unsigned getPatlakStartIndex() const;
  unsigned getPatlakEndIndex() const;
  double getPatlakIntercept() const;
  double getPatlakSlope() const;
  void setPatlakStartIndex(unsigned index);
  void setPatlakEndIndex(unsigned index);
  TimeDensityData *getSampleData();
  const TimeDensityData *getSampleData() const;
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
