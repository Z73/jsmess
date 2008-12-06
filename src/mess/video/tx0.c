/*
	TX-0

	Raphael Nabet, 2004
*/

#include "driver.h"

#include "cpu/pdp1/tx0.h"
#include "includes/tx0.h"
#include "video/crt.h"


static bitmap_t *panel_bitmap;
static bitmap_t *typewriter_bitmap;

INLINE void tx0_plot_pixel(bitmap_t *bitmap, int x, int y, UINT32 color)
{
	*BITMAP_ADDR16(bitmap, y, x) = color;
}

static const rectangle panel_bitmap_bounds =
{
	0,	panel_window_width-1,	/* min_x, max_x */
	0,	panel_window_height-1,	/* min_y, max_y */
};

static const rectangle typewriter_bitmap_bounds =
{
	0,	typewriter_window_width-1,	/* min_x, max_x */
	0,	typewriter_window_height-1,	/* min_y, max_y */
};

static void tx0_draw_panel_backdrop(running_machine *machine, bitmap_t *bitmap);
static void tx0_draw_panel(running_machine *machine, bitmap_t *bitmap);



/*
	video init
*/
VIDEO_START( tx0 )
{
	/* alloc bitmaps for our private fun */
	panel_bitmap = auto_bitmap_alloc(panel_window_width, panel_window_height, BITMAP_FORMAT_INDEXED16);
	typewriter_bitmap = auto_bitmap_alloc(typewriter_window_width, typewriter_window_height, BITMAP_FORMAT_INDEXED16);

	/* set up out bitmaps */
	tx0_draw_panel_backdrop(machine, panel_bitmap);

	bitmap_fill(typewriter_bitmap, &typewriter_bitmap_bounds, pen_typewriter_bg);

	/* initialize CRT */
	video_start_crt(pen_crt_num_levels, crt_window_offset_x, crt_window_offset_y, crt_window_width, crt_window_height);
}


/*
	schedule a pixel to be plotted
*/
void tx0_plot(int x, int y)
{
	/* compute pixel coordinates and plot */
	x = x*crt_window_width/0777;
	y = y*crt_window_height/0777;
	crt_plot(x, y);
}


/*
	VIDEO_UPDATE( tx0 ): effectively redraw the screen
*/
VIDEO_UPDATE( tx0 )
{
	VIDEO_UPDATE_CALL(crt);

	tx0_draw_panel(screen->machine, panel_bitmap);
	copybitmap(bitmap, panel_bitmap, 0, 0, panel_window_offset_x, panel_window_offset_y, cliprect);

	copybitmap(bitmap, typewriter_bitmap, 0, 0, typewriter_window_offset_x, typewriter_window_offset_y, cliprect);
	return 0;
}



/*
	Operator control panel code
*/

enum
{
	x_panel_col1a_offset = 0,
	x_panel_col1b_offset = 24,
	x_panel_col2_offset = x_panel_col1a_offset+184+8
};

enum
{
	/* column 1: registers, test accumulator, test buffer, toggle switch storage */
	y_panel_pc_offset = 0,
	y_panel_mar_offset = y_panel_pc_offset+2*8,
	y_panel_mbr_offset = y_panel_mar_offset+2*8,
	y_panel_ac_offset = y_panel_mbr_offset+2*8,
	y_panel_lr_offset = y_panel_ac_offset+2*8,
	y_panel_xr_offset = y_panel_lr_offset+2*8,
	y_panel_tbr_offset = y_panel_xr_offset+2*8,
	y_panel_tac_offset = y_panel_tbr_offset+2*8,
	y_panel_tss_offset = y_panel_tac_offset+2*8,

