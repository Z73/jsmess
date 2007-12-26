/******************************************************************************
 sharp pocket computers
 pc1401/pc1403
 PeT mess@utanet.at May 2000
******************************************************************************/

#include "driver.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1401.h"
#include "includes/pc1251.h"
#include "includes/pc1350.h"
#include "includes/pc1403.h"


/* pc1430 no peek poke operations! */

/* pc1280?? */

/* pc126x
   port
   1 ?
   2 +6v
   3 gnd
   4 f0
   5 f1
   6 load
   7 save
   8 ib7
   9 ib6
  10 ib5
  11 ib4 */

/* pc1350 other keyboard,
   f2 instead f0 at port
   port
   1 ?
   2 +6v
   3 gnd
   4 f2
   5 f1
   6 load
   7 save
   8 ib7
   9 ib6
  10 ib5
  11 ib4
*/

/* similar computers
   pc1260/1261
   pc1402/1403
   pc1421 */
/* pc140x
   a,b0..b5 keyboard matrix
   b0 off key
   c0 display on
   c1 counter reset
   c2 cpu halt
   c3 computer off
   c4 beeper frequency (1 4khz, 0 2khz), or (c5=0) membran pos1/pos2
   c5 beeper on
   c6 beeper steuerung

   port
   1 ?
   2 +6v
   3 gnd
   4 f0
   5 f1
   6 load
   7 save
   8 ib7
   9 ib6
  10 ?
  11 ?
*/

/* special keys
   red c-ce and reset; warm boot, program NOT lost*/

