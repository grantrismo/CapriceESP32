////////////////////////////////////////////////////////
// (T)iny (T)ext (G)raphic (U)ser (I)nterface  - Library
////////////////////////////////////////////////////////

#include "types.h"
#include "sections.h"
#include "trace.h"

#include "Keyboard.h"
#include "ttgui.h"
#include "cheat.h"

#define NUM_TOTAL_PANELS 					(3) // for general allocation
#define NUM_MENU_PANELS						(3) // part of the context setting menu
#define NUM_TOTAL_OPTIONS         (2) // number of total options/preferences to choose from

#define PANEL_GAME_ID 						(0)
#define PANEL_AUTOSTART_ID 				(1)
#define PANEL_SETTINGS_ID 				(2)

#undef DUAL_BUFFER_CONFIG
//#define DUAL_BUFFER_CONFIG

const ttgui_pn_obj pn_obj_setup[NUM_TOTAL_PANELS] = {
	{0,0,1,0,2,NULL,0,NULL,NULL,NULL,"Select Game"        , 0  ,0  ,159,119,12,1,19,0}, // SDCARD selector pannel
	{0,0,1,0,2,NULL,0,NULL,NULL,NULL,"Select Autostart"   , 0  ,161,159,119,12,1,19,0}, // CAT selector pannel
	{0,0,1,0,2,NULL,0,NULL,NULL,NULL,"Preferences (V0.71)", 121,0  ,320,119,12,2,19,0}, // SETTINGS selector panel
};

// caller prototypes
static void ttgui_otobj_change_cheat();
static void ttgui_otobj_change_sound();

// Option Test, Possible Selections, inital selection index, max number of selections
const ttgui_ot_obj ot_obj_setup[NUM_TOTAL_OPTIONS] = {
	{TTGUI_OT_GAME, 0, ttgui_otobj_change_cheat, "Cheat: ", {2, (char*[]){"off","on"}}},
	{TTGUI_OT_PREFS, 0, ttgui_otobj_change_sound, "Sound: ", {2, (char*[]){"on","off"}}},
};

// globals
ttgui_hd_obj* hd_obj = NULL;
ttgui_pn_obj* pn_obj = NULL;
ttgui_tx_obj* tx_obj = NULL;
ttgui_ot_obj* ot_obj = NULL;

static uint32_t oldMenuKeyState = 0;
static char** currentDirFileList = NULL;
static char*  currentCatFileList = NULL;
static char*  currentCatFilename = NULL;
static uint16_t currentDirFileCount = 0;
static char curr_dir[256] = "../games";
static char next_dir[256] = "\0";


// prototypes
static void ttgui_createPanel();
static ttgui_err ttgui_appendTxObj(const char* text,const void* func_call);
static void ttgui_updateTxObj(ttgui_style color_style);
static void ttgui_drawPanelText();
static void ttgui_drawPanelRect();
static void ttgui_fillPanelRect();
static void ttgui_clearPanelArea();
static void ttgui_clearBg();

// on_event_callers
static void ttgui_open_game_dir();
static void ttgui_update_game_dir();

static void ttgui_txobj_lose_focus();
static void ttgui_txobj_get_focus();
static void ttgui_txobj_load_cat();
static void ttgui_txobj_cpc_autostart();

static void ttgui_otobj_options_setup();
static void ttgui_otobj_update_txobj();
static void ttgui_otobj_reset_game();

// helpers
static int ttgui_files_get(const char* path, const char* extension, char*** filesOut);
static int ttgui_get_all_filenames();
static void ttgui_files_free(char** files, int count);

// Basic GFX functions
static void ttgui_drawHLine(uint16_t x1, uint16_t y1, uint16_t x2);
static void ttgui_drawVLine(uint16_t x1, uint16_t y1, uint16_t y2);
static void ttgui_fillRect(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, uint8_t inv);
static void ttgui_drawChars(const char* String, uint16_t Left, uint16_t Top, uint8_t Inverse);


