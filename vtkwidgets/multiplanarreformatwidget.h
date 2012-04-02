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

#ifndef MULTIPLANARREFORMATWIDGET_H
#define MULTIPLANARREFORMATWIDGET_H

#include <QVTKWidget.h>
#include <QtGui>
#include <map>
#include "vtkbinaryimageoverlay.h"
#include "vtkinteractorstyleprojectionview.h"
#include "vtkinteractoractiondispatch.h"
#include <vtkSmartPointer.h>
#include <boost/shared_ptr.hpp>
#include "imagedefinitions.h"
#include "vtkImageViewer2.h"


class vtkImageData;
class vtkImageReslice;
class vtkImageMapToWindowLevelColors;
class vtkImageMapToColors;
class vtkBinaryImageToColor;
//class vtkImageActor;
//class vtkRenderer;
class vtkMatrix4x4;


/// QT-Widget displaying a Volume Slice
class MultiPlanarReformatWidget : public QVTKWidget
{
	public:
	MultiPlanarReformatWidget(QWidget* parent = NULL, Qt::WFlags f = 0);
	~MultiPlanarReformatWidget();  
	void setImage(vtkImageData *image);
	int addBinaryOverlay(vtkImageData *image, const QColor &color, const ActionDispatch &dispatch = ActionDispatch() );
	int addAction(const ActionDispatch &dispatch);
	void removeAction(int actionHandle);
	void resetActions();
	void activateAction(int actionHandle);
	void activateOverlayAction(vtkImageData *image);
	void removeBinaryOverlay(vtkImageData *image);
	virtual void resizeEvent( QResizeEvent * event );
	void setCubicInterpolation(bool cubic=true);
	void setOrientation(int orientation);
	void showCircle(int radius);
  
	protected:
	typedef std::map< vtkImageData *, boost::shared_ptr< vtkBinaryImageOverlay > > OverlayMapType;
	OverlayMapType m_overlays;
	vtkImageData *m_image; ///< volume image data to be displayed - set by setImage()
	vtkImageReslice *m_reslice; ///< vtkImageAlgorithm to reslice the image
	vtkImageMapToWindowLevelColors *m_colormap; ///< used to apply Window and Level
	//vtkImageActor *m_actor; ///< vtkActor which actually displays the resliced volume
	//vtkRenderer *m_renderer; ///< the used renderer
	vtkSmartPointer<vtkImageViewer2> m_imageViewer; ///< the uses image viewer
	vtkMatrix4x4 *m_reslicePlaneTransform; ///< Tranformation Matrix for the Reslicing Plane
	vtkSmartPointer<vtkInteractorStyleProjectionView> m_interactorStyle; ///< special InteractorStyle for Projected Volumes
	int m_orientation;
	QPushButton *m_menuButton;
};

#endif // MULTIPLANARREFORMATWIDGET_H
