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

//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkVectorText.h>
#include "vtkSmartPointer.h"
//#include "vtkKWImage.h"
//#include "vtkImageData.h"
//#include "vtkCamera.h"
//#include "vtkInteractorStyleTrackballCamera.h"
//#include "vtkObjectFactory.h"
//#include "vtkDICOMImageReader.h"
//#include "vtkVolumeRayCastCompositeFunction.h"
//#include "vtkVolumeRayCastMapper.h"
//#include "vtkColorTransferFunction.h"
//#include "vtkPiecewiseFunction.h"
//#include "vtkVolumeProperty.h"
//#include "vtkImageShiftScale.h"
//#include "vtkVolume16Reader.h"
//#include "vtkPolyLine.h"
//#include "vtkPoints.h"
//#include "vtkCellArray.h"
//#include "vtkPolyDataMapper.h"

//#include "itkGDCMSeriesFileNames.h"
//#include "itkOrientedImage.h"
//#include "itkImageSeriesReader.h"
//#include "itkGDCMImageIO.h"
//#include "itkDicomImageIO2.h"
#include <boost/assign.hpp>
#include <boost/foreach.hpp>

#include "QFileDialog.h"
#include "qstring.h"

//#include "MyTestInteractorStyle.h"

//vtkStandardNewMacro(MyTestInteractorStyle);

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

  m_tacDialog = NULL;

	this->ui->mprView->setOrientation(0);
	this->ui->mprView->setOrientation(1);
	this->ui->mprView->setOrientation(2);


/*  vtkImageData* blank = vtkImageData::New();
  blank->SetDimensions(10, 10, 1);
  blank->AllocateScalars();
  for (int i = 0; i < 10; i++)
      for (int j = 0; j < 10; j++)
          blank->SetScalarComponentFromDouble(i, j, 0, 0, 0);
  blank->Update();

  this->ui->mprView->setImage(blank);
  this->ui->mprView_lr->setImage(blank);
  this->ui->mprView_ur->setImage(blank);
  */
/*  for(int i = 0; i < 4; i++)
  {
	  m_pViewer[i] = vtkSmartPointer<vtkImageViewer2>::New();
	  m_pViewer[i]->SetInput(blank);
	  m_pViewer[i]->GetRenderer()->ResetCamera();
//	  m_pViewer[i]->GetRenderer()->SetBackground(0,0,0);
//	  m_pViewer[i]->Render();
  }
  */
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

//set the selected files to the SelectorDialog
void KardioPerfusion::setFiles(const QStringList &names) {
  if (!names.empty()) {
	  // creats new dialog
    DicomSelectorDialogPtr selectDialog( new DicomSelectorDialog( this ) );
    //set Filenames
	selectDialog->setFilesOrDirectories( names );
    loadDicomData( selectDialog );
  }
}

//loads the Dicom images
void KardioPerfusion::loadDicomData(DicomSelectorDialogPtr dicomSelector) {
	//execute the dialog
	dicomSelector->exec();
	//set image data to imageModel
	dicomSelector->getSelectedImageDataList(imageModel);
}

//callback if the selection at the treeview changed
void KardioPerfusion::onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
	// get number of selected items and print it to the statusbar
	int numSelected = this->ui->treeView->selectionModel()->selectedRows().size();
	if (numSelected == 0) 
		this->ui->statusbar->clearMessage();
	else
		this->ui->statusbar->showMessage( QString::number( numSelected ) + tr(" item(s) selected") );
}

