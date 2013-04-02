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
#include "g_all_guis.h"
#include <string.h>

#include "prefs_tcl.h"

#define UNUSED(x) (void)(x)

static t_class *tg_class;
t_widgetbehavior tglgrid_behavior;


typedef struct tg {
  t_object x_obj;

  t_int cols;
  t_int rows;
  t_int cell_size;
  t_int spacing;

  char tglfill[8];
  char untglfill[8];

  t_int mouse_x, mouse_y,tgld_col,tgld_row;
  char  mouse_tgld;
  t_canvas *canvas;

  t_float outputcol;
  t_atom *outputvals;

  int selected;
  char *toggled;
  t_symbol *name;
  t_glist *glist;
  t_outlet *f_out;
} t_tg;

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

static char do_toggle(t_tg* tg, int row, int col) {
  if (row >= 0 && col >= 0 && row < tg->rows && col < tg->cols) {
    char tgld = toggle(tg,col,row);
    if (tgld!='0')
      sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -fill %s\n",
               tg->canvas, tg, col, row, tg->tglfill);
    else
      sys_vgui(".x%lx.c itemconfigure %lxTGLSQ%d.%d -fill %s\n",
               tg->canvas, tg, col, row, tg->untglfill);
    return tgld;
  }
  error("tglgrid: Request to toggle nonexistent cell: %d,%d",col,row);
  return '0';
}


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

void tg_tgl(t_tg* tg, t_floatarg cf, t_floatarg rf) {
  t_int c = (int)cf;
  t_int r = (int)rf;
  do_toggle(tg,r,c);
}

void tg_on(t_tg* tg, t_floatarg cf, t_floatarg rf) {
  t_int c = (int)cf;
  t_int r = (int)rf;
  if (toggle_val(tg,c,r)=='0')
    do_toggle(tg,r,c);
}

void tg_off(t_tg* tg, t_floatarg cf, t_floatarg rf) {
  t_int c = (int)cf;
  t_int r = (int)rf;
  if (toggle_val(tg,c,r)!='0')
    do_toggle(tg,r,c);
}

void tg_say(t_tg* tg, t_floatarg cf, t_floatarg rf) {
  t_int c = (int)cf;
  t_int r = (int)rf;
  if (toggle_val(tg,c,r)=='0')
    outlet_float(tg->f_out,0);
  else
    outlet_float(tg->f_out,1);

}

static int check_arg(t_atom *arg, t_atomtype target, int idx) {
  if (arg->a_type != target) {
    error("tglgrid: invalid type passed for argument %d (is %d, want %d)",idx,arg->a_type,target);
    return 1;
  }
  return 0;
}

static void *tg_new(t_symbol *s, int argc, t_atom *argv) {
  int i,tocheck;
  t_tg *tg = (t_tg*)pd_new(tg_class);

  UNUSED(s);
  if (argc > 7) {
    post("tglgrid: warning, too many arguments passed to tglgrid, ignoring some");
    argc = 7;
  }

  tocheck = argc;
  while (tocheck > 0) {
    t_atomtype target =
      tocheck==1?A_FLOAT: // cols
      tocheck==2?A_FLOAT: // rows
      tocheck==3?A_SYMBOL: // toggled
      tocheck==4?A_FLOAT: // cell_size
      tocheck==5?A_FLOAT: // spacing
      tocheck==6?A_SYMBOL: // toggled fill
      A_SYMBOL; // untoggled fill
    if (check_arg(argv+tocheck-1,target,tocheck))
      return NULL;
    tocheck--;
  }

  tg->cols =  (argc>0)?(t_int)argv[0].a_w.w_float:16;
  tg->rows =  (argc>1)?(t_int)argv[1].a_w.w_float:16;
  tg->cell_size = (argc>3)?(t_int)argv[3].a_w.w_float:20;
  tg->spacing = (argc>4)?(t_int)argv[4].a_w.w_float:2;

  if (argc > 5) snprintf(tg->tglfill,8,argv[5].a_w.w_symbol->s_name);
  else          sprintf(tg->tglfill,"#909090");
  if (argc > 6) snprintf(tg->untglfill,8,argv[6].a_w.w_symbol->s_name);
  else          sprintf(tg->untglfill,"#FFFFFF");

  tg->outputcol = -1;
  tg->outputvals = (t_atom *)getbytes(sizeof(t_atom) * tg->rows);
  for (i = 0;i<tg->rows;i++) tg->outputvals[i].a_type = A_FLOAT;

  tg->toggled = (char*)getbytes(sizeof(char)*tg->rows*tg->cols);
  if (argc > 2) {
    int len = strlen(argv[2].a_w.w_symbol->s_name);
    if (len != 0 && len != (tg->rows*tg->cols)) {
      error("tglgrid: invalid toggled state passed, defaulting to all un-toggeled");
      len = 0;
    }
    if (len != 0)
      strcpy(tg->toggled,argv[2].a_w.w_symbol->s_name);
    else
      for (i = 0;i<(tg->rows*tg->cols);i++) tg->toggled[i]='0';
  } else for (i = 0;i<(tg->rows*tg->cols);i++) tg->toggled[i]='0';

  tg->name = gensym("tglgrid");
  tg->glist = (t_glist *)canvas_getcurrent();

  floatinlet_new(&tg->x_obj, &tg->outputcol);
  outlet_new(&tg->x_obj, &s_list);
  tg->f_out = outlet_new(&tg->x_obj, &s_float);

  return tg;
}

