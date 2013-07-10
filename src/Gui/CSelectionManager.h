/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CSELECTIONMANAGER_H
#define CSELECTIONMANAGER_H

#include <QObject>
#include <QSet>
#include <QPoint>

class CSelectionManager : public QObject {
Q_OBJECT
    QSet<int> selection;
    
public:
    CSelectionManager();
    ~CSelectionManager();
    void resetSelection();

    inline bool isSelected(unsigned int id) { return selection.contains(id); }

    void select(unsigned int id);
    void unselect(unsigned int id);

    unsigned int getFirst() { return selection.values().at(0); }
    unsigned int get(int index) { return selection.values().at(index); }

    int size() {return selection.size(); }
    void exclusiveSelection(unsigned int id); 

    QList<int>  getList() { return selection.toList(); }
    bool isEmpty() { return selection.empty(); }

signals:
  void roomSelected(unsigned int id);
};

class CMouseState : public QObject {
Q_OBJECT

public:
    bool          LeftButtonPressed;
    bool          RightButtonPressed;
    QPoint        oldPos;       // last registred position
    QPoint        origPos;      // original position of click


    CMouseState();
    int delta(QPoint pos);  // delta from original Click position to given pos
}; 

#endif
