/*
 * Copyright 2007 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

#include "ui_KardioPerfusion.h"
#include "KardioPerfusion.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkVectorText.h>
#include "vtkSmartPointer.h"
#include "vtkKWImage.h"
#include "vtkImageData.h"

#include "vtkCamera.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkObjectFactory.h"
#include "vtkDICOMImageReader.h"
#include "vtkVolumeRayCastCompositeFunction.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkImageShiftScale.h"
#include "vtkVolume16Reader.h"

#include "itkGDCMSeriesFileNames.h"

#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"
#include "itkDicomImageIO2.h"


#include "QFileDialog.h"
#include "qstring.h"

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera
{
  public:
    static KeyPressInteractorStyle* New();
    vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
 
    virtual void OnKeyPress() 
    {
      // Get the keypress
      vtkRenderWindowInteractor *rwi = this->Interactor;
      std::string key = rwi->GetKeySym();

      // Output the key that was pressed
      std::cout << "Pressed " << key << std::endl;
 
      // Handle an arrow key
      if(key == "Up")
        {
        std::cout << "The up arrow was pressed." << std::endl;
        }
 
      // Handle a "normal" key
      if(key == "a")
        {
        std::cout << "The a key was pressed." << std::endl;
		m_viewer->SetSliceOrientation((m_viewer->GetSliceOrientation()+1)%3);
        }
 
      // Forward events
      vtkInteractorStyleTrackballCamera::OnKeyPress();
    }
 
	virtual void OnMouseWheelForward()
	{
		m_viewer->SetSlice(m_viewer->GetSlice()+1);
		m_viewer->Render();
		
	}
	
	virtual void OnMouseWheelBackward()
	{
		m_viewer->SetSlice(m_viewer->GetSlice()-1);
		m_viewer->Render();
	}

	virtual void OnLeftButtonDown()
	{
	
	}

	virtual void OnLeftButtonUp()
	{
	
	}

	void setImageViewer(vtkImageViewer2 * viewer)
	{
		this->m_viewer = viewer;
	}

private:
	vtkImageViewer2* m_viewer;
};
vtkStandardNewMacro(KeyPressInteractorStyle);

// Constructor
SimpleView::SimpleView() 
{
  this->showFullScreen();
  this->ui = new Ui_SimpleView;
  this->ui->setupUi(this);
  
  for(int i = 0; i < 4; i++)
  {
	  m_pViewer[i] = vtkSmartPointer<vtkImageViewer2>::New();
	  m_pViewer[i]->GetRenderer()->SetBackground(0,0,0);
	  m_pViewer[i]->Render();
  }

 
  // Set up action signals and slots
  connect(this->ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  
};

SimpleView::~SimpleView()
{
  // The smart pointers should clean up

}

// Action to be taken upon file open 
void SimpleView::slotOpenFile()
{
/*	QString dirName =
	 QFileDialog::getExistingDirectory(QDir::currentDirPath(), this, "get existing directory",
	 "Choose a directory");
	*/ 
	QString fname = QFileDialog::getOpenFileName(
    this,
    tr("Select a file to open"),
    ".",
    "", 0, QFileDialog::ReadOnly|QFileDialog::HideNameFilterDetails);
	
	if (!fname.isEmpty()) {
		SimpleView::loadFile(fname);
	}
}

void SimpleView::slotExit() {
  qApp->exit();
}