static void tg_free(t_tg *tg) {
  freebytes(tg->outputvals, sizeof(t_atom)*tg->rows);
  freebytes(tg->toggled, sizeof(char)*tg->rows*tg->cols);
}


static int full_width(t_tg *tg) {
  return ((tg->cell_size+tg->spacing)*tg->cols)+tg->spacing;
}

static int full_height(t_tg *tg) {
  return ((tg->cell_size+tg->spacing)*tg->rows)+tg->spacing;
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

  tg->canvas = canvas;

  curx = text_xpix(&tg->x_obj, glist)+tg->spacing;
  cury = text_ypix(&tg->x_obj, glist)+tg->spacing;

  for (r = 0;r < tg->rows;r++) {
    for (c = 0;c < tg->cols;c++) {
      if (toggle_val(tg,c,r) != '0')
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %lxTGLSQ%d.%d\n",
                 canvas, curx, cury, curx + tg->cell_size, cury + tg->cell_size, tg->tglfill, tg, c, r);
      else
        //sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill [.x%lx.c cget -background] -tags %lxTGLSQ%d.%d\n",
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill %s -tags %lxTGLSQ%d.%d\n",
                 canvas, curx, cury, curx + tg->cell_size, cury + tg->cell_size, tg->untglfill, tg, c, r);

      //set up highlighting
      sys_vgui(".x%lx.c bind %lxTGLSQ%d.%d <Enter> {.x%lx.c itemconfigure %lxTGLSQ%d.%d -outline #FF0000}\n",
               canvas,tg,c,r,canvas,tg,c,r);
      sys_vgui(".x%lx.c bind %lxTGLSQ%d.%d <Leave> {.x%lx.c itemconfigure %lxTGLSQ%d.%d -outline #000000}\n",
               canvas,tg,c,r,canvas,tg,c,r);
      curx += (tg->cell_size+tg->spacing);
    }
    curx = text_xpix(&tg->x_obj, glist)+tg->spacing;
    cury += (tg->cell_size+tg->spacing);
  }

  curx = text_xpix(&tg->x_obj, glist);
  cury = text_ypix(&tg->x_obj, glist);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxTGLBOUND\n",
           canvas, curx, cury, curx + w, cury + h, tg, canvas);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLIN1\n",
           canvas, curx, cury, curx + IOWIDTH, cury + 4, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLIN2\n",
           canvas, curx+w-IOWIDTH, cury, curx + w, cury + 4, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLOUT1\n",
           canvas, curx, cury+h-4, curx + IOWIDTH, cury + h, tg);
  sys_vgui(".x%lx.c create rectangle %d %d %d %d -fill #000000 -tags %lxTGLOUT2\n",
           canvas, curx+w-IOWIDTH, cury+h-4, curx + w, cury + h, tg);
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

  curx = text_xpix(&tg->x_obj, glist)+tg->spacing;
  cury = text_ypix(&tg->x_obj, glist)+tg->spacing;

  for (j = 0;j < tg->rows;j++) {
    for (i = 0;i < tg->cols;i++) {
      sys_vgui(".x%lx.c coords %lxTGLSQ%d.%d %d %d %d %d\n",
               canvas, tg, i, j, curx, cury, curx + tg->cell_size, cury + tg->cell_size);
      curx += (tg->cell_size+tg->spacing);
    }
    curx = text_xpix(&tg->x_obj, glist)+tg->spacing;
    cury += (tg->cell_size+tg->spacing);
  }

  curx = text_xpix(&tg->x_obj, glist);
  cury = text_ypix(&tg->x_obj, glist);
  sys_vgui(".x%lx.c coords %lxTGLBOUND %d %d %d %d\n",
           canvas, tg, curx, cury, curx + w, cury + h);
  sys_vgui(".x%lx.c coords %lxTGLIN1 %d %d %d %d\n",
           canvas, tg, curx, cury, curx + IOWIDTH, cury + 4);
  sys_vgui(".x%lx.c coords %lxTGLIN2 %d %d %d %d\n",
           canvas, tg, curx+w-IOWIDTH, cury, curx + w, cury + 4);
  sys_vgui(".x%lx.c coords %lxTGLOUT1 %d %d %d %d\n",
           canvas, tg, curx, cury+h-4, curx + IOWIDTH, cury + h);
  sys_vgui(".x%lx.c coords %lxTGLOUT2 %d %d %d %d\n",
           canvas, tg, curx+w-IOWIDTH, cury+h-4, curx + w, cury + h);
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
  binbuf_addv(b,"ssiisiisiiss",gensym("#X"),gensym("obj"),
              (t_int)tg->x_obj.te_xpix, (t_int)tg->x_obj.te_ypix,
              tg->name,tg->cols,tg->rows,gensym(tg->toggled),
              tg->cell_size,tg->spacing,
              gensym(tg->tglfill),gensym(tg->untglfill));
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

  int fss = tg->cell_size+tg->spacing;
  int rm = relx % fss;
  if (rm > tg->cell_size) {
    *row = *col = -1;
    return;
  }

  rm = rely % fss;
  if (rm > tg->cell_size) {
    *row = *col = -1;
    return;
  }

  *col = relx / fss;
  *row = rely / fss;
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
    do_toggle(tg,row,col);
    tg->tgld_col = col;
    tg->tgld_row = row;
  }
}