ttgui_err ttgui_PanelConstructor(gbuf_t* frame_buffer, uint16_t frame_width, uint16_t frame_height)
/**********************************************
*
*	ttgui_panel_constructor
*
**********************************************/
{
	// alloc header structure
	if (hd_obj == NULL)
	{
		hd_obj = (ttgui_hd_obj*)MemPtrNew(sizeof(ttgui_hd_obj));
		if (hd_obj == NULL)
		{
			printf("Header Issue\n");
			return (TTGUI_NOMEM);
		}
	}

	// alloc the full panel structure
	ttgui_pn_obj* pn_obj_base = (ttgui_pn_obj*)MemPtrNew(sizeof(ttgui_pn_obj) * NUM_TOTAL_PANELS);
	if (pn_obj_base == NULL)
	{
		MemPtrFree(hd_obj);
		printf("Base Issue\n");
		return (TTGUI_NOMEM);
	}

	// copy the setup into the base memory
	memcpy(pn_obj_base,pn_obj_setup,sizeof(ttgui_pn_obj) * NUM_TOTAL_PANELS);

	// fill header structure
	hd_obj->frame_buffer = frame_buffer;
	hd_obj->setup_pn_obj = pn_obj_setup;
	hd_obj->num_pn_objs = NUM_TOTAL_PANELS;
	hd_obj->base_pn_obj = pn_obj_base;
	hd_obj->onfocus_pn_obj_index = 0;
	hd_obj->font8x8 = &CPC_KEYBOARD_FONT[0][0];

	// allocate all possible tx_objects
	for (int i=0;i<NUM_TOTAL_PANELS; i++)
	{
		pn_obj = &hd_obj->base_pn_obj[i];
		pn_obj->tx_obj_base = (ttgui_tx_obj*)MemPtrNew(sizeof(ttgui_tx_obj) * pn_obj->tx_layout_nc * pn_obj->tx_layout_nr);
		if (pn_obj->tx_obj_base == NULL)
			return(TTGUI_NOMEM);
		memset(pn_obj->tx_obj_base,0,sizeof(ttgui_tx_obj) * pn_obj->tx_layout_nc * pn_obj->tx_layout_nr);
	}

	// setup the options
	ot_obj = (ttgui_ot_obj*)MemPtrNew(sizeof(ttgui_ot_obj) * NUM_TOTAL_OPTIONS);
	memcpy(ot_obj, ot_obj_setup, sizeof(ttgui_ot_obj) * NUM_TOTAL_OPTIONS);

	// fill the caller functions
	pn_obj_base[PANEL_GAME_ID].on_access_caller = ttgui_open_game_dir; // setup the sdcard dir structure
	pn_obj_base[PANEL_GAME_ID].on_update_caller = ttgui_update_game_dir; // setup the sdcard dir structure
	pn_obj_base[PANEL_SETTINGS_ID].on_access_caller = ttgui_otobj_options_setup; // setup the option configuration

	return (TTGUI_ERRNONE);
}

ttgui_err ttgui_updateFrameBuffer(gbuf_t* frame_buffer, uint16_t frame_width, uint16_t frame_height)
{
		if (hd_obj == NULL)
			return (TTGUI_NOHDOBJ);

		hd_obj->frame_buffer = frame_buffer;
		return (TTGUI_ERRNONE);
}

void ttgui_setup()
{
	// clear the full background of the GUI
	ttgui_clearBg();

	// setup the basic outline of all the panes on the display
	// draw the panel(s) outline and title
	for (int i=0;i<NUM_MENU_PANELS; i++)
	{
		pn_obj = &hd_obj->base_pn_obj[i];
		pn_obj->num_tx_objs = 0;

		ttgui_createPanel();

		if (pn_obj -> on_access_caller != NULL)
			pn_obj -> on_access_caller();
	}

	// call the pannel on focus function
	hd_obj->onfocus_pn_obj_index = 0;
	pn_obj = &hd_obj->base_pn_obj[hd_obj->onfocus_pn_obj_index];

	if (tx_obj != NULL && pn_obj->num_tx_objs>0)
	{
		if (tx_obj -> on_access_caller != NULL)
			tx_obj -> on_access_caller();
	}

	printf("panel menu done\n");
}

void ttgui_PanelDeConstructor()
{
	// free the Memory

}

