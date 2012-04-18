
#ifndef PERFUSIONANALYZER_H
#define PERFUSIONANALYZER_H

#include <set>
#include "segmentlistmodel.h"
#include "qwt_plot.h"
#include <QWidget>

class CTImageTreeItem;

class PerfusionAnalyzer
{
	public:
		PerfusionAnalyzer(QWidget* p);
		~PerfusionAnalyzer();

		///Add an image to the dialog.
		/*!
		\param image The image, that is to be added.
		*/
		void addImage(CTImageTreeItem *image);
		///Add a segment to the dialog.
		/*!
		\param segment The segment, that is to be added. 
		*/
		void addSegment(BinaryImageTreeItem *segment);
		///Gets a QwtPlot with the time attenuation curves
		/*!
		\return QWT plot
		*/
		QwtPlot* getTacPlot();
		///Gets the values of the TAC
		/*!
		\return Comma separated String containing the tac values. 
				The segments are separated by "\r\n" and the values by ";".
		*/
		std::string getTacValues();
		///Abstract method for calculating the perfusion results.
		virtual void calculatePerfusionResults() = 0;

	private:

		void calculateTacValues();

		///A structure that represents a functor for comparing two times
		struct CTImageTimeCompareFunctor {
			typedef CTImageTreeItem * argT;
			bool operator()(const argT &x, const argT &y) const;
		};
		//definitions for an imageSet and a double vector
		typedef std::set<CTImageTreeItem *, CTImageTimeCompareFunctor> ImageSet;
		typedef std::vector< double > DoubleVector;
  

		DoubleVector times;
		ImageSet images;

		SegmentListModel segments;

		QwtPlot* plot;

		QWidget* parent;
};


#endif