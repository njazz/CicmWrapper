/*
 * CicmWrapper
 * Copyright (C) 2013 Pierre Guillot, CICM - Université Paris 8
 * All rights reserved.
 * Website  : https://github.com/CICM/CicmWrapper
 * Contacts : cicm.mshparisnord@gmail.com
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "egraphics.h"

#include "ceammc_convert.h"

static const float k = 0.55228474983079356430692996582365594804286956787109f;
static char ColBuf[10];
static const char HexDigits[] = "0123456789ABCDEF";

const t_rgba rgba_black = { 0.f, 0.f, 0.f, 1.f };
const t_rgba rgba_greydark = { 0.3f, 0.3, 0.3f, 1.f };
const t_rgba rgba_grey = { 0.5f, 0.5, 0.5f, 1.f };
const t_rgba rgba_greylight = { 0.8f, 0.8, 0.8f, 1.f };
const t_rgba rgba_white = { 1.f, 1.f, 1.f, 1.f };
const t_rgba rgba_blue = { 0.f, 0.f, 1.f, 1.f };
const t_rgba rgba_green = { 0.f, 1.f, 0.f, 1.f };
const t_rgba rgba_red = { 1.f, 0.f, 0.f, 1.f };

static const char* SYM_ITALIC = "italic";
static const char* SYM_ROMAN = "roman";
static const char* SYM_BOLD = "bold";
static const char* SYM_NORMAL = "normal";

static void egraphics_apply_matrix(t_elayer* g, t_egobj* gobj);

void egraphics_set_line_width(t_elayer* g, float width)
{
    g->e_line_width = (int)pd_clip_min(width, 0.);
}

void egraphics_set_line_capstyle(t_elayer* g, t_capstyle style)
{
    g->e_line_capstyle = style;
}

void egraphics_set_color_rgba(t_elayer* g, const t_rgba* rgba)
{
    g->e_color = rgba_to_hex_int(*rgba);
}

void egraphics_set_color_rgb(t_elayer* g, const t_rgb* rgb)
{
    g->e_color = rgb_to_hex_int(*rgb);
}

void egraphics_set_color_hsla(t_elayer* g, const t_hsla* hsla)
{
    t_rgba color = hsla_to_rgba(*hsla);
    g->e_color = rgba_to_hex_int(color);
}

void egraphics_set_color_hsl(t_elayer* g, const t_hsl* hsl)
{
    t_rgb color = hsl_to_rgb(*hsl);
    g->e_color = rgb_to_hex_int(color);
}

static void egraphics_paint(t_elayer* g, bool filled, int preserved)
{
    t_egobj *nobj, *temp;
    if (g->e_new_objects.e_type != E_GOBJ_INVALID && g->e_new_objects.e_npoints) {
        temp = (t_egobj*)realloc(g->e_objects, (size_t)(g->e_number_objects + 1) * sizeof(t_egobj));
        if (temp) {
            g->e_objects = temp;
            nobj = g->e_objects + g->e_number_objects;
            g->e_number_objects++;
            nobj->e_filled = filled;
            nobj->e_type = g->e_new_objects.e_type;
            nobj->e_npoints = g->e_new_objects.e_npoints;
            nobj->e_points = (t_pt*)malloc((size_t)nobj->e_npoints * sizeof(t_pt));
            if (!nobj->e_points) {
                nobj->e_npoints = 0;
                nobj->e_type = E_GOBJ_INVALID;
                return;
            }
            memcpy(nobj->e_points, g->e_new_objects.e_points, sizeof(t_pt) * (size_t)nobj->e_npoints);

            nobj->e_color = g->e_color;
            nobj->e_width = g->e_line_width;
            nobj->e_capstyle = g->e_line_capstyle;
            nobj->e_dashstyle = g->e_line_dashstyle;
            nobj->e_justify = g->e_new_objects.e_justify;
            nobj->e_anchor = g->e_new_objects.e_anchor;
            nobj->e_text = g->e_new_objects.e_text;
            nobj->e_image = g->e_new_objects.e_image;

            egraphics_apply_matrix(g, nobj);
            if (!preserved) {
                g->e_new_objects.e_npoints = 0;
                free(g->e_new_objects.e_points);
                g->e_new_objects.e_points = NULL;
                g->e_new_objects.e_type = E_GOBJ_INVALID;
            }
        }
    } else {
        g->e_number_objects = 0;
    }
}

void egraphics_fill_preserve(t_elayer* g)
{
    egraphics_paint(g, true, 1);
}

void egraphics_fill(t_elayer* g)
{
    egraphics_paint(g, true, 0);
}

void egraphics_stroke_preserve(t_elayer* g)
{
    egraphics_paint(g, false, 1);
}

void egraphics_stroke(t_elayer* g)
{
    egraphics_paint(g, false, 0);
}

void etext_layout_draw(t_etext* textlayout, t_elayer* g)
{
    t_egobj* temp;
    long index;
    temp = (t_egobj*)realloc(g->e_objects, (size_t)(g->e_number_objects + 1) * sizeof(t_egobj));
    if (temp) {
        g->e_objects = temp;
        index = g->e_number_objects;
        g->e_number_objects++;

        g->e_objects[index].e_type = E_GOBJ_TEXT;
        g->e_objects[index].e_npoints = 1;
        g->e_objects[index].e_points = (t_pt*)malloc(2 * sizeof(t_pt));
        if (g->e_objects[index].e_points) {
            g->e_objects[index].e_points[0].x = textlayout->c_rect.x;
            g->e_objects[index].e_points[0].y = textlayout->c_rect.y;
            g->e_objects[index].e_points[1].x = textlayout->c_rect.width;
            g->e_objects[index].e_points[1].y = textlayout->c_rect.height;
            g->e_objects[index].e_color = rgba_to_hex_int(textlayout->c_color);

            g->e_objects[index].e_font = textlayout->c_font;
            g->e_objects[index].e_justify = textlayout->c_justify;
            g->e_objects[index].e_text = textlayout->c_text;
            g->e_objects[index].e_anchor = textlayout->c_anchor;
            egraphics_apply_matrix(g, g->e_objects + index);
        } else {
            g->e_objects[index].e_npoints = 0;
            g->e_objects[index].e_type = E_GOBJ_INVALID;
        }
    }
}

static void egraphics_preallocate(t_elayer* g, size_t inc)
{

    if (g->e_new_objects.e_points == NULL) {
        g->e_new_objects.e_points = (t_pt*)malloc(inc * sizeof(t_pt));
        g->e_new_objects.e_npoints = 0;
    } else {
        t_pt* temp = (t_pt*)realloc(g->e_new_objects.e_points, ((size_t)g->e_new_objects.e_npoints + inc) * sizeof(t_pt));
        if (temp) {
            g->e_new_objects.e_points = temp;
        } else {
            free(g->e_new_objects.e_points);
            g->e_new_objects.e_points = NULL;
            g->e_new_objects.e_npoints = 0;
        }
    }
    if (g->e_new_objects.e_points) {
        g->e_new_objects.e_type = E_GOBJ_PATH;
    } else {
        g->e_new_objects.e_type = E_GOBJ_INVALID;
    }
}

void egraphics_move_to(t_elayer* g, const float x, const float y)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 2);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_MOVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = y;
            g->e_new_objects.e_npoints += 2;
        }
    }
}

void egraphics_line_to(t_elayer* g, const float x, const float y)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 2);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_LINE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = y;
            g->e_new_objects.e_npoints += 2;
        }
    }
}

void egraphics_close_path(t_elayer* g)
{
    if (g->e_state == EGRAPHICS_OPEN && g->e_new_objects.e_npoints >= 1) {
        egraphics_preallocate(g, 1);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_CLOSE;
            g->e_new_objects.e_npoints += 1;
        }
    }
}

void egraphics_line(t_elayer* g, const float x0, const float y0, const float x1, const float y1)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 4);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_MOVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = x0;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = y0;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 2].x = E_PATH_LINE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].x = x1;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].y = y1;
            g->e_new_objects.e_npoints += 4;
        }
    }
}

void egraphics_line_fast(t_elayer* g, float x0, float y0, float x1, float y1)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 4);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_MOVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = x0;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = y0;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 2].x = E_PATH_LINE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].x = x1;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].y = y1;
            g->e_new_objects.e_npoints += 4;
        }
    }
    egraphics_stroke(g);
}

void egraphics_curve_to(t_elayer* g, float ctrl1x, float ctrl1y, float ctrl2x, float ctrl2y, float endx, float endy)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 4);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_CURVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = ctrl1x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = ctrl1y;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 2].x = ctrl2x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 2].y = ctrl2y;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].x = endx;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].y = endy;
            g->e_new_objects.e_npoints += 4;
        }
    }
}

void egraphics_curve(t_elayer* g, float startx, float starty,
    float ctrl1x, float ctrl1y, float ctrl2x, float ctrl2y, float endx, float endy)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 6);
        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[g->e_new_objects.e_npoints].x = E_PATH_MOVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = startx;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = starty;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 2].x = E_PATH_CURVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].x = ctrl1x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 3].y = ctrl1y;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 4].x = ctrl2x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 4].y = ctrl2y;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 5].x = endx;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 5].y = endy;
            g->e_new_objects.e_npoints += 6;
        }
    }
}

void egraphics_rectangle(t_elayer* g, float x, float y, float width, float height)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 3);
        g->e_new_objects.e_type = E_GOBJ_SHAPE;

        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[0].x = E_SHAPE_RECT;
            g->e_new_objects.e_points[1].x = x;
            g->e_new_objects.e_points[1].y = y;
            g->e_new_objects.e_points[2].x = x + width;
            g->e_new_objects.e_points[2].y = y + height;
            g->e_new_objects.e_npoints = 3;
        }
    }
}

void egraphics_oval(t_elayer* g, float xc, float yc, float radiusx, float radiusy)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 3);
        g->e_new_objects.e_type = E_GOBJ_SHAPE;

        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[0].x = E_SHAPE_OVAL;
            g->e_new_objects.e_points[1].x = xc - radiusx;
            g->e_new_objects.e_points[1].y = yc - radiusy;
            g->e_new_objects.e_points[2].x = xc + radiusx;
            g->e_new_objects.e_points[2].y = yc + radiusy;
            g->e_new_objects.e_npoints = 3;
        }
    }
}

void egraphics_circle(t_elayer* g, float xc, float yc, float radius)
{
    egraphics_oval(g, xc, yc, radius, radius);
}

void egraphics_arc_oval(t_elayer* g, float xc, float yc, float radiusx, float radiusy, float angle1, float angle2)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 4);
        g->e_new_objects.e_type = E_GOBJ_SHAPE;

        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[0].x = E_SHAPE_ARC;
            g->e_new_objects.e_points[1].x = xc - radiusx;
            g->e_new_objects.e_points[1].y = yc - radiusy;
            g->e_new_objects.e_points[2].x = xc + radiusx;
            g->e_new_objects.e_points[2].y = yc + radiusy;
            g->e_new_objects.e_points[3].x = (angle1 / EPD_PI) * 180.f;
            g->e_new_objects.e_points[3].y = ((angle2 - angle1) / EPD_PI) * 180.f;
            g->e_new_objects.e_npoints = 4;
        }
    }
}

void egraphics_poly(t_elayer* g, const std::vector<t_pt>& points)
{
    if (points.empty())
        return;

    if (g->e_state == EGRAPHICS_OPEN) {
        const size_t N = points.size() * 2 + 1;
        egraphics_preallocate(g, N);
        g->e_new_objects.e_type = E_GOBJ_PATH;

        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[0].x = E_PATH_MOVE;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].x = points[0].x;
            g->e_new_objects.e_points[g->e_new_objects.e_npoints + 1].y = points[0].y;

            for (size_t i = 1; i < points.size(); i++) {
                g->e_new_objects.e_points[g->e_new_objects.e_npoints + i * 2].x = E_PATH_LINE;
                g->e_new_objects.e_points[g->e_new_objects.e_npoints + i * 2 + 1].x = points[i].x;
                g->e_new_objects.e_points[g->e_new_objects.e_npoints + i * 2 + 1].y = points[i].y;
            }

            g->e_new_objects.e_points[g->e_new_objects.e_npoints + points.size() * 2].x = E_PATH_CLOSE;
            g->e_new_objects.e_npoints += N;
        }
    }
}

void egraphics_arc(t_elayer* g, float xc, float yc, float radius, float angle1, float angle2)
{
    egraphics_arc_oval(g, xc, yc, radius, radius, angle1, angle2);
}

static void rotate(const float cosz, const float sinz, t_pt* p1)
{
    const float rx = p1->x * cosz - p1->y * sinz;
    p1->y = (p1->x * sinz + p1->y * cosz);
    p1->x = rx;
}

static void create_small_arc(const float r, const float start, const float extend, t_pt ct, t_pt* p2, t_pt* p3, t_pt* p4)
{
    const float a = extend;
    const float cosz = cosf(a * 0.5f + start);
    const float sinz = sinf(a * 0.5f + start);
    t_pt p1;
    p4->x = (float)(r * cosf(a * 0.5f));
    p4->y = (float)(r * sinf(a * 0.5f));
    p1.x = p4->x;
    p1.y = -p4->y;
    p2->x = (float)(p1.x + k * tan(a * 0.5f) * p4->y);
    p2->y = (float)(p1.y + k * tan(a * 0.5f) * p4->x);
    p3->x = p2->x;
    p3->y = -p2->y;

    rotate(cosz, sinz, p2);
    rotate(cosz, sinz, p3);
    rotate(cosz, sinz, p4);
    p2->x += ct.x;
    p2->y += ct.y;
    p3->x += ct.x;
    p3->y += ct.y;
    p4->x += ct.x;
    p4->y += ct.y;
}

void egraphics_arc_to(t_elayer* g, float cx, float cy, float extend)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        if (g->e_new_objects.e_type == E_GOBJ_PATH && g->e_new_objects.e_points) {
            t_pt p2, p3, p4, c = { cx, cy }, prev = g->e_new_objects.e_points[g->e_new_objects.e_npoints - 1];
            float radius = pd_radius(prev.x - cx, prev.y - cy);
            float angle = pd_angle(prev.x - cx, prev.y - cy);

            while (extend > EPD_2PI) {
                extend -= EPD_2PI;
            }
            while (extend < -EPD_2PI) {
                extend += EPD_2PI;
            }

            while (fabsf(extend) >= EPD_PI4) {
                if (extend < 0.) {
                    create_small_arc(radius, angle, -EPD_PI4, c, &p2, &p3, &p4);
                    extend += EPD_PI4;
                    angle -= EPD_PI4;
                } else {
                    create_small_arc(radius, angle, EPD_PI4, c, &p2, &p3, &p4);
                    extend -= EPD_PI4;
                    angle += EPD_PI4;
                }
                egraphics_curve_to(g, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
            }
            if (fabsf(extend) > 1e-6) {
                create_small_arc(radius, angle, extend, c, &p2, &p3, &p4);
                egraphics_curve_to(g, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
            }
        } else {
            g->e_new_objects.e_type = E_GOBJ_INVALID;
        }
    }
}

static void create_small_arc_oval(const float r1, const float r2,
    const float start, const float extend, t_pt ct, t_pt* p2, t_pt* p3, t_pt* p4)
{
    t_pt p1;

    const float a = extend;
    const float cosz = cosf(a * 0.5f + start);
    const float sinz = sinf(a * 0.5f + start);
    const float cosa = cosf(a * 0.5f);
    const float sina = sinf(a * 0.5f);
    const float k1 = (4.f * (1.f - cosf(a * 0.5f)) / sinf(a * 0.5f)) / 3.f;
    const float k2 = (4.f * (1.f - cosf(-a * 0.5f)) / sinf(-a * 0.5f)) / 3.f;
    p4->x = (float)(r2 * cosa);
    p4->y = (float)(r2 * sina);
    p1.x = (float)(r1 * cosa);
    p1.y = (float)(-r1 * sina);
    p2->x = (float)(p1.x + k1 * p4->y);
    p2->y = (float)(p1.y + k1 * p4->x);
    p3->x = (float)(p4->x + k2 * p1.y);
    p3->y = (float)(p4->y + k2 * p1.y);

    rotate(cosz, sinz, p2);
    rotate(cosz, sinz, p3);
    rotate(cosz, sinz, p4);
    p2->x += ct.x;
    p2->y += ct.y;
    p3->x += ct.x;
    p3->y += ct.y;
    p4->x += ct.x;
    p4->y += ct.y;
}

void egraphics_arc_oval_to(t_elayer* g, float cx, float cy, float radius, float extend)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        if (g->e_new_objects.e_type == E_GOBJ_PATH && g->e_new_objects.e_points) {
            t_pt p2, p3, p4, c = { cx, cy }, prev = g->e_new_objects.e_points[g->e_new_objects.e_npoints - 1];
            float r1 = pd_radius(prev.x - cx, prev.y - cy);
            float angle = pd_angle(prev.x - cx, prev.y - cy);
            float ratio = (radius - r1) / (fabsf(extend) / EPD_PI4);

            while (extend > EPD_2PI) {
                extend -= EPD_2PI;
            }
            while (extend < -EPD_2PI) {
                extend += EPD_2PI;
            }

            while (fabsf(extend) >= EPD_PI4) {
                if (extend < 0.) {

                    create_small_arc_oval(r1, r1 + ratio, angle, -EPD_PI4, c, &p2, &p3, &p4);
                    extend += EPD_PI4;
                    angle -= EPD_PI4;
                    r1 += ratio;
                } else {
                    create_small_arc_oval(r1, r1 + ratio, angle, EPD_PI4, c, &p2, &p3, &p4);
                    extend -= EPD_PI4;
                    angle += EPD_PI4;
                    r1 += ratio;
                }
                egraphics_curve_to(g, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
            }
            if (fabsf(extend) > 1e-6) {
                create_small_arc_oval(r1, radius, angle, extend, c, &p2, &p3, &p4);
                egraphics_curve_to(g, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
            }
        } else {
            g->e_new_objects.e_type = E_GOBJ_INVALID;
        }
    }
}

int rgba_to_hex_int(const t_rgba& color)
{
    const auto r = static_cast<unsigned int>(color.red * 255.f) & 0xFF;
    const auto g = static_cast<unsigned int>(color.green * 255.f) & 0xFF;
    const auto b = static_cast<unsigned int>(color.blue * 255.f) & 0xFF;

    return (r << 16) | (g << 8) | b;
}

int rgb_to_hex_int(const t_rgb& color)
{
    const auto r = static_cast<unsigned int>(color.red * 255.f) & 0xFF;
    const auto g = static_cast<unsigned int>(color.green * 255.f) & 0xFF;
    const auto b = static_cast<unsigned int>(color.blue * 255.f) & 0xFF;

    return (r << 16) | (g << 8) | b;
}

char* rgba_to_hex(t_rgba color)
{
    int r = (int)(color.red * 255.f);
    int g = (int)(color.green * 255.f);
    int b = (int)(color.blue * 255.f);
    ColBuf[0] = '#';
    ColBuf[1] = HexDigits[(r >> 4) & 15];
    ColBuf[2] = HexDigits[r & 15];
    ColBuf[3] = HexDigits[(g >> 4) & 15];
    ColBuf[4] = HexDigits[g & 15];
    ColBuf[5] = HexDigits[(b >> 4) & 15];
    ColBuf[6] = HexDigits[b & 15];
    ColBuf[7] = '\0';
    return &ColBuf[0];
}

t_rgba hex_to_rgba(const char* color)
{
    int hexvalue = (int)strtol(color + 1, NULL, 16);
    t_rgba ncolor;
    ncolor.red = (float)((hexvalue >> 16) & 0xFF) / 255.f;
    ncolor.green = (float)((hexvalue >> 8) & 0xFF) / 255.f;
    ncolor.blue = (float)((hexvalue)&0xFF) / 255.f;
    ncolor.alpha = 1.f;
    return ncolor;
}

t_rgb hex_to_rgb(const char* color)
{
    int hexvalue = (int)strtol(color + 1, NULL, 16);
    t_rgb ncolor;
    ncolor.red = (float)((hexvalue >> 16) & 0xFF) / 255.f;
    ncolor.green = (float)((hexvalue >> 8) & 0xFF) / 255.f;
    ncolor.blue = (float)((hexvalue)&0xFF) / 255.f;
    return ncolor;
}

t_hsla rgba_to_hsla(t_rgba color)
{
    t_hsla ncolor = { 0., 0., 0., 0. };
    float delta, deltar, deltag, deltab;
    float max = color.red;
    float min = color.red;
    if (min > color.green)
        min = color.green;
    if (min > color.blue)
        min = color.blue;
    if (max < color.green)
        max = color.green;
    if (max < color.blue)
        max = color.blue;
    delta = max - min;
    ncolor.alpha = color.alpha;
    ncolor.lightness = (max + min) / 2.f;
    if (delta == 0) {
        ncolor.hue = 0;
        ncolor.saturation = 1;
        ncolor.lightness = 1;
        return ncolor;
    }

    if (max == 0) {
        ncolor.hue = 0;
        ncolor.saturation = 0;
    } else {
        if (ncolor.lightness < 0.5f)
            ncolor.saturation = delta / (max + min);
        else
            ncolor.saturation = delta / (2.f - max - min);

        deltar = (((max - color.red) / 6) + (delta / 2)) / delta;
        deltag = (((max - color.green) / 6) + (delta / 2)) / delta;
        deltab = (((max - color.blue) / 6) + (delta / 2)) / delta;

        if (color.red == max)
            ncolor.hue = deltab - deltag;
        else if (color.green == max)
            ncolor.hue = (1.f / 3.f) + deltar - deltab;
        else if (color.blue == max)
            ncolor.hue = (2.f / 3.f) + deltag - deltar;

        if (ncolor.hue < 0.f)
            ncolor.hue += 1;
        if (ncolor.hue > 1.f)
            ncolor.hue -= 1;
    }

    return ncolor;
}

t_hsl rgb_to_hsl(t_rgb color)
{
    t_hsl ncolor = { 0., 0., 0. };
    float delta, deltar, deltag, deltab;
    float max = color.red;
    float min = color.red;
    if (min > color.green)
        min = color.green;
    if (min > color.blue)
        min = color.blue;
    if (max < color.green)
        max = color.green;
    if (max < color.blue)
        max = color.blue;
    delta = max - min;
    ncolor.lightness = (max + min) / 2.f;
    if (max == 0) {
        ncolor.hue = 0;
        ncolor.saturation = 0;
    } else {
        if (ncolor.lightness < 0.5f)
            ncolor.saturation = delta / (max + min);
        else
            ncolor.saturation = delta / (2.f - max - min);

        deltar = (((max - color.red) / 6) + (delta / 2)) / delta;
        deltag = (((max - color.green) / 6) + (delta / 2)) / delta;
        deltab = (((max - color.blue) / 6) + (delta / 2)) / delta;

        if (color.red == max)
            ncolor.hue = deltab - deltag;
        else if (color.green == max)
            ncolor.hue = (1.f / 3.f) + deltar - deltab;
        else if (color.blue == max)
            ncolor.hue = (2.f / 3.f) + deltag - deltar;

        if (ncolor.hue < 0.f)
            ncolor.hue += 1;
        if (ncolor.hue > 1.f)
            ncolor.hue -= 1;
    }

    return ncolor;
}

static float Hue_2_RGB(float P, float Q, float Tc)
{
    if (Tc < 0.f)
        Tc += 1;
    if (Tc > 1.f)
        Tc -= 1;

    if (Tc < 0.1666666f) // 1/6
        return P + ((Q - P) * 6.f * Tc);
    else if (0.1666666f <= Tc && Tc < 0.5f)
        return Q;
    else if (0.5f <= Tc && Tc < 0.666666f) // 2/3
        return (P + (Q - P) * ((2.f / 3.f) - Tc) * 6.f);
    else
        return P;
}

t_rgb hsl_to_rgb(t_hsl color)
{
    float var1, var2;
    t_rgb ncolor;
    if (color.saturation == 0.f) {
        ncolor.red = color.lightness;
        ncolor.green = color.lightness;
        ncolor.blue = color.lightness;
    } else {
        if (color.lightness < 0.5f)
            var2 = color.lightness * (1.f + color.saturation);
        else
            var2 = (color.lightness + color.saturation) - (color.saturation * color.lightness);

        var1 = 2 * color.lightness - var2;

        ncolor.red = Hue_2_RGB(var1, var2, color.hue + (1.f / 3.f));
        ncolor.green = Hue_2_RGB(var1, var2, color.hue);
        ncolor.blue = Hue_2_RGB(var1, var2, color.hue - (1.f / 3.f));
    }
    return ncolor;
}

t_rgba hsla_to_rgba(t_hsla color)
{
    t_rgba res;
    if (color.saturation == 0.f) {
        res.red = color.lightness;
        res.green = color.lightness;
        res.blue = color.lightness;
    } else {
        float Q;

        if (color.lightness < 0.5f)
            Q = color.lightness * (1.f + color.saturation);
        else
            Q = (color.lightness + color.saturation) - (color.lightness * color.saturation);

        float P = 2.f * color.lightness - Q;

        res.red = Hue_2_RGB(P, Q, color.hue + 0.333333f);
        res.green = Hue_2_RGB(P, Q, color.hue);
        res.blue = Hue_2_RGB(P, Q, color.hue - 0.333333f);
    }

    res.alpha = color.alpha;
    return res;
}

t_rgba rgba_addContrast(t_rgba color, float contrast)
{
    t_rgba new_color = color;
    new_color.red = (float)pd_clip_minmax(new_color.red += contrast, 0., 1.);
    new_color.green = (float)pd_clip_minmax(new_color.green += contrast, 0., 1.);
    new_color.blue = (float)pd_clip_minmax(new_color.blue += contrast, 0., 1.);
    return new_color;
}

t_rgb rgb_addContrast(t_rgb color, float contrast)
{
    t_rgb new_color = color;
    new_color.red = (float)pd_clip_minmax(new_color.red += contrast, 0., 1.);
    new_color.green = (float)pd_clip_minmax(new_color.green += contrast, 0., 1.);
    new_color.blue = (float)pd_clip_minmax(new_color.blue += contrast, 0., 1.);
    return new_color;
}

void rgba_set(t_rgba* color, float red, float green, float blue, float alpha)
{
    color->red = red;
    color->green = green;
    color->blue = blue;
    color->alpha = alpha;
}

void rgb_set(t_rgb* color, float red, float green, float blue)
{
    color->red = red;
    color->green = green;
    color->blue = blue;
}

void hsla_set(t_hsla* color, float hue, float saturation, float lightness, float alpha)
{
    color->hue = hue;
    color->saturation = saturation;
    color->lightness = lightness;
    color->alpha = alpha;
}

void hsl_set(t_hsl* color, float hue, float saturation, float lightness)
{
    color->hue = hue;
    color->saturation = saturation;
    color->lightness = lightness;
}

void egraphics_matrix_init(t_matrix* x, float xx, float yx, float xy, float yy, float x0, float y0)
{
    x->xx = xx;
    x->yx = yx;
    x->xy = xy;
    x->yy = yy;
    x->x0 = x0;
    x->y0 = y0;
}

void egraphics_set_matrix(t_elayer* g, const t_matrix* matrix)
{
    egraphics_matrix_init(&g->e_matrix, matrix->xx, matrix->yx, matrix->xy, matrix->yy, matrix->x0, matrix->y0);
}

void egraphics_rotate(t_elayer* g, float angle)
{
    const float cosRad = cosf(angle);
    const float sinRad = sinf(angle);
    t_matrix temp = g->e_matrix;

    g->e_matrix.xx = temp.xx * cosRad - temp.yx * sinRad;
    g->e_matrix.yx = temp.xx * sinRad + temp.yx * cosRad;
    g->e_matrix.xy = temp.xy * cosRad - temp.yy * sinRad;
    g->e_matrix.yy = temp.xy * sinRad + temp.yy * cosRad;
}

static void egraphics_apply_matrix(t_elayer* g, t_egobj* gobj)
{
    int i;
    float x_p, y_p;
    if (gobj->e_type == E_GOBJ_PATH) {
        for (i = 0; i < gobj->e_npoints;) {
            if (gobj->e_points[i].x == E_PATH_MOVE || gobj->e_points[i].x == E_PATH_LINE) {
                x_p = gobj->e_points[i + 1].x * g->e_matrix.xx + gobj->e_points[i + 1].y * g->e_matrix.xy + g->e_matrix.x0;
                y_p = gobj->e_points[i + 1].x * g->e_matrix.yx + gobj->e_points[i + 1].y * g->e_matrix.yy + g->e_matrix.y0;
                gobj->e_points[i + 1].x = x_p;
                gobj->e_points[i + 1].y = y_p;
                i += 2;
            } else if (gobj->e_points[i].x == E_PATH_CURVE) {
                x_p = gobj->e_points[i + 1].x * g->e_matrix.xx + gobj->e_points[i + 1].y * g->e_matrix.xy + g->e_matrix.x0;
                y_p = gobj->e_points[i + 1].x * g->e_matrix.yx + gobj->e_points[i + 1].y * g->e_matrix.yy + g->e_matrix.y0;
                gobj->e_points[i + 1].x = x_p;
                gobj->e_points[i + 1].y = y_p;
                x_p = gobj->e_points[i + 2].x * g->e_matrix.xx + gobj->e_points[i + 2].y * g->e_matrix.xy + g->e_matrix.x0;
                y_p = gobj->e_points[i + 2].x * g->e_matrix.yx + gobj->e_points[i + 2].y * g->e_matrix.yy + g->e_matrix.y0;
                gobj->e_points[i + 2].x = x_p;
                gobj->e_points[i + 2].y = y_p;
                x_p = gobj->e_points[i + 3].x * g->e_matrix.xx + gobj->e_points[i + 3].y * g->e_matrix.xy + g->e_matrix.x0;
                y_p = gobj->e_points[i + 3].x * g->e_matrix.yx + gobj->e_points[i + 3].y * g->e_matrix.yy + g->e_matrix.y0;
                gobj->e_points[i + 3].x = x_p;
                gobj->e_points[i + 3].y = y_p;
                i += 4;
            } else {
                i++;
            }
        }
    } else {
        for (i = 0; i < gobj->e_npoints; i++) {
            x_p = gobj->e_points[i].x * g->e_matrix.xx + gobj->e_points[i].y * g->e_matrix.xy + g->e_matrix.x0;
            y_p = gobj->e_points[i].x * g->e_matrix.yx + gobj->e_points[i].y * g->e_matrix.yy + g->e_matrix.y0;
            gobj->e_points[i].x = x_p;
            gobj->e_points[i].y = y_p;
        }
    }
}

t_etext* etext_layout_create(void)
{
    t_etext* new_text_layout = (t_etext*)malloc(sizeof(t_etext));
    new_text_layout->c_color.red = 0.;
    new_text_layout->c_color.green = 0.;
    new_text_layout->c_color.blue = 0.;
    new_text_layout->c_color.alpha = 1.;
    new_text_layout->c_buf[0] = 0;
    new_text_layout->c_text = &new_text_layout->c_buf[0];
    new_text_layout->c_is_buffer_used = 1;
    new_text_layout->c_justify = ETEXT_JLEFT;
    new_text_layout->c_anchor = ETEXT_LEFT;

    return new_text_layout;
}

void etext_layout_destroy(t_etext* textlayout)
{
    if (!textlayout->c_is_buffer_used)
        free((void*)textlayout->c_text);

    free(textlayout);
}

void etext_layout_set(t_etext* textlayout, const char* text, t_efont* font,
    float x, float y, float width, float height,
    etextanchor_flags anchor, etextjustify_flags justify, etextwrap_flags wrap)
{
    const size_t N = sizeof(textlayout->c_buf);

    // free old memory if malloc memory used
    if (!textlayout->c_is_buffer_used) {
        free((void*)textlayout->c_text);
        textlayout->c_text = 0;
    }

    size_t txt_len = strlen(text);

    // if short text use struct buffer
    if (txt_len < N) {
        strncpy(textlayout->c_buf, text, N);
        textlayout->c_text = textlayout->c_buf;
        textlayout->c_is_buffer_used = 1;
    } else {
        // or use malloc instead
        textlayout->c_is_buffer_used = 0;
        textlayout->c_text = strdup(text);
    }

    textlayout->c_font = font[0];
    textlayout->c_rect.x = (float)x;
    textlayout->c_rect.y = (float)y;
    textlayout->c_rect.width = (float)width;
    textlayout->c_rect.height = (float)height;

    if (wrap == ETEXT_NOWRAP) {
        textlayout->c_rect.width = 0.;
    }

    textlayout->c_anchor = anchor;
    textlayout->c_justify = justify;
}

void etext_layout_settextcolor(t_etext* textlayout, t_rgba* color)
{
    textlayout->c_color = color[0];
}

t_efont* efont_create(t_symbol* family, t_symbol* slant, t_symbol* weight, float size)
{
    t_efont* new_font = (t_efont*)malloc(sizeof(t_efont));
    if (new_font) {
        new_font[0].c_family = family;
        new_font[0].c_slant = slant;
        if (new_font[0].c_slant != gensym(SYM_ITALIC))
            new_font[0].c_slant = gensym(SYM_ROMAN);

        new_font[0].c_weight = weight;
        if (new_font[0].c_weight != gensym(SYM_BOLD))
            new_font[0].c_weight = gensym(SYM_NORMAL);

        new_font[0].c_size = pd_clip_min(size, 1.f);
    }
    return new_font;
}

void efont_destroy(t_efont* font)
{
    free(font);
}

float pd_clip_minmax(float aValue, float aMinimum, float aMaximum)
{
    if (aValue < aMinimum)
        return aMinimum;
    else if (aValue > aMaximum)
        return aMaximum;
    else
        return aValue;
}

float pd_clip_min(float aValue, float aMinimum)
{
    if (aValue < aMinimum)
        return aMinimum;
    else
        return aValue;
}

float pd_clip_max(float aValue, float aMaximum)
{
    if (aValue > aMaximum)
        return aMaximum;
    else
        return aValue;
}

float pd_ordinate(float radius, float angle)
{
    return radius * sinf(angle);
}

float pd_abscissa(float radius, float angle)
{
    return radius * cosf(angle);
}

float pd_radius(float x, float y)
{
    return sqrtf(x * x + y * y);
}

float pd_angle(float x, float y)
{
    return atan2f(y, x);
}

void egraphics_set_line_dashstyle(t_elayer* g, t_dashstyle style)
{
    g->e_line_dashstyle = style;
}

t_rgba rgba_color_sum(const t_rgba* c1, const t_rgba* c2, float alpha)
{
    t_rgba res;
    res.red = ceammc::clip<float>(c1->red * alpha + c2->red * (1 - alpha), 0, 1);
    res.green = ceammc::clip<float>(c1->green * alpha + c2->green * (1 - alpha), 0, 1);
    res.blue = ceammc::clip<float>(c1->blue * alpha + c2->blue * (1 - alpha), 0, 1);
    res.alpha = 1;
    return res;
}

void egraphics_image(t_elayer* g, float xc, float yc, t_eimage* image)
{
    if (g->e_state == EGRAPHICS_OPEN) {
        egraphics_preallocate(g, 2);
        g->e_new_objects.e_type = E_GOBJ_SHAPE;
        g->e_new_objects.e_image = image;

        if (g->e_new_objects.e_points) {
            g->e_new_objects.e_points[0].x = E_SHAPE_IMAGE;
            g->e_new_objects.e_points[1].x = xc;
            g->e_new_objects.e_points[1].y = yc;
            g->e_new_objects.e_npoints = 2;
        }
    }
}

t_eimage* eimage_create(const char* data, int width, int height, etextanchor_flags anchor)
{
    static int image_counter = 1;

    t_eimage* img = new t_eimage;
    img->data_base64 = data;
    img->width = width;
    img->height = height;
    img->anchor = anchor;

    char buf[32];
    snprintf(buf, 32, "ceammc_img%04i", image_counter++);
    img->name = gensym(buf);
    return img;
}

void eimage_destroy(t_eimage* img)
{
    if (!img)
        return;

    delete img;
}

void eimage_copy(t_eimage* dest, const t_eimage* src)
{
    if (dest == src)
        return;

    dest->data_base64 = src->data_base64;
    dest->width = src->width;
    dest->height = src->height;
    dest->anchor = src->anchor;
}

void eimage_set_base64_data(t_eimage* img, const char* base64)
{
    img->data_base64 = base64;
}

void egraphics_set_color_hex(t_elayer* g, uint32_t c)
{
    g->e_color = c;
}

void egraphics_raise(t_elayer* over, t_elayer* l)
{
    sys_vgui("raise %s %s\n", over->e_id->s_name, l->e_id->s_name);
}
