/*	SCCS Id: @(#)drawing.c	3.4	1999/12/02	*/
/* Copyright (c) NetHack Development Team 1992.			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "tcap.h"

/* Relevent header information in rm.h and objclass.h. */

#ifdef C
#undef C
#endif

#ifdef TEXTCOLOR
#define C(n) n
#else
#define C(n)
#endif

#define g_FILLER(symbol) 0

uchar oc_syms[MAXOCLASSES] = DUMMY; /* the current object  display symbols */
uchar showsyms[MAXPCHARS]  = DUMMY; /* the current feature display symbols */
uchar monsyms[MAXMCLASSES] = DUMMY; /* the current monster display symbols */
uchar warnsyms[WARNCOUNT]  = DUMMY;  /* the current warning display symbols */

/* Default object class symbols.  See objclass.h. */
const char def_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',		/* placeholder for the "random class" */
	ILLOBJ_SYM,
	WEAPON_SYM,
	ARMOR_SYM,
	RING_SYM,
/* 5*/	AMULET_SYM,
	TOOL_SYM,
	FOOD_SYM,
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GOLD_SYM,
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

const char invisexplain[] = "criatura recordada y no vista";

/* Object descriptions.  Used in do_look(). */
const char * const objexplain[] = {	/* these match def_oc_syms, above */
/* 0*/	0,
	"objeto extraño",
	"arma",
	"traje o pieza de armadura",
	"anillo",
/* 5*/	"amuleto",
	"artículo útil (pico, llave, lámpara...)",
	"pieza de comida",
	"poción",
	"pergamino",
/*10*/	"libro de hechizos",
	"varita",
	"pila de monedas",
	"gema o roca",
	"roca enorme o estatua",
/*15*/	"bola de hierro",
	"cadena de hierro",
	"salpicadura de veneno"
};

/* Object class names.  Used in object_detect(). */
const char * const oclass_names[] = {
/* 0*/	0,
	"objetos ilegales",
	"armas",
	"armaduras",
	"anillos",
/* 5*/	"amuletos",
	"herramientas",
	"comida",
	"pociones",
	"pergaminos",
/*10*/	"libros de hechizos",
	"varitas",
	"monedas",
	"rocas",
	"piedras grandes",
/*15*/	"bolas de hierro",
	"cadenas",
	"venenos"
};

/* Default monster class symbols.  See monsym.h. */
const char def_monsyms[MAXMCLASSES] = {
	'\0',		/* holder */
	DEF_ANT,
	DEF_BLOB,
	DEF_COCKATRICE,
	DEF_DOG,
	DEF_EYE,
	DEF_FELINE,
	DEF_GREMLIN,
	DEF_HUMANOID,
	DEF_IMP,
	DEF_JELLY,		/* 10 */
	DEF_KOBOLD,
	DEF_LEPRECHAUN,
	DEF_MIMIC,
	DEF_NYMPH,
	DEF_ORC,
	DEF_PIERCER,
	DEF_QUADRUPED,
	DEF_RODENT,
	DEF_SPIDER,
	DEF_TRAPPER,		/* 20 */
	DEF_UNICORN,
	DEF_VORTEX,
	DEF_WORM,
	DEF_XAN,
	DEF_LIGHT,
	DEF_ZRUTY,
	DEF_ANGEL,
	DEF_BAT,
	DEF_CENTAUR,
	DEF_DRAGON,		/* 30 */
	DEF_ELEMENTAL,
	DEF_FUNGUS,
	DEF_GNOME,
	DEF_GIANT,
	'\0',
	DEF_JABBERWOCK,
	DEF_KOP,
	DEF_LICH,
	DEF_MUMMY,
	DEF_NAGA,		/* 40 */
	DEF_OGRE,
	DEF_PUDDING,
	DEF_QUANTMECH,
	DEF_RUSTMONST,
	DEF_SNAKE,
	DEF_TROLL,
	DEF_UMBER,
	DEF_VAMPIRE,
	DEF_WRAITH,
	DEF_XORN,		/* 50 */
	DEF_YETI,
	DEF_ZOMBIE,
	DEF_HUMAN,
	DEF_GHOST,
	DEF_GOLEM,
	DEF_DEMON,
	DEF_EEL,
	DEF_LIZARD,
	DEF_WORM_TAIL,
	DEF_MIMIC_DEF,		/* 60 */
};

/* The explanations below are also used when the user gives a string
 * for blessed genocide, so no text should wholly contain any later
 * text.  They should also always contain obvious names (eg. cat/feline).
 *
 * LENGUA: these names are not always even remotely obvious in Spanish
 */
const char * const monexplain[MAXMCLASSES] = {
    0,
    "hormiga u otro insecto",
    "burbuja",
    "basilisco",
    "perro u otro canino",
    "esfera u ojo",
    "gato u otro felino",
    "gremlin",
    "humanoide",
    "diablillo o demonio menor",
    "gelatina",
    "kóbold",
    "leprechaun",
    "mímico",
    "ninfa",
    "orco",
    "perforador",
    "cuadrúpedo",
    "roedor",
    "arácnido o ciempiés",
    "trampero o merodeador superior",
    "unicornio o caballo",
    "vórtice",
    "gusano",
    "xan u otro insecto mítico/fantástico",
    "luz",
    "zruty",

    "ser angelical",
    "murciélago o pájaro",
    "centauro",
    "dragón",
    "elemental",
    "hongo o moho",
    "gnomo",
    "humanoide gigante",
    0,
    "jabberwock",
    "Polizonte",
    "liche",
    "momia",
    "naga",
    "ogro",
    "pudín o cieno",
    "mecánico cuántico",
    "monstruo corrosivo o desencantador",
    "serpiente",
    "troll",
    "mole sombría",
    "vampiro",
    "incorpóreo",
    "xorn",
    "criatura simiesca",
    "zombi",

    "humano o elfo",
    "fantasma",
    "gólem",
    "demonio mayor",
    "monstruo marino",
    "lagarto",
    "cola de gusano largo",
    "mímico"
};

