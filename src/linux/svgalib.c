/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Video driver using SVGAlib.
 *
 *      By Stefan T. Boettner.
 * 
 *      Modified extensively by Peter Wang.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/aintern.h"
#include "allegro/aintunix.h"
#include "linalleg.h"


#ifdef ALLEGRO_LINUX_SVGALIB

#include <signal.h>
#include <termios.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/ioctl.h>
#include <vga.h>



static BITMAP *svga_init(int w, int h, int v_w, int v_h, int color_depth);
static void svga_exit(BITMAP *b);
static int  svga_scroll(int x, int y);
static void svga_vsync(void);
static void svga_set_palette(AL_CONST RGB *p, int from, int to, int vsync);
static void svga_save(void);
static void svga_restore(void);

#ifndef ALLEGRO_NO_ASM
unsigned long _svgalib_read_line_asm(BITMAP *bmp, int line);
unsigned long _svgalib_write_line_asm(BITMAP *bmp, int line);
void _svgalib_unwrite_line_asm(BITMAP *bmp);
#endif



GFX_DRIVER gfx_svgalib = 
{
   GFX_SVGALIB,
   empty_string,
   empty_string,
   "SVGAlib", 
   svga_init,
   svga_exit,
   svga_scroll,
   svga_vsync,
   svga_set_palette,
   NULL, NULL, NULL,             /* no triple buffering */
   NULL, NULL, NULL, NULL,       /* no video bitmaps */
   NULL, NULL,                   /* no system bitmaps */
   NULL, NULL, NULL, NULL,       /* no hardware cursor */
   NULL,                         /* no drawing mode hook */
   svga_save,
   svga_restore,
   0, 0,
   TRUE,
   0, 0, 0, 0, FALSE
};



static char svga_desc[256] = EMPTY_STRING;

static int svga_mode;

static unsigned int display_start_mask;
static unsigned int scanline_width;
static int bytes_per_pixel;

static unsigned char *screen_buffer;
static int last_line;



/* _svgalib_read_line:
 *  Return linear offset for reading line.
 */
unsigned long _svgalib_read_line(BITMAP *bmp, int line)
{
   return (unsigned long) (bmp->line[line]);
}



/* _svgalib_write_line:
 *  Update last selected line and select new line.
 */
unsigned long _svgalib_write_line(BITMAP *bmp, int line)
{
   int new_line = line + bmp->y_ofs;
   if ((new_line != last_line) && (last_line >= 0))
      vga_drawscansegment(screen_buffer + last_line * scanline_width, 0, last_line, scanline_width);
   last_line = new_line;
   return (unsigned long) (bmp->line[line]);
}



/* _svgalib_unwrite_line:
 *  Update last selected line.
 */
void _svgalib_unwrite_line(BITMAP *bmp)
{
   if (last_line >= 0) {
      vga_drawscanline(last_line, screen_buffer + last_line * scanline_width);
      last_line = -1;
   }
}



/* save_signals, restore_signals:
 *  Helpers to save and restore signals captured by SVGAlib.
 */
static const int signals[] = {
   SIGUSR1, SIGUSR2,
   SIGHUP, SIGINT, SIGQUIT, SIGILL,
   SIGTRAP, SIGIOT, SIGBUS, SIGFPE,
   SIGSEGV, SIGPIPE, SIGALRM, SIGTERM,
   SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPWR
};

#define NUM_SIGNALS	(sizeof (signals) / sizeof (int))

static struct sigaction old_signals[NUM_SIGNALS];

static void save_signals()
{
   int i;
   for (i = 0; i < NUM_SIGNALS; i++) 
      sigaction(signals[i], NULL, old_signals+i);
}

static void restore_signals()
{
   int i;
   for (i = 0; i < NUM_SIGNALS; i++)
      sigaction(signals[i], old_signals+i, NULL);
}



/* safe_vga_setmode:
 *  We don't want SVGAlib messing with our keyboard driver or taking 
 *  over control of VT switching.  Note that doing all this every 
 *  time is possibly a little excessive.  
 */
static int safe_vga_setmode(int num, int tio)
{
   struct termios termio;
   struct vt_mode vtm;
   int ret;

   save_signals();
   if (tio) 
      ioctl(__al_linux_console_fd, VT_GETMODE, &vtm);
   tcgetattr(__al_linux_console_fd, &termio);

   ret = vga_setmode(num);

   tcsetattr(__al_linux_console_fd, TCSANOW, &termio);
   if (tio) 
      ioctl(__al_linux_console_fd, VT_SETMODE, &vtm);
   restore_signals();

   return ret;
}



/* mode_ok:
 *  Check if the mode passed matches the size and color depth requirements.
 */
