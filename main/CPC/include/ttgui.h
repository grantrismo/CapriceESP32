////////////////////////////////////////////////////////
// (T)iny (T)ext (G)raphic (U)ser (I)nterface  - Library
////////////////////////////////////////////////////////

/*
ttgui_result = ttgui_init()
First all panels will be generated according to position, dimension, and layout
	- needs a low level draw_rectangle function
	- may go into a header structure

ttgui_result = ttgui_panel_manager(event_t event)

window manager will call on_access_caller function of the current panel
	on_access_caller will
		- generate the text object list,
		- display all text object,
		- set the focus of the first text object
		- will hand over to the text object manager
			* the text object manager will call the on_access_caller function of the text object on focus

Navigation
 Who owns the navigation?
	- The panel (local)
	- The window manager (global) *

Window manager knows the active panel and attaches the naviagtion to it
	- navigation is generic (without knowing any layout)
*/

#ifndef __TTGUI_H
#define __TTGUI_H

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "types.h"
#include "sections.h"
#include "vkb_cpcfont.h"
#include "gbuf.h"

#include "event.h"

#include "CPC.h"
#include "Files.h"
#include "Routines.h"

#define TTGUI_TX_LEFT 2					   // how many pixes between the text and it's boundary to the left
#define TTGUI_TX_TOP 2 					   // how many pixes between the text and it's boundary to the top
#define TTGUI_TX_LINEPITCH 9 			   // Font is 8x8, so (TTGUI_TX_LINEPITCH - 8) pixel(s) between text lines
#define TTGUI_NIL 0xFFFF

#define MemPtrNew(p) malloc(p)
#define MemPtrFree(p) free(p)

// OSD
typedef enum
{
	TTGUI_OSD_IDLE = 0,
	TTGUI_OSD_TIMER = 1,
	TTGUI_OSD_ONEHOT = 2,
	TTGUI_OSD_REGISTERED = 4,
	TTGUI_OSD_BLOCKKEYS = 8,
	TTGUI_OSD_TIMEROSD = 16,
	TTGUI_OSD_TERMINATE = 32,
	TTGUI_OSD_FCALL = 64,
} ttgui_osd_state;

// fonts
typedef enum
{
	TTGUI_FONT_BOLT2x3x1 = 0,
	TTGUI_FONT_BOLT3x3x1,
	TTGUI_FONT_BOLT2x2x0,
} ttgui_font_style;

// errors
typedef enum
{
	TTGUI_ERRNONE = 0,
	TTGUI_NOMEM,
	TTGUI_GENERIC,
	TTGUI_WRONGSIZE,
	TTGUI_NOTXOBJ,
	TTGUI_NOHDOBJ,
	TTGUI_NEEDGUIUPDATE,
	TTGUI_OSDBUSY,
} ttgui_err;

// modes
typedef enum
{
  TTGUI_REGULAR = 0,
  TTGUI_INVERSE
} ttgui_style;

typedef enum
{
	TTGUI_OT_PREFS = 0,
	TTGUI_OT_GAME,
	TTGUI_OT_BT,
	TTGUI_OT_AUDIO,
	TTGUI_OT_SAVESNAPSHOT,
	TTGUI_OT_LOADSNAPSHOT,
	TTGUI_OT_COLDRESET,
	// must be last!!
	NUM_TOTAL_OPTIONS,
} ttgui_ot_type;

typedef struct
{
	int lenght;
	char **selection;				// possible Selections
} ttgui_sl_obj;

// clipboard object
typedef struct
{
	uint16_t top;
	uint16_t left;
	uint16_t width;
	uint16_t height;
	uint8_t* chunk;
} ttgui_clip_obj;

// on screen display object
typedef struct
{
		ttgui_osd_state state;
		uint16_t timer_value;
		uint16_t tickms;
		const char* string_text;
		ttgui_font_style style;
		char string_value[8];
		char string_osd[20];
		//uint32_t next_tickms;
		ttgui_clip_obj* clip_obj;
		void (*on_update_caller)(void);
		void (*func_caller)(void);
} ttgui_osd_obj;

// option Object
typedef struct
{
	ttgui_ot_type type;   // type of object
	uint16_t current;					// current option option selected
	void (*on_select_caller)(void);			// on select function call
	char *text;								// the text perfix Visible
	ttgui_sl_obj options;							// the selection object
} ttgui_ot_obj;