	/* column 2: stop c0, stop c1, cm sel, 1-bit indicators, instr, flags */
	y_panel_stop_c0_offset = 8,
	y_panel_stop_c1_offset = y_panel_stop_c0_offset+8,
	y_panel_gbl_cm_sel_offset = y_panel_stop_c1_offset+8,
	y_panel_run_offset = y_panel_gbl_cm_sel_offset+8,
	y_panel_cycle1_offset = y_panel_run_offset+8,
	y_panel_cycle2_offset = y_panel_cycle1_offset+8,
	y_panel_rim_offset = y_panel_cycle2_offset+8,
	y_panel_ioh_offset = y_panel_rim_offset+8,
	y_panel_ios_offset = y_panel_ioh_offset+8,
	y_panel_ir_offset = y_panel_ios_offset+8,
	y_panel_pf_offset = y_panel_ir_offset+2*8
};

/* draw a small 8*8 LED (or is this a lamp? ) */
static void tx0_draw_led(running_machine *machine, bitmap_t *bitmap, int x, int y, int state)
{
	int xx, yy;

	for (yy=1; yy<7; yy++)
		for (xx=1; xx<7; xx++)
			tx0_plot_pixel(bitmap, x+xx, y+yy, state ? pen_lit_lamp : pen_unlit_lamp);
}

/* draw nb_bits leds which represent nb_bits bits in value */
static void tx0_draw_multipleled(running_machine *machine, bitmap_t *bitmap, int x, int y, int value, int nb_bits)
{
	while (nb_bits)
	{
		nb_bits--;

		tx0_draw_led(machine, bitmap, x, y, (value >> nb_bits) & 1);

		x += 8;
	}
}


/* draw a small 8*8 switch */
static void tx0_draw_switch(running_machine *machine, bitmap_t *bitmap, int x, int y, int state)
{
	int xx, yy;
	int i;

	/* erase area */
	for (yy=0; yy<8; yy++)
		for (xx=0; xx<8; xx++)
			tx0_plot_pixel(bitmap, x+xx, y+yy, pen_panel_bg);


	/* draw nut (-> circle) */
	for (i=0; i<4;i++)
	{
		tx0_plot_pixel(bitmap, x+2+i, y+1, pen_switch_nut);
		tx0_plot_pixel(bitmap, x+2+i, y+6, pen_switch_nut);
		tx0_plot_pixel(bitmap, x+1, y+2+i, pen_switch_nut);
		tx0_plot_pixel(bitmap, x+6, y+2+i, pen_switch_nut);
	}
	tx0_plot_pixel(bitmap, x+2, y+2, pen_switch_nut);
	tx0_plot_pixel(bitmap, x+5, y+2, pen_switch_nut);
	tx0_plot_pixel(bitmap, x+2, y+5, pen_switch_nut);
	tx0_plot_pixel(bitmap, x+5, y+5, pen_switch_nut);

	/* draw button (->disc) */
	if (! state)
		y += 4;
	for (i=0; i<2;i++)
	{
		tx0_plot_pixel(bitmap, x+3+i, y, pen_switch_button);
		tx0_plot_pixel(bitmap, x+3+i, y+3, pen_switch_button);
	}
	for (i=0; i<4;i++)
	{
		tx0_plot_pixel(bitmap, x+2+i, y+1, pen_switch_button);
		tx0_plot_pixel(bitmap, x+2+i, y+2, pen_switch_button);
	}
}


/* draw nb_bits switches which represent nb_bits bits in value */
static void tx0_draw_multipleswitch(running_machine *machine, bitmap_t *bitmap, int x, int y, int value, int nb_bits)
{
	while (nb_bits)
	{
		nb_bits--;

		tx0_draw_switch(machine, bitmap, x, y, (value >> nb_bits) & 1);

		x += 8;
	}
}


/* write a single char on screen */
static void tx0_draw_char(running_machine *machine, bitmap_t *bitmap, char character, int x, int y, int color)
{
	drawgfx(bitmap, machine->gfx[0], character-32, color, 0, 0,
				x+1, y, NULL, TRANSPARENCY_PEN, 0);
}

