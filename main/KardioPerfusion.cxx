/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie

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

#include "ui_KardioPerfusion.h"
#include "KardioPerfusion.h"
#include "dicomselectordialog.h"
#include "ctimagetreemodel.h"
#include "realimagetreeitem.h"

#include "qmessagebox.h"
#include <QtGui>

#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <thread>

#include "qfiledialog.h"
#include "qstring.h"

#include "qwt_plot.h"
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

#include "segmentlistmodel.h"
#include "timedensitydatapicker.h"

#include "itkShrinkImageFilter.h"
#include "perfusionMapCreator.h"

#include "autoAlignHeart.h"
#include "itkFindLeftVentricle.h"

#include <itkTimeProbe.h>




const DicomTagList KardioPerfusion::m_CTModelHeaderFields = boost::assign::list_of
  (DicomTagType("Patient Name", "0010|0010"))
  (DicomTagType("#Slices",CTImageTreeItem::getNumberOfFramesTag()))
  (DicomTagType("AcquisitionDatetime","0008|002a"));

// Constructor
KardioPerfusion::KardioPerfusion():
     m_imageModelPtr( new CTImageTreeModel(m_CTModelHeaderFields) )
	,m_pendingAction(-1)
	,m_markerStart(new QwtPlotMarker) 
	,m_markerEnd(new QwtPlotMarker)
	,m_markerPickerX(new QwtPlotMarker)
	,m_markerPickerY(new QwtPlotMarker)
	,m_grid(new QwtPlotGrid) 
	,m_perfusionColorMap(vtkLookupTable::New())
	,m_loadHighResItemStack(new std::stack<CTImageTreeItem*>)
	,m_modelSaved(true)
{
	this->m_ui = new Ui_KardioPerfusion;
	this->m_ui->setupUi(this);
  
	this->m_ui->treeView->setModel( m_imageModelPtr.get() );

	//m_tacDialog = NULL;
	m_oneWindowIsMax = false;

	//m_tacDialog = NULL;
	//mmid4Analyzer = NULL;
	m_maxSlopeAnalyzer = NULL;

	//configure the plot
	this->m_ui->qwtPlot_tac->setTitle(QObject::tr("Time Attenuation Curves"));
	this->m_ui->qwtPlot_tac->setAxisTitle(QwtPlot::xBottom, QObject::tr("Time [s]"));
	this->m_ui->qwtPlot_tac->setAxisTitle(QwtPlot::yLeft, QObject::tr("Density [HU]"));
	this->m_ui->qwtPlot_tac->insertLegend(new QwtLegend(), QwtPlot::RightLegend);
	this->m_ui->qwtPlot_tac->setAutoDelete(false);

	//just temporary until autoscale and zoom works
	this->m_ui->qwtPlot_tac->setAxisScale(2,0,20);
	this->m_ui->qwtPlot_tac->setAxisScale(0,0,500);
	this->m_ui->qwtPlot_tac->setAxisAutoScale(0);
	this->m_ui->qwtPlot_tac->setAxisAutoScale(2);

	m_markerStart->setLabel(tr("Start"));
	m_markerStart->setLabelAlignment(Qt::AlignRight|Qt::AlignTop);
	m_markerStart->setLineStyle(QwtPlotMarker::VLine);
	m_markerStart->setXValue(0);
	m_markerStart->setVisible(false);
	m_markerStart->attach(this->m_ui->qwtPlot_tac);  
  
	m_markerEnd->setLabel(tr("End"));
	m_markerEnd->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_markerEnd->setLineStyle(QwtPlotMarker::VLine);
	m_markerEnd->setXValue(0);
	m_markerEnd->setVisible(false);
	m_markerEnd->attach(this->m_ui->qwtPlot_tac);  
  
	m_markerPickerX->setLineStyle(QwtPlotMarker::VLine);
	m_markerPickerY->setLineStyle(QwtPlotMarker::HLine);
	m_markerPickerX->setLinePen(QPen(Qt::red));
	m_markerPickerY->setLinePen(QPen(Qt::red));
	m_markerPickerX->setVisible(false);
	m_markerPickerY->setVisible(false);
	m_markerPickerX->attach(this->m_ui->qwtPlot_tac);
	m_markerPickerY->attach(this->m_ui->qwtPlot_tac);
  
  
	m_grid->enableX(true); m_grid->enableX(false);
	m_grid->attach(this->m_ui->qwtPlot_tac);

	this->m_ui->slider_startTime->setTracking(true);
	this->m_ui->slider_endTime->setTracking(true);
    
    this->m_ui->tbl_gammaFit->verticalHeader()->setVisible(false);
    this->m_ui->tbl_gammaFit->resizeColumnsToContents();

	this->m_ui->mprView_ul->setOrientation(0);	//axial
	this->m_ui->mprView_ur->setOrientation(1);	//coronal
	this->m_ui->mprView_lr->setOrientation(2);	//sagittal

	this->m_ui->mprView_ul->SetRootItem(&m_imageModelPtr->getRootItem());
	this->m_ui->mprView_ur->SetRootItem(&m_imageModelPtr->getRootItem());
	this->m_ui->mprView_lr->SetRootItem(&m_imageModelPtr->getRootItem());


	this->m_ui->mprView_ul->SetPlot(this->m_ui->qwtPlot_tac);
	this->m_ui->mprView_ur->SetPlot(this->m_ui->qwtPlot_tac);
	this->m_ui->mprView_lr->SetPlot(this->m_ui->qwtPlot_tac);

	// Set up action signals and slots
	connect(this->m_ui->actionImportFile, SIGNAL(triggered()), this, SLOT(slotImportFile()));
	connect(this->m_ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->m_ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
	connect( this->m_ui->treeView, SIGNAL( customContextMenuRequested(const QPoint &) ),
	this, SLOT( treeViewContextMenu(const QPoint &) ) );

	connect(this->m_ui->mprView_lr, SIGNAL(doubleClicked(MultiPlanarReformatWidget &)), 
		this, SLOT(mprWidget_doubleClicked(MultiPlanarReformatWidget &)));
	connect(this->m_ui->mprView_ul, SIGNAL(doubleClicked(MultiPlanarReformatWidget &)), 
		this, SLOT(mprWidget_doubleClicked(MultiPlanarReformatWidget &)));
	connect(this->m_ui->mprView_ur, SIGNAL(doubleClicked(MultiPlanarReformatWidget &)), 
		this, SLOT(mprWidget_doubleClicked(MultiPlanarReformatWidget &)));
	connect(this->m_ui->tw_results, SIGNAL(doubleClicked(MyTabWidget &)),
		this, SLOT(tabWidget_doubleClicked(MyTabWidget &)));

	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_ur, SLOT(updateWidget()));
	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_ul, SLOT(updateWidget()));

	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_lr, SLOT(updateWidget()));
	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_ul, SLOT(updateWidget()));

	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_ur, SLOT(updateWidget()));
	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( ColorTableChanged() ), this->m_ui->mprView_lr, SLOT(updateWidget()));

	// couple zoom
	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_ur->GetInteractorStyle(), SLOT( Zoom(int) ) );
	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_ul->GetInteractorStyle(), SLOT( Zoom(int) ) );

	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_lr->GetInteractorStyle(), SLOT( Zoom(int) ) );
	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_ul->GetInteractorStyle(), SLOT( Zoom(int) ) );

	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_ur->GetInteractorStyle(), SLOT( Zoom(int) ) );
	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( ZoomChanged(int) ), this->m_ui->mprView_lr->GetInteractorStyle(), SLOT( Zoom(int) ) );

	// couple window and level
	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_ur->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );
	connect(this->m_ui->mprView_lr->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_ul->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );

	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_lr->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );
	connect(this->m_ui->mprView_ur->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_ul->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );
	
	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_ur->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );
	connect(this->m_ui->mprView_ul->GetInteractorStyle() , SIGNAL( WindowLevelDeltaChanged(int, int) ) , this->m_ui->mprView_lr->GetInteractorStyle(), SLOT( WindowLevelDelta(int, int) ) );
	
	connect(this, SIGNAL( HighResolutionLoaded(const CTImageTreeItem*) ), this, SLOT( SetHighResolutionImage(const CTImageTreeItem*)) );

};