void SimpleView::loadFile(QString fname){

	// define Pixeltype and set dimension
	typedef signed short PixelType;
	const unsigned int Dimension = 3;

	//define the used Imagetype
    typedef itk::Image<PixelType,Dimension> ImageType;

	//define the Readertype and create Reader object
	typedef itk::ImageFileReader<ImageType> ReaderType;
	//typedef itk::ImageSeriesReader<ImageType> ReaderType;
	
	
	ReaderType::Pointer reader = ReaderType::New();

	//define ImageIOType and create IO object
	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();

	//set input format of the images to Dicom 
	reader->SetImageIO(dicomIO);

	//std::string s = fname.ascii();

	reader->SetFileName(fname.ascii());
	
	//--------------------------------
/*		//define NamesGenerator and create object
	typedef itk::GDCMSeriesFileNames NamesGeneratorType;
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

	//Set path to the Dicom images
	nameGenerator->SetUseSeriesDetails(true);
	nameGenerator->SetDirectory(fname.ascii());

	try
    {
		std::cout << std::endl << "The directory: " << std::endl;
		std::cout << std::endl << fname.ascii() << std::endl << std::endl;
		std::cout << "Contains the following DICOM Series: ";
		std::cout << std::endl << std::endl;

		//try to get containing Dicom series
		typedef std::vector< std::string >    SeriesIdContainer;
		
		//get the unique identifiers
		const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
		
		//create an iterator for the UIDs
		SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
		SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
		
		//iterate through the found Dicom series
		while( seriesItr != seriesEnd )
		{
		  std::cout << seriesItr->c_str() << std::endl;
		  seriesItr++;
		}

		//select the first identifier
		std::string seriesIdentifier = seriesUID.begin()->c_str();

		//define a container for the filenames
		typedef std::vector<std::string> FileNamesContainer;
		FileNamesContainer fileNames;
		//generate the filenames of the serie and set it to the reader
		fileNames = nameGenerator->GetFileNames(seriesIdentifier);

		reader->SetFileNames(fileNames);
}
		catch (itk::ExceptionObject &ex)
		{
		  std::cout << ex << std::endl;
		  return;
		}
*/
	//--------------------------------
/*	try
		{
			//try to read the images
			reader->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
		  std::cout << ex << std::endl;
		  return;
		}
*/	
	//create a new vtkKWImage, that can be used as an interface between ITK and VTK
	vtkKWImage* kwImage = vtkKWImage::New();
	kwImage->SetITKImageBase(reader->GetOutput());

	// An interactor and a style
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor[4];

	vtkSmartPointer<KeyPressInteractorStyle> style[3]; 

	//get the output of the itkKWImage in the VTK format
	for(int i = 0; i < 3; i++)
	{
		renderWindowInteractor[i] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		style[i] =  vtkSmartPointer<KeyPressInteractorStyle>::New();

		m_pViewer[i]->SetInput(kwImage->GetVTKImage());
		m_pViewer[i]->SetupInteractor(renderWindowInteractor[i]);
		style[i]->setImageViewer(m_pViewer[i]);

		renderWindowInteractor[i]->SetInteractorStyle(style[i]);
		style[i]->SetCurrentRenderer(m_pViewer[i]->GetRenderer());

		m_pViewer[i]->GetRenderer()->ResetCamera();
		m_pViewer[i]->SetColorLevel(128);
		m_pViewer[i]->SetColorWindow(254);
	}
	
	 //set the imageviewers to the specific Widgets
	m_pViewer[0]->SetSliceOrientationToXY();
	this->ui->qvtk_axial->SetRenderWindow(m_pViewer[0]->GetRenderWindow());
	m_pViewer[0]->Render();
	renderWindowInteractor[0]->Initialize();

	m_pViewer[1]->SetSliceOrientationToXZ();
	this->ui->qvtk_sagittal->SetRenderWindow(m_pViewer[1]->GetRenderWindow());
	m_pViewer[1]->Render();
	renderWindowInteractor[1]->Initialize();

	m_pViewer[2]->SetSliceOrientationToYZ();
	this->ui->qvtk_coronal->SetRenderWindow(m_pViewer[2]->GetRenderWindow());
	m_pViewer[2]->Render();
	renderWindowInteractor[2]->Initialize();

  //renderWindowInteractor->SetRenderWindow(pImageViewer->GetRenderWindow());
 // renderWindowInteractor->InvokeEvent(vtkCommand::MouseWheelForwardEvent);

	vtkImageShiftScale *scale = vtkImageShiftScale::New();
	scale->SetInput(kwImage->GetVTKImage());
	scale->SetOutputScalarTypeToUnsignedChar();
	
 // Create the renderer, the render window, and the interactor. The renderer
  // draws into the render window, the interactor enables mouse- and 
  // keyboard-based interaction with the scene.
  vtkSmartPointer<vtkRenderer> ren =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  vtkSmartPointer<vtkInteractorStyleTrackballCamera> irenStyle = 
	  vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

  iren->SetInteractorStyle(irenStyle);

  // The volume will be displayed by ray-cast alpha compositing.
  // A ray-cast mapper is needed to do the ray-casting, and a
  // compositing function is needed to do the compositing along the ray. 
  vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction =
    vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

  vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper =
    vtkSmartPointer<vtkVolumeRayCastMapper>::New();
  volumeMapper->SetInput(scale->GetOutput());
  volumeMapper->SetVolumeRayCastFunction(rayCastFunction);


  // The color transfer function maps voxel intensities to colors.
  // It is modality-specific, and often anatomy-specific as well.
  // The goal is to one color for flesh (between 500 and 1000) 
  // and another color for bone (1150 and over).
  vtkSmartPointer<vtkColorTransferFunction>volumeColor =
    vtkSmartPointer<vtkColorTransferFunction>::New();
  volumeColor->AddRGBPoint(0,    0.0, 0.0, 0.0);
  volumeColor->AddRGBPoint(500,  1.0, 0.3, 0.3);
  volumeColor->AddRGBPoint(1000, 1.0, 0.3, 0.3);
  volumeColor->AddRGBPoint(1150, 1.0, 1.0, 0.9);

  // The opacity transfer function is used to control the opacity
  // of different tissue types.
  vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  volumeScalarOpacity->AddPoint(0,    0.00);
  volumeScalarOpacity->AddPoint(500,  0.55);
  volumeScalarOpacity->AddPoint(1000, 0.55);
  volumeScalarOpacity->AddPoint(1150, 0.85);

  // The gradient opacity function is used to decrease the opacity
  // in the "flat" regions of the volume while maintaining the opacity
  // at the boundaries between tissue types.  The gradient is measured
  // as the amount by which the intensity changes over unit distance.
  // For most medical data, the unit distance is 1mm.
  vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  volumeGradientOpacity->AddPoint(0,   0.0);
  volumeGradientOpacity->AddPoint(90,  0.5);
  volumeGradientOpacity->AddPoint(100, 1.0);

  // The VolumeProperty attaches the color and opacity functions to the
  // volume, and sets other volume properties.  The interpolation should
  // be set to linear to do a high-quality rendering.  The ShadeOn option
  // turns on directional lighting, which will usually enhance the
  // appearance of the volume and make it look more "3D".  However,
  // the quality of the shading depends on how accurately the gradient
  // of the volume can be calculated, and for noisy data the gradient
  // estimation will be very poor.  The impact of the shading can be
  // decreased by increasing the Ambient coefficient while decreasing
  // the Diffuse and Specular coefficient.  To increase the impact
  // of shading, decrease the Ambient and increase the Diffuse and Specular.  
  vtkSmartPointer<vtkVolumeProperty> volumeProperty =
    vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->SetColor(volumeColor);
  volumeProperty->SetScalarOpacity(volumeScalarOpacity);
  volumeProperty->SetGradientOpacity(volumeGradientOpacity);
  volumeProperty->SetInterpolationTypeToLinear();
  volumeProperty->ShadeOn();
  volumeProperty->SetAmbient(0.4);
  volumeProperty->SetDiffuse(0.6);
  volumeProperty->SetSpecular(0.2);

  // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
  // and orientation of the volume in world coordinates.
  vtkSmartPointer<vtkVolume> volume =
    vtkSmartPointer<vtkVolume>::New();
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);

  // Finally, add the volume to the renderer
  ren->AddViewProp(volume);

  // Set up an initial view of the volume.  The focal point will be the
  // center of the volume, and the camera position will be 400mm to the
  // patient's left (which is our right).
  vtkCamera *camera = ren->GetActiveCamera();
  double *c = volume->GetCenter();
  camera->SetFocalPoint(c[0], c[1], c[2]);
  camera->SetPosition(c[0] + 400, c[1], c[2]);
  camera->SetViewUp(0, 0, -1);

  // Increase the size of the render window
  //renWin->SetSize(640, 480);
  this->ui->qvtk_3d->SetRenderWindow(renWin);
  // Interact with the data.
  iren->Initialize();
}


