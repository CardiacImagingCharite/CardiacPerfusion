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

#ifndef itkFindLeftVentricle_H
#define itkFindLeftVentricle_H

#include "itkImageTransformer.h"

namespace itk{
    /** \class FindLeftVentricle                                                                                                                                                                                                         
     * \brief find the image of an image model, where the left                                                                                                                                                                           
     * ventricle only is filled with contrast agent                                                                                                                                                                                      
     *                                                                                                                                                                                                                                   
     * FindLeftVentricle search and returns the index of the image                                                                                                                                                                       
     * of an image model, where the left verntricle only is filled                                                                                                                                                                       
     * with contrast agent. The idea is that the number of the                                                                                                                                                                           
     * maximum enhencement of HU compared to the first phase                                                                                                                                                                             
     * (image) is maximal for the phase (image) wiht the left                                                                                                                                                                            
     * ventricle is fill with contrast agent. The procedure is as                                                                                                                                                                        
     * follows:                                                                                                                                                                                                                          
     *                                                                                                                                                                                                                                   
     * - save the value of each pixel in the first image : N                                                                                                                                                                             
     * - save the maximum value (M) and the corresponding image (time) index (T) of                                                                                                                                                      
     *   each pixel of all images                                                                                                                                                                                                        
     * - for each pixel: if (M-N > threshold) fill image (time) index histogram with T                                                                                                                                                   
     * - the maximum bin of the image index histogram is the ventricle with                                                                                                                                                              
     *   left ventricle filled with contrast agent                                                                                                                                                                                       
     */
    
    
    template <class TInputImage>
    class ITK_EXPORT FindLeftVentricle:public ImageTransformer<TInputImage>
    {
	
    public:
	/** Standard class typedefs. */
	typedef FindLeftVentricle             Self;
	typedef ImageTransformer<TInputImage> Superclass;
	typedef SmartPointer<Self>            Pointer;
	typedef SmartPointer<const Self>      ConstPointer;
	
	/** Method for creation through the object factory. */
	itkNewMacro(Self);
	
	/** Run-time type information (and related methods). */
	itkTypeMacro(FindLeftVentricle, ImageTransformer);
	
	/** Some convenient typedefs. */
	typedef TInputImage                         InputImageType;
	typedef typename InputImageType::Pointer    InputImagePointer;
	typedef typename InputImageType::RegionType InputImageRegionType;
	typedef typename InputImageType::PixelType  InputImagePixelType; 
	
	FindLeftVentricle();
	virtual ~FindLeftVentricle() {};
	void setMatrixSize(int MatrixSize);
	/** Get the index number of the image where the left ventricle 
         * only is filled with contrast agent
         */
	int GetImageIndex(CTImageTreeModel *model);
	
    private:
	    int m_AimedMatrixSize;
	
    };
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFindLeftVentricle.txx"
#endif

#endif