const struct symdef def_warnsyms[WARNCOUNT] = {
	{'0', "criatura desconocida que te preocupa", C(CLR_WHITE)},  	/* white warning  */
	{'1', "criatura desconocida que te interesa", C(CLR_RED)},	/* pink warning   */
	{'2', "criatura desconocida que te causa ansiedad", C(CLR_RED)},	/* red warning    */
	{'3', "criatura desconocida que te inquieta", C(CLR_RED)},	/* ruby warning   */
	{'4', "criatura desconocida que te alarma",
						C(CLR_MAGENTA)},        /* purple warning */
	{'5', "criatura desconocida que te asusta",
						C(CLR_BRIGHT_MAGENTA)}	/* black warning  */
};

/*
 *  Default screen symbols with explanations and colors.
 *  Note:  {ibm|dec|mac}_graphics[] arrays also depend on this symbol order.
 */
const struct symdef defsyms[MAXPCHARS] = {
/* 0*/	{' ', "parte oscura de un cuarto",C(NO_COLOR)},	/* stone */
	{'|', "pared",		C(CLR_GRAY)},	/* vwall */
	{'-', "pared",		C(CLR_GRAY)},	/* hwall */
	{'-', "pared",		C(CLR_GRAY)},	/* tlcorn */
	{'-', "pared",		C(CLR_GRAY)},	/* trcorn */
	{'-', "pared",		C(CLR_GRAY)},	/* blcorn */
	{'-', "pared",		C(CLR_GRAY)},	/* brcorn */
	{'-', "pared",		C(CLR_GRAY)},	/* crwall */
	{'-', "pared",		C(CLR_GRAY)},	/* tuwall */
	{'-', "pared",		C(CLR_GRAY)},	/* tdwall */
/*10*/	{'|', "pared",		C(CLR_GRAY)},	/* tlwall */
	{'|', "pared",		C(CLR_GRAY)},	/* trwall */
	{'.', "entrada",	C(CLR_GRAY)},	/* ndoor */
	{'-', "puerta abierta",	C(CLR_BROWN)},	/* vodoor */
	{'|', "puerta abierta",	C(CLR_BROWN)},	/* hodoor */
	{'+', "puerta cerrada",	C(CLR_BROWN)},	/* vcdoor */
	{'+', "puerta cerrada",	C(CLR_BROWN)},	/* hcdoor */
	{'#', "barras de hierro",	C(HI_METAL)},	/* bars */
	{'#', "árbol",		C(CLR_GREEN)},	/* tree */
	{'.', "piso de un cuarto",C(CLR_GRAY)},	/* room */
/*20*/	{'#', "corredor",	C(CLR_GRAY)},	/* dark corr */
	{'#', "corredor iliminado",	C(CLR_GRAY)},	/* lit corr (see mapglyph.c) */
	{'<', "escalera para arriba",	C(CLR_GRAY)},	/* upstair */
	{'>', "escalera para abajo",	C(CLR_GRAY)},	/* dnstair */
	{'<', "escalera de mano para arriba",	C(CLR_BROWN)},	/* upladder */
	{'>', "escalera de mano para abajo",	C(CLR_BROWN)},	/* dnladder */
	{'_', "altar",		C(CLR_GRAY)},	/* altar */
	{'|', "tumba",      C(CLR_GRAY)},   /* grave */
	{'\\', "trono opulento",C(HI_GOLD)},	/* throne */
#ifdef SINKS
	{'#', "fregadero",		C(CLR_GRAY)},	/* sink */
#else
	{'#', "",		C(CLR_GRAY)},	/* sink */
#endif
/*30*/	{'{', "fuente",	C(CLR_BLUE)},	/* fountain */
	{'}', "agua",		C(CLR_BLUE)},	/* pool */
	{'.', "hielo",		C(CLR_CYAN)},	/* ice */
	{'}', "lava fundida",	C(CLR_RED)},	/* lava */
	{'.', "puente levantizo bajado",C(CLR_BROWN)},	/* vodbridge */
	{'.', "puente levantizo bajado",C(CLR_BROWN)},	/* hodbridge */
	{'#', "puente levantizo levantado",C(CLR_BROWN)},/* vcdbridge */
	{'#', "puente levantizo levantado",C(CLR_BROWN)},/* hcdbridge */
	{' ', "aire",		C(CLR_CYAN)},	/* open air */
	{'#', "nube",		C(CLR_GRAY)},	/* [part of] a cloud */
/*40*/	{'}', "agua",		C(CLR_BLUE)},	/* under water */
	{'^', "trampa de flechas",	C(HI_METAL)},	/* trap */
	{'^', "trampa de dardos",	C(HI_METAL)},	/* trap */
	{'^', "trampa de rocas cayentes",C(CLR_GRAY)},	/* trap */
	{'^', "tablero rechinante",	C(CLR_BROWN)},	/* trap */
	{'^', "trampa para osos",	C(HI_METAL)},	/* trap */
	{'^', "mina terrestre",	C(CLR_RED)},	/* trap */
	{'^', "trampa de roca rodante",	C(CLR_GRAY)},	/* trap */
	{'^', "trampa de gas somnífero",C(HI_ZAP)},	/* trap */
	{'^', "trampa de óxido",	C(CLR_BLUE)},	/* trap */
/*50*/	{'^', "trampa de fuego",	C(CLR_ORANGE)},	/* trap */
	{'^', "hoyo",		C(CLR_BLACK)},	/* trap */
	{'^', "hoyo con púas",	C(CLR_BLACK)},	/* trap */
	{'^', "agujero",	C(CLR_BROWN)},	/* trap */
	{'^', "puerta trampa",	C(CLR_BROWN)},	/* trap */
	{'^', "trampa de teleportación", C(CLR_MAGENTA)},	/* trap */
	{'^', "teleportador de nivel", C(CLR_MAGENTA)},	/* trap */
	{'^', "portal mágico",	C(CLR_BRIGHT_MAGENTA)},	/* trap */
	{'"', "telaraña",		C(CLR_GRAY)},	/* web */
	{'^', "trampa de estatua",	C(CLR_GRAY)},	/* trap */
/*60*/	{'^', "trampa mágica",	C(HI_ZAP)},	/* trap */
	{'^', "campo antimágico", C(HI_ZAP)},	/* trap */
	{'^', "trampa de polimorfar",	C(CLR_BRIGHT_GREEN)},	/* trap */
	{'|', "pared",		C(CLR_GRAY)},	/* vbeam */
	{'-', "pared",		C(CLR_GRAY)},	/* hbeam */
	{'\\',"pared",		C(CLR_GRAY)},	/* lslant */
	{'/', "pared",		C(CLR_GRAY)},	/* rslant */
	{'*', "",		C(CLR_WHITE)},	/* dig beam */
	{'!', "",		C(CLR_WHITE)},	/* camera flash beam */
	{')', "",		C(HI_WOOD)},	/* boomerang open left */
/*70*/	{'(', "",		C(HI_WOOD)},	/* boomerang open right */
	{'0', "",		C(HI_ZAP)},	/* 4 magic shield symbols */
	{'#', "",		C(HI_ZAP)},
	{'@', "",		C(HI_ZAP)},
	{'*', "",		C(HI_ZAP)},
	{'/', "",		C(CLR_GREEN)},	/* swallow top left	*/
	{'-', "",		C(CLR_GREEN)},	/* swallow top center	*/
	{'\\', "",		C(CLR_GREEN)},	/* swallow top right	*/
	{'|', "",		C(CLR_GREEN)},	/* swallow middle left	*/
	{'|', "",		C(CLR_GREEN)},	/* swallow middle right	*/
/*80*/	{'\\', "",		C(CLR_GREEN)},	/* swallow bottom left	*/
	{'-', "",		C(CLR_GREEN)},	/* swallow bottom center*/
	{'/', "",		C(CLR_GREEN)},	/* swallow bottom right	*/
	{'/', "",		C(CLR_ORANGE)},	/* explosion top left     */
	{'-', "",		C(CLR_ORANGE)},	/* explosion top center   */
	{'\\', "",		C(CLR_ORANGE)},	/* explosion top right    */
	{'|', "",		C(CLR_ORANGE)},	/* explosion middle left  */
	{' ', "",		C(CLR_ORANGE)},	/* explosion middle center*/
	{'|', "",		C(CLR_ORANGE)},	/* explosion middle right */
	{'\\', "",		C(CLR_ORANGE)},	/* explosion bottom left  */
/*90*/	{'-', "",		C(CLR_ORANGE)},	/* explosion bottom center*/
	{'/', "",		C(CLR_ORANGE)},	/* explosion bottom right */
/*
 *  Note: Additions to this array should be reflected in the
 *	  {ibm,dec,mac}_graphics[] arrays below.
 */
};