ttgui_err ttgui_windowManager(event_t* event)
{

	uint32_t keyDiff, keyState;

	keyState = KeyCurrentState();
  keyDiff = keyState ^ oldMenuKeyState;
  if (!keyDiff)
    return(TTGUI_ERRNONE);

  // Save key states
  oldMenuKeyState = keyState;

	//printf("ttgui_windowManager: %d, %d, %p\n",keyState, keyDiff, tx_obj);


// Panel Navigation
	if((keyState & keyBitRockerNextPanelLeft) == keyBitRockerNextPanelLeft )
	{
		// Move to panel to the left_pn_obj_index
		printf("Panel go left!\n");

		ttgui_pn_obj* tmp_pn_obj = &hd_obj->base_pn_obj[pn_obj->left_pn_obj_index];
		// Do we have objects inside the panel object?
		if (tmp_pn_obj->num_tx_objs)
		{
			tx_obj -> on_leave_caller();
			hd_obj -> onfocus_pn_obj_index = pn_obj->left_pn_obj_index;
			pn_obj = tmp_pn_obj;
			tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
			tx_obj -> on_access_caller();
		}
		return(TTGUI_NEEDGUIUPDATE);
	}

	if((keyState & keyBitRockerNextPanelRight) == keyBitRockerNextPanelRight )
	{
		printf("Panel go right!\n");
		// Move to panel to the right_pn_obj_index
		ttgui_pn_obj* tmp_pn_obj = &hd_obj->base_pn_obj[pn_obj->right_pn_obj_index];
		// Do we have objects inside the panel object?
		if (tmp_pn_obj->num_tx_objs)
		{
			tx_obj -> on_leave_caller();
			hd_obj -> onfocus_pn_obj_index = pn_obj->right_pn_obj_index;
			pn_obj = tmp_pn_obj;
			tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
			tx_obj -> on_access_caller();
		}
		return(TTGUI_NEEDGUIUPDATE);
	}

	if((keyState & keyBitRockerNextPanelUp) == keyBitRockerNextPanelUp )
	{
		printf("Panel go up!\n");
		// Move to panel to the above_pn_obj_index
		ttgui_pn_obj* tmp_pn_obj = &hd_obj->base_pn_obj[pn_obj->above_pn_obj_index];
		// Do we have objects inside the panel object?
		if (tmp_pn_obj->num_tx_objs)
		{
			tx_obj -> on_leave_caller();
			hd_obj -> onfocus_pn_obj_index = pn_obj->above_pn_obj_index;
			pn_obj = tmp_pn_obj;
			tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
			tx_obj -> on_access_caller();
		}
		return(TTGUI_NEEDGUIUPDATE);
	}

	if((keyState & keyBitRockerNextPanelDown) == keyBitRockerNextPanelDown )
	{
		printf("Panel go down!\n");
		// Move to panel to the below_pn_obj_index
		ttgui_pn_obj* tmp_pn_obj = &hd_obj->base_pn_obj[pn_obj->below_pn_obj_index];
		// Do we have objects inside the panel object?
		if (tmp_pn_obj->num_tx_objs)
		{
			tx_obj -> on_leave_caller();
			hd_obj -> onfocus_pn_obj_index = pn_obj->below_pn_obj_index;
			pn_obj = tmp_pn_obj;
			tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
			tx_obj -> on_access_caller();
		}
		return(TTGUI_NEEDGUIUPDATE);
	}

	// Navigation inside panel
	if (tx_obj == NULL)
		return(TTGUI_NOTXOBJ);

	if (keyDiff & (keyBitRockerUp))
	{
		if (keyState & (keyBitRockerUp))
		{
			if (tx_obj-> above != TTGUI_NIL)
			{
				tx_obj -> on_leave_caller();
				pn_obj-> onfocus_tx_obj_index = tx_obj -> above;
				tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
				tx_obj -> on_access_caller();
			}
			else
			{
				// page up support
				if (tx_obj->on_leave_caller != NULL)
					tx_obj -> on_leave_caller();

				if (pn_obj -> on_update_caller != NULL)
					pn_obj -> on_update_caller();

				if (tx_obj -> on_access_caller != NULL)
					tx_obj -> on_access_caller();
			}
		}
		else
		{
			// released
		}
	}

	if (keyDiff & (keyBitRockerDown))
	{
		if (keyState & (keyBitRockerDown))
		{
			if (tx_obj-> below != TTGUI_NIL)
			{
				tx_obj -> on_leave_caller();
				pn_obj-> onfocus_tx_obj_index = tx_obj -> below;
				tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
				tx_obj -> on_access_caller();
			}
			else
			{
				// page up support
				if (tx_obj->on_leave_caller != NULL)
					tx_obj -> on_leave_caller();

				if (pn_obj -> on_update_caller != NULL)
					pn_obj -> on_update_caller();

				if (tx_obj -> on_access_caller != NULL)
					tx_obj -> on_access_caller();
			}
		}
		else
		{
			// released
		}
	}

	// Button A
	if (keyDiff & (keyBitRockerButtonA))
	{
		if (keyState & (keyBitRockerButtonA))
		{
			if (tx_obj -> on_press_caller != NULL)
				tx_obj -> on_press_caller();
		}
		else
		{
			// released
		}
	}

	return(TTGUI_NEEDGUIUPDATE);
}

static void ttgui_createPanel()
{
	// background of the panel
	ttgui_fillPanelRect();

	// foreground of the panel
	ttgui_drawPanelRect();

	// titel of the panel
	ttgui_drawPanelText();
}

/**********************************************
*
*	ttgui_helpers
*
**********************************************/

 static void ttgui_drawHLine(uint16_t x1, uint16_t y1, uint16_t x2)
/**********************************************
*
*	ttgui_drawHline
*
**********************************************/
{
	uint16_t m_h = DISPLAY_HEIGHT;
	uint16_t m_w = hd_obj->frame_buffer->width;

    if (y1 >= m_h) return;
    if (x2 < x1) x1 = 0;
    if (x1 >= m_w) return;
    if (x2 >= m_w) x2 = m_w - 1;

	uint32_t offset = y1 * m_w + x1;
	uint8_t* m_bytes = hd_obj->frame_buffer->data[0] + offset;

    for(uint16_t x = x1; x<=x2; x++, offset++)
		{
#ifdef DUAL_BUFFER_CONFIG
			if (offset >= hd_obj->frame_buffer->block_len)
				m_bytes = hd_obj->frame_buffer->data[1] - hd_obj->frame_buffer->block_len + offset;
			else
#endif
				m_bytes = hd_obj->frame_buffer->data[0] + offset;

			*m_bytes = hd_obj->fg_col_index;
		}
};

static void ttgui_drawVLine(uint16_t x1, uint16_t y1, uint16_t y2)
/**********************************************
*
*	ttgui_drawVLine
*
**********************************************/
{
	uint16_t m_h = DISPLAY_HEIGHT;
	uint16_t m_w = hd_obj->frame_buffer->width;
	uint8_t* m_bytes = NULL;

  if (x1 >= m_w) return;
  if (y2 < y1) y1 = 0;
  if (y1 >= m_h) return;
  if (y2 >= m_h) y2 = m_h - 1;

	uint32_t offset = y1 * m_w + x1;
  for(uint16_t y = y1; y<=y2; y++, offset+=m_w)
	{
#ifdef DUAL_BUFFER_CONFIG
		if (offset >= hd_obj->frame_buffer->block_len)
			m_bytes = hd_obj->frame_buffer->data[1] - hd_obj->frame_buffer->block_len + offset;
		else
#endif
			m_bytes = hd_obj->frame_buffer->data[0] + offset;

		*m_bytes = hd_obj->fg_col_index;
	}
}

