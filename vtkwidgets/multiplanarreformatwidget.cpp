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

#include "multiplanarreformatwidget.h"
#include <vtkMatrix4x4.h>
#include <vtkMatrix3x3.h>
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
#include <vtkRegularPolygonSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>
#include <vtkSphereSource.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>


#include <vtkCamera.h>

/** Default Constructor.
Nothing fancy - just basic setup */
MultiPlanarReformatWidget::MultiPlanarReformatWidget(QWidget* parent, Qt::WFlags f):
  QVTKWidget(parent, f),
  m_reslice(vtkImageReslice::New()),
  m_imageViewer(vtkSmartPointer<vtkImageViewer2>::New()),
  m_annotation(vtkSmartPointer<vtkCornerAnnotation>::New()),
  m_reslicePlaneTransform(vtkMatrix4x4::New()),
  m_interactorStyle(vtkSmartPointer<vtkInteractorStyleProjectionView>::New())
{
	m_sliceCount = 0;
	//create button and add menu to it
	m_menuButton = new QPushButton(this);
	m_thickness=1;
	m_menuButton->resize(20,20);
	m_menuButton->move(this->size().width()-20,this->size().height()-20);

	QAction *resetViewAction = new QAction("Reset View", m_menuButton);
	connect(resetViewAction, SIGNAL(triggered()), this, SLOT(resetView()));

	QMenu *menu = new QMenu(this);
	menu->addAction(resetViewAction);
	menu->addAction("action 2");
	menu->addAction("action 3");

	m_menuButton->setMenu(menu);

	// Set up reslice
	m_reslice->SetOutputDimensionality(2);
	m_reslice->SetBackgroundLevel(-1000);
	m_reslice->SetInterpolationModeToCubic();
	m_reslice->SetSlabModeToMean();
	m_reslice->SetSlabNumberOfSlices(m_thickness);
	// bind vtkRenderWindow to Qt window
	this->SetRenderWindow(m_imageViewer->GetRenderWindow());

	// Set up the interaction
//	m_interactorStyle->SetImageMapToWindowLevelColors( m_colormap );
	m_interactorStyle->SetOrientationMatrix( m_reslicePlaneTransform );
	m_interactorStyle->SetImageViewer(m_imageViewer);

	// Set up annotation (Window/Level)
	m_annotation->SetLinearFontScaleFactor( 2 );
	m_annotation->SetNonlinearFontScaleFactor( 1 );
	m_annotation->SetMaximumFontSize( 12 );
	//m_annotation->SetText( 0, "Off Image" );
	m_annotation->SetText( 2, "<window>\n<level>" );
	//m_annotation->SetText( 3, "<image_and_max>" );
	m_annotation->GetTextProperty()->SetColor( 1,0,0);

	m_interactorStyle->SetAnnotation(m_annotation);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor = this->GetRenderWindow()->GetInteractor();
  
	interactor->SetInteractorStyle(m_interactorStyle);
	m_interactorStyle->SetCurrentRenderer(m_imageViewer->GetRenderer());

	m_reslice->SetResliceAxes(m_reslicePlaneTransform);

	//init widget with a black image to supress error messages (input is 0)
	vtkImageData* blank = vtkImageData::New();
	blank->SetDimensions(100, 100, 1);
	blank->AllocateScalars();
	for (int i = 0; i < 100; i++)
	  for (int j = 0; j < 100; j++)
	    blank->SetScalarComponentFromDouble(i, j, 0, 0, 0);
	blank->Update();
 	setImage(blank);
	m_imageViewer->Render();
}

/** Destructor*/
MultiPlanarReformatWidget::~MultiPlanarReformatWidget() {
  this->hide();
  m_binaryOverlays.clear();
  m_coloredOverlays.clear();
  if (m_reslice) m_reslice->Delete();
  if (m_reslicePlaneTransform) m_reslicePlaneTransform->Delete();
}

