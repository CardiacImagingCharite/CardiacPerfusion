/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
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

#ifndef PATLAKDATA_H
#define PATLAKDATA_H

#include <qwt_data.h>


class PatlakData : public QwtData
{
  public:
    PatlakData( const QwtData &tissueSource, const QwtData &arterySource );
    virtual double y(size_t i) const;
    virtual double x(size_t i) const;
    virtual size_t size() const;
    virtual QwtData* copy() const { return new PatlakData( *this ); }
    inline void setTissueBaseline(double tb) { m_tissueBaseline = tb; }
    inline void setArteryBaseline(double ab) { m_arteryBaseline = ab; }
    inline size_t getStartIndex() const { return m_startIndex; }
    inline size_t getEndIndex() const { return m_endIndex; }
    inline void setStartIndex(size_t index) { m_startIndex = std::min(index,std::min(m_tissue.size(), m_artery.size())-1); }
    inline void setEndIndex(size_t index) { m_endIndex = std::min(index,std::min(m_tissue.size(), m_artery.size())-1);  }
  private:
    const QwtData &m_tissue;
    const QwtData &m_artery;
    double m_tissueBaseline, m_arteryBaseline;
    size_t m_startIndex, m_endIndex;
};

#endif // PATLAKDATA_H