#undef C

#ifdef ASCIIGRAPH

#ifdef PC9800
void NDECL((*ibmgraphics_mode_callback)) = 0;	/* set in tty_start_screen() */
#endif /* PC9800 */

/* LENGUA:  The font used by a Spanish-speaking user may sacrifice some
   graphics to accommodate the accented characters.  To allow more flexibility
   in choice of fonts, IBMgraphics is changed from a boolean to an integer from
   0 to 3, where:
        0 is no IBM graphics
        1 is box drawing only
        2 is box drawing plus a set of 38 symbols common to most PC code pages
        3 is all IBM graphics as implemented in the original Nethack

   ibm_graphics_1 and ibm_graphics_2 implement the reduced IBM graphics.
   ibm_graphics implements the original IBM graphics.
*/

static uchar ibm_graphics_1[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xb3,	/* S_vwall:	meta-3, vertical rule */
	0xc4,	/* S_hwall:	meta-D, horizontal rule */
	0xda,	/* S_tlcorn:	meta-Z, top left corner */
	0xbf,	/* S_trcorn:	meta-?, top right corner */
	0xc0,	/* S_blcorn:	meta-@, bottom left */
	0xd9,	/* S_brcorn:	meta-Y, bottom right */
	0xc5,	/* S_crwall:	meta-E, cross */
	0xc1,	/* S_tuwall:	meta-A, T up */
	0xc2,	/* S_tdwall:	meta-B, T down */
/*10*/	0xb4,	/* S_tlwall:	meta-4, T left */
	0xc3,	/* S_trwall:	meta-C, T right */
	g_FILLER(S_ndoor),
	g_FILLER(S_vodoor),
	g_FILLER(S_hodoor),
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	g_FILLER(S_bars),
	g_FILLER(S_tree),
	g_FILLER(S_room),
/*20*/	g_FILLER(S_corr),
	g_FILLER(S_litcorr),
	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_altar),
	g_FILLER(S_grave),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
/*30*/	g_FILLER(S_fountain),
	g_FILLER(S_pool),
	g_FILLER(S_ice),
	g_FILLER(S_lava),
	g_FILLER(S_vodbridge),
	g_FILLER(S_hodbridge),
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
/*40*/	g_FILLER(S_water),
	g_FILLER(S_arrow_trap),
	g_FILLER(S_dart_trap),
	g_FILLER(S_falling_rock_trap),
	g_FILLER(S_squeaky_board),
	g_FILLER(S_bear_trap),
	g_FILLER(S_land_mine),
	g_FILLER(S_rolling_boulder_trap),
	g_FILLER(S_sleeping_gas_trap),
	g_FILLER(S_rust_trap),