static void ttgui_drawPanelRect()
/**********************************************
*
*	ttgui_drawPanelRect
*
**********************************************/
{
	uint16_t x1 = pn_obj->left;
  uint16_t y1 = pn_obj->top;
  uint16_t x2 = pn_obj->left + pn_obj->width - 1;
  uint16_t y2 = pn_obj->top + pn_obj->height - 1;

  ttgui_drawHLine(x1, y1, x2);
  ttgui_drawHLine(x1, y2, x2);
  ttgui_drawVLine(x1, y1, y2);
  ttgui_drawVLine(x2, y1, y2);
};

static void ttgui_fillPanelRect()
/**********************************************
*
*	ttgui_fillPanelRect
*
**********************************************/
{
	uint16_t x1 = pn_obj->left;
  uint16_t y1 = pn_obj->top;
  uint16_t x2 = pn_obj->left + pn_obj->width - 1;
  uint16_t y2 = pn_obj->top + pn_obj->height - 1;

	ttgui_fillRect(x1, y1, x2, y2, 0);

};

static void ttgui_clearPanelArea()
/**********************************************
*
*	ttgui_clearPanelArea
*
**********************************************/
{
	uint16_t x1 = pn_obj->left + 1;
  uint16_t y1 = pn_obj->top + 8;
  uint16_t x2 = pn_obj->left + pn_obj->width - 3;
  uint16_t y2 = pn_obj->top + pn_obj->height - 3;

	ttgui_fillRect(x1, y1, x2, y2, 0);

};

static void ttgui_fillRect(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, uint8_t inv)
/**********************************************
*
*	ttgui_fillRect
*
**********************************************/
{
	uint16_t m_h = DISPLAY_HEIGHT;
	uint16_t m_w = hd_obj->frame_buffer->width;
	uint32_t offset = 0;
	uint8_t* m_bytes = 0;

	if ((x1 < x2) && (x1 >= m_w)) return;
	if ((y1 < y2) && (y1 >= m_h)) return;
	if (x1 > x2) x1 = 0;
	if (y1 > y2) y1 = 0;
	if (x2 >= m_w) x2 = m_w -1;
	if (y2 >= m_h) y2 = m_h -1;

	for(int y = y1; y<=y2; y++)
	{
		offset =  y * m_w + x1;

		for(int x = x1; x<=x2; x++, offset++)
		{
#ifdef DUAL_BUFFER_CONFIG
			if (offset >= hd_obj->frame_buffer->block_len)
				m_bytes = hd_obj->frame_buffer->data[1] - hd_obj->frame_buffer->block_len + offset;
			else
#endif
				m_bytes = hd_obj->frame_buffer->data[0] + offset;

			*m_bytes = (inv == 0) ? hd_obj-> bg_col_index : hd_obj-> fg_col_index;
		}

	}
}

void ttgui_setFgColorIndex(uint8_t color)
/**********************************************
*
*	ttgui_setFgColorIndex
*
**********************************************/
{
	hd_obj-> fg_col_index = color;
}

void ttgui_setBgColorIndex(uint8_t color)
/**********************************************
*
*	ttgui_setBgColorIndex
*
**********************************************/
{
	hd_obj-> bg_col_index = color;
}


static void ttgui_clearBg()
/**********************************************
*
*	ttgui_clearBg
*
**********************************************/
{

	uint16_t m_h = DISPLAY_HEIGHT;
	uint16_t m_w = hd_obj->frame_buffer->width;
	uint16_t x1 = 0;
	uint16_t y1 = 0;
	uint16_t x2 = m_w - 1;
	uint16_t y2 = m_h - 1;
	uint32_t offset = 0;

	for(int y = y1; y<=y2; y++)
	{
		offset = y * m_w + x1;
		uint8_t* m_bytes = hd_obj->frame_buffer->data[0] + offset;

		for(int x = x1; x<=x2; x++, offset++)
		{
#ifdef DUAL_BUFFER_CONFIG
			if (offset >= hd_obj->frame_buffer->block_len)
				m_bytes = hd_obj->frame_buffer->data[1] - hd_obj->frame_buffer->block_len + offset;
			else
#endif
				m_bytes = hd_obj->frame_buffer->data[0] + offset;

			*m_bytes++ = hd_obj-> bg_col_index;
		}
	}
}

static void ttgui_drawPanelText()
/**********************************************
*
*	ttgui_ttgui_drawPanelText
*
**********************************************/
{
	if (pn_obj->title[0] == '\0')
		return;

	uint16_t top = pn_obj->top + 1;
	uint16_t left = pn_obj->left + TTGUI_TX_LEFT;
	ttgui_fillRect(pn_obj->left+1,top, left-1, pn_obj->top+CPC_KEYBOARD_FONT_HEIGHT+1, 1);
	ttgui_drawChars(pn_obj->title, left, top, 1);
	left += strlen(pn_obj->title)*CPC_KEYBOARD_FONT_WIDTH;
	ttgui_fillRect(left, top, pn_obj->left+pn_obj->width-1, pn_obj->top+CPC_KEYBOARD_FONT_HEIGHT, 1);
}

