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

#include "timedensitydatapicker.h"
#include <boost/foreach.hpp>
#include <qwt_plot_curve.h>
#include "qwt_plot.h"
#include "timedensitydata.h"
#include "binaryimagetreeitem.h"


TimeDensityDataPicker::TimeDensityDataPicker(QwtPlotMarker *markerX_, QwtPlotMarker *markerY_, 
  const SegmentListModel &segmentList_, QwtPlotCanvas *c):QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPicker::PointSelection,QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOn, c),
        m_markerX(markerX_),
        m_markerY(markerY_),
        m_segmentList(segmentList_) {
}

QwtText TimeDensityDataPicker::trackerText(const QPoint &p) const {
  int minDist = std::numeric_limits< int >::max();
  double minX = 0;
  SegmentationValues minValues;
   // prevent Warning
  minValues.m_min = 0; minValues.m_max = 0; minValues.m_mean = 0; minValues.m_stddev = 0; minValues.m_sampleCount = 0; minValues.m_segment = 0;
  QwtDoublePoint pdv;
  QPoint pv;
  BOOST_FOREACH( const SegmentInfo &currentSegment, segmentList) {
    SegmentationValues values;
    const TimeDensityData *data = currentSegment.getSampleData();
    for(unsigned i=0; i < data->size(); ++i) {
      pdv.setX( data->getTimeAndValues(i, values) );
      pdv.setY( values.m_mean );
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
    const BinaryImageTreeItem *binseg = dynamic_cast<const BinaryImageTreeItem *>(minValues.m_segment);
    QString text( binseg->getName() );
    text += "\nTime:" + QString::number(minX) + " s";
    text += "\nMean:" + QString::number(minValues.m_mean) + " HU";
    text += "\nStdDev:" + QString::number(minValues.m_stddev) + " HU";
    text += "\nMin:" + QString::number(minValues.m_min) + " HU";
    text += "\nMax:" + QString::number(minValues.m_max) + " HU";
    text += "\n#Samples:" + QString::number(minValues.m_sampleCount);
    if (!m_markerX->isVisible() || m_markerX->xValue()!= minX || m_markerY->yValue()!=minValues.m_mean) {
      m_markerX->setXValue(minX);
      m_markerY->setYValue(minValues.m_mean);
      m_markerX->setVisible(true);
      m_markerY->setVisible(true);
      const_cast<TimeDensityDataPicker*>(this)->canvas()->update();
    }
    return text;
  } else {
    if (m_markerX->isVisible()) {
      m_markerX->setVisible(false);
      m_markerY->setVisible(false);
      const_cast<TimeDensityDataPicker*>(this)->canvas()->update();
    }
    return QwtText();
  }
}