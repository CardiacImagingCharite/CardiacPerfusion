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

#include "multiplanarreformatwidget.h"
#include <vtkMatrix4x4.h>
#include <vtkinteractorstyleprojectionview.h>
#include <vtkCommand.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapToColors.h>
#include <vtkScalarsToColors.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkTransform.h>
#include <vtkCursor2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>


#include <vtkCamera.h>

/** Default Constructor.
Nothing fancy - just basic setup */
MultiPlanarReformatWidget::MultiPlanarReformatWidget(QWidget* parent, Qt::WFlags f):
  QVTKWidget(parent, f),
  m_reslice(vtkImageReslice::New()),
  m_colormap(vtkImageMapToWindowLevelColors::New()),
  m_imageViewer(vtkSmartPointer<vtkImageViewer2>::New()),
  m_reslicePlaneTransform(vtkMatrix4x4::New()),
  m_interactorStyle(vtkSmartPointer<vtkInteractorStyleProjectionView>::New())
{
	m_reslice->SetOutputDimensionality(2);
	m_reslice->SetBackgroundLevel(-1000);
	m_reslice->SetInterpolationModeToCubic();
  
	m_colormap->SetOutputFormatToRGB();

	m_colormap->SetInputConnection(m_reslice->GetOutputPort());
	//m_imageViewer->GetRenderer()->ResetCamera();
	vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
	actor->SetInput(m_colormap->GetOutput());
	m_imageViewer->GetRenderer()->AddActor(actor);
	//m_imageViewer->GetImageActor()->SetInput(m_colormap->GetOutput());
	
	//m_actor->SetInput(m_colormap->GetOutput());
	//m_renderer->AddActor(m_actor);
	//m_imageViewer->GetRenderer()->ResetCamera();
	this->SetRenderWindow(m_imageViewer->GetRenderWindow());

	// Set up the interaction
	vtkTransform *transform = vtkTransform::New();
	transform->SetMatrix( m_reslicePlaneTransform );
	transform->RotateX(180);
	m_reslicePlaneTransform->DeepCopy( transform->GetMatrix() );
	transform->Delete();
  
	m_interactorStyle->SetImageMapToWindowLevelColors( m_colormap );
	m_interactorStyle->SetOrientationMatrix( m_reslicePlaneTransform );
	m_interactorStyle->SetImageViewer(m_imageViewer);

	//vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();//this->GetRenderWindow()->GetInteractor();
	vtkSmartPointer<vtkRenderWindowInteractor> interactor = this->GetRenderWindow()->GetInteractor();
  

	m_imageViewer->SetupInteractor(interactor);
  
	interactor->SetInteractorStyle(m_interactorStyle);
	m_interactorStyle->SetCurrentRenderer(m_imageViewer->GetRenderer());

	m_reslice->SetResliceAxes(m_reslicePlaneTransform);
	m_reslice->SetOutputDimensionality(2);


	//init widget with a black image to supress error messages (input is 0)
	vtkImageData* blank = vtkImageData::New();
	blank->SetDimensions(100, 100, 1);
	blank->AllocateScalars();
	for (int i = 0; i < 100; i++)
      for (int j = 0; j < 100; j++)
          blank->SetScalarComponentFromDouble(i, j, 0, 0, 255);
	blank->Update();
	setImage(blank);	//works but has effects on visualization 
	//m_reslice->SetInput(blank);
	m_imageViewer->Render();
	
}

/** Destructor*/
MultiPlanarReformatWidget::~MultiPlanarReformatWidget() {
  this->hide();
  //if (m_renderer) m_renderer->Delete();
  //if(m_imageViewer) m_imageViewer->Delete();
  m_overlays.clear();
  if (m_colormap) m_colormap->Delete();
  if (m_reslice) m_reslice->Delete();
  //if (m_actor) m_actor->Delete();
  //if (m_interactorStyle) m_interactorStyle->Delete();
  if (m_reslicePlaneTransform) m_reslicePlaneTransform->Delete();
}

void MultiPlanarReformatWidget::resizeEvent( QResizeEvent * event ) {
  QVTKWidget::resizeEvent(event);
  int xres = this->size().width();
  int yres = this->size().height();
  m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
  m_reslice->SetOutputOrigin(-xres/2.0,-yres/2.0,0);
  BOOST_FOREACH(OverlayMapType::value_type it, m_overlays) {
    it.second->resize( xres, yres );
  }
}

void MultiPlanarReformatWidget::setCubicInterpolation(bool cubic) {
  if (cubic) m_reslice->SetInterpolationModeToCubic();
  else m_reslice->SetInterpolationModeToLinear();
}


