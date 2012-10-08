/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
	Copyright 2010 Henning Meyer

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
