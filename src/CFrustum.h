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

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "CSquare.h"

class CFrustum {
    float         m_Frustum[6][4];
    void          normalizePlane(float frustum[6][4], int side);
    int           square_frustum_checks;

    
    enum FrustumSide
    {
        RIGHT   = 0,        // The RIGHT side of the frustum
        LEFT    = 1,        // The LEFT  side of the frustum
        BOTTOM  = 2,        // The BOTTOM side of the frustum
        TOP     = 3,        // The TOP side of the frustum
        BACK    = 4,        // The BACK side of the frustum
        FRONT   = 5         // The FRONT side of the frustum
    }; 
    
    // Like above, instead of saying a number for the ABC and D of the plane, we
    // want to be more descriptive.
    enum PlaneData
    {
        A = 0,              // The X value of the plane's normal
        B = 1,              // The Y value of the plane's normal
        C = 2,              // The Z value of the plane's normal
        D = 3               // The distance the plane is from the origin
    };
    
    int curx, cury, curz;
    int current_plane_z;

public:
    void calculateFrustum(int cx, int cy, int cz);
    bool isPointInFrustum(float x, float y, float z);
    bool isSquareInFrustum(CSquare *p);
    float distance(float x, float y, float z);
};

#endif
