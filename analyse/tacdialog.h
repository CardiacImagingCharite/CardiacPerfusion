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

#ifndef TACDIALOG_H
#define TACDIALOG_H

#include <QDialog>
#include "ui_TacDialog.h"
#include <set>
#include "segmentlistmodel.h"


class CTImageTreeItem;
/*! \class TacDialog TacDialog.h "TacDialog.h"
 *  \brief The GUI class for the dialog, which can show the time attenuation curves.
 */
class TacDialog : public QDialog, private Ui_TacDialog
{
	Q_OBJECT
	
public:
	///Constructor
	TacDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
	///Destructor
	~TacDialog();
	///Add an image to the dialog.
	/*!
	\param image The image, that is to be added.
	*/
	void addImage(CTImageTreeItem *image);
	///Add a segment to the dialog.
	/*!
	\param segment The segment, that is to be added. 
	*/
	void addSegment(BinaryImageTreeItem *segment);
	///Executes/shows the dialog.
	int exec(void);
	///Shows the dialog.
	void show();

public slots:
      

private:

	///A structure that represents a functor for comparing two times
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