static ADDRESS_MAP_START( pc1401_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_ROM
	AM_RANGE( 0x2000, 0x47ff) AM_READ( MRA8_RAM )
/*	{ 0x2000, 0x3fff, MWA8_RAM }, // done in pc1401_machine_init */
/*	{ 0x5000, 0x57ff, ? }, */
	AM_RANGE( 0x6000, 0x67ff) AM_READWRITE( pc1401_lcd_read, pc1401_lcd_write ) AM_MIRROR(0x1000)
	AM_RANGE( 0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1251_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_ROM
//	{ 0x2000, 0x3fff, MRA8_RAM },
	AM_RANGE( 0x4000, 0x7fff) AM_ROM
	AM_RANGE( 0xa000, 0xcbff) AM_ROM
	AM_RANGE( 0xf800, 0xf8ff) AM_READWRITE( pc1251_lcd_read, pc1251_lcd_write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1350_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_ROM
	AM_RANGE( 0x7000, 0x7eff) AM_READWRITE( pc1350_lcd_read, pc1350_lcd_write )
	AM_RANGE( 0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1403_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_ROM
	AM_RANGE( 0x3000, 0x30bf) AM_READWRITE( pc1403_lcd_read, pc1403_lcd_write )  
	AM_RANGE( 0x3800, 0x3fff) AM_READWRITE( pc1403_asic_read, pc1403_asic_write )    
	AM_RANGE( 0x4000, 0x7fff) AM_ROMBANK(1)
	AM_RANGE( 0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END


#if 0
static ADDRESS_MAP_START( pc1421_readmem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_READ( MRA8_ROM )
	AM_RANGE( 0x3800, 0x47ff) AM_READ( MRA8_RAM )
	AM_RANGE( 0x8000, 0xffff) AM_READ( MRA8_ROM )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1421_writemem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_WRITE( MWA8_ROM )
	AM_RANGE( 0x2000, 0x37ff) AM_WRITE( MWA8_RAM )
	AM_RANGE( 0x3800, 0x47ff) AM_WRITE( MWA8_RAM )
	AM_RANGE( 0x8000, 0xffff) AM_WRITE( MWA8_ROM )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1260_readmem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_READ( MRA8_ROM )
	AM_RANGE( 0x4000, 0x5fff) AM_READ( MRA8_RAM )
	AM_RANGE( 0x8000, 0xffff) AM_READ( MRA8_ROM )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1260_writemem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE( 0x0000, 0x1fff) AM_WRITE( MWA8_ROM )
	AM_RANGE( 0x4000, 0x57ff) AM_WRITE( MWA8_RAM ) /* 1261 */
	AM_RANGE( 0x5800, 0x67ff) AM_WRITE( MWA8_RAM )
	AM_RANGE( 0x6000, 0x6fff) AM_WRITE( MWA8_RAM )

	AM_RANGE( 0x8000, 0xffff) AM_WRITE( MWA8_ROM )
ADDRESS_MAP_END
#endif

#define PC1401_HELPER1 \
	PORT_START \
	PORT_DIPNAME (0x80, 0x00,  "Power") \
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) ) \
	PORT_DIPSETTING( 0x00, DEF_STR( On ) ) \
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAL") PORT_CODE(KEYCODE_F1) \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BASIC") PORT_CODE(KEYCODE_F2) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BRK   ON") PORT_CODE(KEYCODE_F4) \
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEF") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) \
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("down") PORT_CODE(KEYCODE_DOWN) \
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("up") PORT_CODE(KEYCODE_UP) \
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("left  DEL") PORT_CODE(KEYCODE_LEFT) \
	PORT_START \
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("right INS") PORT_CODE(KEYCODE_RIGHT)

#define PC1401_HELPER2 \
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) \
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q     !") PORT_CODE(KEYCODE_Q)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W     \"") PORT_CODE(KEYCODE_W)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E     #") PORT_CODE(KEYCODE_E)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R     $") PORT_CODE(KEYCODE_R)\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T     %") PORT_CODE(KEYCODE_T)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y     &") PORT_CODE(KEYCODE_Y)\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U     ?") PORT_CODE(KEYCODE_U)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I     At") PORT_CODE(KEYCODE_I)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O     :") PORT_CODE(KEYCODE_O)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P     ;") PORT_CODE(KEYCODE_P)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A     INPUT") PORT_CODE(KEYCODE_A)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S     IF") PORT_CODE(KEYCODE_S)\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D     THEN") PORT_CODE(KEYCODE_D)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F     GOTO") PORT_CODE(KEYCODE_F)\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G     FOR") PORT_CODE(KEYCODE_G)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H     TO") PORT_CODE(KEYCODE_H)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J     STEP") PORT_CODE(KEYCODE_J)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K     NEXT") PORT_CODE(KEYCODE_K)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L     LIST") PORT_CODE(KEYCODE_L)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",     RUN") PORT_CODE(KEYCODE_COMMA)\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z     PRINT") PORT_CODE(KEYCODE_Z)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X     USING") PORT_CODE(KEYCODE_X)\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C     GOSUB") PORT_CODE(KEYCODE_C)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V     RETURN") PORT_CODE(KEYCODE_V)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B     DIM") PORT_CODE(KEYCODE_B)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N     END") PORT_CODE(KEYCODE_N)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M     CSAVE") PORT_CODE(KEYCODE_M)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPC   CLOAD") PORT_CODE(KEYCODE_SPACE)\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER N<>NP") PORT_CODE(KEYCODE_ENTER)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HYP   ARCHYP")\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SIN   SIN^-1")\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COS   COS^-1")\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("TAN   TAN^-1")\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F<>E  TAB")\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C-CE  CA") PORT_CODE(KEYCODE_ESC)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("->HEX ->DEC")\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("->DEG ->D.MS")\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LN    e^x    E")\
 	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LOG   10^x   F")\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1/x   ->re")\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("chnge STAT")\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EXP   Pi     A")\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("y^x   xROOTy B")\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("sqrt  3root  C")\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("sqr   tri%   D")\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(     ->xy") PORT_CODE(KEYCODE_8)\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")     n!") PORT_CODE(KEYCODE_9)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7     y mean") PORT_CODE(KEYCODE_7_PAD)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8     Sy") PORT_CODE(KEYCODE_8_PAD)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9     sigmay") PORT_CODE(KEYCODE_9_PAD)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH_PAD)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X->M  Sum y  Sum y^2")\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4     x mean") PORT_CODE(KEYCODE_4_PAD)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5     Sx") PORT_CODE(KEYCODE_5_PAD)\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6     sigmax") PORT_CODE(KEYCODE_6_PAD)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*     <") PORT_CODE(KEYCODE_ASTERISK)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RM    (x,y)")\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1     r") PORT_CODE(KEYCODE_1_PAD)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2     a") PORT_CODE(KEYCODE_2_PAD)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3     b") PORT_CODE(KEYCODE_3_PAD)\
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-     >") PORT_CODE(KEYCODE_MINUS_PAD)\
	PORT_START\
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M+    DATA   CD")\
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0     x'") PORT_CODE(KEYCODE_0_PAD)\
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+/-   y'") PORT_CODE(KEYCODE_NUMLOCK)\
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".     DRG") PORT_CODE(KEYCODE_DEL_PAD)\
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+     ^") PORT_CODE(KEYCODE_PLUS_PAD)\
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_ENTER_PAD)\
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)