//callback if click on treeview occurs
void KardioPerfusion::on_treeView_clicked(const QModelIndex &index) {
	if (index.isValid()) {
		//get clicked item
		TreeItem &item = imageModel.getItem( index );
		//check if item is a CT image
		if (item.isA(typeid(CTImageTreeItem))) {
			//hide actual image
			if (displayedCTImage && &item == displayedCTImage->getBaseItem()) {
				setImage( NULL );
			} else {
				//show image
				setImage( dynamic_cast<CTImageTreeItem*>(&item) );
			}
			//check if item is a segement
		} else if (item.isA(typeid(BinaryImageTreeItem))) {
			//get selected segment
			BinaryImageTreeItem *SegItem = dynamic_cast<BinaryImageTreeItem*>(&item);
			//if segment is not in the list of displayed segments
			if (displayedSegments.find( SegItem->getVTKConnector() )==displayedSegments.end()) {
				//show segement
				segmentShow( SegItem );
			} else {
				//else hide it
				segmentHide( SegItem );
			}
		}
	}
}
//set image to the widget
void KardioPerfusion::setImage(const CTImageTreeItem *imageItem) {
	vtkImageData *vtkImage = NULL;
	//create ITK VTK connector
	CTImageTreeItem::ConnectorHandle connectorPtr;
	if (imageItem) {
		//get the VTK image
		connectorPtr = imageItem->getVTKConnector();
		vtkImage = connectorPtr->getVTKImageData();
	}
	// if displayed image and new image is different
	if (connectorPtr != displayedCTImage) {
		//hide all associated segments
		while(!displayedSegments.empty()) {
		segmentHide( dynamic_cast<const BinaryImageTreeItem*>((*displayedSegments.begin())->getBaseItem()) );
		}
		//show VTK image
		this->ui->mprView->setImage( vtkImage );
		//this->ui->mprView->setOrientation(0);

		this->ui->mprView_ur->setImage(vtkImage);
	//	this->ui->mprView_ur->setOrientation(1);

		this->ui->mprView_lr->setImage(vtkImage);
		//this->ui->mprView_lr->setOrientation(2);

     /*	for(int i=0;i<3;i++)
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


/*void KardioPerfusion::setCustomStyle()
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
*/

//callback for draw button
void KardioPerfusion::on_btn_draw_clicked()
{
	//get selected segment
	BinaryImageTreeItem *seg = focusSegmentFromSelection();
	if (seg)
		//activate drawing action on VTK image data
		this->ui->mprView->activateOverlayAction(seg->getVTKConnector()->getVTKImageData());
}

//callback for regionGrow button
void KardioPerfusion::on_btn_regionGrow_clicked()
{
	//get threshold value
	int threshold = this->ui->sb_regionGrowThreshold->value();
	//get selected segment
	BinaryImageTreeItem *seg = focusSegmentFromSelection();

    if (seg) {
		//create action for region growing
		ActionDispatch regionGrowAction(std::string("click to region grow inside ") + seg->getName().toAscii().data(), 
			boost::bind(&BinaryImageTreeItem::regionGrow, seg, 
			_3, _4, _5, threshold,
			  boost::function<void()>(boost::bind(&KardioPerfusion::clearPendingAction, this))
			),
			ActionDispatch::ClickingAction, ActionDispatch::UnRestricted );
		//add action to mprView
		pendingAction = this->ui->mprView->addAction(regionGrowAction);
		//activate the pending action
		this->ui->mprView->activateAction(pendingAction);
    }
}

//callback for erode button
void KardioPerfusion::on_btn_erode_clicked()
{
	//get selected segement
	BinaryImageTreeItem *seg = focusSegmentFromSelection();
    if (seg) {
		bool ok;
		//open dialog and ask for number of iterations
		int iterations = QInputDialog::getInt(this,tr("Interations"), tr("Enter number of erosion iterations"),
		  1, 1, 100, 1, &ok);
		if (!ok) return;
		//erode selected segment and update mprVied
		seg->binaryErode(iterations);
		this->ui->mprView->update();
	}
}

//action for dilate button
void KardioPerfusion::on_btn_dilate_clicked()
{
	//get selected segment
	BinaryImageTreeItem *seg = focusSegmentFromSelection();
	if (seg) {
		bool ok;
		//open dialog and ask for number of iterations
		int iterations = QInputDialog::getInt(this,tr("Interations"), tr("Enter number of dilation iterations"),
		  1, 1, 100, 1, &ok);
		if (!ok) return;
		//dilate selected segment and update mprView
		seg->binaryDilate(iterations);
		this->ui->mprView->update();
	}
}

//callback for cannyEdge button
/*void KardioPerfusion::on_btn_cannyEdges_clicked()
{
	//get selected segment
	BinaryImageTreeItem *seg = focusSegmentFromSelection();
	if (seg) {
		//extract canny edges and update mprView
		seg->extractEdges();
	    this->ui->mprView->update();
	}
}
*/

//callback for analyse button
void KardioPerfusion::on_btn_analyse_clicked()
{
	//create plot dialog 
	if(m_tacDialog == NULL)
	{
		m_tacDialog = new TacDialog(this);
	}
	else
	{
		QSize size = m_tacDialog->size();
		QPoint pos = m_tacDialog->pos();
		m_tacDialog->close();
		m_tacDialog = new TacDialog(this);

		m_tacDialog->resize(size);
		m_tacDialog->move(pos);
	}

	//get list of selected items
	QModelIndexList selectedIndex = this->ui->treeView->selectionModel()->selectedRows();
	//iterate over selected items
	for(QModelIndexList::Iterator index = selectedIndex.begin(); index != selectedIndex.end(); ++index) {
		if (index->isValid()) {
			//get item at specific index
			TreeItem *item = &imageModel.getItem( *index );
			//add image to the dialog if it is a CT image
			if (item->isA(typeid(CTImageTreeItem))) {
				m_tacDialog->addImage( dynamic_cast<CTImageTreeItem*>(item) );
			}
		}
	}
	std::list<TreeItem *> itemList;
	//add root item to the item list
	itemList.push_back( &imageModel.getItem(QModelIndex()) );
	while(!itemList.empty()) {
		//get the last item of the list
		TreeItem *currentItem = itemList.back();
		//delete last element
		itemList.pop_back();
		//get number of child nodes
		int cnum = currentItem->childCount();
		//add childnodes to the item list
		for(int i = 0; i < cnum; i++ ) {
			itemList.push_back( &currentItem->child(i) );
		}
		//if actual item is a segment add it to the dialog
		if (currentItem->isA(typeid(BinaryImageTreeItem)))
			m_tacDialog->addSegment( dynamic_cast<BinaryImageTreeItem*>(currentItem) );
	}

	//execute the dialog
	m_tacDialog->show();
}

//callback for exit
void KardioPerfusion::slotExit() {
	qApp->exit();
}

//get selected segment
BinaryImageTreeItem *KardioPerfusion::focusSegmentFromSelection(void) {
	clearPendingAction();
	//get list of selected items
	QModelIndexList selectedIndex = this->ui->treeView->selectionModel()->selectedRows();
	//if more or less then one segment is selected return an error
	if (selectedIndex.size() != 1) {
		QMessageBox::warning(this,tr("Segment Error"),tr("Select one volume to edit"));
		return NULL;
	}
	//if index is valid
	if (selectedIndex[0].isValid()) {
		//get selected item
		TreeItem *item = &imageModel.getItem( selectedIndex[0] );
		//if item is a CT image
		if (item->isA(typeid(CTImageTreeItem))) {
			// get CT item
			CTImageTreeItem *ctitem = dynamic_cast<CTImageTreeItem*>(item);
			//if item is not shown, set the image
			if (ctitem != displayedCTImage->getBaseItem())
				setImage( ctitem );
			//if CT image has no segment, create a new one
			if (ctitem->childCount() == 0) {
				item = ctitem->generateSegment();
			//if CT image has one segment, take it
			} else if (ctitem->childCount()==1) {
				item = &ctitem->child(0);
			//if CT image has more segments, show warning and return
			} else {
				QMessageBox::warning(this,tr("Segment Error"),tr("Choose the segment to edit"));
				return  NULL;
			}
		}
		//if child item of the CT image is a segment
		if (item->isA(typeid(BinaryImageTreeItem))) {
			//get the segment, show it and return the segment
 			BinaryImageTreeItem *seg = dynamic_cast<BinaryImageTreeItem*>(item);
			segmentShow(seg);
			return seg;
		}
	}
	return NULL;
}

//clear pending actions
void KardioPerfusion::clearPendingAction() {
  if (pendingAction != -1) {
    this->ui->mprView->removeAction( pendingAction );
    pendingAction = -1;
  }
}

//show a segment at the mpr widget
void KardioPerfusion::segmentShow( const BinaryImageTreeItem *segItem ) {
	if (segItem) {
		if (displayedCTImage && displayedCTImage->getBaseItem() != segItem->parent()) {
			setImage(dynamic_cast<const CTImageTreeItem*>(segItem->parent()));
		}
		//create overlay action
		ActionDispatch overlayAction(std::string("draw sphere on ") + segItem->getName().toAscii().data(), 
			boost::bind(&BinaryImageTreeItem::drawSphere, const_cast<BinaryImageTreeItem*>(segItem), 
				boost::bind( &QSpinBox::value, this->ui->sb_size ),
				_3, _4, _5,
				boost::bind( &QCheckBox::checkState, this->ui->cb_erase )
				),
			ActionDispatch::ClickingAction, ActionDispatch::UnRestricted );
		//create ITK VTK connector
		BinaryImageTreeItem::ConnectorHandle segmentConnector = segItem->getVTKConnector();
		//add overlay at the widget
		this->ui->mprView->addBinaryOverlay( segmentConnector->getVTKImageData(), segItem->getColor(), overlayAction);
		//add segment to the list of displayed semgents and set actual segment as active
		displayedSegments.insert( segmentConnector );
		segItem->setActive();
  }
}

//callback for context menu at specific position
void KardioPerfusion::treeViewContextMenu(const QPoint &pos) {
	//get index for position
	QModelIndex idx = this->ui->treeView->indexAt(pos);
	//get selected rows
	QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
	//if index list is not empty
	if (indexList.count()>0) {
		//create menu
		QMenu cm;
		//if one item is selected
		if (indexList.count() == 1) {
			//get tree item
			TreeItem &item = imageModel.getItem(indexList[0]);
			//if item is a CT image
			if (item.isA(typeid(CTImageTreeItem))) {
				//create action for adding a segment and connect is to the callback
				QAction* addSegAction = cm.addAction("&Add Segment");
				connect( addSegAction, SIGNAL( triggered() ),
					this, SLOT( createSegmentForSelectedImage())  );
			//if item is a segment
			} else if (item.isA(typeid(BinaryImageTreeItem))) {
				//create action for changing the color and connect it to the callback
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
		//create action for deleting the selected images
		QAction* addSegAction = cm.addAction("&Delete");
		connect( addSegAction, SIGNAL( triggered() ),
			this, SLOT( removeSelectedImages()  ) );
		//execute the context menu at the specific position
		cm.exec(this->ui->treeView->mapToGlobal(pos));
	}
}

//callback for removing selected tree items
void KardioPerfusion::removeSelectedImages() {
	//get list of selected items
	QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
	//iterate over index list
	BOOST_FOREACH( const QModelIndex &idx, indexList) {
		//get tree item
		TreeItem &remitem = imageModel.getItem( idx );
		//if item type is a CT image
		if (remitem.isA(typeid(CTImageTreeItem))) {
			//get CT image
			CTImageTreeItem *remitemPtr = dynamic_cast<CTImageTreeItem*>(&remitem);
			//if CT image is visible, remove it from the widget
			if (displayedCTImage && displayedCTImage->getBaseItem() == remitemPtr) {
				setImage(NULL);
			}
		//if item is a segment
		} else if (remitem.isA(typeid(BinaryImageTreeItem))) {
			//get segment
			BinaryImageTreeItem *remitemPtr = dynamic_cast<BinaryImageTreeItem*>(&remitem);
			//hide select segment
			segmentHide( remitemPtr );
		}
		//remove the index from the image model
		imageModel.removeItem( idx );
	}
}

//hide segment from widget
void KardioPerfusion::segmentHide( const BinaryImageTreeItem *segItem ) {
	if (segItem) {
		// clear pending action
		clearPendingAction();
		//find segment in the list of displayed segments
		DisplayedSegmentContainer::const_iterator it = displayedSegments.find( segItem->getVTKConnector() );
		//if segment was found
		if (it != displayedSegments.end()) {
			//remove overlay from widget and erase it from the list of displayed segments
			this->ui->mprView->removeBinaryOverlay( (*it)->getVTKImageData() );
			displayedSegments.erase( it );
		}
		//set segment to inactive
		segItem->setActive(false);
	}
}

//create a segment for selected image
void KardioPerfusion::createSegmentForSelectedImage() {
	//get list of selected items
	QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
	//if one item is selected
	if (indexList.count() == 1) {
		//get selected item 
		TreeItem &item = imageModel.getItem(indexList[0]);
		//if item is a CT image
		if (item.isA(typeid(CTImageTreeItem))) {
			//generate segment
			dynamic_cast<CTImageTreeItem&>(item).generateSegment();
		}
	}
}

//change the color for the selected item
void KardioPerfusion::changeColorForSelectedSegment() {
	//get list of selected items
	QModelIndexList indexList = this->ui->treeView->selectionModel()->selectedRows();
	//if one item is selected
	if (indexList.count() == 1) {
		//get selected item
		TreeItem &item = imageModel.getItem(indexList[0]);
		//if item is a segment
		if (item.isA(typeid(BinaryImageTreeItem))) {
			//get segment
			BinaryImageTreeItem &binItem = dynamic_cast<BinaryImageTreeItem&>(item);
			//get color of selected item
			QColor color = binItem.getColor();
			//create color dialog
			color = QColorDialog::getColor(color, this, tr("Choose new Segment Color for ") + binItem.getName());
			//if valid color returns, set color of the segment
			if (color.isValid())
				binItem.setColor(color);
		}
	}
}

/*void KardioPerfusion::loadFile(QString fname){

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
/*	//create a new vtkKWImage, that can be used as an interface between ITK and VTK
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

*/