/*50*/	g_FILLER(S_fire_trap),
	g_FILLER(S_pit),
	g_FILLER(S_spiked_pit),
	g_FILLER(S_hole),
	g_FILLER(S_trap_door),
	g_FILLER(S_teleportation_trap),
	g_FILLER(S_level_teleporter),
	g_FILLER(S_magic_portal),
	g_FILLER(S_web),
	g_FILLER(S_statue_trap),
/*60*/	g_FILLER(S_magic_trap),
	g_FILLER(S_anti_magic_trap),
	g_FILLER(S_polymorph_trap),
	0xb3,	/* S_vbeam:	meta-3, vertical rule */
	0xc4,	/* S_hbeam:	meta-D, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
/*70*/	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	0xb3,	/* S_sw_ml:	meta-3, vertical rule */
	0xb3,	/* S_sw_mr:	meta-3, vertical rule */
/*80*/	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	0xb3,	/* S_explode4:	meta-3, vertical rule */
	g_FILLER(S_explode5),
	0xb3,	/* S_explode6:	meta-3, vertical rule */
	g_FILLER(S_explode7),
/*90*/	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};

static uchar ibm_graphics_2[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xb3,	/* S_vwall:	meta-3, vertical rule */
	0xc4,	/* S_hwall:	meta-D, horizontal rule */
	0xda,	/* S_tlcorn:	meta-Z, top left corner */
	0xbf,	/* S_trcorn:	meta-?, top right corner */
	0xc0,	/* S_blcorn:	meta-@, bottom left */
	0xd9,	/* S_brcorn:	meta-Y, bottom right */
	0xc5,	/* S_crwall:	meta-E, cross */
	0xc1,	/* S_tuwall:	meta-A, T up */
	0xc2,	/* S_tdwall:	meta-B, T down */
/*10*/	0xb4,	/* S_tlwall:	meta-4, T left */
	0xc3,	/* S_trwall:	meta-C, T right */
	g_FILLER(S_ndoor),
	0xfe,	/* S_vodoor:	meta-~, small centered square */
	0xfe,	/* S_hodoor:	meta-~, small centered square */
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	g_FILLER(S_bars),
	g_FILLER(S_tree),
	g_FILLER(S_room),
/*20*/	0xb0,	/* S_corr:	meta-0, light shading */
	0xb1,	/* S_litcorr:	meta-1, medium shading */
	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_altar),
	g_FILLER(S_grave),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
/*30*/	g_FILLER(S_fountain),
	g_FILLER(S_pool),
	g_FILLER(S_ice),
	g_FILLER(S_lava),
	g_FILLER(S_vodbridge),
	g_FILLER(S_hodbridge),
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
/*40*/	g_FILLER(S_water),
	g_FILLER(S_arrow_trap),
	g_FILLER(S_dart_trap),
	g_FILLER(S_falling_rock_trap),
	g_FILLER(S_squeaky_board),
	g_FILLER(S_bear_trap),
	g_FILLER(S_land_mine),
	g_FILLER(S_rolling_boulder_trap),
	g_FILLER(S_sleeping_gas_trap),
	g_FILLER(S_rust_trap),
/*50*/	g_FILLER(S_fire_trap),
	g_FILLER(S_pit),
	g_FILLER(S_spiked_pit),
	g_FILLER(S_hole),
	g_FILLER(S_trap_door),
	g_FILLER(S_teleportation_trap),
	g_FILLER(S_level_teleporter),
	g_FILLER(S_magic_portal),
	g_FILLER(S_web),
	g_FILLER(S_statue_trap),
/*60*/	g_FILLER(S_magic_trap),
	g_FILLER(S_anti_magic_trap),
	g_FILLER(S_polymorph_trap),
	0xb3,	/* S_vbeam:	meta-3, vertical rule */
	0xc4,	/* S_hbeam:	meta-D, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
/*70*/	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	0xb3,	/* S_sw_ml:	meta-3, vertical rule */
	0xb3,	/* S_sw_mr:	meta-3, vertical rule */
/*80*/	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	0xb3,	/* S_explode4:	meta-3, vertical rule */
	g_FILLER(S_explode5),
	0xb3,	/* S_explode6:	meta-3, vertical rule */
	g_FILLER(S_explode7),
/*90*/	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};

static uchar ibm_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xb3,	/* S_vwall:	meta-3, vertical rule */
	0xc4,	/* S_hwall:	meta-D, horizontal rule */
	0xda,	/* S_tlcorn:	meta-Z, top left corner */
	0xbf,	/* S_trcorn:	meta-?, top right corner */
	0xc0,	/* S_blcorn:	meta-@, bottom left */
	0xd9,	/* S_brcorn:	meta-Y, bottom right */
	0xc5,	/* S_crwall:	meta-E, cross */
	0xc1,	/* S_tuwall:	meta-A, T up */
	0xc2,	/* S_tdwall:	meta-B, T down */
/*10*/	0xb4,	/* S_tlwall:	meta-4, T left */
	0xc3,	/* S_trwall:	meta-C, T right */
	0xfa,	/* S_ndoor:	meta-z, centered dot */
	0xfe,	/* S_vodoor:	meta-~, small centered square */
	0xfe,	/* S_hodoor:	meta-~, small centered square */
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	240,	/* S_bars:	equivalence symbol */
	241,	/* S_tree:	plus or minus symbol */
	0xfa,	/* S_room:	meta-z, centered dot */