/* write a string on screen */
static void tx0_draw_string(running_machine *machine, bitmap_t *bitmap, const char *buf, int x, int y, int color)
{
	while (* buf)
	{
		tx0_draw_char(machine, bitmap, *buf, x, y, color);

		x += 8;
		buf++;
	}
}


/* draw a vertical line */
static void tx0_draw_vline(bitmap_t *bitmap, int x, int y, int height, int color)
{
	while (height--)
		tx0_plot_pixel(bitmap, x, y++, color);
}

#ifdef UNUSED_FUNCTION
/* draw a horizontal line */
static void tx0_draw_hline(bitmap_t *bitmap, int x, int y, int width, int color)
{
	while (width--)
		tx0_plot_pixel(bitmap, x++, y, color);
}
#endif

/*
	draw the operator control panel (fixed backdrop)
*/
static void tx0_draw_panel_backdrop(running_machine *machine, bitmap_t *bitmap)
{
	int i;
	char buf[3];

	/* fill with black */
	bitmap_fill(panel_bitmap, &panel_bitmap_bounds, pen_panel_bg);

	/* column 1: registers, test accumulator, test buffer, toggle switch storage */
	tx0_draw_string(machine, bitmap, "program counter", x_panel_col1b_offset, y_panel_pc_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "memory address reg.", x_panel_col1b_offset, y_panel_mar_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "memory buffer reg.", x_panel_col1b_offset, y_panel_mbr_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "accumulator", x_panel_col1b_offset, y_panel_ac_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "live register", x_panel_col1b_offset, y_panel_lr_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "index register", x_panel_col1b_offset, y_panel_xr_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "TBR", x_panel_col1b_offset, y_panel_tbr_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "TAC", x_panel_col1b_offset, y_panel_tac_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "cm", x_panel_col1a_offset+8, y_panel_tss_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "TSS", x_panel_col1a_offset+24, y_panel_tss_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "lr", x_panel_col1a_offset+168, y_panel_tss_offset, color_panel_caption);
	for (i=0; i<16; i++)
	{
		sprintf(buf, "%2o", i);
		tx0_draw_string(machine, bitmap, buf, x_panel_col1a_offset, y_panel_tss_offset+8+i*8, color_panel_caption);
	}

	/* column separator */
	tx0_draw_vline(bitmap, x_panel_col2_offset-4, 8, 248, pen_panel_caption);

	/* column 2: stop c0, stop c1, cm sel, 1-bit indicators, instr, flags */
	tx0_draw_string(machine, bitmap, "stop c0", x_panel_col2_offset+8, y_panel_stop_c0_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "stop c1", x_panel_col2_offset+8, y_panel_stop_c1_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "cm select", x_panel_col2_offset+8, y_panel_gbl_cm_sel_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "run", x_panel_col2_offset+8, y_panel_run_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "cycle1", x_panel_col2_offset+8, y_panel_cycle1_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "cycle2", x_panel_col2_offset+8, y_panel_cycle2_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "read in", x_panel_col2_offset+8, y_panel_rim_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "i-o halt", x_panel_col2_offset+8, y_panel_ioh_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "i-o sync", x_panel_col2_offset+8, y_panel_ios_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "instr", x_panel_col2_offset+8, y_panel_ir_offset, color_panel_caption);
	tx0_draw_string(machine, bitmap, "pgm flags", x_panel_col2_offset+8, y_panel_pf_offset, color_panel_caption);
}


