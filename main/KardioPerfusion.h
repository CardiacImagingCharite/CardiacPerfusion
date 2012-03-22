/*=========================================================================

  Program:   Visualization Toolkit
  Module:    KardioPerfusion.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/
#ifndef KardioPerfusion_H
#define KardioPerfusion_H

#include <QMainWindow>
#include <QObject>
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
//#include "vtkImageViewer2.h"
#include "ctimagetreemodel.h"
#include "binaryimagetreeitem.h"
#include "ctimagetreeitem.h"
#include <qitemselectionmodel.h>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// Forward Qt class declarations
class Ui_KardioPerfusion;
class DicomSelectorDialog;
class MultiPlanarReformatWidget;

class KardioPerfusion : public QMainWindow
{
  Q_OBJECT

public:

  // Constructor/Destructor
  KardioPerfusion(); 
  ~KardioPerfusion();

//
//  void loadFile(QString fname);
  //set the selected files to the SelectorDialog
  void setFiles(const QStringList &names);

public slots:

  // Action to be taken upon file open
  virtual void slotOpenFile();
  //Action to exit the application
  virtual void slotExit();
  //Button- and TreeView-Actions for interaction
  void on_treeView_doubleClicked(const QModelIndex &index);
  void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
  void on_btn_draw_clicked();
  void on_btn_regionGrow_clicked();
  void on_btn_erode_clicked();
  void on_btn_dilate_clicked();
  void on_btn_cannyEdges_clicked();
  void on_btn_analyse_clicked();
  void treeViewContextMenu(const QPoint &pos);
  //create a segment for selected image
  void createSegmentForSelectedImage();
  //change the color for the selected item
  void changeColorForSelectedSegment();

protected:
	//clear pending actions
     void clearPendingAction();
protected slots:

private:
	//Get selected TreeView-Item 
	BinaryImageTreeItem *focusSegmentFromSelection(void);
	typedef std::auto_ptr<DicomSelectorDialog> DicomSelectorDialogPtr;
	//vtkSmartPointer<vtkImageViewer2> m_pViewer[4];

    // Designer form
    Ui_KardioPerfusion *ui;

	//load selected Dicom-File
	void loadDicomData(DicomSelectorDialogPtr dicomSelector);
	//sets a given Image to the output view
	void setImage(const CTImageTreeItem *imageItem);
	//show/hide a given segment
	void segmentShow(const BinaryImageTreeItem *segItem );
	void segmentHide(const BinaryImageTreeItem *segItem );
	//removes a selected Image from the image model
	void removeSelectedImages();
	//set interaction style
//	void setCustomStyle();

	CTImageTreeModel imageModel;
	CTImageTreeItem::ConnectorHandle displayedCTImage;

	typedef std::set< BinaryImageTreeItem::ConnectorHandle > DisplayedSegmentContainer;
    DisplayedSegmentContainer displayedSegments;

	static const DicomTagList CTModelHeaderFields;
    int pendingAction;
};

#endif // KardioPerfusion_H
