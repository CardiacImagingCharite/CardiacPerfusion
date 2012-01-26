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
#include "dicomselectordialog.h"
#include "qmessagebox.h"
#include <QtGui>

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
#include "vtkPolyLine.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"

#include "itkGDCMSeriesFileNames.h"
#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"
#include "itkDicomImageIO2.h"
#include <boost/assign.hpp>
#include <boost/foreach.hpp>

#include "QFileDialog.h"
#include "qstring.h"

#include "MyTestInteractorStyle.h"

vtkStandardNewMacro(MyTestInteractorStyle);

const DicomTagList KardioPerfusion::CTModelHeaderFields = boost::assign::list_of
  (DicomTagType("Patient Name", "0010|0010"))
  (DicomTagType("#Slices",CTImageTreeItem::getNumberOfFramesTag()))
  (DicomTagType("AcquisitionDatetime","0008|002a"));

// Constructor
KardioPerfusion::KardioPerfusion():imageModel(CTModelHeaderFields),pendingAction(-1) 
{
  //this->showFullScreen();
  this->ui = new Ui_KardioPerfusion;
  this->ui->setupUi(this);
  
  this->ui->treeView->setModel( &imageModel );
   
  vtkImageData* blank = vtkImageData::New();
  blank->SetDimensions(10, 10, 1);
  blank->AllocateScalars();
  for (int i = 0; i < 10; i++)
      for (int j = 0; j < 10; j++)
          blank->SetScalarComponentFromDouble(i, j, 0, 0, 0);
  blank->Update();

  for(int i = 0; i < 4; i++)
  {
	  m_pViewer[i] = vtkSmartPointer<vtkImageViewer2>::New();
	  m_pViewer[i]->SetInput(blank);
	  m_pViewer[i]->GetRenderer()->ResetCamera();
//	  m_pViewer[i]->GetRenderer()->SetBackground(0,0,0);
//	  m_pViewer[i]->Render();
  }

  // Set up action signals and slots
  connect(this->ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
  connect(this->ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	   this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
  connect( this->ui->treeView, SIGNAL( customContextMenuRequested(const QPoint &) ),
    this, SLOT( treeViewContextMenu(const QPoint &) ) );


  
};

KardioPerfusion::~KardioPerfusion()
{
  // The smart pointers should clean up

}

// Action to be taken upon file open 
void KardioPerfusion::slotOpenFile()
{
/*	QString fname = QFileDialog::getOpenFileName(
    this,
    tr("Select a file to open"),
    ".",
    "", 0, QFileDialog::ReadOnly|QFileDialog::HideNameFilterDetails);
	
	if (!fname.isEmpty()) {
		KardioPerfusion::loadFile(fname);
	}
	*/
	QStringList fnames = QFileDialog::getOpenFileNames(
    this,
    tr("Select one or more files to open"),
    ".",
    "", 0, QFileDialog::ReadOnly|QFileDialog::HideNameFilterDetails);
  setFiles( fnames );
}

void KardioPerfusion::setFiles(const QStringList &names) {
  if (!names.empty()) {
    DicomSelectorDialogPtr selectDialog( new DicomSelectorDialog( this ) );
    selectDialog->setFilesOrDirectories( names );
    loadDicomData( selectDialog );
  }
}

void KardioPerfusion::loadDicomData(DicomSelectorDialogPtr dicomSelector) {
  dicomSelector->exec();
  dicomSelector->getSelectedImageDataList(imageModel);
}

void KardioPerfusion::onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
  int numSelected = this->ui->treeView->selectionModel()->selectedRows().size();
  if (numSelected == 0) 
    this->ui->statusbar->clearMessage();
  else
    this->ui->statusbar->showMessage( QString::number( numSelected ) + tr(" item(s) selected") );
}

void KardioPerfusion::on_treeView_doubleClicked(const QModelIndex &index) {
  if (index.isValid()) {
    TreeItem &item = imageModel.getItem( index );
    if (item.isA(typeid(CTImageTreeItem))) {
      if (displayedCTImage && &item == displayedCTImage->getBaseItem()) {
	setImage( NULL );
      } else {
	setImage( dynamic_cast<CTImageTreeItem*>(&item) );
      }
    } else if (item.isA(typeid(BinaryImageTreeItem))) {
      BinaryImageTreeItem *SegItem = dynamic_cast<BinaryImageTreeItem*>(&item);
      if (displayedSegments.find( SegItem->getVTKConnector() )==displayedSegments.end()) {
	segmentShow( SegItem );
      } else {
	segmentHide( SegItem );
      }
    }
  }
}

void KardioPerfusion::setImage(const CTImageTreeItem *imageItem) {
  vtkImageData *vtkImage = NULL;
  CTImageTreeItem::ConnectorHandle connectorPtr;
  if (imageItem) {
    connectorPtr = imageItem->getVTKConnector();
    vtkImage = connectorPtr->getVTKImageData();
  }
  if (connectorPtr != displayedCTImage) {
    while(!displayedSegments.empty()) {
      segmentHide( dynamic_cast<const BinaryImageTreeItem*>((*displayedSegments.begin())->getBaseItem()) );
    }
    this->ui->mprView->setImage( vtkImage );

/*	  for(int i=0;i<3;i++)
	  {
		m_pViewer[i]->SetInput(vtkImage);
		m_pViewer[i]->GetRenderer()->ResetCamera();
		m_pViewer[i]->SetColorLevel(128);
		m_pViewer[i]->SetColorWindow(254);
		m_pViewer[i]->Render();
	  }



	  m_pViewer[0]->SetSliceOrientationToXY();
	  m_pViewer[1]->SetSliceOrientationToXZ();
	  m_pViewer[2]->SetSliceOrientationToYZ();

	  for(int i = 0; i < 3; i++)
	  {
		m_pViewer[i]->Render();
	  }

	  setCustomStyle();
	  */
	//volumeView->setImage( vtkImage );
    if (displayedCTImage && displayedCTImage->getBaseItem()) displayedCTImage->getBaseItem()->clearActiveDown();
    displayedCTImage = connectorPtr;
    if (displayedCTImage && displayedCTImage->getBaseItem()) displayedCTImage->getBaseItem()->setActive();
  }
}


void KardioPerfusion::setCustomStyle()
{
	// An interactor and a style
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor[4];

	vtkSmartPointer<MyTestInteractorStyle> style[3]; 

	//get the output of the itkKWImage in the VTK format
	for(int i = 0; i < 3; i++)
	{
		renderWindowInteractor[i] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		style[i] =  vtkSmartPointer<MyTestInteractorStyle>::New();
		
		m_pViewer[i]->SetupInteractor(renderWindowInteractor[i]);
		style[i]->setImageViewer(m_pViewer[i]);

		renderWindowInteractor[i]->SetInteractorStyle(style[i]);
		style[i]->SetCurrentRenderer(m_pViewer[i]->GetRenderer());	

	    renderWindowInteractor[i]->Initialize();
	}
}

void KardioPerfusion::on_btn_draw_clicked()
{
  BinaryImageTreeItem *seg = focusSegmentFromSelection();
  if (seg)
    this->ui->mprView->activateOverlayAction(seg->getVTKConnector()->getVTKImageData());
}

void KardioPerfusion::slotExit() {
  qApp->exit();
}

BinaryImageTreeItem *KardioPerfusion::focusSegmentFromSelection(void) {
  clearPendingAction();
  QModelIndexList selectedIndex = this->ui->treeView->selectionModel()->selectedRows();
  if (selectedIndex.size() != 1) {
    QMessageBox::warning(this,tr("Segment Error"),tr("Select one volume to edit"));
    return NULL;
  }
  if (selectedIndex[0].isValid()) {
    TreeItem *item = &imageModel.getItem( selectedIndex[0] );
    if (item->isA(typeid(CTImageTreeItem))) {
      CTImageTreeItem *ctitem = dynamic_cast<CTImageTreeItem*>(item);
      if (ctitem != displayedCTImage->getBaseItem())
	setImage( ctitem );
      if (ctitem->childCount() == 0) {
	item = ctitem->generateSegment();
      } else if (ctitem->childCount()==1) {
	item = &ctitem->child(0);
      } else {
	QMessageBox::warning(this,tr("Segment Error"),tr("Choose the segment to edit"));
	return  NULL;
      }
    }
    if (item->isA(typeid(BinaryImageTreeItem))) {
      BinaryImageTreeItem *seg = dynamic_cast<BinaryImageTreeItem*>(item);
      segmentShow(seg);
      return seg;
    }
  }
  return NULL;
}

void KardioPerfusion::clearPendingAction() {
  if (pendingAction != -1) {
    this->ui->mprView->removeAction( pendingAction );
    pendingAction = -1;
  }
}

void KardioPerfusion::segmentShow( const BinaryImageTreeItem *segItem ) {
  if (segItem) {
    if (displayedCTImage && displayedCTImage->getBaseItem() != segItem->parent()) {
      setImage(dynamic_cast<const CTImageTreeItem*>(segItem->parent()));
    }
    ActionDispatch overlayAction(std::string("draw sphere on ") + segItem->getName().toAscii().data(), 
      boost::bind(&BinaryImageTreeItem::drawSphere, const_cast<BinaryImageTreeItem*>(segItem), 
        boost::bind( &QSpinBox::value, this->ui->sb_size ),
        _3, _4, _5,
        boost::bind( &QCheckBox::checkState, this->ui->cb_erase )
      ),
      ActionDispatch::ClickingAction, ActionDispatch::UnRestricted );
    BinaryImageTreeItem::ConnectorHandle segmentConnector = segItem->getVTKConnector();
    this->ui->mprView->addBinaryOverlay( segmentConnector->getVTKImageData(), segItem->getColor(), overlayAction);
    displayedSegments.insert( segmentConnector );
    segItem->setActive();
  }
}

void KardioPerfusion::treeViewContextMenu(const QPoint &pos) {
  QModelIndex idx = this->ui->treeView->indexAt(pos);
  QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
  if (indexList.count()>0) {
    QMenu cm;
    if (indexList.count() == 1) {
      TreeItem &item = imageModel.getItem(indexList[0]);
      if (item.isA(typeid(CTImageTreeItem))) {
	QAction* addSegAction = cm.addAction("&Add Segment");
	connect( addSegAction, SIGNAL( triggered() ),
	  this, SLOT( createSegmentForSelectedImage())  );
      } else if (item.isA(typeid(BinaryImageTreeItem))) {
	QAction* addSegAction = cm.addAction("&Change Color");
	connect( addSegAction, SIGNAL( triggered() ),
	  this, SLOT( changeColorForSelectedSegment())  );
/*	if (item.isA(typeid(WatershedSegmentTreeItem))) {
	  QAction* setupAction = cm.addAction("&Setup");
	  connect( setupAction, SIGNAL( triggered() ),
	    this, SLOT( setupSelectedWatershedSegment())  );
	  QAction* updateAction = cm.addAction("&Update");
	  connect( updateAction, SIGNAL( triggered() ),
	    this, SLOT( updateSelectedWatershedSegment())  );
	} */
      }
    }
    QAction* addSegAction = cm.addAction("&Delete");
    connect( addSegAction, SIGNAL( triggered() ),
      this, SLOT( removeSelectedImages()  ) );
    cm.exec(this->ui->treeView->mapToGlobal(pos));
  }
}

void KardioPerfusion::removeSelectedImages() {
  QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
  BOOST_FOREACH( const QModelIndex &idx, indexList) {
    TreeItem &remitem = imageModel.getItem( idx );
    if (remitem.isA(typeid(CTImageTreeItem))) {
      CTImageTreeItem *remitemPtr = dynamic_cast<CTImageTreeItem*>(&remitem);
      if (displayedCTImage && displayedCTImage->getBaseItem() == remitemPtr) {
	setImage(NULL);
      }
    } else if (remitem.isA(typeid(BinaryImageTreeItem))) {
      BinaryImageTreeItem *remitemPtr = dynamic_cast<BinaryImageTreeItem*>(&remitem);
      segmentHide( remitemPtr );
    }
    imageModel.removeItem( idx );
  }
}

void KardioPerfusion::segmentHide( const BinaryImageTreeItem *segItem ) {
  if (segItem) {
    clearPendingAction();
    DisplayedSegmentContainer::const_iterator it = displayedSegments.find( segItem->getVTKConnector() );
    if (it != displayedSegments.end()) {
      this->ui->mprView->removeBinaryOverlay( (*it)->getVTKImageData() );
      displayedSegments.erase( it );
    }
    segItem->setActive(false);
  }
}
void KardioPerfusion::loadFile(QString fname){

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

	vtkSmartPointer<MyTestInteractorStyle> style[3]; 

	//get the output of the itkKWImage in the VTK format
	for(int i = 0; i < 3; i++)
	{
		renderWindowInteractor[i] = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		style[i] =  vtkSmartPointer<MyTestInteractorStyle>::New();

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
//	this->ui->qvtk_axial->SetRenderWindow(m_pViewer[0]->GetRenderWindow());
	m_pViewer[0]->Render();
	renderWindowInteractor[0]->Initialize();

	m_pViewer[1]->SetSliceOrientationToXZ();
//	this->ui->qvtk_sagittal->SetRenderWindow(m_pViewer[1]->GetRenderWindow());
	m_pViewer[1]->Render();
	renderWindowInteractor[1]->Initialize();

	m_pViewer[2]->SetSliceOrientationToYZ();
//	this->ui->qvtk_coronal->SetRenderWindow(m_pViewer[2]->GetRenderWindow());
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
//  this->ui->qvtk_3d->SetRenderWindow(renWin);
  // Interact with the data.
  iren->Initialize();
}