KardioPerfusion::~KardioPerfusion()
{
  // The smart pointers should clean up

}

// Action to be taken upon file open 
void KardioPerfusion::slotImportFile()
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
    tr("Select one or more files to import"),
    "F:/data",
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
	//set image data to m_imageModel
	dicomSelector->getSelectedImageDataList(*m_imageModelPtr);
	m_modelChanged = true;
}

//callback if the selection at the treeview changed
void KardioPerfusion::onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
	// get number of selected items and print it to the statusbar
	int numSelected = this->m_ui->treeView->selectionModel()->selectedRows().size();
	if (numSelected == 0) 
		this->m_ui->statusbar->clearMessage();
	else
		this->m_ui->statusbar->showMessage( QString::number( numSelected ) + tr(" item(s) selected") );

	if(numSelected == 1)
	{
		if(selected.indexes()[0].isValid())
		{
			//get clicked item
			TreeItem& item = m_imageModelPtr->getItem(selected.indexes()[0]);
			//check if item is a CT image
			if (item.isA(typeid(CTImageTreeItem))) {
			
				setImage(dynamic_cast<CTImageTreeItem*>(&item));
				for( unsigned int i = 0; i < item.childCount(); i++)
				{
					if(item.child(i).isA(typeid(BinaryImageTreeItem)))
					{
						BinaryImageTreeItem *SegItem = dynamic_cast<BinaryImageTreeItem*>(&item.child(i));
						segmentShow(SegItem);
					}
				}
				//display number of phase
				this->m_ui->num_phase->display(selected.indexes()[0].row());

				// lock stack and push the item for load high resolution
				m_loadHighResItemStackMutex.lock();
				m_loadHighResItemStack->push(dynamic_cast<CTImageTreeItem*>(&item));
				m_loadHighResItemStackMutex.unlock();
			}
		}
	}
}


//callback if click on treeview occurs
void KardioPerfusion::on_treeView_clicked(const QModelIndex &index) {
	if (index.isValid()) {
  		//get clicked item
		TreeItem &item = m_imageModelPtr->getItem(index);
		//check if item is a CT image
		if (item.isA(typeid(CTImageTreeItem))) {
			
			setImage(dynamic_cast<CTImageTreeItem*>(&item));
			for( unsigned int i = 0; i < item.childCount(); i++)
			{
				if(item.child(i).isA(typeid(BinaryImageTreeItem)))
				{
					BinaryImageTreeItem *SegItem = dynamic_cast<BinaryImageTreeItem*>(&item.child(i));
					segmentShow(SegItem);
				}
			}
		}	
	}
}

//Callback if double click on treeview occurs
void KardioPerfusion::on_treeView_doubleClicked(const QModelIndex &index) {
	//check if the index is valid
	if (index.isValid()) {
		//get clicked item
		TreeItem &item = m_imageModelPtr->getItem( index );
		if(item.isA(typeid(BinaryImageTreeItem)))
		{
			BinaryImageTreeItem *SegItem = dynamic_cast<BinaryImageTreeItem*>(&item);
			if(m_displayedSegments.find(SegItem->getVTKConnector()) == m_displayedSegments.end())
			{
				segmentShow(SegItem);
			}
			else 
			{
				segmentHide(SegItem);
			}
		}
		else if(item.isA(typeid(RealImageTreeItem)))
		{
			RealImageTreeItem *PerfusionItem = dynamic_cast<RealImageTreeItem*>(&item);
			if(m_displayedPerfusionMaps.find(PerfusionItem->getVTKConnector()) == m_displayedPerfusionMaps.end())
			{
				perfusionMapShow(PerfusionItem);
			}
			else 
			{
				perfusionMapHide(PerfusionItem);
			}
		}
	}
}

