/* tglgrid.c
 * An external for a grid of toggable cells
 * Copyright 2013 Nick Lanham <nick@afternight.org>
 *
 * Public License v3. source code is available at
 * <http://github.com/nicklan/tglgrid>

 * THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include "m_pd.h"
#include "g_canvas.h"
#include <string.h>

#define UNUSED(x) (void)(x)

static t_class *tg_class;
t_widgetbehavior tglgrid_behavior;

typedef struct tg {
  t_object x_obj;

  t_int cols;
  t_int rows;
  t_int ssize;
  t_int spacing;

  t_int last_col;
  t_int last_row;
  t_int mouse_x, mouse_y,tgld_col,tgld_row;
  char  mouse_tgld;
  t_canvas *mouse_canvas;

  t_float outputcol;
  t_atom *outputvals;

  int selected;
  char *toggled;
  t_symbol *name;
  t_glist *glist;
} t_tg;

void tg_bang(t_tg* tg) {
  int i,j,coff;
  t_int outcol = tg->outputcol;
  if (outcol < 0) outcol = 0;
  if (outcol >= tg->cols) outcol = tg->cols-1;
  coff = outcol*tg->rows;

  for (j=0,i = 0;i < tg->rows;i++)
    if (tg->toggled[i+coff]!='0')
      tg->outputvals[j++].a_w.w_float = (t_float)(tg->rows-i);

  outlet_list(tg->x_obj.ob_outlet, &s_list, j, tg->outputvals);
}

static void *tg_new(t_floatarg w, t_floatarg h, t_symbol *toggled) {
  int i;
  t_tg *tg = (t_tg*)pd_new(tg_class);
  tg->cols  = (w>0)?w:16;
  tg->rows = (h>0)?h:16;

  tg->last_col = -1;
  tg->last_row = -1;
  tg->outputcol = -1;
  tg->ssize = 20;
  tg->spacing = 2;

  tg->outputvals = (t_atom *)getbytes(sizeof(t_atom) * tg->rows);
  for (i = 0;i<tg->rows;i++) tg->outputvals[i].a_type = A_FLOAT;

  tg->toggled = (char*)getbytes(sizeof(char)*tg->rows*tg->cols);
  if (toggled->s_name != NULL) {
    int len = strlen(toggled->s_name);
    if (len != 0 && len != (tg->rows*tg->cols)) {
      error("tglgrid: invalid toggled state passed, defaulting to all un-toggeled");
      len = 0;
    }
    if (len != 0)
      strcpy(tg->toggled,toggled->s_name);
    else
      for (i = 0;i<(tg->rows*tg->cols);i++) tg->toggled[i]='0';
  } else for (i = 0;i<(tg->rows*tg->cols);i++) tg->toggled[i]='0';

  tg->name = gensym("tglgrid");
  tg->glist = (t_glist *)canvas_getcurrent();

  floatinlet_new(&tg->x_obj, &tg->outputcol);
  outlet_new(&tg->x_obj, &s_list);

  return tg;
}

static void tg_free(t_tg *tg) {
  freebytes(tg->outputvals, sizeof(t_atom)*tg->rows);
  freebytes(tg->toggled, sizeof(char)*tg->rows*tg->cols);
}


static int full_width(t_tg *tg) {
  return ((tg->ssize+tg->spacing)*tg->cols)+tg->spacing;
}

static int full_height(t_tg *tg) {
  return ((tg->ssize+tg->spacing)*tg->rows)+tg->spacing;
}

static inline char toggle(t_tg *tg, int col, int row) {
  int coff = col*tg->rows;
  // use 'x' since '1' will make pd think our saved binbuf is a number
  tg->toggled[row+coff] = tg->toggled[row+coff]!='0'?'0':'x';
  return tg->toggled[row+coff];
}

static inline char toggle_val(t_tg *tg, int col, int row) {
 int coff = col*tg->rows;
 return tg->toggled[row+coff];
}

/* ******* *
 * Drawing *
 * ******* */

static void draw_new(t_tg* tg, t_glist *glist) {
  t_canvas *canvas = glist_getcanvas(glist);
  int c,r;
  int curx,cury;
  int w = full_width(tg);
  int h = full_height(tg);

  curx = text_xpix(&tg->x_obj, glist);
  cury = text_ypix(&tg->x_obj, glist);

  for (r = 0;r < tg->rows;r++) {
    for (c = 0;c < tg->cols;c++) {
      if (toggle_val(tg,c,r) != '0')
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #909090 -tags %lxTGLSQ%d.%d\n",
                 canvas, curx, cury, curx + tg->ssize, cury + tg->ssize, tg, c, r);
      else
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxTGLSQ%d.%d\n",
                 canvas, curx, cury, curx + tg->ssize, cury + tg->ssize, tg, c, r);
      curx += (tg->ssize+tg->spacing);
    }
    curx = text_xpix(&tg->x_obj, glist);
    cury += (tg->ssize+tg->spacing);
  }

  curx = text_xpix(&tg->x_obj, glist)-tg->spacing;
  cury = text_ypix(&tg->x_obj, glist)-tg->spacing;
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxTGLBOUND\n",
           canvas, curx, cury, curx + w, cury + h, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLIN1\n",
           canvas, curx, cury, curx + IOWIDTH, cury + 4, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLIN2\n",
           canvas, curx+w-IOWIDTH, cury, curx + w, cury + 4, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLOUT\n",
           canvas, curx, cury+h-4, curx + IOWIDTH, cury + h, tg);
  canvas_fixlinesfor(canvas, (t_text*)tg);
}