/*20*/	0xb0,	/* S_corr:	meta-0, light shading */
	0xb1,	/* S_litcorr:	meta-1, medium shading */
	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_altar),
	g_FILLER(S_grave),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
/*30*/	0xf4,	/* S_fountain:	meta-t, integral top half */
	0xf7,	/* S_pool:	meta-w, approx. equals */
	0xfa,	/* S_ice:	meta-z, centered dot */
	0xf7,	/* S_lava:	meta-w, approx. equals */
	0xfa,	/* S_vodbridge:	meta-z, centered dot */
	0xfa,	/* S_hodbridge:	meta-z, centered dot */
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
/*40*/	0xf7,	/* S_water:	meta-w, approx. equals */
	g_FILLER(S_arrow_trap),
	g_FILLER(S_dart_trap),
	g_FILLER(S_falling_rock_trap),
	g_FILLER(S_squeaky_board),
	g_FILLER(S_bear_trap),
	g_FILLER(S_land_mine),
	g_FILLER(S_rolling_boulder_trap),
	g_FILLER(S_sleeping_gas_trap),
	g_FILLER(S_rust_trap),
/*50*/	g_FILLER(S_fire_trap),
	g_FILLER(S_pit),
	g_FILLER(S_spiked_pit),
	g_FILLER(S_hole),
	g_FILLER(S_trap_door),
	g_FILLER(S_teleportation_trap),
	g_FILLER(S_level_teleporter),
	g_FILLER(S_magic_portal),
	g_FILLER(S_web),
	g_FILLER(S_statue_trap),
/*60*/	g_FILLER(S_magic_trap),
	g_FILLER(S_anti_magic_trap),
	g_FILLER(S_polymorph_trap),
	0xb3,	/* S_vbeam:	meta-3, vertical rule */
	0xc4,	/* S_hbeam:	meta-D, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
/*70*/	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	0xb3,	/* S_sw_ml:	meta-3, vertical rule */
	0xb3,	/* S_sw_mr:	meta-3, vertical rule */
/*80*/	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	0xb3,	/* S_explode4:	meta-3, vertical rule */
	g_FILLER(S_explode5),
	0xb3,	/* S_explode6:	meta-3, vertical rule */
	g_FILLER(S_explode7),
/*90*/	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};
#endif  /* ASCIIGRAPH */

#ifdef TERMLIB
void NDECL((*decgraphics_mode_callback)) = 0;  /* set in tty_start_screen() */

static uchar dec_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xf8,	/* S_vwall:	meta-x, vertical rule */
	0xf1,	/* S_hwall:	meta-q, horizontal rule */
	0xec,	/* S_tlcorn:	meta-l, top left corner */
	0xeb,	/* S_trcorn:	meta-k, top right corner */
	0xed,	/* S_blcorn:	meta-m, bottom left */
	0xea,	/* S_brcorn:	meta-j, bottom right */
	0xee,	/* S_crwall:	meta-n, cross */
	0xf6,	/* S_tuwall:	meta-v, T up */
	0xf7,	/* S_tdwall:	meta-w, T down */
/*10*/	0xf5,	/* S_tlwall:	meta-u, T left */
	0xf4,	/* S_trwall:	meta-t, T right */
	0xfe,	/* S_ndoor:	meta-~, centered dot */
	0xe1,	/* S_vodoor:	meta-a, solid block */
	0xe1,	/* S_hodoor:	meta-a, solid block */
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	0xfb,	/* S_bars:	meta-{, small pi */
	0xe7,	/* S_tree:	meta-g, plus-or-minus */
	0xfe,	/* S_room:	meta-~, centered dot */
/*20*/	g_FILLER(S_corr),
	g_FILLER(S_litcorr),
	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	0xf9,	/* S_upladder:	meta-y, greater-than-or-equals */
	0xfa,	/* S_dnladder:	meta-z, less-than-or-equals */
	g_FILLER(S_altar),	/* 0xc3, \E)3: meta-C, dagger */
	g_FILLER(S_grave),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
/*30*/	g_FILLER(S_fountain),	/* 0xdb, \E)3: meta-[, integral top half */
	0xe0,	/* S_pool:	meta-\, diamond */
	0xfe,	/* S_ice:	meta-~, centered dot */
	0xe0,	/* S_lava:	meta-\, diamond */
	0xfe,	/* S_vodbridge:	meta-~, centered dot */
	0xfe,	/* S_hodbridge:	meta-~, centered dot */
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
/*40*/	0xe0,	/* S_water:	meta-\, diamond */
	g_FILLER(S_arrow_trap),
	g_FILLER(S_dart_trap),
	g_FILLER(S_falling_rock_trap),
	g_FILLER(S_squeaky_board),
	g_FILLER(S_bear_trap),
	g_FILLER(S_land_mine),
	g_FILLER(S_rolling_boulder_trap),
	g_FILLER(S_sleeping_gas_trap),
	g_FILLER(S_rust_trap),
/*50*/	g_FILLER(S_fire_trap),
	g_FILLER(S_pit),
	g_FILLER(S_spiked_pit),
	g_FILLER(S_hole),
	g_FILLER(S_trap_door),
	g_FILLER(S_teleportation_trap),
	g_FILLER(S_level_teleporter),
	g_FILLER(S_magic_portal),
	g_FILLER(S_web),	/* 0xbd, \E)3: meta-=, int'l currency */
	g_FILLER(S_statue_trap),
/*60*/	g_FILLER(S_magic_trap),
	g_FILLER(S_anti_magic_trap),
	g_FILLER(S_polymorph_trap),
	0xf8,	/* S_vbeam:	meta-x, vertical rule */
	0xf1,	/* S_hbeam:	meta-q, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
/*70*/	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	0xef,	/* S_sw_tc:	meta-o, high horizontal line */
	g_FILLER(S_sw_tr),
	0xf8,	/* S_sw_ml:	meta-x, vertical rule */
	0xf8,	/* S_sw_mr:	meta-x, vertical rule */
