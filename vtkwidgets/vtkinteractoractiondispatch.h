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

#ifndef VTKINTERACTORACTIONDISPATCH_H
#define VTKINTERACTORACTIONDISPATCH_H

#include "signalslib.hpp"
#include <boost/shared_ptr.hpp>

typedef boost::signal<void (int, int, float , float, float)> ActionSignal; // dx, dy, posx, posy, posz
typedef ActionSignal::slot_type ActionSlotType;
struct ActionDispatch {
  enum ActionType {
    MovingAction,
    ClickingAction
  };
  enum RestrictionType {
    Restricted,
    UnRestricted
  };
  boost::shared_ptr< ActionSignal > m_sig;
  std::string m_label;
  ActionType m_atype;
  RestrictionType m_restrict;
  bool m_valid;
  ActionDispatch(const std::string &label_, const ActionSignal::slot_type &slot, ActionType atype_, RestrictionType restrict_ )
    :m_sig( new ActionSignal ), m_label(label_), m_atype(atype_), m_restrict(restrict_), m_valid( true ) { m_sig->connect(slot); }
  ActionDispatch():m_valid(false) {}
};


#endif // VTKINTERACTORACTIONDISPATCH_H