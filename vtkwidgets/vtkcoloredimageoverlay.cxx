
#include "vtkcoloredimageoverlay.h"
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkScalarsToColors.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkScalarBarActor.h>
#include "vtkinteractorstyleprojectionview.h"
#include <boost/bind.hpp>


vtkColoredImageOverlay::vtkColoredImageOverlay( vtkRenderer *renderer,
			  vtkInteractorStyleProjectionView *interactorStyle,
			  const ActionDispatch &action, vtkImageData *image, vtkMatrix4x4 *reslicePlaneTransform,
			  int &actionHandle, double opacity)
  :m_image(image),
  m_reslice(vtkImageReslice::New()),
  m_colorMapper(vtkImageMapToColors::New()),
  m_colorMap(vtkLookupTable::New()),
  m_actor(vtkImageActor::New()),
  m_renderer( renderer ),
  m_interactorStyle(interactorStyle), actionHandle(-1) {
    
  //Set number of created colors
  m_colorMap->SetNumberOfTableValues(100);
  //Set the range of the image values
  m_colorMap->SetTableRange( 0, 10);
  //Set the range of the available colors
  m_colorMap->SetHueRange(0.0,0.667);
  //Set the saturation range of the colors
  //m_colorMap->SetSaturationRange( 0.7, 1 );
  //set the brightness of the colors
  m_colorMap->SetValueRange( 0.8, 1 );
  //generate LUT
  m_colorMap->Build();
  //in order to hide the maximum and minimum values, 
  //set the alpha of the borders to zero
  m_colorMap->SetTableValue(0,0,0,0,0);
  m_colorMap->SetTableValue(99,0,0,0,0);

  m_colorMapper->SetInputConnection( m_reslice->GetOutputPort());
  m_colorMapper->SetLookupTable( m_colorMap );

  m_actor->SetOpacity(opacity);
  
  m_reslice->SetOutputDimensionality(2);
  m_reslice->SetBackgroundLevel(-1000);
  
  //m_colormap->SetLookupTable(m_lookup);
  //m_colormap->SetInputConnection(m_reslice->GetOutputPort());

  m_actor->SetInput(m_colorMapper->GetOutput());
  m_reslice->SetResliceAxes(reslicePlaneTransform);
  m_image->UpdateInformation();
  m_reslice->SetInput( m_image );

  m_reslice->SetOutputSpacing(1,1,1);

  //create legend for LUT
  vtkSmartPointer<vtkScalarBarActor> legend =
	  vtkSmartPointer<vtkScalarBarActor>::New();
  
  //create legend for the LUT
  legend->SetLookupTable(m_colorMap);
  legend->SetTitle("Values");
  legend->SetOrientationToHorizontal();
  legend->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  legend->GetPositionCoordinate()->SetValue(0.57, 0.87);
  legend->SetWidth(0.4);
  legend->SetHeight(0.15);

  if (m_renderer)
  {
    m_renderer->AddActor(m_actor);
	m_renderer->AddActor(legend);
  }

  if (action.valid) {
    action.sig->connect( boost::bind(&vtkImageReslice::Modified, m_reslice) );
    this->actionHandle = interactorStyle->addAction(action);
  }
  actionHandle = this->actionHandle;
}

vtkColoredImageOverlay::~vtkColoredImageOverlay() {
  if (m_renderer) {
    m_renderer->RemoveActor( m_actor );
  }
  if (actionHandle!=-1 && m_interactorStyle) 
    m_interactorStyle->removeAction( actionHandle );
  if (m_reslice) m_reslice->Delete();
  if (m_colorMap) m_colorMap->Delete();
  if (m_colorMapper) m_colorMapper->Delete();
  if (m_actor) m_actor->Delete();
}

void vtkColoredImageOverlay::resize( unsigned int xres, unsigned int yres ) {

	m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
	m_reslice->SetOutputOrigin(xres/-2.0,yres/-2.0,0);
}

void vtkColoredImageOverlay::activateAction() {
  m_interactorStyle->activateAction(actionHandle);
}

bool vtkColoredImageOverlay::operator<(const vtkColoredImageOverlay &other) const {
  return this->m_image < other.m_image;
}