void MultiPlanarReformatWidget::resizeEvent( QResizeEvent * event ) {
  QVTKWidget::resizeEvent(event);
  int xres = this->size().width();
  int yres = this->size().height();
  this->m_menuButton->move(xres-20,yres-20);
  m_reslice->SetOutputExtent(0,xres,0,yres,0,0);
  m_reslice->SetOutputOrigin(-xres/2.0,-yres/2.0,0);
  
  m_imageViewer->UpdateDisplayExtent();
  
	BOOST_FOREACH(BinaryOverlayMapType::value_type it, m_binaryOverlays) {
		it.second->resize( xres, yres );
	}

	BOOST_FOREACH(ColoredOverlayMapType::value_type it, m_coloredOverlays) {
		it.second->resize( xres, yres );
	}
}

void MultiPlanarReformatWidget::setCubicInterpolation(bool cubic) {
  if (cubic) m_reslice->SetInterpolationModeToCubic();
  else m_reslice->SetInterpolationModeToLinear();
}


/** Volume Setter*/
void MultiPlanarReformatWidget::setImage(vtkImageData *image/**<[in] Volume (3D) Image with one component*/,
					 bool changeTranslation) {
  if (image==NULL) {
    m_image = NULL;
    vtkRenderWindow *window = this->GetRenderWindow();
    window->RemoveRenderer( m_imageViewer->GetRenderer() );
  } else {
    vtkRenderWindow *window = this->GetRenderWindow();
    window->RemoveRenderer( m_imageViewer->GetRenderer() );
    m_image = image;
    m_image->UpdateInformation();

    if ( changeTranslation ) setTranslation();
      
    m_reslice->SetInput( m_image );
   //  m_reslice->SetOutputSpacing(m_image->GetSpacing());
   m_reslice->SetOutputSpacing(1,1,1);
		m_imageViewer->SetInput(m_reslice->GetOutput());

    window->AddRenderer(m_imageViewer->GetRenderer());
  }
  int extent[6];
  for(int i = 0; i < 3; i++)
    m_image->GetAxisUpdateExtent(i, extent[i*2], extent[i*2+1]); 
  
  if(m_orientation ==0)//axial
	{
				this->m_interactorStyle->SetIncrement(m_image->GetSpacing()[2]);
				this->m_interactorStyle->SetSliceNumber((extent[1] - extent[0])/2);
					m_reslice->SetOutputSpacing(1,1,m_image->GetSpacing()[2]);				
	}
	else if (m_orientation==1)//coronal
	{
				this->m_interactorStyle->SetIncrement(m_image->GetSpacing()[1]);
				this->m_interactorStyle->SetSliceNumber( (extent[3] - extent[2])/2);
					m_reslice->SetOutputSpacing(1,1,m_image->GetSpacing()[0]);
				
	}
	else if (m_orientation==2)//sagittal
	{
				this->m_interactorStyle->SetIncrement(m_image->GetSpacing()[0]);
				this->m_interactorStyle->SetSliceNumber((extent[5] - extent[4])/2);
					m_reslice->SetOutputSpacing(1,1,m_image->GetSpacing()[1]);				
	}


  this->update();
	
}

void MultiPlanarReformatWidget::setTranslation() {

  int extent[6];
  for(int i = 0; i < 3; i++)
    m_image->GetAxisUpdateExtent(i, extent[i*2], extent[i*2+1]); 
  
  double spacing[3];
  double origin[3];
  m_image->GetSpacing(spacing);
  m_image->GetOrigin(origin);

  double center[3];
  center[0] = origin[0] + spacing[0] * 0.5 * (extent[1] - extent[0]); 
  center[1] = origin[1] + spacing[1] * 0.5 * (extent[3] - extent[2]); 
  center[2] = origin[2] + spacing[2] * 0.5 * (extent[5] - extent[4]); 
      
  // Set the point through which to slice
  m_reslicePlaneTransform->SetElement(0, 3, center[0]);
  m_reslicePlaneTransform->SetElement(1, 3, center[1]);
  m_reslicePlaneTransform->SetElement(2, 3, center[2]);
}

