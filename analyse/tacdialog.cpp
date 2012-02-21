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

TacDialog::TacDialog(QWidget * parent, Qt::WindowFlags f)
  :QDialog( parent, f )
  ,segments(this)
  {
  setupUi( this );
  plot->setTitle(tr("Time Density Curves"));
  plot->setAxisTitle(QwtPlot::xBottom, tr("Time [s]"));
  plot->setAxisTitle(QwtPlot::yLeft, tr("Density [HU]"));
//  plot->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
  plot->setAxisScale(2,0,20);
}

TacDialog::~TacDialog() {

}

void TacDialog::addImage(CTImageTreeItem *image) {
  images.insert(image);
//  sliderStart->setRange(0, images.size()-1 );
//  sliderEnd->setRange(0, images.size()-1 );
}

void TacDialog::addSegment(BinaryImageTreeItem *segment) {
  segments.addSegment(segment);
}

int TacDialog::exec(void)
{
	if (!images.size() || !segments.rowCount()) {
    QMessageBox::warning(this,tr("Analyse Error"),tr("Select at least one volume with at least one segment"));
    return QDialog::Rejected;
  }
  double firstTime = (*images.begin())->getTime();
  int imageIndex = 0;
  SegmentationValues values; values.accuracy = SegmentationValues::SimpleAccuracy;
  for(ImageSet::const_iterator ii = images.begin(); ii != images.end(); ++ii) {
    const CTImageTreeItem *ct = *ii;
    double relTime = ct->getTime() - firstTime;
    times.push_back(relTime);
    BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
      values.segment = currentSegment.getSegment();
      if (ct->getSegmentationValues( values )) {
	currentSegment.pushSample(relTime, values);
      } else {
	std::cerr << "Analyse Error: Could not apply Segment " << currentSegment.getName().toStdString() << " on image #" << imageIndex << std::endl;
      }
    }
    ++imageIndex;
  }
  BOOST_FOREACH( SegmentInfo &currentSegment, segments) {
    currentSegment.attachSampleCurves(plot);
  }
//  sliderClearanceStart->setMaximum(images.size()-1);
//  sliderClearanceEnd->setMaximum(images.size()-1);
  //plot->replot();
  return QDialog::exec();
}

bool TacDialog::CTImageTimeCompareFunctor::operator()(const argT &x, const argT &y) const {
  return (x->getTime() < y->getTime());
}