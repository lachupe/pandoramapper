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

#include "GLPrimitives.h"
#include "Renderer/renderer.h"

static const float markerSize = ROOM_SIZE / 1.85f;

void glDrawMarkerPrimitive(int dx, int dy, int dz, int mode)
{
    /* upper */
    glBegin(GL_TRIANGLES);
    glVertex3f(dx, markerSize + dy + ROOM_SIZE, 0.0f + dz);
    glVertex3f(-markerSize + dx, dy + ROOM_SIZE, 0.0f + dz);
    glVertex3f(+markerSize + dx, dy + ROOM_SIZE, 0.0f + dz);
    glEnd();

    /* lower */
    glBegin(GL_TRIANGLES);
    glVertex3f(dx, -markerSize + dy - ROOM_SIZE, 0.0f + dz);
    glVertex3f(-markerSize + dx, dy - ROOM_SIZE, 0.0f + dz);
    glVertex3f(+markerSize + dx, dy - ROOM_SIZE, 0.0f + dz);
    glEnd();

    /* right */
    glBegin(GL_TRIANGLES);
    glVertex3f(dx + ROOM_SIZE, +markerSize + dy, 0.0f + dz);
    glVertex3f(markerSize + dx + ROOM_SIZE, dy, 0.0f + dz);
    glVertex3f(dx + ROOM_SIZE, -markerSize + dy, 0.0f + dz);
    glEnd();

    /* left */
    glBegin(GL_TRIANGLES);
    glVertex3f(dx - ROOM_SIZE, +markerSize + dy, 0.0f + dz);
    glVertex3f(-markerSize + dx - ROOM_SIZE, dy, 0.0f + dz);
    glVertex3f(dx - ROOM_SIZE, -markerSize + dy, 0.0f + dz);
    glEnd();

    if (mode == 1) {
        /* left */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy - ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE, dy - ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE, dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glEnd();

        /* right */
        glBegin(GL_QUADS);
        glVertex3f(dx + ROOM_SIZE, dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE, dy - ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy - ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glEnd();

        /* upper */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy + ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy + ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy + ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glEnd();

        /* lower */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy - ROOM_SIZE, 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (markerSize / 3.5f), dy - ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy - ROOM_SIZE + (markerSize / 3.5f), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (markerSize / 3.5f), dy - ROOM_SIZE, 0.0f + dz);
        glEnd();
    }
}

