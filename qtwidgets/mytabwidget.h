
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