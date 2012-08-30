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

#ifndef SEGMENTLISTMODEL_H
#define SEGMENTLISTMODEL_H

#include <QAbstractItemModel>
#include <boost/ptr_container/ptr_vector.hpp>
#include "segmentinfo.h"

/*! \class SegmentListModel SegmentListModel.h "SegmentListModel.h"
 *  \brief This class represents a list model with segements.
 */
class SegmentListModel : public QAbstractListModel
{
  public:
    typedef boost::ptr_vector<SegmentInfo> SegmentListType;
    typedef SegmentListType::iterator iterator;
    typedef SegmentListType::const_iterator const_iterator;
    SegmentListModel(QObject *parent = 0);
    void addSegment( const BinaryImageTreeItem *seg );
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    void refresh();
    
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    SegmentInfo &getSegment(const QModelIndex& index);
    SegmentInfo &getSegment(unsigned row);
    void setArterySegment(const QModelIndex& index, const SegmentInfo *arterySegment);
    const_iterator begin() const { return segments.begin(); }
    const_iterator end() const { return segments.end(); }
    iterator begin() { return segments.begin(); }
    iterator end() { return segments.end(); }
  private:
    SegmentListType segments;
};

#endif // SEGMENTLISTMODEL_H