//set image to the widget
void KardioPerfusion::setImage(const CTImageTreeItem *imageItem) {
	bool ResolutionIsChanged = false;
	vtkImageData *vtkImage = NULL;
	//create ITK VTK connector
	CTImageTreeItem::ConnectorHandle connectorPtr;
	if (imageItem) {
		//get the VTK image
		connectorPtr = imageItem->getVTKConnector();
		vtkImage = connectorPtr->getVTKImageData();
		ResolutionIsChanged = imageItem->isResolutionChanged();
	}
	// if displayed image and new image is different
	if (connectorPtr != m_displayedCTImage || ResolutionIsChanged ) {
		//hide all associated segments
		while(!m_displayedSegments.empty()) {
		segmentHide( dynamic_cast<const BinaryImageTreeItem*>((*m_displayedSegments.begin())->getBaseItem()) );
		}
		//show VTK image at the different windows
		//recalculate translation if the model is changed
		this->m_ui->mprView_ul->setImage( vtkImage, m_modelChanged );
		this->m_ui->mprView_ur->setImage( vtkImage, m_modelChanged );
		this->m_ui->mprView_lr->setImage( vtkImage, m_modelChanged );

		// set back to false... until model is changed
		if ( m_modelChanged ) m_modelChanged = false;
		
		if (m_displayedCTImage && m_displayedCTImage->getBaseItem()) m_displayedCTImage->getBaseItem()->clearActiveDown();
		m_displayedCTImage = connectorPtr;
		if (m_displayedCTImage && m_displayedCTImage->getBaseItem()) m_displayedCTImage->getBaseItem()->setActive();
		//set resolution is changed back to false
		if ( ResolutionIsChanged ) 
			imageItem->setResolutionChanged(false);
	}
}

//callback for draw button
void KardioPerfusion::on_btn_draw_clicked()
{
	bool checked = this->m_ui->btn_draw->isChecked();
	if(checked)
	{
		//get selected segment
		BinaryImageTreeItem *seg = focusSegmentFromSelection();
		if (seg)
		{
			//activate drawing action on VTK image data
			this->m_ui->mprView_ul->activateOverlayAction(seg->getVTKConnector()->getVTKImageData());
			this->m_ui->mprView_ur->activateOverlayAction(seg->getVTKConnector()->getVTKImageData());
			this->m_ui->mprView_lr->activateOverlayAction(seg->getVTKConnector()->getVTKImageData());
		}
		else 
			this->m_ui->btn_draw->setChecked(false);
	}
	else
	{
		this->m_ui->mprView_ul->resetActions();
		this->m_ui->mprView_ur->resetActions();
		this->m_ui->mprView_lr->resetActions();
	}
}

//callback for regionGrow button
void KardioPerfusion::on_btn_regionGrow_clicked()
{
	//get threshold value
	int threshold = this->m_ui->sb_regionGrowThreshold->value();
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
		m_pendingAction = this->m_ui->mprView_ul->addAction(regionGrowAction);
		m_pendingAction = this->m_ui->mprView_ur->addAction(regionGrowAction);
		m_pendingAction = this->m_ui->mprView_lr->addAction(regionGrowAction);
		//activate the pending action
		this->m_ui->mprView_ul->activateAction(m_pendingAction);
		this->m_ui->mprView_ur->activateAction(m_pendingAction);
		this->m_ui->mprView_lr->activateAction(m_pendingAction);

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
		this->m_ui->mprView_ul->update();
		this->m_ui->mprView_ur->update();
		this->m_ui->mprView_lr->update();
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
		this->m_ui->mprView_ul->update();
		this->m_ui->mprView_ur->update();
		this->m_ui->mprView_lr->update();
	}
}

//callback for analyse button
void KardioPerfusion::on_btn_analyse_clicked()
{
	
	m_maxSlopeAnalyzer = new MaxSlopeAnalyzer(this);

	this->m_ui->tbl_gammaFit->setModel( m_maxSlopeAnalyzer->getSegments() );
	this->m_ui->btn_arteryInput->setSegmentListModel( m_maxSlopeAnalyzer->getSegments() );

	this->m_ui->treeView->selectAll();
	//get list of selected items
	QModelIndexList selectedIndex = this->m_ui->treeView->selectionModel()->selectedRows();
	//iterate over selected items
	for(QModelIndexList::Iterator index = selectedIndex.begin(); index != selectedIndex.end(); ++index) {
		if (index->isValid()) {
			//get item at specific index
			TreeItem *item = &m_imageModelPtr->getItem( *index );
			//add image to the dialog if it is a CT image
			if (item->isA(typeid(CTImageTreeItem))) {
				m_maxSlopeAnalyzer->addImage( dynamic_cast<CTImageTreeItem*>(item) );
			}
		}
	}
	this->m_ui->treeView->selectionModel()->clearSelection();
	

	std::list<TreeItem *> itemList;
	//add root item to the item list
	itemList.push_back( &m_imageModelPtr->getItem(QModelIndex()) );
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
		//if actual item is a segment add it
		if (currentItem->isA(typeid(BinaryImageTreeItem)))
			m_maxSlopeAnalyzer->addSegment( dynamic_cast<BinaryImageTreeItem*>(currentItem) );
	}
	m_maxSlopeAnalyzer->calculateTacValues();
	SegmentListModel *segments = m_maxSlopeAnalyzer->getSegments();


	//m_picker = new TimeDensityDataPicker(m_markerPickerX, m_markerPickerY, segments, this->m_ui->qwtPlot_tac->canvas());
	
	//iterate over the list of segments
	BOOST_FOREACH( SegmentInfo &currentSegment, *segments) {
		//attach the curves for the actual segment to the plot
		currentSegment.attachSampleCurves(this->m_ui->qwtPlot_tac);
		
	}
	

	this->m_ui->slider_startTime->setMaximum(m_maxSlopeAnalyzer->getImageCount()-1);
	this->m_ui->slider_endTime->setMaximum(m_maxSlopeAnalyzer->getImageCount()-1);

	this->m_ui->qwtPlot_tac->replot();
	
}