void glDrawConePrimitive()
{
    glNormal3f(0.634392, 0.773011, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.036944, 0.059248, -0.361754);
    glVertex3f(0.036944, 0.059248, -0.000645);
    glVertex3f(0.046794, 0.051164, -0.000645);
    glVertex3f(0.046794, 0.051164, -0.361754);
    glEnd();

    glNormal3f(0.471394, 0.881923, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.025707, 0.065254, -0.361754);
    glVertex3f(0.025706, 0.065255, -0.000645);
    glVertex3f(0.036944, 0.059248, -0.000645);
    glVertex3f(0.036944, 0.059248, -0.361754);
    glEnd();

    glNormal3f(0.290282, 0.956941, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.013513, 0.068953, -0.361754);
    glVertex3f(0.013512, 0.068953, -0.000645);
    glVertex3f(0.025706, 0.065255, -0.000645);
    glVertex3f(0.025707, 0.065254, -0.361754);
    glEnd();

    glNormal3f(0.098014, 0.995185, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.070202, -0.361754);
    glVertex3f(0.000832, 0.070202, -0.000645);
    glVertex3f(0.013512, 0.068953, -0.000645);
    glVertex3f(0.013513, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.098020, 0.995184, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.011849, 0.068953, -0.361754);
    glVertex3f(-0.011849, 0.068953, -0.000645);
    glVertex3f(0.000832, 0.070202, -0.000645);
    glVertex3f(0.000832, 0.070202, -0.361754);
    glEnd();

    glNormal3f(-0.290287, 0.956940, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.024042, 0.065254, -0.361754);
    glVertex3f(-0.024043, 0.065254, -0.000645);
    glVertex3f(-0.011849, 0.068953, -0.000645);
    glVertex3f(-0.011849, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.471399, 0.881920, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.035280, 0.059248, -0.361754);
    glVertex3f(-0.035280, 0.059247, -0.000645);
    glVertex3f(-0.024043, 0.065254, -0.000645);
    glVertex3f(-0.024042, 0.065254, -0.361754);
    glEnd();

    glNormal3f(-0.634395, 0.773009, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.045130, 0.051164, -0.361754);
    glVertex3f(-0.045130, 0.051164, -0.000645);
    glVertex3f(-0.035280, 0.059247, -0.000645);
    glVertex3f(-0.035280, 0.059248, -0.361754);
    glEnd();

    glNormal3f(-0.773012, 0.634392, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.053213, 0.041314, -0.361754);
    glVertex3f(-0.053213, 0.041314, -0.000645);
    glVertex3f(-0.045130, 0.051164, -0.000645);
    glVertex3f(-0.045130, 0.051164, -0.361754);
    glEnd();

    glNormal3f(-0.881922, 0.471395, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.059220, 0.030077, -0.361754);
    glVertex3f(-0.059220, 0.030076, -0.000645);
    glVertex3f(-0.053213, 0.041314, -0.000645);
    glVertex3f(-0.053213, 0.041314, -0.361754);
    glEnd();

    glNormal3f(-0.956941, 0.290283, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.062919, 0.017883, -0.361754);
    glVertex3f(-0.062919, 0.017883, -0.000645);
    glVertex3f(-0.059220, 0.030076, -0.000645);
    glVertex3f(-0.059220, 0.030077, -0.361754);
    glEnd();

    glNormal3f(-0.995185, 0.098016, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.064168, 0.005202, -0.361754);
    glVertex3f(-0.064168, 0.005202, -0.000645);
    glVertex3f(-0.062919, 0.017883, -0.000645);
    glVertex3f(-0.062919, 0.017883, -0.361754);
    glEnd();

    glNormal3f(-0.995185, -0.098018, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.062919, -0.007478, -0.361754);
    glVertex3f(-0.062919, -0.007479, -0.000645);
    glVertex3f(-0.064168, 0.005202, -0.000645);
    glVertex3f(-0.064168, 0.005202, -0.361754);
    glEnd();

    glNormal3f(-0.956940, -0.290286, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.059220, -0.019672, -0.361754);
    glVertex3f(-0.059220, -0.019672, -0.000645);
    glVertex3f(-0.062919, -0.007479, -0.000645);
    glVertex3f(-0.062919, -0.007478, -0.361754);
    glEnd();

    glNormal3f(-0.881921, -0.471398, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.053213, -0.030909, -0.361754);
    glVertex3f(-0.053213, -0.030910, -0.000645);
    glVertex3f(-0.059220, -0.019672, -0.000645);
    glVertex3f(-0.059220, -0.019672, -0.361754);
    glEnd();

    glNormal3f(-0.773010, -0.634394, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.045130, -0.040759, -0.361754);
    glVertex3f(-0.045129, -0.040759, -0.000645);
    glVertex3f(-0.053213, -0.030910, -0.000645);
    glVertex3f(-0.053213, -0.030909, -0.361754);
    glEnd();

    glNormal3f(-0.634392, -0.773011, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.035280, -0.048843, -0.361754);
    glVertex3f(-0.035280, -0.048843, -0.000645);
    glVertex3f(-0.045129, -0.040759, -0.000645);
    glVertex3f(-0.045130, -0.040759, -0.361754);
    glEnd();

    glNormal3f(-0.471396, -0.881922, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.024042, -0.054850, -0.361754);
    glVertex3f(-0.024042, -0.054850, -0.000645);
    glVertex3f(-0.035280, -0.048843, -0.000645);
    glVertex3f(-0.035280, -0.048843, -0.361754);
    glEnd();

    glNormal3f(-0.290285, -0.956940, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(-0.011849, -0.058548, -0.361754);
    glVertex3f(-0.011849, -0.058548, -0.000645);
    glVertex3f(-0.024042, -0.054850, -0.000645);
    glVertex3f(-0.024042, -0.054850, -0.361754);
    glEnd();

    glNormal3f(-0.098016, -0.995185, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, -0.059797, -0.361754);
    glVertex3f(0.000832, -0.059797, -0.000645);
    glVertex3f(-0.011849, -0.058548, -0.000645);
    glVertex3f(-0.011849, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.098017, -0.995185, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.013513, -0.058548, -0.361754);
    glVertex3f(0.013513, -0.058548, -0.000645);
    glVertex3f(0.000832, -0.059797, -0.000645);
    glVertex3f(0.000832, -0.059797, -0.361754);
    glEnd();

    glNormal3f(0.290284, -0.956940, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.025706, -0.054850, -0.361754);
    glVertex3f(0.025706, -0.054850, -0.000645);
    glVertex3f(0.013513, -0.058548, -0.000645);
    glVertex3f(0.013513, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.471397, -0.881921, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.036944, -0.048843, -0.361754);
    glVertex3f(0.036944, -0.048843, -0.000645);
    glVertex3f(0.025706, -0.054850, -0.000645);
    glVertex3f(0.025706, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.634393, -0.773011, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.046794, -0.040759, -0.361754);
    glVertex3f(0.046794, -0.040759, -0.000645);
    glVertex3f(0.036944, -0.048843, -0.000645);
    glVertex3f(0.036944, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.773010, -0.634394, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.054877, -0.030910, -0.361754);
    glVertex3f(0.054877, -0.030910, -0.000645);
    glVertex3f(0.046794, -0.040759, -0.000645);
    glVertex3f(0.046794, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.881921, -0.471397, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.060884, -0.019672, -0.361754);
    glVertex3f(0.060884, -0.019672, -0.000645);
    glVertex3f(0.054877, -0.030910, -0.000645);
    glVertex3f(0.054877, -0.030910, -0.361754);
    glEnd();

    glNormal3f(0.956940, -0.290285, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.064583, -0.007478, -0.361754);
    glVertex3f(0.064583, -0.007479, -0.000645);
    glVertex3f(0.060884, -0.019672, -0.000645);
    glVertex3f(0.060884, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.995185, -0.098018, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.065832, 0.005202, -0.361754);
    glVertex3f(0.065832, 0.005202, -0.000645);
    glVertex3f(0.064583, -0.007479, -0.000645);
    glVertex3f(0.064583, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.995185, 0.098015, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.064583, 0.017883, -0.361754);
    glVertex3f(0.064583, 0.017883, -0.000645);
    glVertex3f(0.065832, 0.005202, -0.000645);
    glVertex3f(0.065832, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.956941, 0.290284, -0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.060884, 0.030077, -0.361754);
    glVertex3f(0.060884, 0.030077, -0.000645);
    glVertex3f(0.064583, 0.017883, -0.000645);
    glVertex3f(0.064583, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.881922, 0.471396, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.054877, 0.041314, -0.361754);
    glVertex3f(0.054878, 0.041314, -0.000645);
    glVertex3f(0.060884, 0.030077, -0.000645);
    glVertex3f(0.060884, 0.030077, -0.361754);
    glEnd();

    glNormal3f(0.773011, 0.634393, 0.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.046794, 0.051164, -0.361754);
    glVertex3f(0.046794, 0.051164, -0.000645);
    glVertex3f(0.054878, 0.041314, -0.000645);
    glVertex3f(0.054877, 0.041314, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.046794, 0.051164, -0.000645);
    glVertex3f(0.036944, 0.059248, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.036944, 0.059248, -0.361754);
    glVertex3f(0.046794, 0.051164, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.036944, 0.059248, -0.000645);
    glVertex3f(0.025706, 0.065255, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.025707, 0.065254, -0.361754);
    glVertex3f(0.036944, 0.059248, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.025706, 0.065255, -0.000645);
    glVertex3f(0.013512, 0.068953, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.013513, 0.068953, -0.361754);
    glVertex3f(0.025707, 0.065254, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.013512, 0.068953, -0.000645);
    glVertex3f(0.000832, 0.070202, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.000832, 0.070202, -0.361754);
    glVertex3f(0.013513, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.000832, 0.070202, -0.000645);
    glVertex3f(-0.011849, 0.068953, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.011849, 0.068953, -0.361754);
    glVertex3f(0.000832, 0.070202, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.011849, 0.068953, -0.000645);
    glVertex3f(-0.024043, 0.065254, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.024042, 0.065254, -0.361754);
    glVertex3f(-0.011849, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.024043, 0.065254, -0.000645);
    glVertex3f(-0.035280, 0.059247, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.035280, 0.059248, -0.361754);
    glVertex3f(-0.024042, 0.065254, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.035280, 0.059247, -0.000645);
    glVertex3f(-0.045130, 0.051164, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.045130, 0.051164, -0.361754);
    glVertex3f(-0.035280, 0.059248, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.045130, 0.051164, -0.000645);
    glVertex3f(-0.053213, 0.041314, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.053213, 0.041314, -0.361754);
    glVertex3f(-0.045130, 0.051164, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.053213, 0.041314, -0.000645);
    glVertex3f(-0.059220, 0.030076, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.059220, 0.030077, -0.361754);
    glVertex3f(-0.053213, 0.041314, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.059220, 0.030076, -0.000645);
    glVertex3f(-0.062919, 0.017883, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.062919, 0.017883, -0.361754);
    glVertex3f(-0.059220, 0.030077, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.062919, 0.017883, -0.000645);
    glVertex3f(-0.064168, 0.005202, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.064168, 0.005202, -0.361754);
    glVertex3f(-0.062919, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.064168, 0.005202, -0.000645);
    glVertex3f(-0.062919, -0.007479, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.062919, -0.007478, -0.361754);
    glVertex3f(-0.064168, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.062919, -0.007479, -0.000645);
    glVertex3f(-0.059220, -0.019672, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.059220, -0.019672, -0.361754);
    glVertex3f(-0.062919, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.059220, -0.019672, -0.000645);
    glVertex3f(-0.053213, -0.030910, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.053213, -0.030909, -0.361754);
    glVertex3f(-0.059220, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.053213, -0.030910, -0.000645);
    glVertex3f(-0.045129, -0.040759, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.045130, -0.040759, -0.361754);
    glVertex3f(-0.053213, -0.030909, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.045129, -0.040759, -0.000645);
    glVertex3f(-0.035280, -0.048843, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.035280, -0.048843, -0.361754);
    glVertex3f(-0.045130, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.035280, -0.048843, -0.000645);
    glVertex3f(-0.024042, -0.054850, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.024042, -0.054850, -0.361754);
    glVertex3f(-0.035280, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.024042, -0.054850, -0.000645);
    glVertex3f(-0.011849, -0.058548, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(-0.011849, -0.058548, -0.361754);
    glVertex3f(-0.024042, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(-0.011849, -0.058548, -0.000645);
    glVertex3f(0.000832, -0.059797, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.000832, -0.059797, -0.361754);
    glVertex3f(-0.011849, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.000832, -0.059797, -0.000645);
    glVertex3f(0.013513, -0.058548, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.013513, -0.058548, -0.361754);
    glVertex3f(0.000832, -0.059797, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.013513, -0.058548, -0.000645);
    glVertex3f(0.025706, -0.054850, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.025706, -0.054850, -0.361754);
    glVertex3f(0.013513, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.025706, -0.054850, -0.000645);
    glVertex3f(0.036944, -0.048843, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.036944, -0.048843, -0.361754);
    glVertex3f(0.025706, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.036944, -0.048843, -0.000645);
    glVertex3f(0.046794, -0.040759, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.046794, -0.040759, -0.361754);
    glVertex3f(0.036944, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.046794, -0.040759, -0.000645);
    glVertex3f(0.054877, -0.030910, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.054877, -0.030910, -0.361754);
    glVertex3f(0.046794, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.054877, -0.030910, -0.000645);
    glVertex3f(0.060884, -0.019672, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.060884, -0.019672, -0.361754);
    glVertex3f(0.054877, -0.030910, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.060884, -0.019672, -0.000645);
    glVertex3f(0.064583, -0.007479, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.064583, -0.007478, -0.361754);
    glVertex3f(0.060884, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.064583, -0.007479, -0.000645);
    glVertex3f(0.065832, 0.005202, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.065832, 0.005202, -0.361754);
    glVertex3f(0.064583, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.065832, 0.005202, -0.000645);
    glVertex3f(0.064583, 0.017883, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.064583, 0.017883, -0.361754);
    glVertex3f(0.065832, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.064583, 0.017883, -0.000645);
    glVertex3f(0.060884, 0.030077, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.060884, 0.030077, -0.361754);
    glVertex3f(0.064583, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.060884, 0.030077, -0.000645);
    glVertex3f(0.054878, 0.041314, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.054877, 0.041314, -0.361754);
    glVertex3f(0.060884, 0.030077, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.000645);
    glVertex3f(0.054878, 0.041314, -0.000645);
    glVertex3f(0.046794, 0.051164, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000832, 0.005202, -0.361754);
    glVertex3f(0.046794, 0.051164, -0.361754);
    glVertex3f(0.054877, 0.041314, -0.361754);
    glEnd();

    glNormal3f(0.653378, 0.536213, 0.534390);
    glBegin(GL_POLYGON);
    glVertex3f(0.192695, 0.128755, 0.000645);
    glVertex3f(0.163874, 0.163874, 0.000645);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glEnd();

    glNormal3f(0.745118, 0.398271, 0.534957);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.214111, 0.088688, 0.000645);
    glVertex3f(0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(0.808079, 0.245128, 0.535650);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.227299, 0.045213, 0.000645);
    glVertex3f(0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(0.839872, 0.082722, 0.536443);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.231752, 0.000000, 0.000645);
    glVertex3f(0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(0.839327, -0.082666, 0.537305);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.227299, -0.045213, 0.000645);
    glVertex3f(0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(0.806524, -0.244657, 0.538202);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.214111, -0.088688, 0.000645);
    glVertex3f(0.227299, -0.045213, 0.000645);
    glEnd();

    glNormal3f(0.742791, -0.397029, 0.539100);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.192695, -0.128755, 0.000645);
    glVertex3f(0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.650634, -0.533961, 0.539965);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.163874, -0.163874, 0.000645);
    glVertex3f(0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.533636, -0.650238, 0.540762);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.128755, -0.192695, 0.000645);
    glVertex3f(0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.396315, -0.741453, 0.541463);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.088688, -0.214111, 0.000645);
    glVertex3f(0.128755, -0.192695, 0.000645);
    glEnd();

    glNormal3f(0.243941, -0.804167, 0.542041);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.045213, -0.227299, 0.000645);
    glVertex3f(0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(0.082341, -0.836028, 0.542473);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.000000, -0.231752, 0.000645);
    glVertex3f(0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.082325, -0.835853, 0.542745);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.045213, -0.227299, 0.000645);
    glVertex3f(-0.000000, -0.231752, 0.000645);
    glEnd();

    glNormal3f(-0.243790, -0.803670, 0.542845);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.088688, -0.214111, 0.000645);
    glVertex3f(-0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.466876, -0.702349, 0.537339);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.127585, -0.187549, 0.001567);
    glVertex3f(-0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.470363, -0.700011, 0.537348);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.163874, -0.163874, 0.000645);
    glVertex3f(-0.127585, -0.187549, 0.001567);
    glEnd();

    glNormal3f(-0.649565, -0.533083, 0.542114);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.192695, -0.128755, 0.000645);
    glVertex3f(-0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(-0.741400, -0.396286, 0.541557);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.214111, -0.088688, 0.000645);
    glVertex3f(-0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.804887, -0.244159, 0.540873);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.227299, -0.045212, 0.000645);
    glVertex3f(-0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.837556, -0.082492, 0.540088);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.231752, 0.000000, 0.000645);
    glVertex3f(-0.227299, -0.045212, 0.000645);
    glEnd();

    glNormal3f(-0.838103, 0.082547, 0.539231);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.227299, 0.045213, 0.000645);
    glVertex3f(-0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(-0.806442, 0.244633, 0.538336);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.214111, 0.088688, 0.000645);
    glVertex3f(-0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(-0.743727, 0.397532, 0.537436);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.192695, 0.128755, 0.000645);
    glVertex3f(-0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.652310, 0.535338, 0.536567);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.163873, 0.163874, 0.000645);
    glVertex3f(-0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.535661, 0.652707, 0.535762);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.128754, 0.192695, 0.000645);
    glVertex3f(-0.163873, 0.163874, 0.000645);
    glEnd();

    glNormal3f(-0.398244, 0.745064, 0.535052);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.088687, 0.214111, 0.000645);
    glVertex3f(-0.128754, 0.192695, 0.000645);
    glEnd();

    glNormal3f(-0.245345, 0.808797, 0.534465);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(-0.045212, 0.227299, 0.000645);
    glVertex3f(-0.088687, 0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.082869, 0.841398, 0.534024);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.000000, 0.231752, 0.000645);
    glVertex3f(-0.045212, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.082889, 0.841572, 0.533747);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.045213, 0.227299, 0.000645);
    glVertex3f(0.000000, 0.231752, 0.000645);
    glEnd();

    glNormal3f(0.245498, 0.809292, 0.533645);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.088688, 0.214111, 0.000645);
    glVertex3f(0.045213, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.398643, 0.745805, 0.533721);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.128755, 0.192695, 0.000645);
    glVertex3f(0.088688, 0.214111, 0.000645);
    glEnd();

    glNormal3f(0.536381, 0.653581, 0.533973);
    glBegin(GL_POLYGON);
    glVertex3f(0.000844, 0.002644, 0.361754);
    glVertex3f(0.163874, 0.163874, 0.000645);
    glVertex3f(0.128755, 0.192695, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.163874, 0.163874, 0.000645);
    glVertex3f(0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.192695, 0.128755, 0.000645);
    glVertex3f(0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.214111, 0.088688, 0.000645);
    glVertex3f(0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.227299, 0.045213, 0.000645);
    glVertex3f(0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.231752, 0.000000, 0.000645);
    glVertex3f(0.227299, -0.045213, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.227299, -0.045213, 0.000645);
    glVertex3f(0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.214111, -0.088688, 0.000645);
    glVertex3f(0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.192695, -0.128755, 0.000645);
    glVertex3f(0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.163874, -0.163874, 0.000645);
    glVertex3f(0.128755, -0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.128755, -0.192695, 0.000645);
    glVertex3f(0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.088688, -0.214111, 0.000645);
    glVertex3f(0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.045213, -0.227299, 0.000645);
    glVertex3f(-0.000000, -0.231752, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.000000, -0.231752, 0.000645);
    glVertex3f(-0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.045213, -0.227299, 0.000645);
    glVertex3f(-0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.018483, 0.007656, -0.999800);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.088688, -0.214111, 0.000645);
    glVertex3f(-0.127585, -0.187549, 0.001567);
    glEnd();

    glNormal3f(0.015380, -0.015380, -0.999763);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.127585, -0.187549, 0.001567);
    glVertex3f(-0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.163874, -0.163874, 0.000645);
    glVertex3f(-0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.192695, -0.128755, 0.000645);
    glVertex3f(-0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.214111, -0.088688, 0.000645);
    glVertex3f(-0.227299, -0.045212, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.227299, -0.045212, 0.000645);
    glVertex3f(-0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.231752, 0.000000, 0.000645);
    glVertex3f(-0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.227299, 0.045213, 0.000645);
    glVertex3f(-0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.214111, 0.088688, 0.000645);
    glVertex3f(-0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.192695, 0.128755, 0.000645);
    glVertex3f(-0.163873, 0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.163873, 0.163874, 0.000645);
    glVertex3f(-0.128754, 0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.128754, 0.192695, 0.000645);
    glVertex3f(-0.088687, 0.214111, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.088687, 0.214111, 0.000645);
    glVertex3f(-0.045212, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(-0.045212, 0.227299, 0.000645);
    glVertex3f(0.000000, 0.231752, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.000000, 0.231752, 0.000645);
    glVertex3f(0.045213, 0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.045213, 0.227299, 0.000645);
    glVertex3f(0.088688, 0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glVertex3f(0.088688, 0.214111, 0.000645);
    glVertex3f(0.128755, 0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
    glVertex3f(0.128755, 0.192695, 0.000645);
    glVertex3f(0.163874, 0.163874, 0.000645);
    glVertex3f(0.000000, -0.000000, 0.000645);
    glEnd();
}
