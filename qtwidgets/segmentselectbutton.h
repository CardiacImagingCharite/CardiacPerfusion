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

#ifndef SEGMENTSELECTBUTTON_H
#define SEGMENTSELECTBUTTON_H

#include <QPushButton>
#include "segmentlistmodel.h"


class SegmentSelectButton : public QPushButton
{
  Q_OBJECT
  public:
  SegmentSelectButton(QWidget *parent = 0);
  const SegmentInfo * getSelectedSegment(void) const { return selectedSegment; }
  void setSegmentListModel(const SegmentListModel *model) { segmentList = model; }
  void setSelectedSegment(const SegmentInfo *segment);
  
  signals:
  void selected(const SegmentInfo *segment);
  
  private slots:
  void onClick();
  private:
  void updateText();
  const SegmentInfo *selectedSegment;
  const SegmentListModel *segmentList;
};

#endif // SEGMENTSELECTBUTTON_H
