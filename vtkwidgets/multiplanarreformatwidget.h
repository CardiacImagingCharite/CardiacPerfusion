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

#ifndef MULTIPLANARREFORMATWIDGET_H
#define MULTIPLANARREFORMATWIDGET_H

#include <QVTKWidget.h>
#include <QtGui>
#include <map>
#include "vtkbinaryimageoverlay.h"
#include "vtkcoloredimageoverlay.h"
#include "vtkinteractorstyleprojectionview.h"
#include "vtkinteractoractiondispatch.h"
#include <vtkSmartPointer.h>
#include <boost/shared_ptr.hpp>
#include "imagedefinitions.h"
#include "vtkImageViewer2.h"
#include <QMouseEvent>


class vtkImageData;
class vtkImageReslice;
class vtkImageMapToWindowLevelColors;
class vtkImageMapToColors;
class vtkBinaryImageToColor;
//class vtkImageActor;
//class vtkRenderer;
class vtkMatrix4x4;
class vtkCornerAnnotation;


/*! \class MultiPlanarReformatWidget MultiPlanarReformatWidget.h "MultiPlanarReformatWidget.h"
 *  \brief This is a widget that can display VTK images on a GUI.
 */
class MultiPlanarReformatWidget : public QVTKWidget
{
	Q_OBJECT

	public:
	///Constructor of the widget.
	MultiPlanarReformatWidget(QWidget* parent = NULL, Qt::WFlags f = 0);
	///Deconstructor of the widget.
	~MultiPlanarReformatWidget();  
	///Sets the image that is to be shown
	/*!
	\param image Contains a pointer to the image.
	*/
	void setImage(vtkImageData *image);
	///Adds a binary overlay to the actual image
	/*!
	\param image Contains a pointer to the image.
	\param color Contains the color of the overlay.
	\param dispatch (optional) Contains the associated action. Default creates new action.
	*/
	int addBinaryOverlay(vtkImageData *image, const QColor &color, const ActionDispatch &dispatch = ActionDispatch() );
	///Adds a colored overlay to the actual image
	/*!
	\param image Contains a pointer to the image.
	\param dispatch (optional) Contains the associated action. Default creates new action.
	*/
	int addColoredOverlay(vtkImageData *image, const ActionDispatch &dispatch = ActionDispatch() );
	///Adds a colored overlay to the actual image
	/*!
	\param overlay Contains a pointer to an overlay.
	*/
	int addColoredOverlay(vtkImageData *image, vtkLookupTable* customColorMap, const ActionDispatch &dispatch = ActionDispatch());


	///Adds an action to the action list.
	/*!
	\param dispatch The action that is to be added.
	*/
	int addAction(const ActionDispatch &dispatch);
	///Removes an action of the actoin list.
	/*!
	\param actionHandle An action handle of the action that is to be removed.
	*/
	void removeAction(int actionHandle);
	///Resets all actions.
	void resetActions();
	///Activates a specific action.
	/*!
	\param actionHandle An action handle of the action that is to be removed.
	*/
	void activateAction(int actionHandle);
	///Activates an overlay to a specific image.
	/*!
	\param image A pointer to the image.
	*/
	void activateOverlayAction(vtkImageData *image);
	///Removes an overlay from a specific image.
	/*!
	\param image A pointer to the image.
	*/
	void removeBinaryOverlay(vtkImageData *image);
	///Removes an overlay from a specific image.
	/*!
	\param image A pointer to the image.
	*/
	void removeColoredOverlay(vtkImageData *image);
	///Resizes the form.
	/*!
	\param event The resize event.
	*/
	virtual void resizeEvent( QResizeEvent * event );
	///Sets interpolation to cubic or linear
	/*!
	\param cubic (optional) Default is TRUE, if FALSE the interpolation is set to linear.
	*/
	void setCubicInterpolation(bool cubic=true);
	///Sets the orientation of the slices
	/*!
	\param orientation A number that appoints the desired orientation
	*/
	void setOrientation(int orientation);

	///Gets the colorMap of the PerfusionMap.
	/*!
	\return colormap.
	*/
	vtkLookupTable* getOverlayColorMap();
	///Refresh the view.
	void refreshView();

  
	public slots:
	///Resets the view.
	void resetView();

	signals:
	void doubleClicked(MultiPlanarReformatWidget &); ///< double click with LMB

	protected:
	///double click event
	void mouseDoubleClickEvent ( QMouseEvent * e );
	typedef std::map< vtkImageData *, boost::shared_ptr< vtkBinaryImageOverlay > > BinaryOverlayMapType;
	typedef std::map< vtkImageData *, boost::shared_ptr< vtkColoredImageOverlay > > ColoredOverlayMapType;
	ColoredOverlayMapType m_coloredOverlays;
	BinaryOverlayMapType m_binaryOverlays;
	vtkImageData *m_image; ///< volume image data to be displayed - set by setImage()
	vtkImageReslice *m_reslice; ///< vtkImageAlgorithm to reslice the image
	//vtkImageMapToWindowLevelColors *m_colormap; ///< used to apply Window and Level
	//vtkImageActor *m_actor; ///< vtkActor which actually displays the resliced volume
	//vtkRenderer *m_renderer; ///< the used renderer
	vtkSmartPointer<vtkImageViewer2> m_imageViewer; ///< the uses image viewer
	vtkSmartPointer<vtkCornerAnnotation> m_annotation;
	vtkMatrix4x4 *m_reslicePlaneTransform; ///< Tranformation Matrix for the Reslicing Plane
	vtkSmartPointer<vtkInteractorStyleProjectionView> m_interactorStyle; ///< special InteractorStyle for Projected Volumes
	int m_orientation;
	QPushButton *m_menuButton;

	private:
	static void mouseMoveCallback(vtkObject* caller, unsigned long eid, void* clientdata, void *calldata);
};

#endif // MULTIPLANARREFORMATWIDGET_H
