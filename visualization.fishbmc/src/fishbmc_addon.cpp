/*   fishBMC visualization plugin
 *   Copyright (C) 2012 Marcel Ebmer

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.

 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.

 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifdef __GNUC__
#define __cdecl
#endif

#include "addons/include/xbmc_vis_types.h"
#include "addons/include/xbmc_vis_dll.h"

#include "fische.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// global variables
FISCHE*     g_fische;
double      g_aspect;
GLuint      g_texture;
bool        g_isrotating;
double      g_angle;
double      g_lastangle;
bool        g_errorstate;
int         g_framedivisor;
double      g_angleincrement;
double      g_texright;
double      g_texleft;
bool        g_filemode;
int         g_size;
uint8_t*    g_axis = 0;

// OpenGL: paint a textured quad
void textured_quad_GL (double center_x,
                       double center_y,
                       double angle,
                       double axis,
                       double width,
                       double height,
                       double tex_left,
                       double tex_right,
                       double tex_top,
                       double tex_bottom)
{
    glPushMatrix();

    glTranslatef (center_x, center_y, 0);
    glRotatef (angle, axis, 1 - axis, 0);

    double scale = 1 - sin (angle / 360 * M_PI) / 3;
    glScalef (scale, scale, scale);

    glBegin (GL_QUADS);
    glTexCoord2d (tex_left, tex_top);
    glVertex3d (- width / 2, - height / 2, 0);
    glTexCoord2d (tex_right, tex_top);
    glVertex3d (width / 2, - height / 2, 0);
    glTexCoord2d (tex_right, tex_bottom);
    glVertex3d (width / 2, height / 2, 0);
    glTexCoord2d (tex_left, tex_bottom);
    glVertex3d (- width / 2, height / 2, 0);
    glEnd();

    glPopMatrix();
}

void render_GL()
{
    // Save State
    glPushAttrib (GL_ENABLE_BIT | GL_TEXTURE_BIT);

    // OpenGL settings
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);
    glPolygonMode (GL_FRONT, GL_FILL);

    // OpenGL matrix setup
    glMatrixMode (GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glFrustum (-1, 1, 1, -1, 3, 15);

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // actual painting
    glBindTexture (GL_TEXTURE_2D, g_texture);

    if (g_isrotating) {
        if (g_angle - g_lastangle > 180) {
            g_lastangle = g_lastangle ? 0 : 180;
            g_angle = g_lastangle;
            g_isrotating = false;
        }
    }

    int n = 8;
    int m = (g_aspect * 8 + 0.5);

    if (!g_axis) {
        g_axis = new uint8_t[m * n];
        for (int i = 0; i < m * n; ++ i) {
            g_axis[i] = rand() % 2;
        }
    }

    glTranslatef (0, 0, -6.0f);
    glRotatef (g_angle, 0, 1, 0);

    for (int i = 0; i < m; i += 1) {
        for (int j = 0; j < n; j += 1) {
            double di = i;
            double dj = j;
            double dx = -2 + (di + 0.5) * 4 / m;
            double dy = -2 + (dj + 0.5) * 4 / n;
            double w = 4.0 / m;
            double h = 4.0 / n;
            double tw = (g_texright - g_texleft);
            double tl = g_texleft + tw * di / m;
            double tr = g_texleft + tw * (di + 1) / m;
            double tt = dj / n;
            double tb = (dj + 1) / n;
            double angle = (g_angle - g_lastangle) * 4 - (di + dj * m) / (m * n) * 360;
            if (angle < 0) angle = 0;
            if (angle > 360) angle = 360;
            textured_quad_GL (dx, dy, angle, g_axis[i + j * m], w, h, tl, tr, tt, tb);
        }
    }

    // OpenGL matrix to original state
    glPopMatrix();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix();

    // Restore original state
    glPopAttrib();
}

void on_beat (double frames_per_beat)
{
    if (!g_isrotating) {
        g_isrotating = true;
        if (frames_per_beat < 1) frames_per_beat = 12;
        g_angleincrement = 180 / 4 / frames_per_beat;
    }
}

void write_vectors (const void* data, size_t bytes)
{
    char const * homedir = getenv ("HOME");
    if (!homedir)
        return;

    std::string dirname = std::string (homedir) + "/.fishBMC-data";
    mkdir (dirname.c_str(), 0755);

    std::ostringstream filename;
    filename << dirname << "/" << g_fische->height;

    // open the file
    std::fstream vectorsfile (filename.str().c_str(), std::fstream::out | std::fstream::binary);
    if (!vectorsfile.good())
        return;

    // write it
    vectorsfile.write (reinterpret_cast<const char*> (data), bytes);
    vectorsfile.close();
}

size_t read_vectors (void** data)
{
    char const * homedir = getenv ("HOME");
    if (!homedir)
        return 0;

    std::string dirname = std::string (homedir) + "/.fishBMC-data";
    mkdir (dirname.c_str(), 0755);

    std::ostringstream filename;
    filename << dirname << "/" << g_fische->height;

    // open the file
    std::fstream vectorsfile (filename.str().c_str(), std::fstream::in);
    if (!vectorsfile.good())
        return 0;

    vectorsfile.seekg (0, std::ios::end);
    size_t n = vectorsfile.tellg();
    vectorsfile.seekg (0, std::ios::beg);

    *data = malloc (n);
    vectorsfile.read (reinterpret_cast<char*> (*data), n);
    vectorsfile.close();

    return n;
}

void delete_vectors()
{
    char const * homedir = getenv ("HOME");
    if (!homedir)
        return;

    std::string dirname = std::string (homedir) + "/.fishBMC-data";
    mkdir (dirname.c_str(), 0755);

    for (int i = 64; i <= 2048; i *= 2) {
        std::ostringstream filename;
        filename << dirname << "/" << i;
        unlink (filename.str().c_str());
    }
}

extern "C" ADDON_STATUS ADDON_Create (void* hdl, void* props)
{
    if (!props)
        return ADDON_STATUS_UNKNOWN;

    VIS_PROPS* visProps = (VIS_PROPS*) props;

    g_fische = fische_new();
    g_fische->on_beat = &on_beat;
    g_fische->pixel_format = FISCHE_PIXELFORMAT_0xAABBGGRR;
    g_fische->line_style = FISCHE_LINESTYLE_THICK;
    g_aspect = double (visProps->width) / double (visProps->height);
    g_texleft = (2 - g_aspect) / 4;
    g_texright = 1 - g_texleft;
    g_framedivisor = 1;
    g_filemode = false;
    g_size = 128;

    return ADDON_STATUS_NEED_SETTINGS;
}

extern "C" void Start (int, int, int, const char*)
{
    g_errorstate = false;

    g_fische->audio_format = FISCHE_AUDIOFORMAT_S16;

    g_fische->height = g_size;
    g_fische->width = 2 * g_size;

    if (g_filemode) {
        g_fische->read_vectors = &read_vectors;
        g_fische->write_vectors = &write_vectors;
    }

    else {
        delete_vectors();
    }

    if (fische_start (g_fische) != 0) {
        std::cerr << "fische failed to start" << std::endl;
        g_errorstate = true;
        return;
    }

    uint32_t* pixels = fische_render (g_fische);

    // generate a texture for drawing into
    glEnable (GL_TEXTURE_2D);
    glGenTextures (1, &g_texture);
    glBindTexture (GL_TEXTURE_2D, g_texture);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, g_fische->width, g_fische->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    g_isrotating = false;
    g_angle = 0;
    g_lastangle = 0;
    g_angleincrement = 0;
}

extern "C" void AudioData (const short* pAudioData, int iAudioDataLength, float*, int)
{
    fische_audiodata (g_fische, pAudioData, iAudioDataLength * 4);
}

extern "C" void Render()
{
    static int frame = 0;

    // check if this frame is to be skipped
    if (++ frame % g_framedivisor == 0) {
        uint32_t* pixels = fische_render (g_fische);
        glBindTexture (GL_TEXTURE_2D, g_texture);
        glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, g_fische->width, g_fische->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        if (g_isrotating)
            g_angle += g_angleincrement;
    }

    // actual rendering
    render_GL();
}

extern "C" void GetInfo (VIS_INFO* pInfo)
{
    // std::cerr << "fishBMC::GetInfo" << std::endl;
    pInfo->bWantsFreq = false;
    pInfo->iSyncDelay = 0;
}

extern "C" bool OnAction (long flags, const void *param)
{
    return false;
}

extern "C" unsigned int GetPresets (char ***presets)
{
    return 0;
}

extern "C" unsigned GetPreset()
{
    return 0;
}

extern "C" bool IsLocked()
{
    return false;
}

extern "C" unsigned int GetSubModules (char ***names)
{
    return 0;
}

extern "C" void ADDON_Stop()
{
    fische_free (g_fische);
    g_fische = 0;
    glDeleteTextures (1, &g_texture);
    delete [] g_axis;
    g_axis = 0;
}

extern "C" void ADDON_Destroy()
{
    return;
}

extern "C" bool ADDON_HasSettings()
{
    return false;
}

extern "C" ADDON_STATUS ADDON_GetStatus()
{
    if (g_errorstate)
        return ADDON_STATUS_UNKNOWN;

    return ADDON_STATUS_OK;
}

extern "C" unsigned int ADDON_GetSettings (ADDON_StructSetting ***sSet)
{
    return 0;
}

extern "C" void ADDON_FreeSettings()
{
}

extern "C" ADDON_STATUS ADDON_SetSetting (const char *strSetting, const void* value)
{
    if (!strSetting || !value)
        return ADDON_STATUS_UNKNOWN;

    if (!strncmp (strSetting, "nervous", 7)) {
        bool nervous = * ( (bool*) value);
        g_fische->nervous_mode = nervous ? 1 : 0;
    }

    else if (!strncmp (strSetting, "filemode", 7)) {
        bool filemode = * ( (bool*) value);
        g_filemode = filemode;
    }

    else if (!strncmp (strSetting, "detail", 6)) {
        int detail = * ( (int*) value);
        g_size = 128;
        while (detail--) {
            g_size *= 2;
        }
    }

    else if (!strncmp (strSetting, "divisor", 7)) {
        int divisor = * ( (int*) value);
        g_framedivisor = 8;
        while (divisor--) {
            g_framedivisor /= 2;
        }
    }

    return ADDON_STATUS_OK;
}