static INPUT_PORTS_START( pc1401 )
    PC1401_HELPER1
	PORT_BIT ( 0x40, 0x0,	 IPT_UNUSED )
    PC1401_HELPER2
	PORT_START
    PORT_DIPNAME   ( 0xc0, 0x80, "RAM")
	PORT_DIPSETTING( 0x00, "2KB" )
	PORT_DIPSETTING( 0x40, "PC1401(4KB)" )
	PORT_DIPSETTING( 0x80, "PC1402(10KB)" )
    PORT_DIPNAME   ( 7, 2, "Contrast")
	PORT_DIPSETTING( 0, "0/Low" )
	PORT_DIPSETTING( 1, "1" )
	PORT_DIPSETTING( 2, "2" )
	PORT_DIPSETTING( 3, "3" )
	PORT_DIPSETTING( 4, "4" )
	PORT_DIPSETTING( 5, "5" )
	PORT_DIPSETTING( 6, "6" )
	PORT_DIPSETTING( 7, "7/High" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc1403 )
    PC1401_HELPER1
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SML") PORT_CODE(KEYCODE_CAPSLOCK)
    PC1401_HELPER2
	PORT_START
    PORT_DIPNAME   ( 0x80, 0x80, "RAM")
	PORT_DIPSETTING( 0x00, "PC1403(8KB)" )
	PORT_DIPSETTING( 0x80, "PC1403H(32KB)" )

    // normally no contrast control!
    PORT_DIPNAME   ( 7, 2, "Contrast")
	PORT_DIPSETTING( 0, "0/Low" )
	PORT_DIPSETTING( 1, "1" )
	PORT_DIPSETTING( 2, "2" )
	PORT_DIPSETTING( 3, "3" )
	PORT_DIPSETTING( 4, "4" )
	PORT_DIPSETTING( 5, "5" )
	PORT_DIPSETTING( 6, "6" )
	PORT_DIPSETTING( 7, "7/High" )
INPUT_PORTS_END


static INPUT_PORTS_START( pc1251 )
	PORT_START
    PORT_DIPNAME   ( 7, 0, "Mode")
	PORT_DIPSETTING( 4, DEF_STR(Off) )
	PORT_DIPSETTING( 0, "On/RUN" )
	PORT_DIPSETTING( 2, "On/PRO" )
	PORT_DIPSETTING( 1, "On/RSV" )
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEF") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN  (") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP    )") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT  DEL") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT INS") PORT_CODE(KEYCODE_RIGHT)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BRK   ON") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q     !") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W     \"") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E     #") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R     $") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T     %") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y     &") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U     ?") PORT_CODE(KEYCODE_U)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I     :") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O     ,") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P     ;") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("      SPC") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER N<>NP") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CL    CA") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/     ^") PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_START
 	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3     @") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*     <") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0     Pi") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".     SquareRoot") PORT_CODE(KEYCODE_STOP) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+     Exp") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-     >") PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_START
#if 0
    PORT_DIPNAME   ( 0xc0, 0xc0, "RAM")
	PORT_DIPSETTING( 0x00, "1KB" )
	PORT_DIPSETTING( 0x40, "3.5KB" )
	PORT_DIPSETTING( 0x80, "6KB" )
	PORT_DIPSETTING( 0xc0, "11KB" )