/*80*/	g_FILLER(S_sw_bl),
	0xf3,	/* S_sw_bc:	meta-s, low horizontal line */
	g_FILLER(S_sw_br),
	g_FILLER(S_explode1),
	0xef,	/* S_explode2:	meta-o, high horizontal line */
	g_FILLER(S_explode3),
	0xf8,	/* S_explode4:	meta-x, vertical rule */
	g_FILLER(S_explode5),
	0xf8,	/* S_explode6:	meta-x, vertical rule */
	g_FILLER(S_explode7),
/*90*/	0xf3,	/* S_explode8:	meta-s, low horizontal line */
	g_FILLER(S_explode9)
};
#endif  /* TERMLIB */

#ifdef MAC_GRAPHICS_ENV
static uchar mac_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xba,	/* S_vwall */
	0xcd,	/* S_hwall */
	0xc9,	/* S_tlcorn */
	0xbb,	/* S_trcorn */
	0xc8,	/* S_blcorn */
	0xbc,	/* S_brcorn */
	0xce,	/* S_crwall */
	0xca,	/* S_tuwall */
	0xcb,	/* S_tdwall */
/*10*/	0xb9,	/* S_tlwall */
	0xcc,	/* S_trwall */
	0xb0,	/* S_ndoor */
	0xee,	/* S_vodoor */
	0xee,	/* S_hodoor */
	0xef,	/* S_vcdoor */
	0xef,	/* S_hcdoor */
	0xf0,	/* S_bars:	equivalency symbol */
	0xf1,	/* S_tree:	plus-or-minus */
	g_FILLER(S_Room),
/*20*/	0xB0,	/* S_corr */
	g_FILLER(S_litcorr),
	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_altar),
	0xef,	/* S_grave:	same as open door */
	g_FILLER(S_throne),
	g_FILLER(S_sink),
/*30*/	g_FILLER(S_fountain),
	0xe0,	/* S_pool */
	g_FILLER(S_ice),
	g_FILLER(S_lava),
	g_FILLER(S_vodbridge),
	g_FILLER(S_hodbridge),
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
/*40*/	g_FILLER(S_water),
	g_FILLER(S_arrow_trap),
	g_FILLER(S_dart_trap),
	g_FILLER(S_falling_rock_trap),
	g_FILLER(S_squeaky_board),
	g_FILLER(S_bear_trap),
	g_FILLER(S_land_mine),
	g_FILLER(S_rolling_boulder_trap),
	g_FILLER(S_sleeping_gas_trap),
	g_FILLER(S_rust_trap),
/*50*/	g_FILLER(S_fire_trap),
	g_FILLER(S_pit),
	g_FILLER(S_spiked_pit),
	g_FILLER(S_hole),
	g_FILLER(S_trap_door),
	g_FILLER(S_teleportation_trap),
	g_FILLER(S_level_teleporter),
	g_FILLER(S_magic_portal),
	g_FILLER(S_web),
	g_FILLER(S_statue_trap),
/*60*/	g_FILLER(S_magic_trap),
	g_FILLER(S_anti_magic_trap),
	g_FILLER(S_polymorph_trap),
	g_FILLER(S_vbeam),
	g_FILLER(S_hbeam),
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
/*70*/	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	g_FILLER(S_sw_ml),
	g_FILLER(S_sw_mr),
/*80*/	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	g_FILLER(S_explode4),
	g_FILLER(S_explode5),
	g_FILLER(S_explode6),
	g_FILLER(S_explode7),
/*90*/	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};
#endif	/* MAC_GRAPHICS_ENV */

#ifdef PC9800
void NDECL((*ascgraphics_mode_callback)) = 0;	/* set in tty_start_screen() */
#endif

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in detect.c invent.c,
 * options.c, pickup.c, sp_lev.c, and lev_main.c.
 */
