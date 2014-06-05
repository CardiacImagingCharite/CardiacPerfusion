/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

#include "vtkMatrix4x4.h"

class vtkImageData;
class vtkImageReslice;
class vtkImageMapToWindowLevelColors;
class vtkImageMapToColors;
class vtkBinaryImageToColor;
class vtkMatrix4x4;
class vtkCornerAnnotation;
class CTImageTreeModel;
class SegmentInfo;


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
	\param changeTranslation true if the translation of the image should be recalculated
	*/
	void setImage(vtkImageData *image, bool changeTranslation = true);
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
	//int addColoredOverlay(vtkImageData *image, const ActionDispatch &dispatch = ActionDispatch() );
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
	///Reset the orientation matrix
	void ResetOrientation();
	///Sets the orientation of the slices, oblique view
	/*!
	\param orientation A number that appoints the desired orientation
	*/
	void setOrientation(int orientation);
	///Sets the orientation of the slices for autoAlignHeart view
	void setAlignHeartOrientation();
	///Sets the translation vector of the image m_reslicePlaneTransform
	void setTranslation();
	///Gets the interactor style of the widget.
	/*!
	\return Interactor style.
	*/
	vtkSmartPointer<vtkInteractorStyleProjectionView> GetInteractorStyle() { return this->m_interactorStyle; }
	///Sets the root item of the CT image tree.
	/*!
	\param root The root item.
	*/
	void SetRootItem(TreeItem* root) {m_interactorStyle->SetRootItem(root); }
	///Sets the QwtPlot for displaying the TACs.
	/*!
	\param plot The QwtPlot.
	*/
	void SetPlot(QwtPlot* plot) { m_interactorStyle->SetPlot(plot); }
	///Sets the artery segment.
	/*!
	\param seg The artery Segment of the perfusion map.
	*/
	void SetArterySegment(BinaryImageTreeItem* seg) { m_interactorStyle->SetArterySegment(seg); }
	///Rotates the image by changing m_reslicePlaneTransform, e.g. after autoAlignHeart
	/*!
	\param RotationTrafoElements Elements of a 3x3-rotation matrix
	*/
        void rotateImage(const double RotationTrafoElements[]);
	///Multiplies only the top left 3x3 submatrices of 4x4 matrices
	/*!
	\param a4 Left 4x4 matrix of Multiplication
	\param b4 Right 4x4 matrix of Multiplication
	\param c4 Resulting 4x4 matrix of Multiplication
	*/
        void Multiply3x3of4x4Matrix(vtkMatrix4x4 *a4, vtkMatrix4x4 *b4, vtkMatrix4x4 *c4);
	///Pan the image
	/*!
	\param pos new transpose
	*/
	void panImage(const double pos[3]);
	///Scale the image
	/*!
	\param length reference for scaling
	*/
	void scaleImage(const double length);

public slots:
	///Resets the view.
	void resetView();
	void updateWidget();
	void setThickness(int s){m_thickness=s;};
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
	vtkSmartPointer<vtkImageViewer2> m_imageViewer; ///< the uses image viewer
	vtkSmartPointer<vtkCornerAnnotation> m_annotation;
	vtkMatrix4x4 *m_reslicePlaneTransform; ///< Tranformation Matrix for the Reslicing Plane
	vtkSmartPointer<vtkInteractorStyleProjectionView> m_interactorStyle; ///< special InteractorStyle for Projected Volumes
	int m_orientation;
	QPushButton *m_menuButton;
	int m_thickness;

private:
	static void mouseMoveCallback(vtkObject* caller, unsigned long eid, void* clientdata, void *calldata);
};

#endif // MULTIPLANARREFORMATWIDGET_H