void MultiPlanarReformatWidget::ResetOrientation() {
    
    vtkTransform *transform = vtkTransform::New();
    vtkSmartPointer<vtkMatrix4x4> IdentMatrix = vtkMatrix4x4::New();
    IdentMatrix->Identity();
    transform->SetMatrix( IdentMatrix );
    transform->RotateX(180);
    m_reslicePlaneTransform->DeepCopy( transform->GetMatrix() );
    transform->Delete();
}

void MultiPlanarReformatWidget::setOrientation(int orientation)
{
        ResetOrientation();
    
	m_orientation = orientation;
	// Matrices for axial, coronal, sagittal, oblique view orientations
	static double axialElements[16] = {
	         1, 0, 0, 0,
	         0, 1, 0, 0,
	         0, 0, 1, 0,
	         0, 0, 0, 1 };
		 
	static double coronalElements[16] = {
	         1, 0, 0, 0,
	         0, 0, 1, 0,
	         0,-1, 0, 0,
	         0, 0, 0, 1 };

	static double sagittalElements[16] = {
		 0, 0,-1, 0,
		 1, 0, 0, 0,
		 0,-1, 0, 0,
		 0, 0, 0, 1 };

	vtkSmartPointer<vtkMatrix4x4> axialMatrix = vtkMatrix4x4::New();
	axialMatrix->DeepCopy(axialElements);

	vtkSmartPointer<vtkMatrix4x4> coronalMatrix = vtkMatrix4x4::New();
	coronalMatrix->DeepCopy(coronalElements);
	
	vtkSmartPointer<vtkMatrix4x4> sagittalMatrix = vtkMatrix4x4::New();
	sagittalMatrix->DeepCopy(sagittalElements);

	switch(m_orientation)
	{
	case 0: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, axialMatrix, m_reslicePlaneTransform);
		break;
	case 1: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, coronalMatrix, m_reslicePlaneTransform);
		break;
	case 2: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, sagittalMatrix, m_reslicePlaneTransform);
		break;
	}
}


void MultiPlanarReformatWidget::setAlignHeartOrientation()
{
        ResetOrientation();
	
	// Matrices for 3-chamber, 4-chamber and axial axes view
	static double ThreeChamberElements[16] = {
	         1, 0, 0, 0,
	         0, 1, 0, 0,
	         0, 0, 1, 0,
	         0, 0, 0, 1 };
		 
	static double FourChamberElements[16] = {
	         0, 1, 0, 0,
	         0, 0,-1, 0,
	        -1, 0, 0, 0,
	         0, 0, 0, 1 };

	static double AxialElements[16] = {
		 0, 0, 1, 0,
	         0, 1, 0, 0,
	        -1, 0, 0, 0,
		 0, 0, 0, 1 };

	vtkSmartPointer<vtkMatrix4x4> ThreeChamberMatrix = vtkMatrix4x4::New();
	ThreeChamberMatrix->DeepCopy(ThreeChamberElements);

	vtkSmartPointer<vtkMatrix4x4> FourChamberMatrix = vtkMatrix4x4::New();
	FourChamberMatrix->DeepCopy(FourChamberElements);
	
	vtkSmartPointer<vtkMatrix4x4> AxialMatrix = vtkMatrix4x4::New();
	AxialMatrix->DeepCopy(AxialElements);

	switch(m_orientation)
	{
	case 0: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, ThreeChamberMatrix, m_reslicePlaneTransform);
		break;
	case 1: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, FourChamberMatrix, m_reslicePlaneTransform);
		break;
	case 2: vtkMatrix4x4::Multiply4x4(m_reslicePlaneTransform, AxialMatrix, m_reslicePlaneTransform);
		break;
	}
}

