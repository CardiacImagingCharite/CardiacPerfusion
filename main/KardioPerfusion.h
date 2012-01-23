/*=========================================================================

  Program:   Visualization Toolkit
  Module:    SimpleView.h
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
#ifndef SimpleView_H
#define SimpleView_H

#include <QMainWindow>
#include <QObject>
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
#include "vtkImageViewer2.h"
#include "ctimagetreemodel.h"
#include "binaryimagetreeitem.h"
#include "ctimagetreeitem.h"
#include <qitemselectionmodel.h>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// Forward Qt class declarations
class Ui_SimpleView;
class DicomSelectorDialog;

class SimpleView : public QMainWindow
{
  Q_OBJECT

public:

  // Constructor/Destructor
  SimpleView(); 
  ~SimpleView();
  void loadFile(QString fname);
  void setFiles(const QStringList &names);

public slots:

  virtual void slotOpenFile();
  virtual void slotExit();
  void on_treeView_doubleClicked(const QModelIndex &index);
  void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

protected:
   
protected slots:

private:

	typedef std::auto_ptr<DicomSelectorDialog> DicomSelectorDialogPtr;
	vtkSmartPointer<vtkImageViewer2> m_pViewer[4];
    // Designer form
    Ui_SimpleView *ui;

	void loadDicomData(DicomSelectorDialogPtr dicomSelector);
	void setImage(const CTImageTreeItem *imageItem);

	CTImageTreeModel imageModel;
	CTImageTreeItem::ConnectorHandle displayedCTImage;

	static const DicomTagList CTModelHeaderFields;
    int pendingAction;
};

#endif // SimpleView_H
