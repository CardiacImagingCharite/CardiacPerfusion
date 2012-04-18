
#include "mytabwidget.h"

MyTabWidget::MyTabWidget(QWidget* parent) : QTabWidget(parent)
{
}

MyTabWidget::~MyTabWidget()
{
}

void MyTabWidget::mouseDoubleClickEvent( QMouseEvent * e ) {
	if(e->button() == Qt::LeftButton)
		emit doubleClicked(*this);
	QWidget::mouseDoubleClickEvent(e);
}