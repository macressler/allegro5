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
 *      Stuff for BeOS.
 *
 *      By Jason Wilkins.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"
#include "allegro/aintern.h"
#include "allegro/aintbeos.h"

#ifndef ALLEGRO_BEOS
#error something is wrong with the makefile
#endif 



GFX_DRIVER gfx_beos_fullscreen = {
   GFX_BEOS_FULLSCREEN,               // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Fullscreen",                      // char *ascii_name;
   be_gfx_fullscreen_init,            // AL_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   be_gfx_fullscreen_exit,            // AL_METHOD(void, exit, (struct BITMAP *b));
   be_gfx_fullscreen_scroll,          // AL_METHOD(int, scroll, (int x, int y));
   be_gfx_fullscreen_vsync,           // AL_METHOD(void, vsync, (void));
   be_gfx_fullscreen_set_palette,     // AL_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
   NULL,                              // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                              // AL_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(struct BITMAP *, create_system_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_system_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, set_mouse_sprite, (struct BITMAP *sprite, int xfocus, int yfocus));
   NULL,                              // AL_METHOD(int, show_mouse, (struct BITMAP *bmp, int x, int y));
   NULL,                              // AL_METHOD(void, hide_mouse, (void));
   NULL,                              // AL_METHOD(void, move_mouse, (int x, int y));
   NULL,                              // AL_METHOD(void, drawing_mode, (void));
   NULL,                              // AL_METHOD(void, save_state, (void));
   NULL,                              // AL_METHOD(void, restore_state, (void));
   0, 0,                              // int w, h;  /* physical (not virtual!) screen size */
   TRUE,                              // int linear;  /* true if video memory is linear */
   0,                                 // long bank_size;  /* bank size, in bytes */
   0,                                 // long bank_gran;  /* bank granularity, in bytes */
   0,                                 // long vid_mem;  /* video memory size, in bytes */
   0,                                 // long vid_phys_base;  /* physical address of video memory */
   FALSE                              // int windowed;  /* true if driver runs windowed */
};



GFX_DRIVER gfx_beos_fullscreen_safe = {
   GFX_BEOS_FULLSCREEN_SAFE,          // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Safe Fullscreen",                 // char *ascii_name;
   be_gfx_fullscreen_safe_init,       // AL_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   be_gfx_fullscreen_exit,            // AL_METHOD(void, exit, (struct BITMAP *b));
   be_gfx_fullscreen_scroll,          // AL_METHOD(int, scroll, (int x, int y));
   be_gfx_fullscreen_vsync,           // AL_METHOD(void, vsync, (void));
   be_gfx_fullscreen_set_palette,     // AL_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
   NULL,                              // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                              // AL_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(struct BITMAP *, create_system_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_system_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, set_mouse_sprite, (struct BITMAP *sprite, int xfocus, int yfocus));
   NULL,                              // AL_METHOD(int, show_mouse, (struct BITMAP *bmp, int x, int y));
   NULL,                              // AL_METHOD(void, hide_mouse, (void));
   NULL,                              // AL_METHOD(void, move_mouse, (int x, int y));
   NULL,                              // AL_METHOD(void, drawing_mode, (void));
   NULL,                              // AL_METHOD(void, save_state, (void));
   NULL,                              // AL_METHOD(void, restore_state, (void));
   0, 0,                              // int w, h;  /* physical (not virtual!) screen size */
   TRUE,                              // int linear;  /* true if video memory is linear */
   0,                                 // long bank_size;  /* bank size, in bytes */
   0,                                 // long bank_gran;  /* bank granularity, in bytes */
   0,                                 // long vid_mem;  /* video memory size, in bytes */
   0,                                 // long vid_phys_base;  /* physical address of video memory */
   FALSE                              // int windowed;  /* true if driver runs windowed */
};



GFX_DRIVER gfx_beos_windowed = {
   GFX_BEOS_WINDOWED,                 // int id;
   empty_string,                      // char *name;
   empty_string,                      // char *desc;
   "Windowed",                        // char *ascii_name;
   be_gfx_windowed_init,              // AL_METHOD(struct BITMAP *, init, (int w, int h, int v_w, int v_h, int color_depth));
   be_gfx_windowed_exit,              // AL_METHOD(void, exit, (struct BITMAP *b));
   NULL,                              // AL_METHOD(int, scroll, (int x, int y));
   be_gfx_windowed_vsync,             // AL_METHOD(void, vsync, (void));
   be_gfx_windowed_set_palette,       // AL_METHOD(void, set_palette, (struct RGB *p, int from, int to, int vsync));
   NULL,                              // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                              // AL_METHOD(int, poll_scroll, (void));
   NULL,                              // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,                              // AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, show_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, request_video_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(struct BITMAP *, create_system_bitmap, (int width, int height));
   NULL,                              // AL_METHOD(void, destroy_system_bitmap, (struct BITMAP *bitmap));
   NULL,                              // AL_METHOD(int, set_mouse_sprite, (struct BITMAP *sprite, int xfocus, int yfocus));
   NULL,                              // AL_METHOD(int, show_mouse, (struct BITMAP *bmp, int x, int y));
   NULL,                              // AL_METHOD(void, hide_mouse, (void));
   NULL,                              // AL_METHOD(void, move_mouse, (int x, int y));
   NULL,                              // AL_METHOD(void, drawing_mode, (void));
   NULL,                              // AL_METHOD(void, save_state, (void));
   NULL,                              // AL_METHOD(void, restore_state, (void));
   0, 0,                              // int w, h;  /* physical (not virtual!) screen size */
   TRUE,                              // int linear;  /* true if video memory is linear */
   0,                                 // long bank_size;  /* bank size, in bytes */
   0,                                 // long bank_gran;  /* bank granularity, in bytes */
   0,                                 // long vid_mem;  /* video memory size, in bytes */
   0,                                 // long vid_phys_base;  /* physical address of video memory */
   TRUE                               // int windowed;  /* true if driver runs windowed */
};