/** Volume Setter*/
void MultiPlanarReformatWidget::setImage(vtkImageData *image/**<[in] Volume (3D) Image with one component*/) {
  if (image==NULL) {
    m_image = NULL;
    //vtkRenderWindow *window = this->GetRenderWindow();
    //window->RemoveRenderer( m_imageViewer->GetRenderer() );
	//this->GetRenderWindow()->RemoveRenderer(
  } else {
    //vtkRenderWindow *window = this->GetRenderWindow();
    //window->RemoveRenderer( m_imageViewer->GetRenderer() );
    m_image = image;
    m_image->UpdateInformation();
    int extent[6];
    for(int i = 0; i < 3; i++)
      m_image->GetAxisUpdateExtent(i, extent[i*2], extent[i*2+1]);

    double spacing[3];
    double origin[3];
    m_image->GetSpacing(spacing);
    m_image->GetOrigin(origin);

    double center[3];
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]); 
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]); 
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]); 
	
	// Matrices for axial, coronal, sagittal, oblique view orientations
	/*static double axialElements[16] = {
	         1, 0, 0, 0,
	         0, 1, 0, 0,
	         0, 0, 1, 0,
	         0, 0, 0, 1 };
	*/
	static double axialElements[16] = {
	         1, 0, 0, 0,
	         0,-1, 0, 0,
	         0, 0, 1, 0,
	         0, 0, 0, 1 };

	static double coronalElements[16] = {
	         1, 0, 0, 0,
	         0, 0,-1, 0,
	         0, 1, 0, 0,	//-1
	         0, 0, 0, 1 };

	static double sagittalElements[16] = {
			0, 0,-1, 0,
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 1 };

	// Set the slice orientation
	switch(m_orientation)
	{
	case 0: m_reslicePlaneTransform->DeepCopy(axialElements);
		break;
	case 1: m_reslicePlaneTransform->DeepCopy(coronalElements);
		break;
	case 2: m_reslicePlaneTransform->DeepCopy(sagittalElements);
		break;
	}
	
    // Set the point through which to slice
    m_reslicePlaneTransform->SetElement(0, 3, center[0]);
    m_reslicePlaneTransform->SetElement(1, 3, center[1]);
    m_reslicePlaneTransform->SetElement(2, 3, center[2]);

    m_reslice->SetInput( m_image );
    m_reslice->SetOutputSpacing(1,1,1);
	m_imageViewer->SetInput(m_reslice->GetOutput());
	
	//m_imageViewer->GetRenderWindow()->GetInteractor()->Initialize();
	m_imageViewer->Render();

    //window->AddRenderer(m_imageViewer->GetRenderer());


  }
  this->update();
}

void MultiPlanarReformatWidget::setOrientation(int orientation)
{
	m_orientation = orientation;
	//m_imageViewer->SetSliceOrientation(orientation);
}

int MultiPlanarReformatWidget::addBinaryOverlay(vtkImageData *image, const QColor &color, const ActionDispatch &dispatch) {
  if (m_overlays.find( image ) == m_overlays.end() ) {
    int actionHandle;
    RGBType rgbColor;
    rgbColor[0] = color.red();
    rgbColor[1] = color.green();
    rgbColor[2] = color.blue();
    boost::shared_ptr< vtkBinaryImageOverlay > overlay(
		new vtkBinaryImageOverlay( m_imageViewer->GetRenderer(), m_interactorStyle, dispatch, image, m_reslicePlaneTransform, rgbColor, actionHandle ) );
    m_overlays.insert( OverlayMapType::value_type( image, overlay ) );
    overlay->resize( this->size().width(), this->size().height() );
    this->update();
    return actionHandle;
  }
  return -1;
}

void MultiPlanarReformatWidget::removeBinaryOverlay(vtkImageData *image) {
  m_overlays.erase(image);
  this->update();
}

void MultiPlanarReformatWidget::activateOverlayAction(vtkImageData *image) {
  OverlayMapType::iterator it = m_overlays.find( image );
  if (it != m_overlays.end()) {
    it->second->activateAction();
  }
}

void MultiPlanarReformatWidget::activateAction(int actionHandle) {
  m_interactorStyle->activateAction(actionHandle);
}

int MultiPlanarReformatWidget::addAction(const ActionDispatch &dispatch) {
  return m_interactorStyle->addAction(dispatch);
}

void MultiPlanarReformatWidget::removeAction(int actionHandle) {
  m_interactorStyle->removeAction(actionHandle);
}

void MultiPlanarReformatWidget::resetActions(){
	m_interactorStyle->resetActions();
}

void MultiPlanarReformatWidget::showCircle(int radius){

  vtkSmartPointer<vtkCursor2D> cursor = 
    vtkSmartPointer<vtkCursor2D>::New();

  cursor->SetModelBounds(-10,10,-10,10,0,0);
  cursor->AllOn();
  //cursor->OutlineOff();

  cursor->Update();
 

  vtkSmartPointer<vtkPolyDataMapper> cursorMapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();
  cursorMapper->SetInputConnection(cursor->GetOutputPort());
  vtkSmartPointer<vtkActor> cursorActor = 
    vtkSmartPointer<vtkActor>::New();
  cursorActor->GetProperty()->SetColor(1,0,0);
  cursorActor->SetMapper(cursorMapper);
  m_imageViewer->GetRenderer()->AddActor(cursorActor);

}