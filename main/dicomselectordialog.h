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

/*! \class DicomSelectorDialog DicomSelectorDialog.h "DicomSelectorDialog.h"
 *  \brief This is the associated class to the GUI form of the DicomSelector.
 *
 */
class DicomSelectorDialog : public QDialog, private Ui_DicomSelectDialog
{
	public:
	/// Constructor of the dialog
	DicomSelectorDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
	///Sets a path to a file or a directory.
	/*!
	\param name Contains the filename to the file or directory.
	*/
	void setFileOrDirectory( const QString &name ) { fileNames.clear(); fileNames.push_back( name ); }
	///Sets a list of filenames.
	/*!
	\param names Contains the names to the files.
	*/
	void setFilesOrDirectories( const QStringList &names ) { fileNames = names; }
	/// Gets a list of the selected image data.
	/*!
	\param other Contains the image model.
	*/
	void getSelectedImageDataList(CTImageTreeModel &other) const;
	/// Executes the dialog.
	virtual void exec();
	///Hides the dialog and sets the result code to Rejected.
	virtual void reject();
  
	private:
	CTImageTreeModel ctImageModel;
	QStringList fileNames;
	static const DicomTagList HeaderFields;
};




#endif // DICOMSELECTORDIALOG_H