void KardioPerfusion::on_btn_perfusionMap_clicked()
{
	bool ok;
	//show dialog for segment name
	QString mapName = QInputDialog::getText(NULL, QObject::tr("Map Name"),
		QObject::tr("Name:"), QLineEdit::Normal,
		QObject::tr("Unnamed Map"), &ok);

	//if name is valid and dialog was closed with OK
	if (ok && !mapName.isEmpty()) {

		m_maxSlopeAnalyzer = new MaxSlopeAnalyzer(this);

		//get list of selected items
		QModelIndexList selectedIndexes = this->m_ui->treeView->selectionModel()->selectedRows();

		//test if one element is selected
		if(selectedIndexes.count() == 1)
		{
			//get the item from the image model
			TreeItem* item = &m_imageModelPtr->getItem(selectedIndexes[0]);
			//test if item is a CT image
			if(item->isA(typeid(BinaryImageTreeItem)))
			{
				m_maxSlopeAnalyzer->addSegment(dynamic_cast<BinaryImageTreeItem*>(item));

				SegmentInfo* arterySegment = &m_maxSlopeAnalyzer->getSegments()->getSegment( selectedIndexes[0] );

				this->m_ui->mprView_lr->SetArterySegment(dynamic_cast<BinaryImageTreeItem*>(item));
				this->m_ui->mprView_ul->SetArterySegment(dynamic_cast<BinaryImageTreeItem*>(item));
				this->m_ui->mprView_ur->SetArterySegment(dynamic_cast<BinaryImageTreeItem*>(item));

				this->m_ui->treeView->selectAll();
				//get list of selected items
				QModelIndexList selectedIndex = this->m_ui->treeView->selectionModel()->selectedRows();
				//iterate over selected items
				for(QModelIndexList::Iterator index = selectedIndex.begin(); index != selectedIndex.end(); ++index) {
					if (index->isValid()) {
						//get item at specific index
						TreeItem *item = &m_imageModelPtr->getItem( *index );
						//add image to the dialog if it is a CT image
						if (item->isA(typeid(CTImageTreeItem))) {
							m_maxSlopeAnalyzer->addImage( dynamic_cast<CTImageTreeItem*>(item) );
						}
					}
				}
				this->m_ui->treeView->selectionModel()->clearSelection();

				//m_maxSlopeAnalyzer->getSegments()->setArterySegment(selectedIndexes.at(0), arterySegment);
				m_maxSlopeAnalyzer->calculateTacValues();

				PerfusionMapCreator* mapCreator = new PerfusionMapCreator(m_maxSlopeAnalyzer, arterySegment, this->m_ui->sb_shrinkFactor->value());

				//RealImageType::Pointer perfusionMap = mapCreator->getPerfusionMap(m_imageModel.get());
				RealImageTreeItem::ImageType::Pointer perfusionMap;
				perfusionMap = mapCreator->calculatePerfusionMap(m_imageModelPtr.get());


				TreeItem* root = &m_imageModelPtr->getRootItem();

			
				double opacity = (double)this->m_ui->slider_opacity->value()/10;

				RealImageTreeItem* result = new RealImageTreeItem(root, perfusionMap, mapName, opacity);
				root->insertChild(result);

				perfusionMapShow(result);

			/*	vtkSmartPointer<vtkCallbackCommand> updateCallback = 
					vtkSmartPointer<vtkCallbackCommand>::New();

				updateCallback->SetCallback( updateFunc );
				updateCallback->SetClientData( this );

				result->getColorMap()->AddObserver(vtkCommand::UserEvent + 1, updateCallback);
				*/
			//	this->m_ui->mprView_ur->addColoredOverlay(result->getVTKConnector()->getVTKImageData(), result->getColorMap());
			//	this->m_ui->mprView_ul->addColoredOverlay(result->getVTKConnector()->getVTKImageData(), result->getColorMap());
			//	this->m_ui->mprView_lr->addColoredOverlay(result->getVTKConnector()->getVTKImageData(), result->getColorMap());
			}
			else{
				QMessageBox::warning(this,tr("Selection Error"),tr("Please select an image with one AIF segment"));
				return;
			}
		}
		else{
			QMessageBox::warning(this,tr("Selection Error"),tr("Please select an image with one AIF segment"));
			return;
		}
	}
}