static void ttgui_drawChars(const char* String, uint16_t Left, uint16_t Top, uint8_t Inverse)
/***********************************************************************
 *
 *  drawChars
 *
 ***********************************************************************/
{
  uint8_t c_string = 0;
	uint8_t c_bittest;
	uint16_t m_w = hd_obj->frame_buffer->width;

  for(int j=0; j<strlen(String); j++) {
    c_string = (uint8_t)String[j];
    if (c_string<CPC_KEYBOARD_FONT_FIRST_CHAR)
      return;

		uint32_t offset = Top*m_w + j*CPC_KEYBOARD_FONT_WIDTH + Left;
		uint8_t* sc_buffer = hd_obj->frame_buffer->data[0] + offset;

    for(int i=0;i<CPC_KEYBOARD_FONT_WIDTH; i++, offset-=(CPC_KEYBOARD_FONT_HEIGHT*m_w - 1)) {
      char charColumn = *(hd_obj->font8x8 + (c_string-CPC_KEYBOARD_FONT_FIRST_CHAR)*CPC_KEYBOARD_FONT_WIDTH + i);
      for (int pos=0;pos<CPC_KEYBOARD_FONT_HEIGHT;pos++,offset+=m_w) {

				// get the correct frame buffer
#ifdef DUAL_BUFFER_CONFIG
				if (offset >= hd_obj->frame_buffer->block_len)
					sc_buffer = hd_obj->frame_buffer->data[1] - hd_obj->frame_buffer->block_len + offset;
				else
#endif
					sc_buffer = hd_obj->frame_buffer->data[0] + offset;


				// set the pixel
				c_bittest = ((1<<pos) & charColumn) != 0;
        if ( (c_bittest & (!Inverse)) || ((!c_bittest) & Inverse) )
          *sc_buffer = hd_obj->fg_col_index;
        else
          *sc_buffer = hd_obj->bg_col_index;
      }
    }
  }
}

static void ttgui_setupPnOnAccessCallee(const void* func)
{
		pn_obj -> on_access_caller = func;
}

static ttgui_err ttgui_appendTxObj(const char* text, const void* func_call)
/***********************************************************************
 *
 *  ttgui_appendTxObj
 *
 ***********************************************************************/
{
	// allow only a ringbuffer of tx_objects by now
	printf("append TxObj: %s:%d\n",text,pn_obj->num_tx_objs);

	ttgui_tx_obj* tmp_tx_obj = tx_obj;

	uint16_t a_txobj_i = pn_obj->num_tx_objs % (pn_obj->tx_layout_nc * pn_obj->tx_layout_nr);
	uint16_t l_txobj_i = a_txobj_i - 1;

	// pointer to new allocated tx_object
	tx_obj = &pn_obj->tx_obj_base[a_txobj_i];

	// old text object de-alloc and new text allocate and copy
	if (tx_obj->text != NULL)
		MemPtrFree(tx_obj->text);

	tx_obj->text = (char*)MemPtrNew(pn_obj->tx_max_len+1);
	if (tx_obj->text==NULL)
	{
		tx_obj = tmp_tx_obj;
		return TTGUI_NOMEM;
	}

	memset(tx_obj->text,'\0',pn_obj->tx_max_len+1);
	strncpy(tx_obj->text,text,pn_obj->tx_max_len);

	//TODO // link todo matrix like horizontally
	tx_obj->left = 0;
	tx_obj->right = 0;

	// link vertically
	if (!a_txobj_i)
	{
		tx_obj->above = TTGUI_NIL;
	}
	else
	{
		pn_obj->tx_obj_base[l_txobj_i].below = a_txobj_i;
		tx_obj->above = l_txobj_i;
	}

	// fill new tx_objects
	tx_obj->below = TTGUI_NIL;
	tx_obj->pn_obj_id = hd_obj->onfocus_pn_obj_index;
	tx_obj->tx_obj_id = a_txobj_i;
	tx_obj->on_access_caller = ttgui_txobj_get_focus;
	tx_obj->on_leave_caller = ttgui_txobj_lose_focus;
	tx_obj->on_press_caller = func_call;

	// show me
	ttgui_txobj_lose_focus();

	// ok, we have a new tx_object member in the current panel object
	pn_obj->num_tx_objs++;
	tx_obj = tmp_tx_obj;
	return (TTGUI_ERRNONE);
}

///                  CALLERS

static void ttgui_open_game_dir()
/***********************************************************************
 *
 *  ttgui_open_game_dir
 *
 ***********************************************************************/
 {
	 const char ext_dsk[] = {".dsk"};
	 int max_tx_count = (pn_obj->tx_layout_nc*pn_obj->tx_layout_nr);

	 // clear background
	 ttgui_createPanel();

	 if (strcmp(curr_dir,next_dir)!=0)
	 {

		 // construct full path
		 strcat(next_dir, DEFAULT_DISK_PATH);

		 // load relevant files to Buffer
		 currentDirFileCount = ttgui_files_get(next_dir, ext_dsk, &currentDirFileList);
		 strcpy(curr_dir, next_dir);
		 printf("Total items loaded %d on path: %s\n",currentDirFileCount, next_dir);
		 pn_obj->tx_page_n = 0;
	 }

	 // reset the nuber of objects
	 pn_obj->num_tx_objs = 0;

	 // move files
	 for (int i=(pn_obj->tx_page_n*max_tx_count);i<currentDirFileCount;i++)
	 {
		 ttgui_appendTxObj(currentDirFileList[i], ttgui_txobj_load_cat);
		 if (pn_obj->num_tx_objs >= max_tx_count)
		  	break;
	 }

	 // set current tx-object focus
	 if (pn_obj->num_tx_objs)
	 {
		 pn_obj-> onfocus_tx_obj_index = 0;
		 tx_obj = &pn_obj->tx_obj_base[pn_obj-> onfocus_tx_obj_index];
	 }

 }


