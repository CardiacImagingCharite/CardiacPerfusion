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

#ifndef KardioPerfusion_H
#define KardioPerfusion_H


#include <QMainWindow>
#include <QObject>
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
#include "ctimagetreemodel.h"
#include "binaryimagetreeitem.h"
#include "ctimagetreeitem.h"
#include <qitemselectionmodel.h>
#include <QMouseEvent>
#include "mytabwidget.h"
#include "maxSlopeAnalyzer.h"
#include "realimagetreeitem.h"
#include <thread>
#include <stack>
#include <mutex>


#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// Forward Qt class declarations
class Ui_KardioPerfusion;
class DicomSelectorDialog;
class MultiPlanarReformatWidget;
class QwtPlotMarker;
class QwtPlotGrid;
class QwtPlotPicker;
class vtkLookupTable;

/*! \class KardioPerfusion KardioPerfusion.h "KardioPerfusion.h"
 *  \brief This is the main class for the graphical user interface.
 *
 */
class KardioPerfusion : public QMainWindow
{
	Q_OBJECT

	public:

	//! default Constructor of the class
	KardioPerfusion(); 
	//! Deconstructor of the class
	~KardioPerfusion();

//  void loadFile(QString fname);

	///Set the selected files to the SelectorDialog
	/*! 
	\param names contains the filenames to the images
	*/
	void setFiles(const QStringList &names);

	public slots:

	///Action to be taken upon file open
	virtual void slotImportFile();
	///Action to exit the application
	virtual void slotExit();
	///Action to save a project
	void on_actionSave_Project_triggered();
      
	///Action to load a project
	void on_actionOpen_Project_triggered();
	
	//void on_treeView_doubleClicked(const QModelIndex &index);

	///Action for clicking on treeView.
	/*! 
	\param index contains the clicked index.
	*/
	void on_treeView_clicked(const QModelIndex &index);
	///Action for double clicking on treeview.
	/*!
	\param index contains the double clicked index.
	*/
	void on_treeView_doubleClicked(const QModelIndex &index);

	///Action for changing the selection of the treeView.
	/*! 
	\param selected contains the selected elements.
	\param deselected contains the deselected elements.
	*/
	void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
	///Action for clicking on Draw-Button.
	void on_btn_draw_clicked();
	///Action for clicking on RegionGrow-Button.
	void on_btn_regionGrow_clicked();
	///Action for clicking on Erode-Button.
	void on_btn_erode_clicked();
	///Action for clicking on Erode-Button.
	void on_btn_dilate_clicked();
//	void on_btn_cannyEdges_clicked();
	///Action for clicking on Analyse-Button.
	void on_btn_analyse_clicked();
	///Action for clicking on PerfusionMap-Button
	void on_btn_perfusionMap_clicked();
	///Action for double clicking on a mprWidget.
	void mprWidget_doubleClicked(MultiPlanarReformatWidget &w);
	///Action for double clicking on a mytabwidget.
	void tabWidget_doubleClicked(MyTabWidget &w);
        ///Action for clicking on autoAlignHeart-Button
        void on_btn_autoAlignHeart_clicked();
	///Action for clicking on clearPlot-Button
	void on_btn_clearPlot_clicked();
	///Action for clicking on writeResult-Button
	void on_btn_writeResults_clicked();
	///Action for clicking on Play-button
	void on_btn_play_clicked();
	///Slot for the contextmenu of the treeView.
	/*! 
	\param pos contains the position of the contextmenu.
	*/
	void treeViewContextMenu(const QPoint &pos);
	///creates a segment for selected image.
	void createSegmentForSelectedImage();
	///changes the color for the selected item.
	void changeColorForSelectedSegment();
	///removes a selected Image from the image model.
	void removeSelectedImages();
	///Slot will be called, if the value of the startSlider has changed.
	void sliderStartValue_changed();
	///Slot will be called, if the value of the endSlider has changed.
	void sliderEndValue_changed();
	///Slot will be called, if the table was clicked.
	void tableGamma_clicked(const QModelIndex & index);
	///Slot will be called, if the checkbutton was toggled.
	void cb_enableGamma_toggled();
	///Slot will be called, if the button was pressed
	void on_btn_arteryInput_selected(const SegmentInfo *segment);
	///Slot will be called, if the slider value has changed
	void slider_opacity_changed();
	///Slot will be called, if the slider value has changed
	void slider_thickness_changed();
	///Renames a selected item
	void renameTreeviewItem();
	///Slot will be called, if the value of the loopSpeed slider has changed
	void on_slider_loopSpeed_changed();
protected:
	///clears pending actions
    void clearPendingAction();

signals:
	void HighResolutionLoaded(const CTImageTreeItem* imageItem);
	
protected slots:
	void SetHighResolutionImage(const CTImageTreeItem *imageItem);
	///sets the next image of the image tree to the output view
	void setNextImage();

protected slots:

private:
	///Get selected TreeView-Item
	/*! 
	\return a pointer to the selected BinaryImageTreeItem
	*/
	BinaryImageTreeItem *focusSegmentFromSelection(void);
	typedef std::auto_ptr<DicomSelectorDialog> DicomSelectorDialogPtr;
	//vtkSmartPointer<vtkImageViewer2> m_pViewer[4];

