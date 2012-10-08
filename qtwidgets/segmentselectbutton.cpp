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
