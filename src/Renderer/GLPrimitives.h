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

#ifndef GLPRIMITIVES_H
#define GLPRIMITIVES_H

#include <GL/gl.h>

// Constants used by primitive drawing functions
#define ROOM_SIZE 0.47f
#define MARKER_SIZE (ROOM_SIZE/1.85)

// Draw a marker around a room position
// mode 1 = full marker with border, mode 2 = partial marker (triangles only)
void glDrawMarkerPrimitive(int dx, int dy, int dz, int mode);

// Draw a 3D cone primitive (directional indicator)
void glDrawConePrimitive();

#endif // GLPRIMITIVES_H
