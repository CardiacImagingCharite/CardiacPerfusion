/*
    Copyright 2012 Christian Freye
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

#include "timedensitydatapicker.h"
#include <boost/foreach.hpp>
#include <qwt_plot_curve.h>
#include "qwt_plot.h"
#include "timedensitydata.h"
#include "binaryimagetreeitem.h"


TimeDensityDataPicker::TimeDensityDataPicker(QwtPlotMarker *markerX_, QwtPlotMarker *markerY_, 
  const SegmentListModel &segmentList_, QwtPlotCanvas *c):QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPicker::PointSelection,QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, c),
        markerX(markerX_),
        markerY(markerY_),
        segmentList(segmentList_) {
}

QwtText TimeDensityDataPicker::trackerText(const QPoint &p) const {
  int minDist = std::numeric_limits< int >::max();
  double minX = 0;
  SegmentationValues minValues;
   // prevent Warning
  minValues.min = 0; minValues.max = 0; minValues.mean = 0; minValues.stddev = 0; minValues.sampleCount = 0; minValues.segment = 0;
  QwtDoublePoint pdv;
  QPoint pv;
  BOOST_FOREACH( const SegmentInfo &currentSegment, segmentList) {
    SegmentationValues values;
    const TimeDensityData *data = currentSegment.getSampleData();
    for(unsigned i=0; i < data->size(); ++i) {
      pdv.setX( data->getTimeAndValues(i, values) );
      pdv.setY( values.mean );
      pv = transform(pdv);
      int dx = pv.x() - p.x();
      int dy = pv.y() - p.y();
      int dist = dx * dx + dy * dy;
      if (dist < minDist) {
	minDist = dist;
	minX = pdv.x();
	minValues = values;
      }
    }
  }
  if (minDist < 25) {
    const BinaryImageTreeItem *binseg = dynamic_cast<const BinaryImageTreeItem *>(minValues.segment);
    QString text( binseg->getName() );
    text += "\nTime:" + QString::number(minX) + " s";
    text += "\nMean:" + QString::number(minValues.mean) + " HU";
    text += "\nStdDev:" + QString::number(minValues.stddev) + " HU";
    text += "\nMin:" + QString::number(minValues.min) + " HU";
    text += "\nMax:" + QString::number(minValues.max) + " HU";
    text += "\n#Samples:" + QString::number(minValues.sampleCount);
    if (!markerX->isVisible() || markerX->xValue()!= minX || markerY->yValue()!=minValues.mean) {
      markerX->setXValue(minX);
      markerY->setYValue(minValues.mean);
      markerX->setVisible(true);
      markerY->setVisible(true);
      const_cast<TimeDensityDataPicker*>(this)->canvas()->update();
    }
    return text;
  } else {
    if (markerX->isVisible()) {
      markerX->setVisible(false);
      markerY->setVisible(false);
      const_cast<TimeDensityDataPicker*>(this)->canvas()->update();
    }
    return QwtText();
  }
}