int
def_char_to_objclass(ch)
    char ch;
{
    int i;
    for (i = 1; i < MAXOCLASSES; i++)
	if (ch == def_oc_syms[i]) break;
    return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 */
int
def_char_to_monclass(ch)
    char ch;
{
    int i;
    for (i = 1; i < MAXMCLASSES; i++)
	if (def_monsyms[i] == ch) break;
    return i;
}

void
assign_graphics(graph_chars, glth, maxlen, offset)
register uchar *graph_chars;
int glth, maxlen, offset;
{
    register int i;

    for (i = 0; i < maxlen; i++)
	showsyms[i+offset] = (((i < glth) && graph_chars[i]) ?
		       graph_chars[i] : defsyms[i+offset].sym);
}

void
switch_graphics(gr_set_flag, level)
int gr_set_flag;
int level;
{
    if (gr_set_flag==IBM_GRAPHICS && level<=0)
	gr_set_flag = ASCII_GRAPHICS;
    switch (gr_set_flag) {
	default:
	case ASCII_GRAPHICS:
	    iflags.IBMgraphics = 0;
	    iflags.DECgraphics = FALSE;
	    assign_graphics((uchar *)0, 0, MAXPCHARS, 0);
#ifdef PC9800
	    if (ascgraphics_mode_callback) (*ascgraphics_mode_callback)();
#endif
	    break;
#ifdef ASCIIGRAPH
	case IBM_GRAPHICS:
/*
 * Use the nice IBM Extended ASCII line-drawing characters (codepage 437).
 *
 * OS/2 defaults to a multilingual character set (codepage 850, corresponding
 * to the ISO 8859 character set.  We should probably do a VioSetCp() call to
 * set the codepage to 437.
 */
	    if (level < 0) level = 0;
	    if (level > 3) level = 3;
	    iflags.IBMgraphics = level;
	    iflags.DECgraphics = FALSE;
	    if (level == 1)
		assign_graphics(ibm_graphics_1, SIZE(ibm_graphics_1), MAXPCHARS, 0);
	    else if (level == 2)
		assign_graphics(ibm_graphics_2, SIZE(ibm_graphics_2), MAXPCHARS, 0);
	    else
		assign_graphics(ibm_graphics, SIZE(ibm_graphics), MAXPCHARS, 0);
#ifdef PC9800
	    if (ibmgraphics_mode_callback) (*ibmgraphics_mode_callback)();
#endif
	    break;
#endif /* ASCIIGRAPH */
#ifdef TERMLIB
	case DEC_GRAPHICS:
/*
 * Use the VT100 line drawing character set.
 */
	    iflags.DECgraphics = TRUE;
	    iflags.IBMgraphics = FALSE;
	    assign_graphics(dec_graphics, SIZE(dec_graphics), MAXPCHARS, 0);
	    if (decgraphics_mode_callback) (*decgraphics_mode_callback)();
	    break;
#endif /* TERMLIB */
#ifdef MAC_GRAPHICS_ENV
	case MAC_GRAPHICS:
	    assign_graphics(mac_graphics, SIZE(mac_graphics), MAXPCHARS, 0);
	    break;
#endif
	}
    return;
}


#ifdef REINCARNATION

/*
 * saved display symbols for objects & features.
 */
static uchar save_oc_syms[MAXOCLASSES] = DUMMY;
static uchar save_showsyms[MAXPCHARS]  = DUMMY;
static uchar save_monsyms[MAXPCHARS]   = DUMMY;

static const uchar r_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',
	ILLOBJ_SYM,
	WEAPON_SYM,
	']',			/* armor */
	RING_SYM,
/* 5*/	',',			/* amulet */
	TOOL_SYM,
	':',			/* food */
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GEM_SYM,		/* gold -- yes it's the same as gems */
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

# ifdef ASCIIGRAPH
/* Rogue level graphics.  Under IBM graphics mode, use the symbols that were
 * used for Rogue on the IBM PC.  Unfortunately, this can't be completely
 * done because some of these are control characters--armor and rings under
 * DOS, and a whole bunch of them under Linux.  Use the TTY Rogue characters
 * for those cases.
 */
static const uchar IBM_r_oc_syms[MAXOCLASSES] = {	/* a la EPYX Rogue */
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x18,			/* weapon: up arrow */
/*	0x0a, */ ']',		/* armor:  Vert rect with o */
/*	0x09, */ RING_SYM,	/* ring:   circle with arrow */
/* 5*/	0x0c,			/* amulet: "female" symbol */
	TOOL_SYM,
	0x05,			/* food:   club (as in cards) */
	0xad,			/* potion: upside down '!' */
	0x0e,			/* scroll: musical note */
/*10*/	SPBOOK_SYM,
	0xe7,			/* wand:   greek tau */
	0x0f,			/* gold:   yes it's the same as gems */
	0x0f,			/* gems:   fancy '*' */
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

static const uchar IBM2_r_oc_syms[MAXOCLASSES] = {	/* a la EPYX Rogue */
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x18,			/* weapon: up arrow */
/*	0x0a, */ ']',		/* armor:  Vert rect with o */
/*	0x09, */ RING_SYM,	/* ring:   circle with arrow */
/* 5*/	0x0c,			/* amulet: "female" symbol */
	TOOL_SYM,
	0x05,			/* food:   club (as in cards) */
	POTION_SYM,
	0x0e,			/* scroll: musical note */
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	0x0f,			/* gold:   yes it's the same as gems */
	0x0f,			/* gems:   fancy '*' */
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

#  if !defined(MSDOS) && !defined(OS2) && !defined(WIN32)
static const uchar linux_r_oc_syms[MAXOCLASSES] = {	/* a la EPYX Rogue */
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x18,			/* weapon: up arrow */
	']',			/* armor */
	RING_SYM,		/* ring */
/* 5*/	',',			/* amulet  */
	TOOL_SYM,
	0x05,			/* food:   club (as in cards) */
	0xad,			/* potion: upside down '!' */
	SCROLL_SYM,		/* scroll  */
/*10*/	SPBOOK_SYM,
	0xe7,			/* wand:   greek tau */
	GEM_SYM,		/* gold:   yes it's the same as gems */
	GEM_SYM,		/* gems    */
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};
static const uchar linux2_r_oc_syms[MAXOCLASSES] = {	/* a la EPYX Rogue */
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x18,			/* weapon: up arrow */
	']',			/* armor */
	RING_SYM,		/* ring */
/* 5*/	',',			/* amulet  */
	TOOL_SYM,
	0x05,			/* food:   club (as in cards) */
	POTION_SYM,		/* potion */
	SCROLL_SYM,		/* scroll  */
/*10*/	SPBOOK_SYM,
	WAND_SYM,		/* wand */
	GEM_SYM,		/* gold:   yes it's the same as gems */
	GEM_SYM,		/* gems    */
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};
#  endif
# endif /* ASCIIGRAPH */

void
assign_rogue_graphics(is_rlevel)
boolean is_rlevel;
{
    /* Adjust graphics display characters on Rogue levels */

    if (is_rlevel) {
	register int i;
	const uchar *symbols;

	(void) memcpy((genericptr_t)save_showsyms,
		      (genericptr_t)showsyms, sizeof showsyms);
	(void) memcpy((genericptr_t)save_oc_syms,
		      (genericptr_t)oc_syms, sizeof oc_syms);
	(void) memcpy((genericptr_t)save_monsyms,
		      (genericptr_t)monsyms, sizeof monsyms);

	/* Use a loop: char != uchar on some machines. */
	for (i = 0; i < MAXMCLASSES; i++)
	    monsyms[i] = def_monsyms[i];
# if defined(ASCIIGRAPH) && !defined(MSWIN_GRAPHICS)
	if (iflags.IBMgraphics >= 2
#  if defined(USE_TILES) && defined(MSDOS)
		&& !iflags.grmode
#  endif
		)
	    monsyms[S_HUMAN] = 0x01; /* smiley face */
# endif
	for (i = 0; i < MAXPCHARS; i++)
	    showsyms[i] = defsyms[i].sym;

/*
 * Some day if these rogue showsyms get much more extensive than this,
 * we may want to create r_showsyms, and IBM_r_showsyms arrays to hold
 * all of this info and to simply initialize it via a for() loop like r_oc_syms.
 */

	showsyms[S_vodoor]  = showsyms[S_hodoor]  = showsyms[S_ndoor] = '+';
	showsyms[S_upstair] = showsyms[S_dnstair] = '%';
# ifdef ASCIIGRAPH
	if (iflags.IBMgraphics
#  if defined(USE_TILES) && defined(MSDOS)
		&& !iflags.grmode
#  endif
				) {
	    /* a la EPYX Rogue */
	    showsyms[S_vwall]   = 0xba; /* all walls now use	*/
	    showsyms[S_hwall]   = 0xcd; /* double line graphics	*/
	    showsyms[S_tlcorn]  = 0xc9;
	    showsyms[S_trcorn]  = 0xbb;
	    showsyms[S_blcorn]  = 0xc8;
	    showsyms[S_brcorn]  = 0xbc;
	    showsyms[S_crwall]  = 0xce;
	    showsyms[S_tuwall]  = 0xca;
	    showsyms[S_tdwall]  = 0xcb;
	    showsyms[S_tlwall]  = 0xb9;
	    showsyms[S_trwall]  = 0xcc;
	    showsyms[S_ndoor]   = 0xce;
	    showsyms[S_vodoor]  = 0xce;
	    showsyms[S_hodoor]  = 0xce;
	    if (iflags.IBMgraphics >= 2) {
		showsyms[S_corr]    = 0xb1;
		showsyms[S_litcorr] = 0xb2;
	    }
	    if (iflags.IBMgraphics >= 3) {
		showsyms[S_room]    = 0xfa; /* centered dot */
		showsyms[S_upstair] = 0xf0; /* Greek Xi */
		showsyms[S_dnstair] = 0xf0;
	    }

#ifndef MSWIN_GRAPHICS
	    if (iflags.IBMgraphics >= 2) {
		showsyms[S_arrow_trap] = 0x04; /* diamond (cards) */
		showsyms[S_dart_trap] = 0x04;
		showsyms[S_falling_rock_trap] = 0x04;
		showsyms[S_squeaky_board] = 0x04;
		showsyms[S_bear_trap] = 0x04;
		showsyms[S_land_mine] = 0x04;
		showsyms[S_rolling_boulder_trap] = 0x04;
		showsyms[S_sleeping_gas_trap] = 0x04;
		showsyms[S_rust_trap] = 0x04;
		showsyms[S_fire_trap] = 0x04;
		showsyms[S_pit] = 0x04;
		showsyms[S_spiked_pit] = 0x04;
		showsyms[S_hole] = 0x04;
		showsyms[S_trap_door] = 0x04;
		showsyms[S_teleportation_trap] = 0x04;
		showsyms[S_level_teleporter] = 0x04;
		showsyms[S_magic_portal] = 0x04;
		showsyms[S_web] = 0x04;
		showsyms[S_statue_trap] = 0x04;
		showsyms[S_magic_trap] = 0x04;
		showsyms[S_anti_magic_trap] = 0x04;
		showsyms[S_polymorph_trap] = 0x04;
	    }
#endif
	}
#endif /* ASCIIGRAPH */

	symbols = r_oc_syms;
#ifdef ASCIIGRAPH
# if defined(USE_TILES) && defined(MSDOS)
	if (!iflags.grmode)
# endif
	{
	    if (iflags.IBMgraphics == 2)
		symbols = IBM2_r_oc_syms;
	    else if (iflags.IBMgraphics >= 3)
		symbols = IBM_r_oc_syms;
# if !defined(MSDOS) && !defined(OS2) && !defined(WIN32)
	    if (!iflags.unicode) {
		if (iflags.IBMgraphics == 2)
		    symbols = linux2_r_oc_syms;
		else if (iflags.IBMgraphics == 3)
		    symbols = linux_r_oc_syms;
	    }
# endif
	}
#endif
	for (i = 0; i < MAXOCLASSES; i++) {
	    oc_syms[i] = symbols[i];
#if defined(ASCIIGRAPH) && !(defined(MSDOS) || defined(WIN32) || defined(OS2))
	    if (iflags.unicode && iflags.IBMgraphics>=2) {
		if (oc_syms[i] == ']')
		    oc_syms[i] = 0x0A;
		if (oc_syms[i] == RING_SYM)
		    oc_syms[i] = 0x09;
	    }
#endif
	}
#if defined(MSDOS) && defined(USE_TILES)
	if (iflags.grmode) tileview(FALSE);
#endif
    } else {
	(void) memcpy((genericptr_t)showsyms,
		      (genericptr_t)save_showsyms, sizeof showsyms);
	(void) memcpy((genericptr_t)oc_syms,
		      (genericptr_t)save_oc_syms, sizeof oc_syms);
	(void) memcpy((genericptr_t)monsyms,
		      (genericptr_t)save_monsyms, sizeof monsyms);
#if defined(MSDOS) && defined(USE_TILES)
	if (iflags.grmode) tileview(TRUE);
#endif
    }
}
#endif /* REINCARNATION */

/*drawing.c*/