//text Object:
typedef struct
{
	uint16_t tx_obj_id;						//the text object ID, list member number [0..num_tx_objs]
	uint16_t pn_obj_id;						//the parent panel ID, the text object belongs [0..num_pn_objs]
	char  *text;							//pointer to the text to be displayed
	void (*on_access_caller)(void);			// on access function call
  void (*on_leave_caller)(void);			// on leave function call
	void (*on_press_caller)(void);			// on press function call
	uint16_t left;  						// LEFT text object when pressing left
	uint16_t right;							// RIGHT text object when pressing right
	uint16_t above; 						// ABOVE text object when pressing up
	uint16_t below;							// BELOW text object when pressing down
} ttgui_tx_obj;

//panel Object:
typedef struct
{
	uint16_t num_tx_objs;					// number of text objects belonging to panel
	uint8_t left_pn_obj_index;  				// LEFT panel object when pressing B+left
	uint8_t right_pn_obj_index;				// RIGHT panel object when pressing B+right
	uint8_t above_pn_obj_index; 				// ABOVE panel object when pressing B+up
	uint8_t below_pn_obj_index;				// BELOW panel object when pressing B+down
	ttgui_tx_obj* tx_obj_base;				// pointer to the base text object
	uint16_t onfocus_tx_obj_index;			// index to the current text object holding focus inside the panel
	void (*on_access_caller)(void);			// on pannel access function call
  void (*on_leave_caller)(void);			// on pannel leave function call
	void (*on_update_caller)(void);
	char title[19];						// the title of the panel
	uint16_t top;							// dimension of the panel top coord
	uint16_t left;							// dimension of the panel top coord
	uint16_t width;							// dimension of the panel top coord
	uint16_t height;							// dimension of the panel top coord
	uint8_t tx_layout_nr;					// layout of the panel, how many text columns it supports
	uint8_t tx_layout_nc;					// layout of the panel, how many text rows it supports
	uint8_t tx_max_len;						// max text lenght supported by the panel
	uint16_t tx_page_n;						// page number in case panel holds more than (tx_layout_nr*tx_layout_nc) tx_objects
} ttgui_pn_obj;

//header Object:
typedef struct
{
	gbuf_t* frame_buffer;				// hold a ref to the frame buffer object
	uint8_t num_pn_objs;				// the number of available panels on screen
	const ttgui_pn_obj* setup_pn_obj;		// pointer to the panel oject array base settings
	ttgui_pn_obj* base_pn_obj;
	uint8_t onfocus_pn_obj_index;			// current panel object ID holding focus
	const uint8_t* font8x8;					// location of the font
	uint8_t bg_col_index;					// background color index
	uint8_t fg_col_index;					// foreground color index (also font index)
} ttgui_hd_obj;


extern ttgui_hd_obj* hd_obj;
extern ttgui_pn_obj* pn_obj;

extern void ttgui_open_game_cat();
extern void ttgui_open_settings_options();

//desing public caller functions prototypes
extern ttgui_err ttgui_PanelConstructor(gbuf_t* frame_buffer, uint16_t frame_width, uint16_t frame_height);
extern ttgui_err ttgui_updateFrameBuffer(gbuf_t* frame_buffer, uint16_t frame_width, uint16_t frame_height);
extern void ttgui_PanelDeConstructor();
extern ttgui_err ttgui_windowManager(event_t* Event);
extern ttgui_err ttgui_osdManager(event_t* Event);
extern void ttgui_osdExecute();
extern ttgui_err ttgui_osdString(const char* String, uint16_t tickms, uint8_t IsMenu);
extern ttgui_err ttgui_osdRegister(const char* String, uint16_t tickms);
extern ttgui_err ttgui_osdMenu(const char* String, uint16_t tickms);
extern ttgui_osd_state ttgui_getOsdState();
extern void ttgui_osdStop();
extern void ttgui_osdRestore();
extern void ttgui_osdTermTimer();
extern void ttgui_setFgColorIndex(uint8_t color);
extern void ttgui_setBgColorIndex(uint8_t color);
extern void ttgui_setup();

#endif //__TTGUI_H