static int tglgrid_click(t_gobj *z, struct _glist *glist,
                         int xpix, int ypix, int shift,
                         int alt, int dbl, int doit) {
  t_tg* tg = (t_tg *)z;
  int row,col;

  UNUSED(shift);
  UNUSED(alt);
  UNUSED(dbl);

  col_and_row(tg,glist,xpix,ypix,&col,&row);
  if (doit && row>=0) {
    tg->mouse_tgld = do_toggle(tg,row,col);
    tg->tgld_col = col;
    tg->tgld_row = row;
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

static void tglgrid_properties(t_gobj *z, t_glist *owner) {
  t_tg *tg = (t_tg *)z;
  char buf[800];

  UNUSED(owner);

  sprintf(buf, "tglgrid_dialog %%s %s %d %d %d %d %s %s #FFFFFF\n",
          tg->name->s_name,
          (int)tg->cols, (int)tg->rows, (int)tg->cell_size, (int)tg->spacing,
          tg->tglfill, tg->untglfill);

  gfxstub_new(&tg->x_obj.ob_pd, tg, buf);
}

static void tglgrid_dialog(t_tg *tg, t_symbol *s, int argc, t_atom *argv) {
  UNUSED(s);
  if (!tg)
    error("tglgrid: tried to set properties on a non-existant grid");
  else if (argc != 6)
    error("tglgrid: invalid number of arguments passed to tglgrid_dialog.  (Expected 6, got %d)",
          argc);
  else if (argv[0].a_type != A_FLOAT ||
           argv[1].a_type != A_FLOAT ||
           argv[2].a_type != A_FLOAT ||
           argv[3].a_type != A_FLOAT ||
           argv[4].a_type != A_SYMBOL ||
           argv[5].a_type != A_SYMBOL)
    error("tglgrid: invalid parameter types passed to tglgrid_dialog");
  else {
    int i;
    t_int newcols = (t_int)argv[0].a_w.w_float;
    t_int newrows = (t_int)argv[1].a_w.w_float;
    tg->cell_size = (t_int)argv[2].a_w.w_float;
    tg->spacing = (t_int)argv[3].a_w.w_float;
    snprintf(tg->tglfill,8,argv[4].a_w.w_symbol->s_name);
    snprintf(tg->untglfill,8,argv[5].a_w.w_symbol->s_name);

    // need to erase before we change # of rows/cols
    // so we erase the old size
    draw_erase(tg,tg->glist);

    if (newcols != tg->cols ||
        newrows != tg->rows) {
      char* tgld;
      int j,olim;
      if (newrows != tg->rows) {
        tg->outputvals = (t_atom*)resizebytes(tg->outputvals,
                                              sizeof(t_atom)*tg->rows,
                                              sizeof(t_atom)*newrows);
        for(i = tg->rows;i < newrows;i++) tg->outputvals[i].a_type = A_FLOAT;
      }
      tgld = (char*)getbytes(sizeof(char)*newrows*newcols);
      olim = tg->rows*tg->cols;
      for (i = 0,j=0;i<(newrows*newcols);i++) {
        if (j>=olim) tgld[i]='0';
        else {
          if ((i % newrows) >= tg->rows) {
            tgld[i]='0';
          }
          else {
            tgld[i] = tg->toggled[j];
            j++;
            if ((j % tg->rows) >= newrows) j+=(tg->rows-newrows);
          }
        }
      }
      freebytes(tg->toggled, sizeof(char)*tg->rows*tg->cols);
      tg->toggled = tgld;
      tg->cols = newcols;
      tg->rows = newrows;
    }

    draw_new(tg,tg->glist);
  }
}

void tglgrid_setup(void) {
  sys_gui(prefs_tcl);
  tg_class = class_new(gensym("tglgrid"),
                       (t_newmethod)tg_new,
                       (t_method)tg_free,
                       sizeof(t_tg),
                       CLASS_DEFAULT,
                       A_GIMME,
                       0);
  class_addbang(tg_class, tg_bang);
  class_addmethod(tg_class,
                  (t_method)tg_tgl,gensym("tgl"),
                  A_FLOAT,A_FLOAT,0);
  class_addmethod(tg_class,
                  (t_method)tg_on,gensym("on"),
                  A_FLOAT,A_FLOAT,0);
  class_addmethod(tg_class,
                  (t_method)tg_off,gensym("off"),
                  A_FLOAT,A_FLOAT,0);
  class_addmethod(tg_class,
                  (t_method)tg_say,gensym("say"),
                  A_FLOAT,A_FLOAT,0);
  class_addmethod(tg_class,
                  (t_method)tglgrid_dialog,gensym("dialog"),
                  A_GIMME, 0);

  tglgrid_behavior.w_getrectfn =    tglgrid_getrect;
  tglgrid_behavior.w_displacefn =   tglgrid_displace;
  tglgrid_behavior.w_selectfn =     tglgrid_select;
  tglgrid_behavior.w_activatefn =   NULL;
  tglgrid_behavior.w_deletefn =     tglgrid_delete;
  tglgrid_behavior.w_visfn =        tglgrid_vis;
  tglgrid_behavior.w_clickfn =      tglgrid_click;

#if PD_MINOR_VERSION >= 37
  class_setpropertiesfn(tg_class, tglgrid_properties);
  class_setsavefn(tg_class, tglgrid_save);
#else
  grid_widgetbehavior.w_propertiesfn = tglgrid_properties;
  grid_widgetbehavior.w_savefn =    tlggrid_save;
#endif

  class_setwidget(tg_class, &tglgrid_behavior);
}


