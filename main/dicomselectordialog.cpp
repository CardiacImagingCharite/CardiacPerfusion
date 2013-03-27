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

#define BOOST_FILESYSTEM_VERSION 2

#include "dicomselectordialog.h"
#include <QFileSystemModel>
#include <QProgressDialog>
#include <QPushButton>
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include <vector>
#include <utility>
#include <list>
#include <itkImageFileReader.h>
#include "imagedefinitions.h"
#include "ctimagetreeitem.h"

const DicomTagList DicomSelectorDialog::HeaderFields = boost::assign::list_of
	(DicomTagType("Patient Name", "0010|0010"))
	(DicomTagType("#Slices",CTImageTreeItem::getNumberOfFramesTag()))
	(DicomTagType("AcquisitionDatetime","0008|002a"))
	(DicomTagType("SeriesDate","0008|0021"))
	(DicomTagType("SeriesTime","0008|0031"))
	(DicomTagType("Date of Birth","0010|0030"))
	(DicomTagType("Series Description","0008|103e"));
  
DicomSelectorDialog::DicomSelectorDialog(QWidget * parent, Qt::WindowFlags f):
	QDialog( parent, f ), ctImageModel(HeaderFields) {
	setupUi( this );
	buttonBox->button( QDialogButtonBox::Open )->setAutoDefault(true);
	buttonBox->button( QDialogButtonBox::Cancel)->setAutoDefault(false);
}

void DicomSelectorDialog::reject() {
	treeView->clearSelection();
	QDialog::reject();
}


void DicomSelectorDialog::exec() {
	typedef itk::ImageFileReader< CTImageType >  ReaderType;
	int index = 0; bool canceled = false;
	
	QProgressDialog indexProgress(tr("Indexing Files..."), tr("Abort"), 0, fileNames.size(), this);
	indexProgress.setMinimumDuration(1000);
	indexProgress.setWindowModality(Qt::ApplicationModal);
	while( index < fileNames.size() ) {
		indexProgress.setValue(index + 1);
		if (indexProgress.wasCanceled()) break;
		if ( boost::filesystem::is_directory( fileNames[index].toAscii().data() ) ) 
		{
			boost::filesystem::path fpath( fileNames.takeAt(index).toAscii().data() );
			QList< boost::filesystem::path > pathList;
			boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
			pathList.push_back( fpath );
			indexProgress.setMaximum(fileNames.size() + pathList.size());
			while( !pathList.isEmpty() ) {
				if (indexProgress.wasCanceled()) break;
				boost::filesystem::path currentPath = pathList.takeFirst();
				for ( boost::filesystem::directory_iterator itr( currentPath ); itr != end_itr; ++itr ) 
				{
					if (indexProgress.wasCanceled()) break;
					if ( boost::filesystem::is_directory(itr->status()) ) 
					{
						pathList.push_back( itr->path() );
						indexProgress.setMaximum(fileNames.size() + pathList.size());
						indexProgress.setValue(index);
					}
					else if ( boost::filesystem::is_regular_file( itr->status() )) 
					{
						fileNames.push_back( itr->path().directory_string().c_str() );
					}
				}
			}
		} else {
			index++;
		}
	}
	canceled = indexProgress.wasCanceled();
	
	fileNames.removeDuplicates();
	if (!canceled ) {
		QProgressDialog metaReadProgress(tr("Reading MetaData..."), tr("Abort"), 0, fileNames.size(), this);
		metaReadProgress.setMinimumDuration(1000);
		metaReadProgress.setWindowModality(Qt::ApplicationModal);
		for(int i = 0; i < fileNames.size(); i++) {
			metaReadProgress.setValue(i);
			if (metaReadProgress.wasCanceled()) 
				break;
			boost::filesystem::path fpath( fileNames[i].toAscii().data() );
			if ( boost::filesystem::is_regular_file( fpath ) ) {
				try {
					ReaderType::Pointer reader = ReaderType::New();
					reader->SetFileName( fpath.string() );
					reader->GenerateOutputInformation();
					ctImageModel.appendFilename( reader->GetMetaDataDictionary(), fpath.string() );
				} catch (itk::ImageFileReaderException &ifrExep) {
					std::cerr << "Exception caught !" << std::endl;
					std::cerr << ifrExep << std::endl;
				} catch (itk::ExceptionObject & excep) {
					std::cerr << "Exception caught !" << std::endl;
					std::cerr << excep << std::endl;
				}
			}
		}
	}
	if (ctImageModel.rowCount(QModelIndex())==0) return;

	treeView->setModel( &ctImageModel );
	treeView->selectAll();
	for(unsigned int t=0; t < HeaderFields.size(); t++) treeView->resizeColumnToContents(t);
	
	treeView->setSortingEnabled(true);
	treeView->sortByColumn(2,Qt::AscendingOrder);
	
	QDialog::exec();
}

void DicomSelectorDialog::getSelectedImageDataList(CTImageTreeModel &other) const {
	QItemSelectionModel *selectionModel = treeView->selectionModel();
	if (selectionModel == NULL) return;
	QModelIndexList selectedIndexes = selectionModel->selectedRows();
	for(QModelIndexList::const_iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); it++) {
		other.insertItemCopy( ctImageModel.getItem(*it) );
	}
}