static void draw_erase(t_tg* tg, t_glist* glist) {
  t_canvas *canvas = glist_getcanvas(glist);
  int i,j;
  sys_vgui(".x%lx.c delete %lxTGLBOUND\n", canvas, tg);
  sys_vgui(".x%lx.c delete %lxTGLIN1\n", canvas, tg);
  sys_vgui(".x%lx.c delete %lxTGLIN2\n", canvas, tg);
  sys_vgui(".x%lx.c delete %lxTGLOUT\n", canvas, tg);
  for (j = 0;j < tg->rows;j++)
    for (i = 0;i < tg->cols;i++)
      sys_vgui(".x%lx.c delete %lxTGLSQ%d.%d\n", canvas, tg, i, j);
}

static void draw_select(t_tg* tg,t_glist* glist) {
  t_canvas *canvas = glist_getcanvas(glist);
  if(tg->selected) {
    pd_bind(&tg->x_obj.ob_pd, tg->name);
    sys_vgui(".x%lx.c itemconfigure %lxTGLBOUND -outline #0000FF\n", canvas, tg);
  }
  else {
    pd_unbind(&tg->x_obj.ob_pd, tg->name);
    sys_vgui(".x%lx.c itemconfigure %lxTGLBOUND -outline #000000\n", canvas, tg);
  }
}

static void draw_move(t_tg *tg, t_glist *glist) {
  t_canvas *canvas = glist_getcanvas(glist);
  int i,j,curx,cury;
  int w = full_width(tg);
  int h = full_height(tg);

  curx = text_xpix(&tg->x_obj, glist);
  cury = text_ypix(&tg->x_obj, glist);

  for (j = 0;j < tg->rows;j++) {
    for (i = 0;i < tg->cols;i++) {
      sys_vgui(".x%lx.c coords %lxTGLSQ%d.%d %d %d %d %d\n",
               canvas, tg, i, j, curx, cury, curx + tg->ssize, cury + tg->ssize);
      curx += (tg->ssize+tg->spacing);
    }
    curx = text_xpix(&tg->x_obj, glist);
    cury += (tg->ssize+tg->spacing);
  }

  curx = text_xpix(&tg->x_obj, glist)-tg->spacing;
  cury = text_ypix(&tg->x_obj, glist)-tg->spacing;
  sys_vgui(".x%lx.c coords %lxTGLBOUND %d %d %d %d\n",
           canvas, tg, curx, cury, curx + w, cury + h);
  sys_vgui(".x%lx.c coords %lxTGLIN1 %d %d %d %d\n",
           canvas, tg, curx, cury, curx + IOWIDTH, cury + 4);
  sys_vgui(".x%lx.c coords %lxTGLIN2 %d %d %d %d\n",
           canvas, tg, curx+w-IOWIDTH, cury, curx + w, cury + 4);
  sys_vgui(".x%lx.c coords %lxTGLOUT %d %d %d %d\n",
           canvas, tg, curx, cury+h-4, curx + IOWIDTH, cury + h);
  canvas_fixlinesfor(canvas, (t_text*)tg);
}

/* ******** *
 * Behavior *
 * ******** */

static void tglgrid_getrect(t_gobj *z, t_glist *owner,
                            int *xp1, int *yp1, int *xp2, int *yp2) {
   t_tg *tg = (t_tg*)z;
   *xp1 = text_xpix(&tg->x_obj, owner);
   *yp1 = text_ypix(&tg->x_obj, owner);
   *xp2 = text_xpix(&tg->x_obj, owner)+full_width(tg);
   *yp2 = text_ypix(&tg->x_obj, owner)+full_height(tg);
}

static void tglgrid_save(t_gobj *z, t_binbuf *b) {
  t_tg *tg = (t_tg*)z;
  binbuf_addv(b,"ssiisiis",gensym("#X"),gensym("obj"),
              (t_int)tg->x_obj.te_xpix, (t_int)tg->x_obj.te_ypix,
              tg->name,tg->cols,tg->rows,gensym(tg->toggled));
  binbuf_addv(b,";");
}

static void tglgrid_vis(t_gobj *z, t_glist *glist, int vis) {
  t_tg *tg = (t_tg*)z;
  if (vis)
    draw_new(tg, glist);
  else
    draw_erase(tg, glist);
}

static void tglgrid_select(t_gobj *z, t_glist *glist, int selected) {
   t_tg *tg = (t_tg *)z;
   tg->selected = selected;
   draw_select(tg, glist);
}