#endif
    PORT_DIPNAME   ( 7, 2, "Contrast")
	PORT_DIPSETTING( 0, "0/Low" )
	PORT_DIPSETTING( 1, "1" )
	PORT_DIPSETTING( 2, "2" )
	PORT_DIPSETTING( 3, "3" )
	PORT_DIPSETTING( 4, "4" )
	PORT_DIPSETTING( 5, "5" )
	PORT_DIPSETTING( 6, "6" )
	PORT_DIPSETTING( 7, "7/High" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc1350 )
	PORT_START
    PORT_DIPNAME (0x80, 0x00, "Power")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLS   CA") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BRK   ON") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(     <") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")     >") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-     ^") PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q     !") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W     \"") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E     #") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R     $") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T     %") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y     &") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U     ?") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I     Pi") PORT_CODE(KEYCODE_I)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O     Squareroot") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P     Alpha") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEF") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_START
 	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SML") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_START
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPC") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER P<->NP") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START
    PORT_DIPNAME   ( 7, 2, "Contrast")
	PORT_DIPSETTING( 0, "0/Low" )
	PORT_DIPSETTING( 1, "1" )
	PORT_DIPSETTING( 2, "2" )
	PORT_DIPSETTING( 3, "3" )
	PORT_DIPSETTING( 4, "4" )
	PORT_DIPSETTING( 5, "5" )
	PORT_DIPSETTING( 6, "6" )
	PORT_DIPSETTING( 7, "7/High" )
INPUT_PORTS_END

static const gfx_layout pc1401_charlayout =
{
	2,21,
	128,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0,0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,0 },
	/* y offsets */
	{
		7, 7, 7,
		6, 6, 6,
		5, 5, 5,
		4, 4, 4,
		3, 3, 3,
		2, 2, 2,
		1, 1, 1
	},
	1*8
};

static const gfx_layout pc1251_charlayout =
{
	3,21,
	128,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0, },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,0,0 },
	/* y offsets */
	{
		7, 7, 7,
		6, 6, 6,
		5, 5, 5,
		4, 4, 4,
		3, 3, 3,
		2, 2, 2,
		1, 1, 1
	},
	1*8
};

static GFXDECODE_START( pc1401_gfxdecodeinfo )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, pc1401_charlayout, 0, 8 )
GFXDECODE_END

static GFXDECODE_START( pc1251_gfxdecodeinfo )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, pc1251_charlayout, 0, 8 )
GFXDECODE_END

static const SC61860_CONFIG config={
    pc1401_reset, pc1401_brk, NULL,
    pc1401_ina, pc1401_outa,
    pc1401_inb, pc1401_outb,
    pc1401_outc
};

static MACHINE_DRIVER_START( pc1401 )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", SC61860, 192000)        /* 7.8336 Mhz */
	MDRV_CPU_PROGRAM_MAP(pc1401_mem, 0)
	MDRV_CPU_CONFIG(config)

	MDRV_SCREEN_REFRESH_RATE(20)	/* very early and slow lcd */
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(1)

	MDRV_NVRAM_HANDLER( pc1401 )

	/*
	   aim: show sharp with keyboard
	   resolution depends on the dots of the lcd
	   (lcd dot displayed as 2x3 pixel)
	   it seams to have 3/4 ratio in the real pc1401 */
    /* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(594, 273)
	MDRV_SCREEN_VISIBLE_AREA(0, 594-1, 0, 273-1)
//	MDRV_SCREEN_SIZE(640, 273)
//	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 273-1)
	MDRV_GFXDECODE( pc1401_gfxdecodeinfo )
	MDRV_PALETTE_LENGTH(248 + 32768)
	MDRV_COLORTABLE_LENGTH( 16 )
	MDRV_PALETTE_INIT( pocketc )

	MDRV_VIDEO_START( pocketc )
	MDRV_VIDEO_UPDATE( pc1401 )

	/* sound hardware */
	/*MDRV_SOUND_ADD(DAC, pocketc_sound_interface)*/
MACHINE_DRIVER_END


static const SC61860_CONFIG pc1251_config =
{
    NULL, pc1251_brk, NULL,
    pc1251_ina, pc1251_outa,
    pc1251_inb, pc1251_outb,
    pc1251_outc
};

static MACHINE_DRIVER_START( pc1251 )
	MDRV_IMPORT_FROM( pc1401 )
	MDRV_CPU_MODIFY( "main" )
	MDRV_CPU_PROGRAM_MAP( pc1251_mem, 0 )
	MDRV_CPU_CONFIG( pc1251_config )

	MDRV_NVRAM_HANDLER( pc1251 )

	/* video hardware */
	MDRV_SCREEN_SIZE(608, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 608-1, 0, 300-1)