	/// Designer form
	Ui_KardioPerfusion *m_ui;

	///load selected Dicom-File
	/*! 
	\param dicomSelector contains a pointer to the Selector-Dialog
	*/
	void loadDicomData(DicomSelectorDialogPtr dicomSelector);
	///sets a given Image to the output view
	/*! 
	\param imageItem contains the image that should be visualized
	*/
	void setImage(const CTImageTreeItem *imageItem);
	///show a segment
	/*! 
	\param segItem contains a pointer to the BinaryImageTreeItem
	*/
	void segmentShow(const BinaryImageTreeItem *segItem );
	///hide a segment
	/*! 
	\param segItem contains a pointer to the BinaryImageTreeItem
	*/
	void segmentHide(const BinaryImageTreeItem *segItem );
	///show a perfusion map
	/*! 
	\param perfItem contains a pointer to the BinaryImageTreeItem
	*/
	void perfusionMapShow( RealImageTreeItem *perfItem );
	///hide a perfusion map
	/*! 
	\param perfItem contains a pointer to the BinaryImageTreeItem
	*/
	void perfusionMapHide(const RealImageTreeItem *perfItem );
	///Callback to update the widgets
	static void updateFunc(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData );
	///load high resolution image of ctimagetreeitem's in the stack
	void loadHighResolution(); 
	///Check if stack of items for loading high resolution is empty, with use of mutex.
	bool loadHighResItemStackEmpty();
	
private:
	
	std::shared_ptr<CTImageTreeModel> m_imageModelPtr;
	CTImageTreeItem::ConnectorHandle m_displayedCTImage;

        typedef std::set< BinaryImageTreeItem::ConnectorHandle > DisplayedSegmentContainer;
	DisplayedSegmentContainer m_displayedSegments;

	typedef std::set< RealImageTreeItem::ConnectorHandle > DisplayedPerfusionMapContainer;
	DisplayedPerfusionMapContainer m_displayedPerfusionMaps;


	static const DicomTagList m_CTModelHeaderFields;
	int  m_pendingAction;
	bool m_oneWindowIsMax;
	MaxSlopeAnalyzer* m_maxSlopeAnalyzer;

	QwtPlotMarker *m_markerStart, *m_markerEnd;
	QwtPlotMarker *m_markerPickerX, *m_markerPickerY;
	QwtPlotGrid *m_grid;
	QwtPlotPicker *m_picker;
	vtkLookupTable *m_perfusionColorMap;
	QTimer *m_imageLoopTimer; ///< Timer for the image loop
	bool m_play; ///< True if image loop plays
	bool m_modelChanged; ///< True if model is changed until frist image is set
	std::thread *m_loadHighResThread; ///< Thread for loading high resolution of CTImageTreeItem's
	std::stack<CTImageTreeItem*> *m_loadHighResItemStack; ///< Stack of CTImageTreeItem's for loading high resolution
	std::mutex m_loadHighResThreadMutex; ///< Mutex for locking the loadHighResolution thread
	std::mutex m_loadHighResItemStackMutex; ///< Mutex for locking the loadHighResolution item stack
	std::mutex m_modelMutex; ///< Mutex for locking the image tree model
};

#endif // KardioPerfusion_H