//action for autoAlignHeart-Button
void KardioPerfusion::on_btn_autoAlignHeart_clicked() {
  
    typedef itk::FindLeftVentricle<CTImageType> FindLVType;
    
    FindLVType::Pointer findLV = FindLVType::New();
    findLV->setMatrixSize(this->m_ui->sb_matrixSizeFindLV->value());
    
    int i = findLV->GetImageIndex(m_imageModelPtr.get());
    
    // for testing: set phase manual?
    if ( this->m_ui->cb_AAHmanual->isChecked() ) i = this->m_ui->sb_AAHphase->value();
    
    QModelIndex ImIdx = m_imageModelPtr->index(i, 1);
    TreeItem* item = &m_imageModelPtr->getItem(ImIdx);
    
    std::cout << "Image number used for calculating trafo = " << i << endl;
    
    itk::TimeProbe clock;
    clock.Start();
    
    // get current image
    ITKVTKTreeItem<CTImageType> *currentImage = dynamic_cast<ITKVTKTreeItem<CTImageType>*>(item);
    CTImageType::Pointer ImagePtr = currentImage->getITKImage();
    
    clock.Stop();
    std::cout << "FindLeftVentricle: getITKImage\t\t\t\tdone\t( " << clock.GetMean() << "s )" << std::endl;
    
    autoAlignHeart AAH;
    
    // get tranformation from autoAlignHeart
    autoAlignHeart::AffineTransformType::Pointer trafo = AAH.getTrafo(ImagePtr);
    
    // put trafo elements into an array
    double trafoElements[12];
    for (int i = 0; i < 12; i++) {
      trafoElements[i] = trafo->GetParameters()[i];
    }
    
    // apply rotation to widgets
    this->m_ui->mprView_ur->rotateImage( trafoElements );
    this->m_ui->mprView_ul->rotateImage( trafoElements );
    this->m_ui->mprView_lr->rotateImage( trafoElements );

    double ellLength = AAH.getEllipsoidLength();
    
    // scale view to ellipsoid length + 10%
    this->m_ui->mprView_ur->scaleImage(ellLength * 1.1);
    this->m_ui->mprView_ul->scaleImage(ellLength * 1.1);
    this->m_ui->mprView_lr->scaleImage(ellLength * 1.1);
    
    double ellCenter[3];
    
    AAH.getCenter(ellCenter);
    
    // pan the centre of the ellipsoid to the center of the imageItem
    this->m_ui->mprView_ur->panImage(ellCenter);
    this->m_ui->mprView_ul->panImage(ellCenter);
    this->m_ui->mprView_lr->panImage(ellCenter);
}

//callback for exit
void KardioPerfusion::slotExit() {
	qApp->exit();
}