static int mode_ok(vga_modeinfo *info, int w, int h, int v_w, int v_h, 
		   int color_depth)
{
   return ((((color_depth == 8) && (info->colors == 256))
	    || ((color_depth == 15) && (info->colors == 32768))
	    || ((color_depth == 16) && (info->colors == 65536))
	    || ((color_depth == 24) && (info->bytesperpixel == 3))
	    || ((color_depth == 32) && (info->bytesperpixel == 4)))
	   && (((info->width == w) && (info->height == h))
	       || ((w == 0) && (h == 0)))
 	   && (info->linewidth >= (MAX(w, v_w) * info->bytesperpixel))
 	   && (info->maxpixels >= (MAX(w, v_w) * MAX(h, v_h))));
}



/* find_and_set_mode:
 *  Helper to find a suitable video mode and then set it.
 */
static vga_modeinfo *find_and_set_mode(int w, int h, int v_w, int v_h,
				       int color_depth, int flags)
{
   vga_modeinfo *info;
   int i;
    
   for (i = 0; i <= vga_lastmodenumber(); i++) {
      if (!vga_hasmode(i)) 
	 continue;
      
      info = vga_getmodeinfo(i);
      if ((info->flags & IS_MODEX)
	  || ((flags) && !(info->flags & flags))
	  || (!mode_ok(info, w, h, v_w, v_h, color_depth)))
	 continue;
      
      if (safe_vga_setmode(i, 1) == 0) {
	 svga_mode = i;
	 gfx_svgalib.w = vga_getxdim();
	 gfx_svgalib.h = vga_getydim();
	 return info;
      }
   }

   return NULL;
}



/* set_color_shifts:
 *  Set the color shift values for truecolor modes.
 */
static void set_color_shifts(int color_depth, int bgr)
{
   switch (color_depth) {

      #ifdef ALLEGRO_COLOR16

         case 15:
            _rgb_r_shift_15 = 10;
            _rgb_g_shift_15 = 5;
            _rgb_b_shift_15 = 0;
            break;

         case 16:
            _rgb_r_shift_16 = 11;
            _rgb_g_shift_16 = 5;
            _rgb_b_shift_16 = 0;
            break;

      #endif

      #ifdef ALLEGRO_COLOR24

         case 24:
            if (bgr) {
               _rgb_r_shift_24 = 0;
               _rgb_g_shift_24 = 8;
               _rgb_b_shift_24 = 16;
	    }
            else {
               _rgb_r_shift_24 = 16;
               _rgb_g_shift_24 = 8;
               _rgb_b_shift_24 = 0;
	    }
            break;

      #endif

      #ifdef ALLEGRO_COLOR32

         case 32:
            if (bgr) {
               _rgb_a_shift_32 = 0;
               _rgb_r_shift_32 = 8;
               _rgb_g_shift_32 = 16;
               _rgb_b_shift_32 = 24;
	    }
            else {
               _rgb_a_shift_32 = 24;
               _rgb_r_shift_32 = 16;
               _rgb_g_shift_32 = 8;
               _rgb_b_shift_32 = 0;
	    }
            break;
       
      #endif
   }
}



/* do_set_mode:
 *  Do the hard work of setting a video mode, then return a screen bitmap.
 */
static BITMAP *do_set_mode(int w, int h, int v_w, int v_h, int color_depth)
{
   int vid_mem, width, height;
   vga_modeinfo *info;
   BITMAP *bmp;

   /* Try get a linear frame buffer.  */

   info = find_and_set_mode(w, h, v_w, v_h, color_depth, CAPABLE_LINEAR);
   if (info) { 
      vid_mem = vga_setlinearaddressing();
      if (vid_mem < 0) {
	 ustrcpy(allegro_error, get_config_text("Cannot enable linear addressing"));
	 return NULL;
      }

      width = info->linewidth / info->bytesperpixel;
      scanline_width = info->linewidth;

      /* Set entries in gfx_svgalib.  */
      gfx_svgalib.vid_mem = vid_mem;
      gfx_svgalib.scroll = svga_scroll;

      ustrcpy(svga_desc, uconvert_ascii("SVGAlib (linear)", NULL));
      gfx_svgalib.desc = svga_desc;

      /* For hardware scrolling.  */
      display_start_mask = info->startaddressrange;
      bytes_per_pixel = info->bytesperpixel;

      /* Set truecolor format.  */
      set_color_shifts(color_depth, (info->flags & RGB_MISORDERED));

      /* Make the screen bitmap.  */
      return _make_bitmap(width, info->maxpixels / width, 
			  (unsigned long)vga_getgraphmem(),
			  &gfx_svgalib, color_depth, scanline_width);
   }

   /* Try get a banked frame buffer.  */
   
   /* We don't support virtual screens larger than the screen itself 
    * in banked mode.  */
   if ((v_w > w) || (v_h > h)) {
      ustrcpy(allegro_error, get_config_text("Resolution not supported"));
      return NULL;
   }

   info = find_and_set_mode(w, h, v_w, v_h, color_depth, 0);
   if (info) {
      width = gfx_svgalib.w;
      height = gfx_svgalib.h;
      scanline_width = width * info->bytesperpixel;
      vid_mem = scanline_width * height;

      /* Allocate memory buffer for screen.  */
      screen_buffer = malloc(vid_mem);
      if (!screen_buffer) 
	 return NULL;
      last_line = -1;
       
      /* Set entries in gfx_svgalib.  */
      gfx_svgalib.vid_mem = vid_mem;
      gfx_svgalib.scroll = NULL;

      ustrcpy(svga_desc, uconvert_ascii("SVGAlib (banked)", NULL));
      gfx_svgalib.desc = svga_desc;

      /* Set truecolor format.  */
      set_color_shifts(color_depth, 0);

      /* Make the screen bitmap.  */
      bmp = _make_bitmap(width, height, (unsigned long)screen_buffer,
			 &gfx_svgalib, color_depth, scanline_width);
      if (bmp) {
	 /* Set bank switching routines.  */
#ifndef ALLEGRO_NO_ASM
	 bmp->read_bank = _svgalib_read_line_asm;
	 bmp->write_bank = _svgalib_write_line_asm;
	 bmp->vtable->unwrite_bank = _svgalib_unwrite_line_asm;
#else
	 bmp->read_bank = _svgalib_read_line;
	 bmp->write_bank = _svgalib_write_line;
	 bmp->vtable->unwrite_bank = _svgalib_unwrite_line;
#endif
      }

      return bmp;
   }

   ustrcpy(allegro_error, get_config_text("Resolution not supported"));
   return NULL;
}