//	MDRV_SCREEN_SIZE(640, 334)
//	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 334-1)
	MDRV_GFXDECODE( pc1251_gfxdecodeinfo )

	MDRV_VIDEO_UPDATE( pc1251 )
MACHINE_DRIVER_END


static const SC61860_CONFIG pc1350_config =
{
    NULL, pc1350_brk,NULL,
    pc1350_ina, pc1350_outa,
    pc1350_inb, pc1350_outb,
    pc1350_outc
};

static MACHINE_DRIVER_START( pc1350 )
	MDRV_IMPORT_FROM( pc1401 )
	MDRV_CPU_MODIFY( "main" )
	MDRV_CPU_PROGRAM_MAP( pc1350_mem, 0 )
	MDRV_CPU_CONFIG( pc1350_config )

	MDRV_MACHINE_START( pc1350 )
	MDRV_NVRAM_HANDLER( pc1350 )

	/*
	   aim: show sharp with keyboard
	   resolution depends on the dots of the lcd
	   (lcd dot displayed as 2x2 pixel) */
	/* video hardware */
	MDRV_SCREEN_SIZE(640, 252)
	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 252-1)
//	MDRV_SCREEN_SIZE(640, 255)
//	MDRV_SCREEN_VISIBLE_AREA(0, 640-1, 0, 255-1)

	MDRV_VIDEO_UPDATE( pc1350 )
MACHINE_DRIVER_END


static const SC61860_CONFIG pc1403_config =
{
    NULL, pc1403_brk, NULL,
    pc1403_ina, pc1403_outa,
    NULL,NULL,
    pc1403_outc
};

static MACHINE_DRIVER_START( pc1403 )
	MDRV_IMPORT_FROM( pc1401 )
	MDRV_CPU_REPLACE( "main", SC61860, 256000 )
	MDRV_CPU_PROGRAM_MAP( pc1403_mem, 0 )
	MDRV_CPU_CONFIG( pc1403_config )

	MDRV_NVRAM_HANDLER( pc1403 )

	/*
	   aim: show sharp with keyboard
	   resolution depends on the dots of the lcd
	   (lcd dot displayed as 2x2 pixel) */
	/* video hardware */
	MDRV_SCREEN_SIZE(848, 320)
	MDRV_SCREEN_VISIBLE_AREA(0, 848-1, 0, 320-1)
//	MDRV_SCREEN_SIZE(848, 361)
//	MDRV_SCREEN_VISIBLE_AREA(0, 848-1, 0, 361-1)

	MDRV_VIDEO_START( pc1403 )
	MDRV_VIDEO_UPDATE( pc1403 )
MACHINE_DRIVER_END


ROM_START(pc1401)
	ROM_REGION(0x10000,REGION_CPU1,0)
	/* SC61860A08 5H 13LD cpu with integrated rom*/
	ROM_LOAD("sc61860.a08", 0x0000, 0x2000, CRC(44bee438) SHA1(c5106bc8d848be1b49494ace30a26eeb1cc5e504))
/* 5S1 SC613256 D30
   or SC43536LD 5G 13 (LCD chip?) */
	ROM_LOAD("sc613256.d30", 0x8000, 0x8000, CRC(69b9d587) SHA1(fa0602e7dfee548546f801fb4cca7d73da2d8f18))
	ROM_REGION(0x80,REGION_GFX1,0)
ROM_END

#define rom_pc1402 rom_pc1401

ROM_START(pc1251)
	ROM_REGION(0x10000,REGION_CPU1,0)
	/* sc61860a13 6c 13 ld */
	ROM_LOAD("cpu1251.rom", 0x0000, 0x2000, CRC(f7287aca) SHA1(19bfa778e3e05ea06bdca15cd9dfbba9b971340e))
	ROM_LOAD("bas1251.rom", 0x4000, 0x4000, CRC(93ecb629) SHA1(0fe0ad419053ee7814600b0be320dd2e8eb2ec92))
	ROM_REGION(0x80,REGION_GFX1,0)
ROM_END

#define rom_trs80pc3 rom_pc1251