static void tglgrid_delete(t_gobj *z, t_glist *glist) {
  canvas_deletelinesfor(glist, (t_text *)z);
}

static void col_and_row(t_tg *tg, struct _glist *glist,
                        int x, int y, int *col, int *row) {
  int relx = x - text_xpix(&tg->x_obj, glist) - tg->spacing;
  int rely = y - text_ypix(&tg->x_obj, glist) - tg->spacing;
  if (relx < 0 || rely < 0) {
    *row = *col = -1;
    return;
  }

  int fss = tg->ssize+tg->spacing;
  int rm = relx % fss;
  if (rm > tg->ssize) {
    *row = *col = -1;
    return;
  }

  rm = rely % fss;
  if (rm > tg->ssize) {
    *row = *col = -1;
    return;
  }

  *col = relx / fss;
  *row = rely / fss;
}

static char mouse_toggle(t_tg* tg, int row, int col, t_canvas *canvas) {
  char tgld = toggle(tg,col,row);
  if (tgld!='0')
    sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -fill #909090\n",
             canvas, tg, col, row);
  else
    sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -fill #FFFFFF\n",
             canvas, tg, col, row);
  return tgld;
}

static void tglgrid_motion(t_tg* tg, t_floatarg dx, t_floatarg dy) {
  int row,col;
  tg->mouse_x += dx;
  tg->mouse_y += dy;
  col_and_row(tg,tg->glist,tg->mouse_x,tg->mouse_y,&col,&row);
  if (col < tg->cols && row < tg->rows &&
      col >= 0 && row >= 0 &&
      (col != tg->tgld_col || row != tg->tgld_row) &&
      (toggle_val(tg,col,row) != tg->mouse_tgld) ) {
    mouse_toggle(tg,row,col,tg->mouse_canvas);
    tg->tgld_col = col;
    tg->tgld_row = row;
  }
}

static int tglgrid_click(t_gobj *z, struct _glist *glist,
                         int xpix, int ypix, int shift,
                         int alt, int dbl, int doit) {
  t_tg* tg = (t_tg *)z;
  t_canvas *canvas = glist_getcanvas(glist);
  int row,col;

  UNUSED(shift);
  UNUSED(alt);
  UNUSED(dbl);

  col_and_row(tg,glist,xpix,ypix,&col,&row);
  if (row != tg->last_row || col != tg->last_col) {
    if (row >= 0)
      sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -outline #FF0000\n",
               canvas, tg, col, row);
    if (tg->last_row >= 0)
      sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -outline #000000\n",
               canvas, tg, tg->last_col, tg->last_row);
    tg->last_col = col;
    tg->last_row = row;
  }
  if (doit && row>=0) {
    tg->mouse_tgld = mouse_toggle(tg,row,col,canvas);
    tg->tgld_col = col;
    tg->tgld_row = row;
    tg->mouse_canvas = canvas;
    tg->mouse_x = xpix;
    tg->mouse_y = ypix;
    glist_grab(glist,&tg->x_obj.te_g,
               (t_glistmotionfn)tglgrid_motion,0,xpix,ypix);
  }
  return 1;
}

static void tglgrid_displace(t_gobj *z, t_glist *glist, int dx, int dy) {
  t_tg *tg = (t_tg *)z;
  int xold = text_xpix(&tg->x_obj, glist);
  int yold = text_ypix(&tg->x_obj, glist);
  tg->x_obj.te_xpix += dx;
  tg->x_obj.te_ypix += dy;
  if(xold != text_xpix(&tg->x_obj, glist) || yold != text_ypix(&tg->x_obj, glist))
    draw_move(tg, tg->glist);
}

void tglgrid_setup(void) {
  tg_class = class_new(gensym("tglgrid"),
                       (t_newmethod)tg_new,
                       (t_method)tg_free,
                       sizeof(t_tg),
                       CLASS_DEFAULT,
                       A_DEFFLOAT,
                       A_DEFFLOAT,
                       A_DEFSYMBOL,
                       0);
  class_addbang(tg_class, tg_bang);

  tglgrid_behavior.w_getrectfn =    tglgrid_getrect;
  tglgrid_behavior.w_displacefn =   tglgrid_displace;
  tglgrid_behavior.w_selectfn =     tglgrid_select;
  tglgrid_behavior.w_activatefn =   NULL;
  tglgrid_behavior.w_deletefn =     tglgrid_delete;
  tglgrid_behavior.w_visfn =        tglgrid_vis;
  tglgrid_behavior.w_clickfn =      tglgrid_click;

#if PD_MINOR_VERSION >= 37
  //class_setpropertiesfn(tg_class, grid_properties);
  class_setsavefn(tg_class, tglgrid_save);
#else
  //grid_widgetbehavior.w_propertiesfn = grid_properties;
  grid_widgetbehavior.w_savefn =    tlggrid_save;
#endif

  class_setwidget(tg_class, &tglgrid_behavior);
}


