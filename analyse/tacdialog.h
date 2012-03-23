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

#ifndef TACDIALOG_H
#define TACDIALOG_H

#include <QDialog>
#include "ui_TacDialog.h"
#include <set>
#include "segmentlistmodel.h"


class CTImageTreeItem;

class TacDialog : public QDialog, private Ui_TacDialog
{
	Q_OBJECT
	
public:
	//Constructor/Destructor
	TacDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
	~TacDialog();
	//add images to the dialog
	void addImage(CTImageTreeItem *image);
	//add segments to the dialog
	void addSegment(BinaryImageTreeItem *segment);
	//execution method
	int exec(void);
	//show the dialog
	void show();

public slots:
      

private:

	//functor for comparing two times
	struct CTImageTimeCompareFunctor {
		typedef CTImageTreeItem * argT;
		bool operator()(const argT &x, const argT &y) const;
	};
	//definitions for an imageSet and a double vector
	typedef std::set<CTImageTreeItem *, CTImageTimeCompareFunctor> ImageSet;
	typedef std::vector< double > DoubleVector;
  

	DoubleVector times;
	ImageSet images;

	SegmentListModel segments;
};

#endif // ANALYSEDIALOG_H