int MultiPlanarReformatWidget::addBinaryOverlay(vtkImageData *image, const QColor &color, const ActionDispatch &dispatch) {
  if (m_binaryOverlays.find( image ) == m_binaryOverlays.end() ) {
    int actionHandle;
    RGBType rgbColor;
    rgbColor[0] = color.red();
    rgbColor[1] = color.green();
    rgbColor[2] = color.blue();
    boost::shared_ptr< vtkBinaryImageOverlay > overlay(
		new vtkBinaryImageOverlay( m_imageViewer->GetRenderer(), m_interactorStyle, dispatch, image, m_reslicePlaneTransform, rgbColor, actionHandle ) );
    m_binaryOverlays.insert( BinaryOverlayMapType::value_type( image, overlay ) );
    overlay->resize( this->size().width(), this->size().height() );
    this->update();
    return actionHandle;
  }
  return -1;
}

void MultiPlanarReformatWidget::removeBinaryOverlay(vtkImageData *image) {
  m_binaryOverlays.erase(image);
  this->update();
}

int MultiPlanarReformatWidget::addColoredOverlay(vtkImageData *image, vtkLookupTable* customColorMap, const ActionDispatch &dispatch)
{
	if (m_coloredOverlays.find( image ) == m_coloredOverlays.end() ) {
		int actionHandle;
		boost::shared_ptr< vtkColoredImageOverlay > overlay(
			new vtkColoredImageOverlay( m_imageViewer->GetRenderer(), m_interactorStyle, dispatch, image, m_reslicePlaneTransform, actionHandle, customColorMap ) );
    
		//overlay->setColorMap(customColorMap);
		m_coloredOverlays.insert( ColoredOverlayMapType::value_type( image, overlay ) );
		overlay->resize( this->size().width(), this->size().height() );
		//overlay->showLegend();

		this->update();

		m_interactorStyle->SetColorMap(overlay->getColorMap());
		m_interactorStyle->SetOverlayImage(image);
		m_interactorStyle->SetPerfusionOverlay(overlay);
		return actionHandle;
  }
  return -1;
}


void MultiPlanarReformatWidget::removeColoredOverlay(vtkImageData *image) {
	
	ColoredOverlayMapType::const_iterator it = m_coloredOverlays.find( image );
		//if segment was found
	if (it != m_coloredOverlays.end()) {
		it->second->hideLegend();
	}
	m_coloredOverlays.erase(image);
	this->update();
}

