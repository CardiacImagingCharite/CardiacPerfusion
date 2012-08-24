/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

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
*/

#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include <QTabWidget>
#include <QMouseEvent>

/*! \class MyTabWidget MyTabWidget.h "MyTabWidget.h"
 *  \brief This class overrides the QTabWidget and adds a doubleclick ability to the widget.
 */
class MyTabWidget : public QTabWidget
{
	Q_OBJECT

	public:
	///Constructor
	MyTabWidget(QWidget* parent);
	///Destructor
	~MyTabWidget();

	signals:
	///Signal which will be emitted, if a doubleclick event occurs.
	void doubleClicked(MyTabWidget &); ///< double click with LMB

	protected:
	///This method catches a double click event.
	void mouseDoubleClickEvent ( QMouseEvent * e );
};

#endif