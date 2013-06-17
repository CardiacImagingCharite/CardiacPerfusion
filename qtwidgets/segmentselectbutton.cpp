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

#include "segmentselectbutton.h"
#include <QInputDialog>
#include <boost/foreach.hpp>
#include "binaryimagetreeitem.h"

SegmentSelectButton::SegmentSelectButton(QWidget *parent)
  :QPushButton(0, parent),
  m_selectedSegment(NULL),
  m_segmentList(NULL) {
    updateText();
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onClick()));
}


void SegmentSelectButton::onClick() {
  if (m_segmentList && m_segmentList->rowCount()>0) {
    QStringList nameList;
    BOOST_FOREACH(const SegmentInfo &seg, *m_segmentList) {
      nameList << seg.getName();
    }
    bool ok;
    QString selectedName = QInputDialog::getItem(this, tr("Select Segment"), tr("Choose segment"), nameList, 0, false, &ok);
    if (!ok) return;
    BOOST_FOREACH(const SegmentInfo &seg, *m_segmentList) {
      if (seg.getName() == selectedName) {
	m_selectedSegment = &seg;
	updateText();
	emit selected(m_selectedSegment);
	break;
      }
    }
  }
}

void SegmentSelectButton::setSelectedSegment(const SegmentInfo *segment) {
  m_selectedSegment = segment;
  updateText();
}

void SegmentSelectButton::updateText() {
  if (m_selectedSegment!=NULL) {
    setText( m_selectedSegment->getName() );
  } else {
    setText( tr("not selected") );
  }
  update();
}