/*
	draw the operator control panel (dynamic elements)
*/
static void tx0_draw_panel(running_machine *machine, bitmap_t *bitmap)
{
	int cm_sel, lr_sel;
	int i;

	/* column 1: registers, test accumulator, test buffer, toggle switch storage */
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset+2*8, y_panel_pc_offset+8, cpu_get_reg(machine->cpu[0], TX0_PC), 16);
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset+2*8, y_panel_mar_offset+8, cpu_get_reg(machine->cpu[0], TX0_MAR), 16);
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset, y_panel_mbr_offset+8, cpu_get_reg(machine->cpu[0], TX0_MBR), 18);
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset, y_panel_ac_offset+8, cpu_get_reg(machine->cpu[0], TX0_AC), 18);
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset, y_panel_lr_offset+8, cpu_get_reg(machine->cpu[0], TX0_LR), 18);
	tx0_draw_multipleled(machine, bitmap, x_panel_col1b_offset+4*8, y_panel_xr_offset+8, cpu_get_reg(machine->cpu[0], TX0_XR), 14);
	tx0_draw_multipleswitch(machine, bitmap, x_panel_col1b_offset, y_panel_tbr_offset+8, cpu_get_reg(machine->cpu[0], TX0_TBR), 18);
	tx0_draw_multipleswitch(machine, bitmap, x_panel_col1b_offset, y_panel_tac_offset+8, cpu_get_reg(machine->cpu[0], TX0_TAC), 18);
	cm_sel = cpu_get_reg(machine->cpu[0], TX0_CM_SEL);
	lr_sel = cpu_get_reg(machine->cpu[0], TX0_LR_SEL);
	for (i=0; i<16; i++)
	{
		tx0_draw_switch(machine, bitmap, x_panel_col1a_offset+16, y_panel_tss_offset+8+i*8, (cm_sel >> i) & 1);
		tx0_draw_multipleswitch(machine, bitmap, x_panel_col1a_offset+24, y_panel_tss_offset+8+i*8, cpu_get_reg(machine->cpu[0], TX0_TSS00+i), 18);
		tx0_draw_switch(machine, bitmap, x_panel_col1a_offset+168, y_panel_tss_offset+8+i*8, (lr_sel >> i) & 1);
	}

	/* column 2: stop c0, stop c1, cm sel, 1-bit indicators, instr, flags */
	tx0_draw_switch(machine, bitmap, x_panel_col2_offset, y_panel_stop_c0_offset, cpu_get_reg(machine->cpu[0], TX0_STOP_CYC0));
	tx0_draw_switch(machine, bitmap, x_panel_col2_offset, y_panel_stop_c1_offset, cpu_get_reg(machine->cpu[0], TX0_STOP_CYC1));
	tx0_draw_switch(machine, bitmap, x_panel_col2_offset, y_panel_gbl_cm_sel_offset, cpu_get_reg(machine->cpu[0], TX0_GBL_CM_SEL));
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_run_offset, cpu_get_reg(machine->cpu[0], TX0_RUN));
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_cycle1_offset, cpu_get_reg(machine->cpu[0], TX0_CYCLE) & 1);
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_cycle2_offset, cpu_get_reg(machine->cpu[0], TX0_CYCLE) & 2);
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_rim_offset, cpu_get_reg(machine->cpu[0], TX0_RIM));
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_ioh_offset, cpu_get_reg(machine->cpu[0], TX0_IOH));
	tx0_draw_led(machine, bitmap, x_panel_col2_offset, y_panel_ios_offset, cpu_get_reg(machine->cpu[0], TX0_IOS));
	tx0_draw_multipleled(machine, bitmap, x_panel_col2_offset, y_panel_ir_offset+8, cpu_get_reg(machine->cpu[0], TX0_IR), 5);
	tx0_draw_multipleled(machine, bitmap, x_panel_col2_offset, y_panel_pf_offset+8, cpu_get_reg(machine->cpu[0], TX0_PF), 6);
}



/*
	Typewriter code
*/


static int pos;

static int case_shift;

static int color = color_typewriter_black;

enum
{
	typewriter_line_height = 8,
	typewriter_write_offset_y = typewriter_window_height-typewriter_line_height,
	typewriter_scroll_step = typewriter_line_height
};

static const rectangle typewriter_scroll_clear_window =
{
	0, typewriter_window_width-1,	/* min_x, max_x */
	typewriter_window_height-typewriter_scroll_step, typewriter_window_height-1,	/* min_y, max_y */
};

