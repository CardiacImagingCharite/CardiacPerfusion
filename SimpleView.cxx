/*
 * Copyright 2007 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */


#include "ui_SimpleView.h"
#include "SimpleView.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkVectorText.h>
#include "vtkSmartPointer.h"
#include "vtkKWImage.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkCamera.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkObjectFactory.h"

#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"

#include "itkGDCMImageIO.h"

#include "QFileDialog.h"
#include "qstring.h"



#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

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
		//m_viewer->Render();
	}

	virtual void OnMouseWheelBackward()
	{
		m_viewer->SetSlice(m_viewer->GetSlice()-1);
		//m_viewer->Render();
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
	QString fname = QFileDialog::getOpenFileName(
    this,
    tr("Select one or more files to open"),
    ".",
    "", 0, QFileDialog::ReadOnly|QFileDialog::HideNameFilterDetails);

	SimpleView::loadFile(fname);
}

void SimpleView::slotExit() {
  qApp->exit();
}

void SimpleView::loadFile(QString fname){

	// define Pixeltype and set dimension
	typedef signed short PixelType;
	const unsigned int Dimension = 3;

	//define the used Imagetype
    typedef itk::OrientedImage<PixelType,Dimension> ImageType;

	//define the Readertype and create Reader object
	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();

	//define ImageIOType and create IO object
	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();

	//set input format of the images to Dicom 
	reader->SetImageIO(dicomIO);

	std::string s = fname.ascii();

	reader->SetFileName(fname.ascii());

	try
		{
			//try to read the images
			reader->Update();
		}
		catch (itk::ExceptionObject &ex)
		{
		  std::cout << ex << std::endl;
		}

	//create a new vtkKWImage, that can be used as an interface between ITK and VTK
	vtkKWImage* kwImage = vtkKWImage::New();
	kwImage->SetITKImageBase(reader->GetOutput());

	vtkImageViewer2* pImageViewer = vtkImageViewer2::New();
	//get the output of the itkKWImage in the VTK format
	pImageViewer->SetInput(kwImage->GetVTKImage());

	// An interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  //renderWindowInteractor->SetRenderWindow(pImageViewer->GetRenderWindow());
  pImageViewer->SetupInteractor(renderWindowInteractor);

  vtkSmartPointer<KeyPressInteractorStyle> style = 
    vtkSmartPointer<KeyPressInteractorStyle>::New();
 
  style->setImageViewer(pImageViewer);

  renderWindowInteractor->SetInteractorStyle(style);
  style->SetCurrentRenderer(pImageViewer->GetRenderer());

 // renderWindowInteractor->InvokeEvent(vtkCommand::MouseWheelForwardEvent);

  	pImageViewer->GetRenderer()->ResetCamera();
    pImageViewer->SetColorLevel(128);
    pImageViewer->SetColorWindow(254);


	this->ui->qvtk_axial->SetRenderWindow(pImageViewer->GetRenderWindow());
	pImageViewer->Render();
	renderWindowInteractor->Initialize();
	
}