//get selected segment
BinaryImageTreeItem *KardioPerfusion::focusSegmentFromSelection(void) {
	clearPendingAction();
	//get list of selected items
	QModelIndexList selectedIndex = this->m_ui->treeView->selectionModel()->selectedRows();
	//if more or less then one segment is selected return an error
	if (selectedIndex.size() != 1) {
		QMessageBox::warning(this,tr("Segment Error"),tr("Select one volume to edit"));
		return NULL;
	}
	//if index is valid
	if (selectedIndex[0].isValid()) {
		//get selected item
		TreeItem *item = &m_imageModelPtr->getItem( selectedIndex[0] );
		//if item is a CT image
		if (item->isA(typeid(CTImageTreeItem))) {
			// get CT item
			CTImageTreeItem *ctitem = dynamic_cast<CTImageTreeItem*>(item);
			//if item is not shown, set the image
			if (ctitem != m_displayedCTImage->getBaseItem())
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
  if (m_pendingAction != -1) {
    this->m_ui->mprView_ul->removeAction( m_pendingAction );
	this->m_ui->mprView_ur->removeAction( m_pendingAction );
	this->m_ui->mprView_lr->removeAction( m_pendingAction );

    m_pendingAction = -1;
  }
}

//show a segment at the mpr widget
void KardioPerfusion::segmentShow( const BinaryImageTreeItem *segItem ) {
	if (segItem) {
		if (m_displayedCTImage && m_displayedCTImage->getBaseItem() != segItem->parent()) {
			setImage(dynamic_cast<const CTImageTreeItem*>(segItem->parent()));
		}
		//create overlay action
		ActionDispatch overlayAction(std::string("draw sphere on ") + segItem->getName().toAscii().data(), 
			boost::bind(&BinaryImageTreeItem::drawSphere, const_cast<BinaryImageTreeItem*>(segItem), 
				boost::bind( &QSpinBox::value, this->m_ui->sb_size ),
				_3, _4, _5,
				boost::bind( &QCheckBox::checkState, this->m_ui->cb_erase )
				),
			ActionDispatch::ClickingAction, ActionDispatch::UnRestricted );
		
		//create ITK VTK connector
		BinaryImageTreeItem::ConnectorHandle segmentConnector = segItem->getVTKConnector();
		//add overlay at the widget
		this->m_ui->mprView_ul->addBinaryOverlay( segmentConnector->getVTKImageData(), segItem->getColor(), overlayAction);
		this->m_ui->mprView_ur->addBinaryOverlay( segmentConnector->getVTKImageData(), segItem->getColor(), overlayAction);
		this->m_ui->mprView_lr->addBinaryOverlay( segmentConnector->getVTKImageData(), segItem->getColor(), overlayAction);
		
		//add segment to the list of displayed semgents and set actual segment as active
		m_displayedSegments.insert( segmentConnector );
		segItem->setActive();
  }
}

//show a segment at the mpr widget
void KardioPerfusion::perfusionMapShow( RealImageTreeItem *perfItem ) {
	if (perfItem) {
		//if (m_displayedCTImage && m_displayedCTImage->getBaseItem() != perfItem->parent()) {
		//	setImage(dynamic_cast<const CTImageTreeItem*>(perfItem->parent()));
		//}

		//create ITK VTK connector
		RealImageTreeItem::ConnectorHandle perfusionMapConnector = perfItem->getVTKConnector();
		//add overlay at the widget
		this->m_ui->mprView_ul->addColoredOverlay( perfusionMapConnector->getVTKImageData(), perfItem->getColorMap());
		this->m_ui->mprView_ur->addColoredOverlay( perfusionMapConnector->getVTKImageData(), perfItem->getColorMap());
		this->m_ui->mprView_lr->addColoredOverlay( perfusionMapConnector->getVTKImageData(), perfItem->getColorMap());
		
		//add segment to the list of displayed semgents and set actual segment as active
		m_displayedPerfusionMaps.insert( perfusionMapConnector );
		perfItem->setActive();
  }
}

//callback for context menu at specific position
void KardioPerfusion::treeViewContextMenu(const QPoint &pos) {
	//get index for position
	QModelIndex idx = this->m_ui->treeView->indexAt(pos);
	//get selected rows
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	//if index list is not empty
	if (indexList.count()>0) {
		//create menu
		QMenu cm;
		//if one item is selected
		if (indexList.count() == 1) {
			//get tree item
			TreeItem &item = m_imageModelPtr->getItem(indexList[0]);
			//if item is a CT image
			if (item.isA(typeid(CTImageTreeItem))) {
				//create action for adding a segment and connect is to the callback
				QAction* addSegAction = cm.addAction("&Add Segment");
				connect( addSegAction, SIGNAL( triggered() ),
					this, SLOT( createSegmentForSelectedImage())  );
			//if item is a segment
			} else if (item.isA(typeid(BinaryImageTreeItem))) {
				//create action for changing the color and connect it to the callback
				QAction* changeColorAction = cm.addAction("&Change Color");
				connect( changeColorAction, SIGNAL( triggered() ),
					this, SLOT( changeColorForSelectedSegment())  );

				QAction* renameSegAction = cm.addAction("&Rename Item");
				connect(renameSegAction, SIGNAL( triggered() ),
					this, SLOT( renameTreeviewItem() ));
		/*	if (item.isA(typeid(WatershedSegmentTreeItem))) {
			  QAction* setupAction = cm.addAction("&Setup");
			  connect( setupAction, SIGNAL( triggered() ),
				this, SLOT( setupSelectedWatershedSegment())  );
			  QAction* updateAction = cm.addAction("&Update");
			  connect( updateAction, SIGNAL( triggered() ),
				this, SLOT( updateSelectedWatershedSegment())  );
			} */
			} else if (item.isA(typeid(RealImageTreeItem)) ){

				QAction* renameSegAction = cm.addAction("&Rename Item");
				connect(renameSegAction, SIGNAL( triggered() ),
					this, SLOT( renameTreeviewItem() ));
			}
			
		}
		//create action for deleting the selected images
		QAction* addSegAction = cm.addAction("&Delete");
		connect( addSegAction, SIGNAL( triggered() ),
			this, SLOT( removeSelectedImages()  ) );
		//execute the context menu at the specific position
		cm.exec(this->m_ui->treeView->mapToGlobal(pos));
	}
}

//callback for removing selected tree items
void KardioPerfusion::removeSelectedImages() {
	//get list of selected items
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	//iterate over index list
	BOOST_FOREACH( const QModelIndex &idx, indexList) {
		//get tree item
		TreeItem &remitem = m_imageModelPtr->getItem( idx );
		//if item type is a CT image
		if (remitem.isA(typeid(CTImageTreeItem))) {
			//get CT image
			CTImageTreeItem *remitemPtr = dynamic_cast<CTImageTreeItem*>(&remitem);
			//if CT image is visible, remove it from the widget
			if (m_displayedCTImage && m_displayedCTImage->getBaseItem() == remitemPtr) {
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
		m_imageModelPtr->removeItem( idx );
	}
}

//hide segment from widget
void KardioPerfusion::segmentHide( const BinaryImageTreeItem *segItem ) {
	if (segItem) {
		// clear pending action
		clearPendingAction();
		//find segment in the list of displayed segments
		DisplayedSegmentContainer::const_iterator it = m_displayedSegments.find( segItem->getVTKConnector() );
		//if segment was found
		if (it != m_displayedSegments.end()) {
			//remove overlay from widget and erase it from the list of displayed segments
			this->m_ui->mprView_ul->removeBinaryOverlay( (*it)->getVTKImageData() );
			this->m_ui->mprView_ur->removeBinaryOverlay( (*it)->getVTKImageData() );
			this->m_ui->mprView_lr->removeBinaryOverlay( (*it)->getVTKImageData() );
			m_displayedSegments.erase( it );
		}
		//set segment to inactive
		segItem->setActive(false);
	}
}

//hide perfusion map from widget
void KardioPerfusion::perfusionMapHide( const RealImageTreeItem *perfItem ) {
	if (perfItem) {
		// clear pending action
		clearPendingAction();
		//find segment in the list of displayed segments
		DisplayedPerfusionMapContainer::const_iterator it = m_displayedPerfusionMaps.find( perfItem->getVTKConnector() );
		//if segment was found
		if (it != m_displayedPerfusionMaps.end()) {
			//remove overlay from widget and erase it from the list of displayed segments
			this->m_ui->mprView_ul->removeColoredOverlay( (*it)->getVTKImageData() );
			this->m_ui->mprView_ur->removeColoredOverlay( (*it)->getVTKImageData() );
			this->m_ui->mprView_lr->removeColoredOverlay( (*it)->getVTKImageData() );
			m_displayedPerfusionMaps.erase( it );
		}
		//set segment to inactive
		perfItem->setActive(false);
	}
}

//create a segment for selected image
void KardioPerfusion::createSegmentForSelectedImage() {
	//get list of selected items
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	//if one item is selected
	if (indexList.count() == 1) {
		//get selected item 
		TreeItem &item = m_imageModelPtr->getItem(indexList[0]);
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
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	//if one item is selected
	if (indexList.count() == 1) {
		//get selected item
		TreeItem &item = m_imageModelPtr->getItem(indexList[0]);
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


void KardioPerfusion::mprWidget_doubleClicked(MultiPlanarReformatWidget &w)
{
	if(!m_oneWindowIsMax)
	{
		this->m_ui->mprView_ul->setVisible(false);
		this->m_ui->mprView_ur->setVisible(false);
		this->m_ui->mprView_lr->setVisible(false);
		this->m_ui->tw_results->setVisible(false);

		w.setVisible(true);
		m_oneWindowIsMax = true;

//		w.resetActions();
	}
	else
	{
		this->m_ui->mprView_ul->setVisible(true);
		this->m_ui->mprView_ur->setVisible(true);
		this->m_ui->mprView_lr->setVisible(true);
		this->m_ui->tw_results->setVisible(true);

		m_oneWindowIsMax = false;
	}
}

void KardioPerfusion::tabWidget_doubleClicked(MyTabWidget &w)
{
	if(!m_oneWindowIsMax)
	{
		this->m_ui->mprView_ul->setVisible(false);
		this->m_ui->mprView_ur->setVisible(false);
		this->m_ui->mprView_lr->setVisible(false);
		this->m_ui->tw_results->setVisible(false);

		w.setVisible(true);
		m_oneWindowIsMax = true;
	}
	else
	{
		this->m_ui->mprView_ul->setVisible(true);
		this->m_ui->mprView_ur->setVisible(true);
		this->m_ui->mprView_lr->setVisible(true);
		this->m_ui->tw_results->setVisible(true);

		m_oneWindowIsMax = false;
	}
}

void KardioPerfusion::sliderStartValue_changed()
{
	int value = this->m_ui->slider_startTime->value();
	QModelIndexList indexList = this->m_ui->tbl_gammaFit->selectionModel()->selectedRows();
	this->m_ui->lbl_startTime->setText(QString::number(m_maxSlopeAnalyzer->getTime(value)));
	m_markerStart->setXValue(m_maxSlopeAnalyzer->getTime(value));
	if (indexList.size() == 1) 
	{
		m_maxSlopeAnalyzer->setGammaStartIndex(value, indexList);
		m_maxSlopeAnalyzer->recalculateGamma(indexList);
	}
	this->m_ui->qwtPlot_tac->replot();
}

void KardioPerfusion::sliderEndValue_changed()
{
	int value = this->m_ui->slider_endTime->value();
	QModelIndexList indexList = this->m_ui->tbl_gammaFit->selectionModel()->selectedRows();
	this->m_ui->lbl_endTime->setText(QString::number(m_maxSlopeAnalyzer->getTime(value)));
	m_markerEnd->setXValue(m_maxSlopeAnalyzer->getTime(value));
	if (indexList.size() == 1) 
	{
		m_maxSlopeAnalyzer->setGammaEndIndex(value, indexList);
		m_maxSlopeAnalyzer->recalculateGamma(indexList);
	}
	this->m_ui->qwtPlot_tac->replot();
}

void KardioPerfusion::tableGamma_clicked(const QModelIndex & index)
{
	this->m_ui->tbl_gammaFit->selectionModel()->select(index, QItemSelectionModel::Rows);
	this->m_ui->slider_startTime->setEnabled(true);
	this->m_ui->slider_endTime->setEnabled(true);
	this->m_ui->cb_enableGamma->setEnabled(true);
	m_markerStart->setVisible(true);
	m_markerEnd->setVisible(true);
	
	SegmentListModel* segments = m_maxSlopeAnalyzer->getSegments();
	const SegmentInfo &seg = segments->getSegment( index );
	this->m_ui->slider_startTime->setValue(seg.getGammaStartIndex());
	this->m_ui->slider_endTime->setValue(seg.getGammaEndIndex());
	this->m_ui->cb_enableGamma->setChecked(seg.isGammaEnabled());
	this->m_ui->btn_arteryInput->setSelectedSegment(seg.getArterySegment());
}

void KardioPerfusion::cb_enableGamma_toggled()
{
	QModelIndexList indexList = this->m_ui->tbl_gammaFit->selectionModel()->selectedRows();
	if (indexList.size() == 1) {
		SegmentListModel* segments = m_maxSlopeAnalyzer->getSegments();
		SegmentInfo &seg = segments->getSegment(indexList.at(0));
		if (this->m_ui->cb_enableGamma->isChecked()) {
			seg.setEnableGamma(true);
		}
		else 
			seg.setEnableGamma(false);
		m_maxSlopeAnalyzer->recalculateGamma(seg);
  }
}

void KardioPerfusion::on_btn_arteryInput_selected(const SegmentInfo *segment) {
  QModelIndexList indexList = this->m_ui->tbl_gammaFit->selectionModel()->selectedRows();
  if (indexList.size() == 1) {
    m_maxSlopeAnalyzer->getSegments()->setArterySegment(indexList.at(0), segment);
  }
}

void KardioPerfusion::on_actionSave_Project_triggered() {
  QString pname( QString::fromStdString( m_imageModelPtr->getSerializationPath() ) );
  if (pname.isEmpty()) pname = "./unnamed.perfproj";
  pname = QFileDialog::getSaveFileName( this,
    tr("Save Project"),
    pname,
    tr("Project Files (*.perfproj)"));
  if (!pname.isEmpty()) {
    m_imageModelPtr->saveModelToFile(pname.toAscii().data(), 4);
  }
}

void KardioPerfusion::on_actionOpen_Project_triggered() {
	QString pname = QFileDialog::getOpenFileName( this,
						      tr("Open Project"),
						      "./unnamed.perfproj",
					       tr("Project Files (*.perfproj)"));
	if (!pname.isEmpty()) {
		setImage(NULL);
		// clear stack
		while ( !m_loadHighResItemStack->empty() )
			m_loadHighResItemStack->pop();
		// wait till old model is saved for loader thread
		while ( !m_modelSaved )
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		m_imageModelPtr.reset(new CTImageTreeModel(m_CTModelHeaderFields));
		m_modelSaved = false;
		this->m_ui->treeView->setModel( m_imageModelPtr.get() );
		connect(this->m_ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
		m_imageModelPtr->openModelFromFile(pname.toAscii().data());
		m_modelChanged = true;
		// select the first image of the tree
		QModelIndex first = m_imageModelPtr->index(0, 0, QModelIndex() );
		this->m_ui->treeView->setCurrentIndex(first);
		// fill stack for load high resolution
		for ( int i = m_imageModelPtr->rowCount() -1; i >= 0; i-- )
		{
			QModelIndex ImIdx = m_imageModelPtr->index(i, 1);
			TreeItem& item = m_imageModelPtr->getItem(ImIdx);
			m_loadHighResItemStackMutex.lock();
			m_loadHighResItemStack->push(dynamic_cast<CTImageTreeItem*>(&item));
			m_loadHighResItemStackMutex.unlock();
		}
		// start 'load high resolution' thread if not running
		if ( m_loadHighResThreadMutex.try_lock() )
			m_loadHighResThread = new std::thread( &KardioPerfusion::loadHighResolution, this );
	}
}

void KardioPerfusion::loadHighResolution()
{
	// loop over stack
	while ( !m_loadHighResItemStack->empty() )
	{
		// save old model; otherwise retrieveITKImage crashes
		std::shared_ptr<CTImageTreeModel> savedModelPtr = m_imageModelPtr;
		m_modelSaved = true;
		if ( m_loadHighResItemStackMutex.try_lock() )
		{
			// get item from stack and unlock the stack
			CTImageTreeItem* imageItem = m_loadHighResItemStack->top();
			m_loadHighResItemStack->pop();
			m_loadHighResItemStackMutex.unlock();

			if ( imageItem->isCurrentlyShrinked() )
			{
				// retrieve (non shrinked) image from disk
				imageItem->retrieveITKImage();
				imageItem->setCurrentImageShrinked(false);
				if ( savedModelPtr == m_imageModelPtr ) {
					HighResolutionLoaded(imageItem);
				}
			}
		}
	}
	// unlock the thread
	m_loadHighResThreadMutex.unlock();
}

void KardioPerfusion::SetHighResolutionImage(const CTImageTreeItem *imageItem)
{
	//create ITK VTK connector
	CTImageTreeItem::ConnectorHandle connectorPtr;
	connectorPtr = imageItem->getVTKConnector();
	// if displayed item and new item is (still) the same
	if (connectorPtr == m_displayedCTImage )
		setImage(imageItem);
}



void KardioPerfusion::slider_opacity_changed()
{		
	//get list of selected items
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	//if one item is selected
	if (indexList.count() == 1) {
		//get selected item
		TreeItem &item = m_imageModelPtr->getItem(indexList[0]);
		//if item is a segment
		if (item.isA(typeid(RealImageTreeItem))) {
			//get segment
			RealImageTreeItem &perfusionMap = dynamic_cast<RealImageTreeItem&>(item);
			perfusionMap.setOpacity((double)this->m_ui->slider_opacity->value()/10);

			this->m_ui->mprView_lr->update();
			this->m_ui->mprView_ul->update();
			this->m_ui->mprView_ur->update();
		}
	}

	//m_perfusionColorMap->SetAlpha((double)this->m_ui->slider_opacity->value()/10);
	
	//this->m_ui->mprView_lr->refreshView();
	//this->m_ui->mprView_ur->GetRenderWindow()->Render();
	//this->m_ui->mprView_ul->GetRenderWindow()->GetInteractor()->Render();
	
}

void KardioPerfusion::renameTreeviewItem()
{
	QModelIndexList indexList = this->m_ui->treeView->selectionModel()->selectedRows();
	this->m_ui->treeView->edit(indexList[0]);
}

void KardioPerfusion::updateFunc(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	KardioPerfusion* self = reinterpret_cast<KardioPerfusion*>( clientData);

	self->m_ui->mprView_lr->update();
	self->m_ui->mprView_ul->update();
	self->m_ui->mprView_ur->update();
}

void KardioPerfusion::on_btn_clearPlot_clicked()
{
	this->m_ui->qwtPlot_tac->detachItems(QwtPlotItem::Rtti_PlotCurve, false);
	this->m_ui->qwtPlot_tac->replot();
}

void KardioPerfusion::on_btn_writeResults_clicked()
{
	QString pname = "./unnamed.csv";
	pname = QFileDialog::getSaveFileName( this,
    tr("Save Results"),
    pname,
    tr("CSV Files (*.csv)"));
  if (!pname.isEmpty()) {
    
	ofstream resultFile;
	resultFile.open (pname.toAscii().data());

	resultFile << m_maxSlopeAnalyzer->getTacValuesAsString() << std::endl;

	SegmentListModel *segments = m_maxSlopeAnalyzer->getSegments();
	BOOST_FOREACH( SegmentInfo &currentSegment, *segments) {

		if(currentSegment.getArterySegment() != NULL)
		{
			resultFile << &currentSegment.getName() << std::endl;

			double perfusion = 60 * currentSegment.getGammaMaxSlope() / currentSegment.getArterySegment()->getGammaMaximum();
			resultFile << "Perfusion;" << perfusion << std::endl;
			resultFile << "Max Slope;" << currentSegment.getGammaMaxSlope() << std::endl;
			resultFile << "Max Gamma;" << currentSegment.getArterySegment()->getGammaMaximum() << std::endl << std::endl;
		}
	}

	resultFile.close();
  }

}
