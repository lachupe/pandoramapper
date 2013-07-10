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


#include <QGLWidget>
#include <cmath>

#include "Renderer/CFrustum.h"
#include "Renderer/renderer.h"

#include "Gui/mainwindow.h"

// We create an enum of the sides so we don't have to call each side 0 or 1.
// This way it makes it more understandable and readable when dealing with frustum sides.

float CFrustum::distance(float dx, float dy, float dz) {
    return   m_Frustum[FRONT][A] * dx +             
             m_Frustum[FRONT][B] * dy + 
             m_Frustum[FRONT][C] * dz +
             m_Frustum[FRONT][D];
}

void CFrustum::normalizePlane(float frustum[6][4], int side)
{
    // Here we calculate the magnitude of the normal to the plane (point A B C)
    // Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
    // To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
    float magnitude = (float)sqrt( frustum[side][A] * frustum[side][A] + 
                                   frustum[side][B] * frustum[side][B] + 
                                   frustum[side][C] * frustum[side][C] );

    // Then we divide the plane's values by it's magnitude.
    // This makes it easier to work with.
    frustum[side][A] /= magnitude;
    frustum[side][B] /= magnitude;
    frustum[side][C] /= magnitude;
    frustum[side][D] /= magnitude; 
}

void CFrustum::calculateFrustum(int cx, int cy, int cz)
{    
    float   proj[16];                               // This will hold our projection matrix
    float   modl[16];                               // This will hold our modelview matrix
    float   clip[16];                               // This will hold the clipping planes

    curx = cx;
    cury = cy;
    curz = cz;


    glPushMatrix ();
    
    glGetFloatv(GL_PROJECTION_MATRIX, proj);

    glGetFloatv(GL_MODELVIEW_MATRIX, modl);
    
    glLoadMatrixf (proj);
    glMultMatrixf (modl);
    
    glGetFloatv(GL_MODELVIEW_MATRIX, clip);
    glPopMatrix (); 

    // This will extract the RIGHT side of the frustum
    m_Frustum[RIGHT][A] = clip[ 3] - clip[ 0];
    m_Frustum[RIGHT][B] = clip[ 7] - clip[ 4];
    m_Frustum[RIGHT][C] = clip[11] - clip[ 8];
    m_Frustum[RIGHT][D] = clip[15] - clip[12];

    // Now that we have a normal (A,B,C) and a distance (D) to the plane,
    // we want to normalize that normal and distance.

    // Normalize the RIGHT side
    normalizePlane(m_Frustum, RIGHT);

    // This will extract the LEFT side of the frustum
    m_Frustum[LEFT][A] = clip[ 3] + clip[ 0];
    m_Frustum[LEFT][B] = clip[ 7] + clip[ 4];
    m_Frustum[LEFT][C] = clip[11] + clip[ 8];
    m_Frustum[LEFT][D] = clip[15] + clip[12];

    // Normalize the LEFT side
    normalizePlane(m_Frustum, LEFT);

    // This will extract the BOTTOM side of the frustum
    m_Frustum[BOTTOM][A] = clip[ 3] + clip[ 1];
    m_Frustum[BOTTOM][B] = clip[ 7] + clip[ 5];
    m_Frustum[BOTTOM][C] = clip[11] + clip[ 9];
    m_Frustum[BOTTOM][D] = clip[15] + clip[13];

    // Normalize the BOTTOM side
    normalizePlane(m_Frustum, BOTTOM);

    // This will extract the TOP side of the frustum
    m_Frustum[TOP][A] = clip[ 3] - clip[ 1];
    m_Frustum[TOP][B] = clip[ 7] - clip[ 5];
    m_Frustum[TOP][C] = clip[11] - clip[ 9];
    m_Frustum[TOP][D] = clip[15] - clip[13];

    // Normalize the TOP side
    normalizePlane(m_Frustum, TOP);

    // This will extract the BACK side of the frustum
    m_Frustum[BACK][A] = clip[ 3] - clip[ 2];
    m_Frustum[BACK][B] = clip[ 7] - clip[ 6];
    m_Frustum[BACK][C] = clip[11] - clip[10];
    m_Frustum[BACK][D] = clip[15] - clip[14];

    // Normalize the BACK side
    normalizePlane(m_Frustum, BACK);

    // This will extract the FRONT side of the frustum
    m_Frustum[FRONT][A] = clip[ 3] + clip[ 2];
    m_Frustum[FRONT][B] = clip[ 7] + clip[ 6];
    m_Frustum[FRONT][C] = clip[11] + clip[10];
    m_Frustum[FRONT][D] = clip[15] + clip[14];

    // Normalize the FRONT side
    normalizePlane(m_Frustum, FRONT);
    
}

bool CFrustum::isPointInFrustum( float x, float y, float z )
{
    // Go through all the sides of the frustum
    for(int i = 0; i < 6; i++ )
    {
        // Calculate the plane equation and check if the point is behind a side of the frustum
        if(m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= 0)
        {
            // The point was behind a side, so it ISN'T in the frustum
            return false;
        }
    }

    // The point was inside of the frustum (In front of ALL the sides of the frustum)
    return true;
}


bool CFrustum::isSquareInFrustum(CSquare *p)
{
    float x, y, z, size;


    square_frustum_checks++;
    
    /* normalize the coordinates to the real view coordinates */
    /* see glDrawRoom for similar functions */
    x = p->centerx - curx;
    y = p->centery - cury;
    z = (renderer_window->renderer->current_plane_z - curz) /* * DIST_Z */;
    size = ( (p->rightx - p->leftx) / 2 /*- curz */) /* * DIST_Z */;

    for(int i = 0; i < 6; i++ )
    {
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (float) z + m_Frustum[i][D] >= 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (float) z + m_Frustum[i][D] >= 0)
           continue;
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (float) z + m_Frustum[i][D] >= 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (float) z + m_Frustum[i][D] >= 0)
           continue;

        // If we get here, it isn't in the frustum
        return false;
    }

    return true;
}