void MultiPlanarReformatWidget::activateOverlayAction(vtkImageData *image) {
  BinaryOverlayMapType::iterator binIt = m_binaryOverlays.find( image );
  if (binIt != m_binaryOverlays.end()) {
    binIt->second->activateAction();
  }

  ColoredOverlayMapType::iterator colIt = m_coloredOverlays.find( image );
  if (colIt != m_coloredOverlays.end()) {
    colIt->second->activateAction();
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


void MultiPlanarReformatWidget::mouseDoubleClickEvent( QMouseEvent * e ) {
	if(e->button() == Qt::LeftButton)
	{
		emit doubleClicked(*this);
	}
}

void MultiPlanarReformatWidget::resetView()
{
	std::cout << "ButtonTest: Reset View" << std::endl;
}

void MultiPlanarReformatWidget::updateWidget()
{ 

	this->update();
}

void MultiPlanarReformatWidget::Multiply3x3of4x4Matrix(vtkMatrix4x4 *a4, vtkMatrix4x4 *b4, vtkMatrix4x4 *c4) {

  // create 3x3 matrices
  vtkSmartPointer<vtkMatrix3x3> a3 = vtkMatrix3x3::New();
  vtkSmartPointer<vtkMatrix3x3> b3 = vtkMatrix3x3::New();
  vtkSmartPointer<vtkMatrix3x3> c3 = vtkMatrix3x3::New();

  // copy elements from 4x4 to 3x3 (input)matrices
  for (int i = 0; i < 3; i++) 
    for (int j = 0; j < 3; j++) {
      a3->SetElement( i, j, a4->GetElement(i, j) );
      b3->SetElement( i, j, b4->GetElement(i, j) );
    }

  // multipy 3x3 matrices
  vtkMatrix3x3::Multiply3x3(a3, b3, c3);

  // copy elements from 3x3 to 4x4 (output)matrix
  for (int i = 0; i < 3; i++) 
    for (int j = 0; j < 3; j++)
      c4->SetElement( i, j, c3->GetElement(i, j) );
      
}

void MultiPlanarReformatWidget::rotateImage(const double RotationTrafoElements[]) {
	
  // set m_reslicePlaneTransform back to basic orientation
  setAlignHeartOrientation();
  setTranslation();
  
  // create transformation matrix and set elements
  vtkSmartPointer<vtkMatrix4x4> trafoMatrix = vtkMatrix4x4::New();
  trafoMatrix->SetElement(0, 0, RotationTrafoElements[0]);
  trafoMatrix->SetElement(0, 1, RotationTrafoElements[1]);
  trafoMatrix->SetElement(0, 2, RotationTrafoElements[2]);
  trafoMatrix->SetElement(1, 0, RotationTrafoElements[3]);
  trafoMatrix->SetElement(1, 1, RotationTrafoElements[4]);
  trafoMatrix->SetElement(1, 2, RotationTrafoElements[5]);
  trafoMatrix->SetElement(2, 0, RotationTrafoElements[6]);
  trafoMatrix->SetElement(2, 1, RotationTrafoElements[7]);
  trafoMatrix->SetElement(2, 2, RotationTrafoElements[8]);
  trafoMatrix->SetElement(3, 3, 1);

  // apply transformation
  Multiply3x3of4x4Matrix(trafoMatrix, m_reslicePlaneTransform, m_reslicePlaneTransform);

  m_imageViewer->Render();

}

void MultiPlanarReformatWidget::panImage(const double pos[3])
{
	vtkTransform *transform = vtkTransform::New();
	transform->SetMatrix(m_reslicePlaneTransform);
	double oldPos[3];
	transform->GetPosition(oldPos);
	double trans[3];
	trans[0] = pos[0] - oldPos[0];
	trans[1] = pos[1] - oldPos[1];
	trans[2] = pos[2] - oldPos[2];
	transform->Translate(trans);
	transform->GetMatrix(m_reslicePlaneTransform);
	transform->Delete();
	m_imageViewer->Render();
}

void MultiPlanarReformatWidget::scaleImage(const double length)
{
	if (length > 0) 
	{
		double b = 145; // reference length in mm
		vtkTransform *transform = vtkTransform::New();
		transform->SetMatrix(m_reslicePlaneTransform);
		double oldScale = *transform->GetScale();
		double newScale = length / ( b *  oldScale );
		transform->Scale(newScale, newScale, newScale);
		double scalefac[3];
		transform->GetScale(scalefac);
		transform->GetMatrix(m_reslicePlaneTransform);
		transform->Delete();
		m_imageViewer->Render();
	}
}
void MultiPlanarReformatWidget::SetThickness(int s)
{
	if (s==0)
	{
		m_thickness=1;
		std::cout<<"Slice thickness is now overall 1"<<std::endl;

	}
	else if( m_orientation == 0)//axial
	{
	m_thickness =  std::abs(std::floor(s/m_image->GetSpacing()[2]));
	std::cout<<"Axial slice thickness is "<< m_thickness<< " slices"<<std::endl;
	}
	else if (m_orientation == 1)//sagittal
	{
		m_thickness = std::abs( std::floor(s/m_image->GetSpacing()[0]));
	std::cout<<"sagittal slice thickness is "<< m_thickness<< " slices"<<std::endl;
		
	}
	else if (m_orientation == 2) //coronal
	{
		m_thickness =  std::abs(std::floor(s/m_image->GetSpacing()[1]));
	std::cout<<"Coronal slice thickness is "<< m_thickness<< " slices"<<std::endl;
	}
	m_reslice->SetSlabNumberOfSlices(m_thickness);
	return;
}