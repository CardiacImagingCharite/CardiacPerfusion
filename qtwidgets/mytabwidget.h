
#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include <QTabWidget>
#include <QMouseEvent>

class MyTabWidget : public QTabWidget
{
	Q_OBJECT

	public:

	MyTabWidget(QWidget* parent);
	~MyTabWidget();

	signals:
	void doubleClicked(MyTabWidget &); ///< double click with LMB

	protected:
	///double click event
	void mouseDoubleClickEvent ( QMouseEvent * e );
};

#endif