static void ttgui_txobj_get_focus()
/***********************************************************************
 *
 *  ttgui_txobj_get_focus
 *
 ***********************************************************************/
 {
	 uint16_t top = (pn_obj->top + CPC_KEYBOARD_FONT_HEIGHT + 1) + TTGUI_TX_TOP + (tx_obj->tx_obj_id * TTGUI_TX_LINEPITCH);
	 uint16_t left = pn_obj->left + TTGUI_TX_LEFT;
	 ttgui_drawChars(tx_obj->text, left, top, 1);

	 left += strlen(tx_obj->text)*CPC_KEYBOARD_FONT_WIDTH;
	 ttgui_fillRect(left, top, pn_obj->left+pn_obj->width-2, top + CPC_KEYBOARD_FONT_HEIGHT - 1, 0);

 }

static void ttgui_txobj_lose_focus()
 /***********************************************************************
  *
  *  ttgui_txobj_lose_focus
  *
  ***********************************************************************/
  {
		uint16_t top = (pn_obj->top + CPC_KEYBOARD_FONT_HEIGHT + 1) + TTGUI_TX_TOP + (tx_obj->tx_obj_id * TTGUI_TX_LINEPITCH);
		uint16_t left = pn_obj->left + TTGUI_TX_LEFT;
		ttgui_drawChars(tx_obj->text, left, top, 0);

		left += strlen(tx_obj->text)*CPC_KEYBOARD_FONT_WIDTH;
		ttgui_fillRect(left, top, pn_obj->left+pn_obj->width-2, top + CPC_KEYBOARD_FONT_HEIGHT - 1, 0);
  }

static void ttgui_update_game_dir()
/***********************************************************************
*
* 	 ttgui_update_game_dir
*
***********************************************************************/
{
	// top end reached?
	if ((tx_obj->above == TTGUI_NIL) && (pn_obj->tx_page_n == 0))
		return;

	// only one singe tx_obj available?
	if ((tx_obj->above == TTGUI_NIL) && (tx_obj->below == TTGUI_NIL) && (pn_obj->tx_page_n == 0))
		return;


	if (tx_obj->above == TTGUI_NIL)
	{
		pn_obj->tx_page_n--;
		ttgui_open_game_dir();
		return;
	}

	// bottom end reached?
	if((pn_obj->tx_page_n+1)*pn_obj->tx_layout_nr*pn_obj->tx_layout_nc >= currentDirFileCount)
		return;

	if (tx_obj->below == TTGUI_NIL)
	{
		pn_obj->tx_page_n++;
		ttgui_open_game_dir();
	}

}

inline static void swap(char** a, char** b)
/***********************************************************************
*
* 	 swap
*
***********************************************************************/
{
    char* t = *a;
    *a = *b;
    *b = t;
}

static int strcicmp(char const *a, char const *b)
/***********************************************************************
*
* 	 strcicmp
*
***********************************************************************/
{
    for (;; a++, b++)
    {
				char ai=*a>='A'&&*a<='Z'?*a|0x60:*a;
				char bi=*b>='A'&&*b<='Z'?*b|0x60:*b;
        int d = ((int)ai - (int)bi);
        if (d != 0 || !*a) return d;
    }
}

