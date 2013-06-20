/*
 * Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
 * 
 *    This file is part of KardioPerfusion.
 * 
 * KardioPerfusion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * KardioPerfusion is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with KardioPerfusion. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Diese Datei ist Teil von KardioPerfusion.
 * 
 * KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
 * veröffentlichten Version, weiterverbreiten und/oder modifizieren.
 * 
 * KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber
 * OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
 * Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Details.
 * 
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
 */

#include "itkFindLeftVentricle.h"

#include "ctimagetreemodel.h"
#include <itkTimeProbe.h>
#include <itkImageRegionConstIterator.h>
#include <vector>


namespace itk
{
	
	template <class TInputImage>
	FindLeftVentricle<TInputImage>::FindLeftVentricle()
	{
		m_AimedMatrixSize = 32;
	}
	
	template <class TInputImage>
	void FindLeftVentricle<TInputImage>::setMatrixSize(int MatrixSize)
	{
		m_AimedMatrixSize = MatrixSize;
	}
	
	template <class TInputImage>
	int
	FindLeftVentricle<TInputImage>::GetImageIndex(CTImageTreeModel *model)    
	{
		itk::TimeProbe clock;
		clock.Start();
		
		// get number of files		
		int rows = model->rowCount();
		std::cerr << "Number of input files = " << rows << endl;
		
		// Size of the images, value set when analysing 1st image
		unsigned int outputPixelNum = 1;
		
		typedef typename TInputImage::ValueType ValType;
		
		ValType Threshold = 150; // minimum (max value - value of 1st phase)
		ValType MinN = 30;       // minimum value of 1st phase
		ValType MaxN = 150;      // maximum value of 1st phase
		
		// value of 1st phase for each voxel
		std::vector<ValType> N;
		// maximum value for each voxel
		std::vector<ValType> Max;
		// number of phase for the maximum
		std::vector<ValType> Time;
		
		// histogram for the number of phases (of the maxima)
		std::vector<int> TimeHist(rows+1, 0);

		clock.Start();
		
		// loop over images
		for (int i = 0; i < model->rowCount(); i++) 
		{
			itk::TimeProbe clock2;
			// get image
			QModelIndex ImIdx = model->index(i, 1);
			TreeItem* item = &model->getItem(ImIdx);
			CTImageTreeItem *currentItem = dynamic_cast<CTImageTreeItem*>(item);
			clock2.Start();
			typename TInputImage::Pointer ImagePtr = currentItem->getITKImage();
			unsigned int currentShrinkFactor = currentItem->getCurrentImageShrinkFactor();
			
			clock2.Stop();
			std::cout << i << endl;
			std::cout << "getITKImage\t\t\t\tdone\t( " << clock2.GetMean() << "s )" << std::endl;

			// set shrink factors, assuming that x-size and y-size is the same
			unsigned int shrinkFactor = ImagePtr->GetLargestPossibleRegion().GetSize()[0] / m_AimedMatrixSize;
			shrinkFactor *= currentShrinkFactor;

			itk::TimeProbe clock4;
			clock4.Start();
			
			typename TInputImage::Pointer outImage = currentItem->getITKImageByShrinkFactor(shrinkFactor);
			
			clock4.Stop();
			std::cout << "getITKImageByShrinkFactor\t\tdone\t( " << clock4.GetMean() << "s )" << std::endl;

			typename TInputImage::SizeType outSize = outImage->GetLargestPossibleRegion().GetSize();
			
			// set vector size
			if ( i == 0 ) 
			{
				for ( unsigned int dim = 0; dim < TInputImage::ImageDimension; dim++ )
				{
					outputPixelNum *= outSize[dim];
				}
				
				N.resize(outputPixelNum);
				Max.resize(outputPixelNum);
				Time.resize(outputPixelNum);
			}
			
			typename TInputImage::IndexType VoIdx; // Voxel index
			
			typedef ImageRegionConstIterator<TInputImage> ImageIteratorType;
			ImageIteratorType imIt( outImage, outImage->GetLargestPossibleRegion() );
			
			unsigned int outpix = 0;
			
			while ( !imIt.IsAtEnd() )
			{
				ValType val = imIt.Get();
				
				if ( i == 0 )
				{
					N[outpix] = val;
					Max[outpix] = val;
				}
				
				if ( val > Max[outpix] )
				{
					Max[outpix] = val;
					Time[outpix] = i;
				}
				
				++outpix;
				++imIt;
			}

			
		}
		
		// analyse
		
		// loop over all voxel
		for ( unsigned int pix = 0; pix < outputPixelNum; pix ++ )
		{
			if ( Max[pix] - N[pix] > Threshold && N[pix] > MinN && N[pix] < MaxN ) TimeHist[Time[pix]]++;
			else TimeHist[rows]++;
		}
		
		// find the maximum bin
		
		int maxbin = 0; // bin number of maximum
		int maxvalue = 0; // maximum value
		
		// loop over all bin in TimeHist
		for (int i = 0; i < rows; i++ )
		{
			std::cout << "TimeHist[" << i << "] = " << TimeHist[i] << endl;
			if ( TimeHist[i] > maxvalue )
			{
				maxvalue = TimeHist[i];
				maxbin = i;
			}
		}   
		
		clock.Stop();
		std::cout << "analyse\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
		
		return maxbin;
	}
}
