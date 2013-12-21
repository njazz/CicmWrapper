/*
 * PdEnhanced - Pure Data Enhanced 
 *
 * An add-on for Pure Data
 *
 * Copyright (C) 2013 Pierre Guillot, CICM - Université Paris 8
 * All rights reserved.
 *
 * Website  : http://www.mshparisnord.fr/HoaLibrary/
 * Contacts : cicm.mshparisnord@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef DEF_ESTRUCT
#define DEF_ESTRUCT

#include "ecommon/ecommon.h"


typedef struct _pt
{
	float x;
	float y;
} t_pt; ///< A 2D Point.

typedef struct _rect
{
	float x;
	float y;
	float width;
	float height;
} t_rect; ///< A Rectangle.

typedef struct _rgb
{
	float red;
	float green;
	float blue;
} t_rgb; ///< A RGB Color.

typedef struct _rgba
{
	float red;
	float green;
	float blue;
	float alpha;
} t_rgba; ///< A RGBA Color.

typedef struct _hsl
{
	float hue;
	float saturation;
	float lightness;
} t_hsl; ///< A HSL Color.

typedef struct _hsla
{
	float hue;
	float saturation;
	float lightness;
    float alpha;
} t_hsla; ///< A HSLA Color.

typedef struct _matrix
{
	float xx;
	float yx;
	float xy;
	float yy;
	float x0;
	float y0;
} t_matrix; ///< A Graphic Matrix.

typedef struct _ewidget
{
    t_getrectfn     w_getrectfn;
    t_displacefn    w_displacefn;
    t_selectfn      w_selectfn;
    t_activatefn    w_activatefn;
    t_deletefn      w_deletefn;
    t_visfn         w_visfn;
    t_clickfn       w_clickfn;
    method          w_assist;
    method          w_paint;
    method          w_mouseenter;
    method          w_mouseleave;
    method          w_mousemove;
    method          w_mousedown;
    method          w_mousedrag;
    method          w_mouseup;
    method          w_mousewheel;
    method          w_dblclick;
    method          w_key;
    method          w_keyfilter;
    method          w_deserted;
    method          w_getdrawparameters;
    method          w_save;
    method          w_popup;
    method          w_dsp;
    method          w_object_standat_paint;
    method          w_oksize;
    t_err_method    w_notify;
    method          w_write;
    method          w_read;
} t_ewidget;

typedef struct _eattr
{
    t_object*       obj;
	t_symbol*       name;
	t_symbol*       type;
    t_symbol*       category;
    t_symbol*       label;
    t_symbol*       style;
    long            order;
    char            save;
    char            paint;
    char            invisible;
	long			flags;
    long            offset;
    long            sizemax;
    long            size;
    
	method			getter;
	method			setter;
    long            clipped;
	double          minimum;
    double          maximum;
    t_symbol*       defvals;
    
} t_eattr;

typedef struct _eclass
{
    t_class     c_class;
    char        c_box;
    t_ewidget   c_widget;
    t_eattr*    c_attr;
    long        c_nattr;
}t_eclass;

typedef enum _emod_flags
{
    EMOD_NONE           = 0,
	EMOD_SHIFT          = 1,
    EMOD_MAJ            = 2,
    EMOD_CTRL           = 4,
    EMOD_CMD            = 8,
    EMOD_ALT            = 16,
} t_emod_flags;

typedef enum _ekey_flags
{
    EKEY_DEL      = 0,
	EKEY_TAB      = 1,
    EKEY_ENTER    = 2,
    EKEY_ESC      = 3,
    
} t_ekey_flags;

typedef enum _eselitem_flags
{
    EITEM_NONE    = 0,
    EITEM_OBJ     = 1,
	EITEM_INLET   = 2,
    EITEM_OUTLET  = 3,
    EITEM_BOTTOM  = 4,
    EITEM_CORNER  = 5,
    EITEM_RIGHT   = 6,
    
} t_eselitem_flags;

typedef enum _elayer_flags
{
	EGRAPHICS_OPEN      = 0,
	EGRAPHICS_CLOSE     = -1,
	EGRAPHICS_INVALID   = -2,
    EGRAPHICS_TODRAW    = -3,
} t_elayer_flags;

typedef enum _etextanchor_flags
{
    ETEXT_UP            = 0,
    ETEXT_UP_LEFT       = 1,
    ETEXT_UP_RIGHT      = 2,
    ETEXT_DOWN          = 3,
    ETEXT_DOWN_LEFT     = 4,
    ETEXT_DOWN_RIGHT    = 5,
	ETEXT_LEFT          = 6, // first element of text = x
    ETEXT_RIGHT         = 7, // last element of text = x
	ETEXT_CENTER        = 8  // center element of text = x
    
} t_etextanchor_flags;

typedef enum _etextwrap_flags
{
	ETEXT_NOWRAP    = 0,
	ETEXT_WRAP      = 1
} t_etextwrap_flags;

typedef enum _etextjustify_flags
{
	ETEXT_JLEFT    = 0,
	ETEXT_JRIGHT   = 1,
    ETEXT_JCENTER  = 2
} t_etextjustify_flags;

typedef enum
{
    E_GOBJ_INVALID           = 0,
    E_GOBJ_PATH                 ,
    E_GOBJ_RECT                 ,
    E_GOBJ_ARC                  ,
    E_GOBJ_OVAL                 ,
    E_GOBJ_TEXT                 ,
    E_GOBJ_IMAG                 ,
} egraphics_types;

typedef struct _efont
{
    t_symbol*   c_family;   // times, helvetica, ect...
    t_symbol*   c_slant;    // regular, italic
    t_symbol*   c_weight;   // normal, bold
    float       c_size;
} t_efont;

typedef struct _epopup
{
    t_symbol*   c_name;
    t_symbol*   c_send;
}t_epopup;

typedef struct _etextfield
{
    t_symbol*   c_name;
    t_symbol*   c_send;
}t_etextfield;

typedef struct _etext
{
    t_symbol*       c_text;
    t_rgba          c_color;
    t_efont         c_font;
    t_rect          c_rect;
    t_symbol*       c_anchor;
    t_symbol*       c_justify;
} t_etext;

typedef struct _egobj
{
	int             e_type;
    int             e_filled;
    t_symbol*       e_color;
    float           e_width;
    
	t_pt*           e_points;
    long            e_npoints;
    float           e_roundness;
    t_efont         e_font;
    t_symbol*       e_anchor;
    t_symbol*       e_justify;
    t_symbol*       e_text;

} t_egobj;

typedef struct _elayer
{
    t_object*           e_owner;
    t_symbol*           e_name;
    t_symbol*           e_id;
    int                 e_state;
    
    t_rect              e_rect;
    t_symbol*           e_color;
    int                 e_width;
    
    t_matrix            e_matrix;
    t_egobj             e_new_objects;
    t_egobj*            e_objects;
    long                e_number_objects;
} t_elayer;

typedef struct _eview
{
    t_object*   e_owner;
    t_symbol*   e_name;
} t_eview;

typedef struct _edrawparams
{    
	float		d_cornersize;
	float       d_borderthickness;
	t_rgba      d_bordercolor;
	t_rgba      d_boxfillcolor;
    
} t_edrawparams;

t_class *eproxy_class;

typedef struct _eproxy {
	t_pd        p_pd;
	t_object*   p_owner;
    int         p_index;
} t_eproxy;

typedef struct _ebox
{
    t_object            e_obj;
    t_canvas*           e_canvas;
    t_eproxy            e_proxy[256];
    long                e_nproxy;
    long                e_current_proxy;
    
    t_symbol*           e_object_id;
    t_symbol*           e_objuser_id;
    t_symbol*           e_objpreset_id;
    t_symbol*           e_canvas_id;
    t_symbol*           e_drawing_id;
    t_symbol*           e_editor_id;
    t_symbol*           e_window_id;
    t_symbol*           e_all_id;
    
    long                e_flags;
    t_rect              e_rect;
    t_efont             e_font;
    t_rect              e_rect_last;
    int                 e_selected_box;
    int                 e_selected_item;
    int                 e_selected_inlet;
    int                 e_selected_outlet;
    
    t_pt                e_mouse;
    t_pt                e_move_box;
    char                e_mouse_down;
    long                e_modifiers;
    
    char                e_ready_to_draw;
    t_edrawparams       e_boxparameters;
    
    t_elayer*           e_layers;
    long                e_number_of_layers;
    
    t_inlet*            e_inlets[256];
    t_outlet*           e_outlets[256];
    
    float               e_float;                // float value for signal
    long                e_dsp_size;
    t_int               e_dsp_vectors[262];
    long                e_dsp_flag;
    void*               e_dsp_user_param;
    long                z_misc;     // Special Max for Z_NO_INPLACE
    t_float*            z_sigs_out[256];
    method              e_perform_method;
    
    t_object*           b_firstin; // For Max, it doesn't matter !
}t_ebox;





#endif 