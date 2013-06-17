/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie

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

    KardioPerfusion ist Freie Software: Sie k�nnen es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
    ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es n�tzlich sein wird, aber
    OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License f�r weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#include "maxSlopeAnalyzer.h"

MaxSlopeAnalyzer::MaxSlopeAnalyzer(QWidget *p) : PerfusionAnalyzer(p)
{

}

MaxSlopeAnalyzer::~MaxSlopeAnalyzer()
{

}

void MaxSlopeAnalyzer::calculatePerfusionResults()
{

}

void MaxSlopeAnalyzer::setGammaStartIndex(int value, const QModelIndexList& indexList)
{

	SegmentInfo &seg = m_segments->getSegment(indexList[0]);
	seg.setGammaStartIndex(value);
}

void MaxSlopeAnalyzer::recalculateGamma(SegmentInfo &seginfo)
{
	recalculateData(seginfo);
	//segments->refresh();
}

void MaxSlopeAnalyzer::recalculateGamma(const QModelIndexList& indexList)
{
	SegmentInfo &seg = m_segments->getSegment(indexList[0]);
	recalculateData(seg);
}

void MaxSlopeAnalyzer::setGammaEndIndex(int value, const QModelIndexList& indexList)
{
	SegmentInfo &seg = m_segments->getSegment(indexList[0]);
	seg.setGammaEndIndex(value);
}

void MaxSlopeAnalyzer::recalculateData(SegmentInfo &seginfo)
{
	seginfo.recalculateGamma();
	m_segments->refresh();
}