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

#include "segmentlistmodel.h"
#include <qwt_plot_curve.h>
#include "gammafitdata.h"
#include "gammaVariate.h"
#include "binaryimagetreeitem.h"

SegmentListModel::SegmentListModel(QObject *parent):
  QAbstractListModel(parent) {
}


QVariant SegmentListModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole && index.row() < int(segments.size())) {
    const SegmentInfo &seg = segments[index.row()];
    switch(index.column()) {
      case 0: return seg.getName();
      case 1: return seg.getSegment()->getVolumeInML();break;
      case 2: return seg.getMaxStandardError();break;
      case 3: if (seg.isGammaEnabled()) return seg.getGammaMaxSlope();break;
      case 4: if (seg.isGammaEnabled()) return seg.getGammaMaximum();break;
      case 5: if (seg.getArterySegment() != NULL) return seg.getArterySegment()->getName(); break;
      case 6: if (seg.isGammaEnabled()) {
	if (seg.getArterySegment()!=NULL && seg.getArterySegment()->isGammaEnabled())
	  return 60 * seg.getGammaMaxSlope() / seg.getArterySegment()->getGammaMaximum();
      } break;
      case 7: if (seg.isGammaEnabled()) return seg.getGammaCenterOfGravity();break;
      case 8: if (seg.isGammaEnabled()) return seg.getGammaAUC();break;
    }
  }
  return QVariant::Invalid;
}

QVariant SegmentListModel::headerData( int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
    switch( section ) {
      case 0: return tr("segment");
      case 1: return tr("volume [ml]");
      case 2: return tr("max standard error [HU]");
      case 3: return tr("max slope [HU/s]");
      case 4: return tr("enhancement [HU]");
      case 5: return tr("artery");
      case 6: return tr("perfusion [ml/min/ml]");
      case 7: return tr("MTT [s]");
      case 8: return tr("AUC");
    }
  }
  return QVariant::Invalid;
}

int SegmentListModel::columnCount(const QModelIndex & parent) const {
  return 9;
}


SegmentInfo &SegmentListModel::getSegment(const QModelIndex& index) {
  return segments.at( index.row() );
}


int SegmentListModel::rowCount(const QModelIndex& parent) const
{
  return segments.size();
}

void SegmentListModel::setArterySegment(const QModelIndex& index, const SegmentInfo *arterySegment) {
  if (index.row() < int(segments.size()) ) {
    segments[index.row()].setArterySegment( arterySegment );
    emit dataChanged(this->index(index.row(),1), this->index(index.row(),4));
  }
}

void SegmentListModel::refresh() {
    emit dataChanged(this->index(0,1), this->index(segments.size()-1,4));
}


void SegmentListModel::addSegment( const BinaryImageTreeItem *seg ) {
  beginInsertRows(QModelIndex(), segments.size(), segments.size()+1);
  segments.push_back(new SegmentInfo(const_cast<BinaryImageTreeItem*>(seg)));
  endInsertRows();
}