ROM_START(pc1350)
	ROM_REGION(0x10000,REGION_CPU1,0)
	/* sc61860a13 6c 13 ld */
	ROM_LOAD("cpu.rom", 0x0000, 0x2000, CRC(79a924bc) SHA1(2eaef0d53d85863ca70a41c8e1eddc5915136b99))
	ROM_LOAD("basic.rom", 0x8000, 0x8000, CRC(158b28e2) SHA1(b63b37dd510b3c4d9f16d224f87ae2efb3bcc51f))
	ROM_REGION(0x100,REGION_GFX1,0)
ROM_END

ROM_START(pc1403)
	ROM_REGION(0x10000,REGION_CPU1,0)
    ROM_LOAD("introm.bin", 0x0000, 0x2000, CRC(588c500b) SHA1(2fed9ebede27e20a8ee4b4b03b9f8cd7808ada5c))
	ROM_REGION(0x10000,REGION_USER1,0)
    ROM_LOAD("extrom08.bin", 0x0000, 0x4000, CRC(1fa65140) SHA1(f22a9f114486f69733fc43dfec26fb210643aeff))
    ROM_LOAD("extrom09.bin", 0x4000, 0x4000, CRC(4a7da6ab) SHA1(b50fe8a4ca821244c119147b3ff04cee0fd6ad5c))
    ROM_LOAD("extrom0a.bin", 0x8000, 0x4000, CRC(9925174f) SHA1(793a79142cd170ed7ac3f7ecb1b6e6f92c8fa4e0))
    ROM_LOAD("extrom0b.bin", 0xc000, 0x4000, CRC(fa5df9ec) SHA1(6ff62c215f510a3a652d61823f54cd4018d6a771))
	ROM_REGION(0x100,REGION_GFX1,0)
ROM_END

#define rom_pc1403h rom_pc1403

// disk drive support
#define io_pc1403 io_pc1401
#define io_pc1403h io_pc1403

SYSTEM_CONFIG_START(pocketc)
//	CONFIG_DEVICE_CASSETTE(1, "", mycas_init)
SYSTEM_CONFIG_END

SYSTEM_CONFIG_START(pc1350)
//	CONFIG_DEVICE_CASSETTE(1, "", mycas_init)
	CONFIG_RAM_DEFAULT(4 * 1024)
	CONFIG_RAM(12 * 1024)
	CONFIG_RAM(20 * 1024)
SYSTEM_CONFIG_END

/*    YEAR  NAME      PARENT    MACHINE   INPUT     INIT      MONITOR	COMPANY   FULLNAME */

/* cpu sc43177, sc43178 (4bit!)
   pc 1211
   clone tandy trs80 pocket computer
   pc1246/pc1247
*/

/* cpu lh5801
   pc1500 
   clone tandy pc2 look into drivers/pc1500.c 
   pc1600
*/

/*    YEAR  NAME		PARENT	COMPAT	MACHINE	INPUT	INIT	CONFIG		COMPANY		FULLNAME */
/* cpu sc61860 */
COMP( 1982, pc1251,		0, 		0,		pc1251,	pc1251,	pc1251,	pocketc,	"Sharp",	"Pocket Computer 1251", GAME_NOT_WORKING)
COMP( 198?, trs80pc3,	pc1251,	0,		pc1251,	pc1251,	pc1251,	pocketc,	"Tandy",	"TRS80 PC-3", GAME_NOT_WORKING)

/* pc1261/pc1262 */
COMP( 1984, pc1350,		0, 		0,		pc1350,	pc1350,	0,		pc1350,		"Sharp",	"Pocket Computer 1350", GAME_NOT_WORKING)
COMP( 1983, pc1401,		0, 		0,		pc1401,	pc1401,	pc1401,	pocketc,	"Sharp",	"Pocket Computer 1401", GAME_NOT_WORKING)
COMP( 1984, pc1402,		pc1401,	0,		pc1401,	pc1401,	pc1401,	pocketc,	"Sharp",	"Pocket Computer 1402", GAME_NOT_WORKING)

/* 72kb rom, 32kb ram, cpu? pc1360 */
COMP( 198?, pc1403,		0,		0,		pc1403,	pc1403,	pc1403,	pocketc,	"Sharp",	"Pocket Computer 1403", GAME_NOT_WORKING)
COMP( 198?, pc1403h,	pc1403,	0,		pc1403,	pc1403,	pc1403,	pocketc,	"Sharp",	"Pocket Computer 1403H", GAME_NOT_WORKING)