/* svga_init:
 *  Entry point to set a video mode.
 */
static BITMAP *svga_init(int w, int h, int v_w, int v_h, int color_depth)
{
   static int virgin = 1;
   BITMAP *bmp = NULL;
   int svgalib2 = 0;

   /* SVGAlib 2.0 doesn't require special permissions.  */
#ifdef ALLEGRO_LINUX_SVGALIB_HAVE_VGA_VERSION
   svgalib2 = vga_version >= 0x1900;
#endif

   if ((!svgalib2) && (!__al_linux_have_ioperms)) {
      ustrcpy(allegro_error, get_config_text("This driver needs root privileges"));
      return NULL;
   }

   /* Stop interrupts processing, which interferes with the SVGAlib
    * VESA driver.  */
   __al_linux_async_exit();

   /* Initialise SVGAlib.  */
   if (virgin) {
      if (!svgalib2) 
	 seteuid(0);
      else {
	 /* Avoid having SVGAlib calling exit() on us.  */
	 int fd = open("/dev/svga", O_RDWR);
	 if (fd < 0)
	    goto error;
	 close(fd);
      }
      vga_disabledriverreport();
      if (vga_init() != 0)
	 goto error;
      if (!svgalib2)
	 seteuid(getuid());
      virgin = 0;
   }
    
   /* Ask for a video mode.  */
   bmp = do_set_mode(w, h, v_w, v_h, color_depth);

   error:

   /* Restart interrupts processing.  */
   __al_linux_async_init();

   return bmp;
}



/* svga_exit:
 *  Unsets the video mode.
 */
static void svga_exit(BITMAP *b)
{
   if (screen_buffer) {
      free(screen_buffer);
      screen_buffer = NULL;
   }

   safe_vga_setmode(TEXT, 1);
}



/* svga_scroll:
 *  Hardware scrolling routine.
 */
static int svga_scroll(int x, int y)
{
   vga_setdisplaystart((x * bytes_per_pixel + y * scanline_width) 
		       /* & display_start_mask */);
   /* The bitmask seems to mess things up on my machine, even though
    * the documentation says it should be there. -- PW  */
   return 0;
}



/* svga_vsync:
 *  Waits for a retrace.
 */
static void svga_vsync()
{
   vga_waitretrace();
}



/* svga_set_palette:
 *  Sets the palette.
 */
static void svga_set_palette(AL_CONST RGB *p, int from, int to, int vsync)
{
   int i;

   if (vsync)
      vga_waitretrace();

   for (i = from; i <= to; i++)
      vga_setpalette(i, p[i].r, p[i].g, p[i].b);
}



/* svga_save:
 *  Saves the graphics state.
 */
static void svga_save()
{
   safe_vga_setmode(TEXT, 0);
}



/* svga_restore:
 *  Restores the graphics state.
 */
static void svga_restore()
{
   al_linux_set_async_mode(ASYNC_OFF);
   _sigalrm_stop_timer();
    
   safe_vga_setmode(svga_mode, 0);
   vga_setpage(0);
    
   _sigalrm_start_timer();
   al_linux_set_async_mode(ASYNC_DEFAULT);
}



#endif      /* ifdef ALLEGRO_LINUX_SVGALIB */