static int partition (char* arr[], int low, int high)
/***********************************************************************
*
* 	 partition
*
***********************************************************************/
{
    char* pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high- 1; j++)
    {
        if (strcicmp(arr[j], pivot) < 0)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

static void quick_sort(char* arr[], int low, int high)
/***********************************************************************
*
* 	 quicksort
*
***********************************************************************/
{
    if (low < high)
    {
        int pi = partition(arr, low, high);

        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

static void sort_files(char** files, int count)
/***********************************************************************
*
* 	 sort files
*
***********************************************************************/
{
    bool swapped = true;

    if (count > 1)
    {
        quick_sort(files, 0, count - 1);
    }
}



static int ttgui_files_get(const char* path, const char* extension, char*** filesOut)
/***********************************************************************
*
* 	 ttgui_files_get
*
***********************************************************************/
{
    const int MAX_FILES = 1024;

    int count = 0;
    char** result = (char**)malloc(MAX_FILES * sizeof(void*));
    if (!result) abort();


    DIR *dir = opendir(path);
    if( dir == NULL )
    {
        printf("opendir failed on %s.\n",path);
        return 0;
    }

    int extensionLength = strlen(extension);
    if (extensionLength < 1)
			return 0;

    char* temp = (char*)malloc(extensionLength + 1);
    if (!temp)
			return 0;

    memset(temp, 0, extensionLength + 1);


    struct dirent *entry;
    while((entry=readdir(dir)) != NULL)
    {
        size_t len = strlen(entry->d_name);
				printf("read entry %s\n", entry->d_name);

        // ignore 'hidden' files (MAC)
        bool skip = false;
        if (strcmp(entry->d_name,".") == 0)
					skip = true;


        memset(temp, 0, extensionLength + 1);
        if (!skip)
        {
						if(entry->d_type == DT_DIR)
						{
							// it is a directory
							result[count] = (char*)malloc(len + 2);
							if (!result[count])
							{
									abort();
							}

							*result[count] = CPC_DIR_CHAR;
							strcpy(result[count]+1, entry->d_name);
							++count;
						}
						else
						{
							// it is a file -> check for correct extension
	            for (int i = 0; i < extensionLength; ++i)
	            {
									char t_ch = entry->d_name[len - extensionLength + i];
	                temp[i] = ((t_ch >= 'A') && (t_ch <= 'Z')) ? t_ch | 0x60 : t_ch;
	            }

	            if (len > extensionLength)
	            {
	                if (strcmp(temp, extension) == 0)
	                {
	                    result[count] = (char*)malloc(len + 2);
	                    //printf("%s: allocated %p\n", __func__, result[count]);

	                    if (!result[count])
	                    {
	                        abort();
	                    }
											*result[count] = CPC_FILE_CHAR;
	                    strcpy(result[count]+1, entry->d_name);
	                    ++count;

	                    if (count >= MAX_FILES) break;
	                }
	            }
						}
        }
    }

    closedir(dir);
    free(temp);

    sort_files(result, count);

    *filesOut = result;
    return count;
}

void ttgui_files_free(char** files, int count)
/***********************************************************************
*
* 	 ttgui_files_free
*
***********************************************************************/
{
    for (int i = 0; i < count; ++i)
    {
        //printf("%s: freeing item %p\n", __func__, files[i]);
        free(files[i]);
    }

    //printf("%s: freeing array %p\n", __func__, files);
    free(files);
}

void ttgui_txobj_load_cat()
/***********************************************************************
*
* 	 ttgui_files_free
*
***********************************************************************/
{
	uint16_t file_i = pn_obj->tx_page_n*pn_obj->tx_layout_nr*pn_obj->tx_layout_nc + tx_obj->tx_obj_id;
	char *fileref = currentDirFileList[file_i];
	UInt16 num_cat_entries;
	ttgui_pn_obj* old_pn_obj = NULL;
	int max_tx_count = 0;
	char *cat_dir = NULL;

	if(currentCatFilename != NULL)
		free(currentCatFilename);
	currentCatFilename = (char*)malloc(strlen(fileref) + 1);
	strcpy(currentCatFilename, fileref);
	currentCatFilename[0]='/';

	printf("On Press Caller\n");
	printf("Path: %s\n", curr_dir);
	printf("FilenameP: %s\n", currentCatFilename);

	num_cat_entries = CPCLoadDiskAndGetCatalog(curr_dir, currentCatFilename, &currentCatFileList);
	printf("Num CAT files: %d\n", num_cat_entries);

	if (num_cat_entries)
	{
		// fill the autostart panel
		cat_dir = currentCatFileList;
		old_pn_obj = pn_obj;
		// move pointer to the autostart panel
		pn_obj = &hd_obj->base_pn_obj[PANEL_AUTOSTART_ID];
		ttgui_clearPanelArea();
		max_tx_count = (pn_obj->tx_layout_nc*pn_obj->tx_layout_nr);

		// reset the nuber of objects
		pn_obj->num_tx_objs = 0;

		// move files
		for (int i=(pn_obj->tx_page_n*max_tx_count);i<num_cat_entries;i++)
		{
			ttgui_appendTxObj(cat_dir, ttgui_txobj_cpc_autostart);
			cat_dir += CPC_FILE_LENGTH;
			if (pn_obj->num_tx_objs >= max_tx_count)
				 break;
		}

		// set current tx-object focus
		pn_obj-> onfocus_tx_obj_index = 0;

		// rest the the old panel
		pn_obj = old_pn_obj;
	}
}

void ttgui_txobj_cpc_autostart(void)
/***********************************************************************
*
* 	 ttgui_txobj_cpc_autostart
*
***********************************************************************/
{
 char *firstValidEntryP = tx_obj->text;

	// Prepare command to execute
  int nbCharOfCommand = 0;
	tDrive* driveP = NULL;

	if (*(firstValidEntryP) != '|')
	{
	  AutoStartCommand[nbCharOfCommand++] = 'R';
	  AutoStartCommand[nbCharOfCommand++] = 'U';
	  AutoStartCommand[nbCharOfCommand++] = 'N';
	  AutoStartCommand[nbCharOfCommand++] = '\"';
	}


  // Add filename
  for (int loop=0; loop <= 11; loop++)
  {
		// stop on EOL
		if (*(firstValidEntryP+loop)=='\0')
			break;

  	// Do not take spaces into account
  	if (*(firstValidEntryP+loop) != 0x20)
  	{
  	  AutoStartCommand[nbCharOfCommand++] = *(firstValidEntryP+loop);
  	}
  }

	AutoStartCommand[nbCharOfCommand++] = '\n'; // RETURN
	AutoStartCommand[nbCharOfCommand] = '\0';

	printf ("Autostart command: %s", AutoStartCommand);

	// engine Reset
	CPCResetWithConfirmation();

	// load the disc Image
	CPCLoadDiskImage(curr_dir, currentCatFilename);

	// reset all related game options
	ttgui_otobj_reset_game();
}

void ttgui_otobj_options_setup(void)
/***********************************************************************
*
* 	 ttgui_otobj_options_setup
*
***********************************************************************/
{
	char buf[20]={0};
	int index;
	ttgui_pn_obj* old_pn_obj = pn_obj;

	// move pointer to the autostart panel
	pn_obj = &hd_obj->base_pn_obj[PANEL_SETTINGS_ID];

	for (int i=0;i<NUM_TOTAL_OPTIONS;i++)
	{
		index = ot_obj[i].current;
		strcpy(buf,ot_obj[i].text);
		strcat(buf,ot_obj[i].options.selection[index]);
		ttgui_appendTxObj(buf, ot_obj[i].on_select_caller);

	}
	pn_obj = old_pn_obj;

}

void ttgui_otobj_reset_game(void)
/***********************************************************************
*
* 	 ttgui_otobj_reset_game
*
***********************************************************************/
{
	for (int i=0;i<NUM_TOTAL_OPTIONS;i++)
	{
		if (ot_obj[i].type == TTGUI_OT_GAME)
		{
			ot_obj[i].current = ot_obj_setup[i].current;
		}
	}
}

void ttgui_otobj_update_txobj(void)
/***********************************************************************
*
* 	 ttgui_otobj_update_txobj
*
***********************************************************************/
{
		char buf[20]={0};
		uint16_t i = tx_obj-> tx_obj_id;
		int index = ot_obj[i].current;

		strcpy(buf,ot_obj[i].text);
		strcat(buf,ot_obj[i].options.selection[index]);
		strcpy(tx_obj->text, buf);

		// show me
		ttgui_txobj_get_focus();
}


void ttgui_otobj_change_cheat(void)
/***********************************************************************
*
* 	 ttgui_otobj_change_cheat
*
***********************************************************************/
{
	char *ext_t;
	char *buf_t = NULL;
	Err Result;
	CMF_cheat *cmf_cheat_apply = NULL;
	CMF_poke *cmf_poke_apply = NULL;
	uint16_t tx_ot_id = tx_obj-> tx_obj_id;

	if (currentCatFilename == NULL)
		return;

	if (currentCatFilename[0] != '\0')
	{
		buf_t = (char*)calloc(1,MAX_FILE_NAME);
		if (buf_t==NULL)
		{
			printf("Memory allocation issue\n");
			return;
		}
		strcpy(buf_t, DEFAULT_CHEAT_PATH);
		strcat(buf_t,currentCatFilename);
		ext_t = strrchr(buf_t,'.');
		strcpy(ext_t,CHEAT_EXTENSION);

		printf("Looking for Cheat file: %s\n",buf_t);
		Result = cmf_read(buf_t);

		if (Result == errNone)
		{
			// log the cheat file
			cmf_print();

			cmf_cheat_apply = cmf_getcheat();
			if (cmf_cheat_apply != NULL)
			{
				switch (ot_obj[tx_ot_id].current)
				{
					// Enable
					case (0):
					{
						printf("Try to apply patch: %s\n",cmf_cheat_apply->description);

						// apply the list of pokes in case it fits
						for (int i=0; i<cmf_cheat_apply->length; i++)
						{
							cmf_poke_apply = cmf_getpoke(cmf_cheat_apply->index + i);
							if (CPCPeek(cmf_poke_apply->address) == cmf_poke_apply->original_code)
							{
								CPCPoke(cmf_poke_apply->address,cmf_poke_apply->cheat_code);
								printf("Poke applied: %d\n", cmf_cheat_apply->index + i);
							}
							else
							{
									printf("Poke does not match! %04x:%02x\n",cmf_poke_apply->address, CPCPeek(cmf_poke_apply->address));
							}
						}
						ot_obj[tx_ot_id].current = 1;
						break;
					}

					// disable
					case (1):
					{
						printf("Try to revert patch: %s\n",cmf_cheat_apply->description);

						// apply the list of pokes in case it fits
						for (int i=0; i<cmf_cheat_apply->length; i++)
						{
							cmf_poke_apply = cmf_getpoke(cmf_cheat_apply->index + i);
							if (CPCPeek(cmf_poke_apply->address) == cmf_poke_apply->cheat_code)
							{
								CPCPoke(cmf_poke_apply->address,cmf_poke_apply->original_code);
								printf("Poke revert: %d\n", cmf_cheat_apply->index + i);
							}
							else
							{
									printf("Poke does not match! %04x:%02x\n",cmf_poke_apply->address, CPCPeek(cmf_poke_apply->address));
							}
						}
						ot_obj[tx_ot_id].current = 0;
						break;
					}
				} // option switch
			} // cheat file valid

			// update the window text
			ttgui_otobj_update_txobj();
		}
	}

	// clean up
	if (buf_t != NULL)
		free(buf_t);

	cmf_free();
}

void ttgui_otobj_change_sound(void)
/***********************************************************************
*
* 	 ttgui_otobj_change_sound
*
***********************************************************************/
{
	uint16_t tx_ot_id = tx_obj-> tx_obj_id;

	switch (ot_obj[tx_ot_id].current)
	{
		// Action Disable
		case (0):
		{
			DisableSound();
			ot_obj[tx_ot_id].current = 1;
			break;
		}

		// Action Enable
		case (1):
		{
			EnableSound();
			ot_obj[tx_ot_id].current = 0;
			break;
		}
	}
	// update the window text
	ttgui_otobj_update_txobj();
}