enum
{
	tab_step = 8
};


static void tx0_typewriter_linefeed(running_machine *machine)
{
	UINT8 buf[typewriter_window_width];
	int y;

	for (y=0; y<typewriter_window_height-typewriter_scroll_step; y++)
	{
		extract_scanline8(typewriter_bitmap, 0, y+typewriter_scroll_step, typewriter_window_width, buf);
		draw_scanline8(typewriter_bitmap, 0, y, typewriter_window_width, buf, machine->pens, -1);
	}

	bitmap_fill(typewriter_bitmap, &typewriter_scroll_clear_window, pen_typewriter_bg);
}

void tx0_typewriter_drawchar(running_machine *machine, int character)
{
	static const char ascii_table[2][64] =
	{
		{	/* lower case */
			'\0',				'\0',				'e',				'8',
			'\0',				'|',				'a',				'3',
			' ',				'=',				's',				'4',
			'i',				'+',				'u',				'2',
			'\0',/*color shift*/'.',				'd',				'5',
			'r',				'1',				'j',				'7',
			'n',				',',				'f',				'6',
			'c',				'-',				'k',				'\0',
			't',				'\0',				'z',				'\0',/*back space*/
			'l',				'\0',/*tab*/		'w',				'\0',
			'h',				'\0',/*carr. return*/'y',				'\0',
			'p',				'\0',				'q',				'\0',
			'o',				'\0',/*stop*/		'b',				'\0',
			'g',				'\0',				'9',				'\0',
			'm',				'\0',/*upper case*/	'x',				'\0',
			'v',				'\0',/*lower case*/	'0',				'\0'/*delete*/
		},
		{	/* upper case */
			'\0',				'\0',				'E',				'\210',
			'\0',				'_',				'A',				'\203',
			' ',				':',				'S',				'\204',
			'I',				'/',				'U',				'\202',
			'\0',/*color shift*/')',				'D',				'\205',
			'R',				'\201',				'J',				'\207',
			'N',				'(',				'F',				'\206',
			'C',				'\212',/*macron*/	'K',				'\0',
			'T',				'\0',				'Z',				'\0',/*back space*/
			'L',				'\0',/*tab*/		'W',				'\0',
			'H',				'\0',/*carr. return*/'Y',				'\0',
			'P',				'\0',				'Q',				'\0',
			'O',				'\0',/*stop*/		'B',				'\0',
			'G',				'\0',				'\211',				'\0',
			'M',				'\0',/*upper case*/	'X',				'\0',
			'V',				'\0',/*lower case*/	'\200',				'\0'/*delete*/
		}
	};



	character &= 0x3f;

	switch (character)
	{
	case 020:
		/* color shift */
		/*color = color_typewriter_black;
		color = color_typewriter_red;*/
		break;

	case 043:
		/* Backspace */
		if (pos)
			pos--;
		break;

	case 045:
		/* Tab */
		pos = pos + tab_step - (pos % tab_step);
		break;

	case 051:
		/* Carriage Return */
		pos = 0;
		tx0_typewriter_linefeed( machine );
		break;

	case 061:
		/* Stop */
		/* ?????? */
		break;

	case 071:
		/* Upper case */
		case_shift = 1;
		break;

	case 075:
		/* Lower case */
		case_shift = 0;
		break;

	case 077:
		/* Delete */
		/* ?????? */
		break;

	default:
		/* Any printable character... */

		if (pos >= 80)
		{	/* if past right border, wrap around. (Right???) */
			tx0_typewriter_linefeed( machine );	/* next line */
			pos = 0;					/* return to start of line */
		}

		/* print character (lookup ASCII equivalent in table) */
		tx0_draw_char(machine, typewriter_bitmap, ascii_table[case_shift][character],
						8*pos, typewriter_write_offset_y,
						color);	/* print char */

		pos++;		/* step carriage forward */
		break;
	}
}
