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

#include "segmentselectbutton.h"
#include <QInputDialog>
#include <boost/foreach.hpp>
#include "binaryimagetreeitem.h"

SegmentSelectButton::SegmentSelectButton(QWidget *parent)
  :QPushButton(0, parent),
  selectedSegment(NULL),
  segmentList(NULL) {
    updateText();
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onClick()));
}


void SegmentSelectButton::onClick() {
  if (segmentList && segmentList->rowCount()>0) {
    QStringList nameList;
    BOOST_FOREACH(const SegmentInfo &seg, *segmentList) {
      nameList << seg.getName();
    }
    bool ok;
    QString selectedName = QInputDialog::getItem(this, tr("Select Segment"), tr("Choose segment"), nameList, 0, false, &ok);
    if (!ok) return;
    BOOST_FOREACH(const SegmentInfo &seg, *segmentList) {
      if (seg.getName() == selectedName) {
	selectedSegment = &seg;
	updateText();
	emit selected(selectedSegment);
	break;
      }
    }
  }
}

void SegmentSelectButton::setSelectedSegment(const SegmentInfo *segment) {
  selectedSegment = segment;
  updateText();
}

void SegmentSelectButton::updateText() {
  if (selectedSegment!=NULL) {
    setText( selectedSegment->getName() );
  } else {
    setText( tr("not selected") );
  }
  update();
}
