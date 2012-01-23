/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DICOMSELECTORDIALOG_H
#define DICOMSELECTORDIALOG_H

#include <QDialog>
#include "ui_DicomSelectDialog.h"
#include <boost/shared_ptr.hpp>
#include <boost/assign.hpp>
#include "ctimagetreemodel.h"
#include "dicomtagtype.h"

class DicomSelectorDialog : public QDialog, private Ui_DicomSelectDialog
{
  public:
  DicomSelectorDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
  void setFileOrDirectory( const QString &name ) { fileNames.clear(); fileNames.push_back( name ); }
  void setFilesOrDirectories( const QStringList &names ) { fileNames = names; }
  void getSelectedImageDataList(CTImageTreeModel &other) const;
  
  virtual void exec();
  virtual void reject();
  
  private:
  CTImageTreeModel ctImageModel;
  QStringList fileNames;
  static const DicomTagList HeaderFields;
};




#endif // DICOMSELECTORDIALOG_H
