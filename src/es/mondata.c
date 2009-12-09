/*	SCCS Id: @(#)mondata.c	3.4	2003/06/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"
#include "epri.h"

/*	These routines provide basic data for any type of monster. */

#ifdef OVLB

void
set_mon_data(mon, ptr, flag)
struct monst *mon;
struct permonst *ptr;
int flag;
{
    mon->data = ptr;
    if (flag == -1) return;		/* "don't care" */

    if (flag == 1)
	mon->mintrinsics |= (ptr->mresists & 0x00FF);
    else
	mon->mintrinsics = (ptr->mresists & 0x00FF);
    return;
}

#endif /* OVLB */
#ifdef OVL0

struct attack *
attacktype_fordmg(ptr, atyp, dtyp)
struct permonst *ptr;
int atyp, dtyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
	if (a->aatyp == atyp && (dtyp == AD_ANY || a->adtyp == dtyp))
	    return a;

    return (struct attack *)0;
}

boolean
attacktype(ptr, atyp)
struct permonst *ptr;
int atyp;
{
    return attacktype_fordmg(ptr, atyp, AD_ANY) ? TRUE : FALSE;
}

#endif /* OVL0 */
#ifdef OVLB

boolean
poly_when_stoned(ptr)
    struct permonst *ptr;
{
    return((boolean)(is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM] &&
	    !(mvitals[PM_STONE_GOLEM].mvflags & G_GENOD)));
	    /* allow G_EXTINCT */
}

boolean
resists_drli(mon)	/* returns TRUE if monster is drain-life resistant */
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	struct obj *wep = ((mon == &youmonst) ? uwep : MON_WEP(mon));

	return (boolean)(is_undead(ptr) || is_demon(ptr) || is_were(ptr) ||
			 ptr == &mons[PM_DEATH] ||
			 (wep && wep->oartifact && defends(AD_DRLI, wep)));
}

boolean
resists_magm(mon)	/* TRUE if monster is magic-missile resistant */
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	struct obj *o;

	/* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu */
	if (dmgtype(ptr, AD_MAGM) || ptr == &mons[PM_BABY_GRAY_DRAGON] ||
		dmgtype(ptr, AD_RBRE))	/* Chromatic Dragon */
	    return TRUE;
	/* check for magic resistance granted by wielded weapon */
	o = (mon == &youmonst) ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_MAGM, o))
	    return TRUE;
	/* check for magic resistance granted by worn or carried items */
	o = (mon == &youmonst) ? invent : mon->minvent;
	for ( ; o; o = o->nobj)
	    if ((o->owornmask && objects[o->otyp].oc_oprop == ANTIMAGIC) ||
		    (o->oartifact && protects(AD_MAGM, o)))
		return TRUE;
	return FALSE;
}

/* TRUE iff monster is resistant to light-induced blindness */
boolean
resists_blnd(mon)
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	boolean is_you = (mon == &youmonst);
	struct obj *o;

	if (is_you ? (Blind || u.usleep) :
		(mon->mblinded || !mon->mcansee || !haseyes(ptr) ||
		    /* BUG: temporary sleep sets mfrozen, but since
			    paralysis does too, we can't check it */
		    mon->msleeping))
	    return TRUE;
	/* yellow light, Archon; !dust vortex, !cobra, !raven */
	if (dmgtype_fromattack(ptr, AD_BLND, AT_EXPL) ||
		dmgtype_fromattack(ptr, AD_BLND, AT_GAZE))
	    return TRUE;
	o = is_you ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_BLND, o))
	    return TRUE;
	o = is_you ? invent : mon->minvent;
	for ( ; o; o = o->nobj)
	    if ((o->owornmask && objects[o->otyp].oc_oprop == BLINDED) ||
		    (o->oartifact && protects(AD_BLND, o)))
		return TRUE;
	return FALSE;
}

/* TRUE iff monster can be blinded by the given attack */
/* Note: may return TRUE when mdef is blind (e.g. new cream-pie attack) */
boolean
can_blnd(magr, mdef, aatyp, obj)
struct monst *magr;		/* NULL == no specific aggressor */
struct monst *mdef;
uchar aatyp;
struct obj *obj;		/* aatyp == AT_WEAP, AT_SPIT */
{
	boolean is_you = (mdef == &youmonst);
	boolean check_visor = FALSE;
	struct obj *o;
	const char *s;

	/* no eyes protect against all attacks for now */
	if (!haseyes(mdef->data))
	    return FALSE;

	switch(aatyp) {
	case AT_EXPL: case AT_BOOM: case AT_GAZE: case AT_MAGC:
	case AT_BREA: /* assumed to be lightning */
	    /* light-based attacks may be cancelled or resisted */
	    if (magr && magr->mcan)
		return FALSE;
	    return !resists_blnd(mdef);

	case AT_WEAP: case AT_SPIT: case AT_NONE:
	    /* an object is used (thrown/spit/other) */
	    if (obj && (obj->otyp == CREAM_PIE)) {
		if (is_you && Blindfolded)
		    return FALSE;
	    } else if (obj && (obj->otyp == BLINDING_VENOM)) {
		/* all ublindf, including LENSES, protect, cream-pies too */
		if (is_you && (ublindf || u.ucreamed))
		    return FALSE;
		check_visor = TRUE;
	    } else if (obj && (obj->otyp == POT_BLINDNESS)) {
		return TRUE;	/* no defense */
	    } else
		return FALSE;	/* other objects cannot cause blindness yet */
	    if ((magr == &youmonst) && u.uswallow)
		return FALSE;	/* can't affect eyes while inside monster */
	    break;

	case AT_ENGL:
	    if (is_you && (Blindfolded || u.usleep || u.ucreamed))
		return FALSE;
	    if (!is_you && mdef->msleeping)
		return FALSE;
	    break;

	case AT_CLAW:
	    /* e.g. raven: all ublindf, including LENSES, protect */
	    if (is_you && ublindf)
		return FALSE;
	    if ((magr == &youmonst) && u.uswallow)
		return FALSE;	/* can't affect eyes while inside monster */
	    check_visor = TRUE;
	    break;

	case AT_TUCH: case AT_STNG:
	    /* some physical, blind-inducing attacks can be cancelled */
	    if (magr && magr->mcan)
		return FALSE;
	    break;

	default:
	    break;
	}

	/* check if wearing a visor (only checked if visor might help) */
	if (check_visor) {
	    o = (mdef == &youmonst) ? invent : mdef->minvent;
	    for ( ; o; o = o->nobj)
		if ((o->owornmask & W_ARMH) &&
		    (s = OBJ_DESCR(objects[o->otyp])) != (char *)0 &&
		    !strcmp(s, "yelmo viserado"))
		    return FALSE;
	}

	return TRUE;
}

#endif /* OVLB */
#ifdef OVL0

boolean
ranged_attk(ptr)	/* returns TRUE if monster can attack at range */
struct permonst *ptr;
{
	register int i, atyp;
	long atk_mask = (1L << AT_BREA) | (1L << AT_SPIT) | (1L << AT_GAZE);

	/* was: (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP) ||
		attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE) ||
		attacktype(ptr, AT_MAGC));
	   but that's too slow -dlc
	 */
	for (i = 0; i < NATTK; i++) {
	    atyp = ptr->mattk[i].aatyp;
	    if (atyp >= AT_WEAP) return TRUE;
	 /* assert(atyp < 32); */
	    if ((atk_mask & (1L << atyp)) != 0L) return TRUE;
	}

	return FALSE;
}

boolean
hates_silver(ptr)
register struct permonst *ptr;
/* returns TRUE if monster is especially affected by silver weapons */
{
	return((boolean)(is_were(ptr) || ptr->mlet==S_VAMPIRE || is_demon(ptr) ||
		ptr == &mons[PM_SHADE] ||
		(ptr->mlet==S_IMP && ptr != &mons[PM_TENGU])));
}

/* true iff the type of monster pass through iron bars */
boolean
passes_bars(mptr)
struct permonst *mptr;
{
    return (boolean) (passes_walls(mptr) || amorphous(mptr) ||
		      is_whirly(mptr) || verysmall(mptr) ||
		      (slithy(mptr) && !bigmonst(mptr)));
}

#endif /* OVL0 */
#ifdef OVL1

boolean
can_track(ptr)		/* returns TRUE if monster can track well */
	register struct permonst *ptr;
{
	if (uwep && uwep->oartifact == ART_EXCALIBUR)
		return TRUE;
	else
		return((boolean)haseyes(ptr));
}

#endif /* OVL1 */
#ifdef OVLB

boolean
sliparm(ptr)	/* creature will slide out of armor */
	register struct permonst *ptr;
{
	return((boolean)(is_whirly(ptr) || ptr->msize <= MZ_SMALL ||
			 noncorporeal(ptr)));
}

boolean
breakarm(ptr)	/* creature will break out of armor */
	register struct permonst *ptr;
{
	return ((bigmonst(ptr) || (ptr->msize > MZ_SMALL && !humanoid(ptr)) ||
		/* special cases of humanoids that cannot wear body armor */
		ptr == &mons[PM_MARILITH] || ptr == &mons[PM_WINGED_GARGOYLE])
	      && !sliparm(ptr));
}
#endif /* OVLB */
#ifdef OVL1

boolean
sticks(ptr)	/* creature sticks other creatures it hits */
	register struct permonst *ptr;
{
	return((boolean)(dmgtype(ptr,AD_STCK) || dmgtype(ptr,AD_WRAP) ||
		attacktype(ptr,AT_HUGS)));
}

/* number of horns this type of monster has on its head */
int
num_horns(ptr)
struct permonst *ptr;
{
    switch (monsndx(ptr)) {
    case PM_HORNED_DEVIL:	/* ? "more than one" */
    case PM_MINOTAUR:
    case PM_ASMODEUS:
    case PM_BALROG:
	return 2;
    case PM_WHITE_UNICORN:
    case PM_GRAY_UNICORN:
    case PM_BLACK_UNICORN:
    case PM_KI_RIN:
	return 1;
    default:
	break;
    }
    return 0;
}

struct attack *
dmgtype_fromattack(ptr, dtyp, atyp)
struct permonst *ptr;
int dtyp, atyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
	if (a->adtyp == dtyp && (atyp == AT_ANY || a->aatyp == atyp))
	    return a;

    return (struct attack *)0;
}

boolean
dmgtype(ptr, dtyp)
struct permonst *ptr;
int dtyp;
{
    return dmgtype_fromattack(ptr, dtyp, AT_ANY) ? TRUE : FALSE;
}

/* returns the maximum damage a defender can do to the attacker via
 * a passive defense */
int
max_passive_dmg(mdef, magr)
    register struct monst *mdef, *magr;
{
    int	i, dmg = 0;
    uchar adtyp;

    for(i = 0; i < NATTK; i++)
	if(mdef->data->mattk[i].aatyp == AT_NONE ||
		mdef->data->mattk[i].aatyp == AT_BOOM) {
	    adtyp = mdef->data->mattk[i].adtyp;
	    if ((adtyp == AD_ACID && !resists_acid(magr)) ||
		    (adtyp == AD_COLD && !resists_cold(magr)) ||
		    (adtyp == AD_FIRE && !resists_fire(magr)) ||
		    (adtyp == AD_ELEC && !resists_elec(magr)) ||
		    adtyp == AD_PHYS) {
		dmg = mdef->data->mattk[i].damn;
		if(!dmg) dmg = mdef->data->mlevel+1;
		dmg *= mdef->data->mattk[i].damd;
	    } else dmg = 0;

	    return dmg;
	}
    return 0;
}

#endif /* OVL1 */
#ifdef OVL0

int
monsndx(ptr)		/* return an index into the mons array */
	struct	permonst	*ptr;
{
	register int	i;

	i = (int)(ptr - &mons[0]);
	if (i < LOW_PM || i >= NUMMONS) {
		/* ought to switch this to use `fmt_ptr' */
	    panic("monsndx - índice de monstruo no es válido (%lx)",
		  (unsigned long)ptr);
	    return NON_PM;		/* will not get here */
	}

	return(i);
}

#endif /* OVL0 */
#ifdef OVL1


int
name_to_mon(in_str)
const char *in_str;
{
	/* Be careful.  We must check the entire string in case it was
	 * something such as "ettin zombie corpse".  The calling routine
	 * doesn't know about the "corpse" until the monster name has
	 * already been taken off the front, so we have to be able to
	 * read the name with extraneous stuff such as "corpse" stuck on
	 * the end.
	 * This causes a problem for names which prefix other names such
	 * as "ettin" on "ettin zombie".  In this case we want the _longest_
	 * name which exists.
	 * This also permits plurals created by adding suffixes such as 's'
	 * or 'es'.  Other plurals must still be handled explicitly.
	 */
	/*
	 * LENGUA:  "ettin zombie corpse" in Spanish is "cadáver de zombí
	 * ettin":  "cadáver" appears at the beginning.  Code must be modified
	 * to take this into account; viz., we remove "cadáver de " if it is
	 * present.
	 *
	 * We also need to deal with feminine forms, and names that pluralize
	 * by pluralizing multiple parts.
	 */
	register int i;
	register char *str;
	char buf[BUFSZ];

	str = strcpy(buf, in_str);

	if (!strncmp(str, "un ", 3)) str += 3;
	else if (!strncmp(str, "una ", 4)) str += 4;
	if (!strncmp(str, "cadáver de ", 11)) str += 11;
	/* "un cadáver de un perro" */
	if (!strncmp(str, "un ", 3)) str += 3;
	else if (!strncmp(str, "una ", 4)) str += 4;

    {
	static const struct alt_spl { const char* name; short pm_val; }
	    names[] = {
	    /* Alternate spellings */
#if 0
		{ "grey dragon",	PM_GRAY_DRAGON },
		{ "baby grey dragon",	PM_BABY_GRAY_DRAGON },
		{ "grey unicorn",	PM_GRAY_UNICORN },
		{ "grey ooze",		PM_GRAY_OOZE },
		{ "gray-elf",		PM_GREY_ELF },
#endif
	    /* Hyphenated names */
		{ "ki rin",		PM_KI_RIN },
		{ "uruk hai",		PM_URUK_HAI },
		{ "orco capitán",	PM_ORC_CAPTAIN },
		{ "olog hai",		PM_OLOG_HAI },
	    /* end of list */
		{ 0, 0 }
	};
	register const struct alt_spl *namep;

	for (namep = names; namep->name; namep++)
	    if (!strncmpi2(str, namep->name, (int)strlen(namep->name)))
		return namep->pm_val;
    }

	for (i = LOW_PM; i < NUMMONS; i++) {
	    /* Each entry in mons may present as many as four matches:
	       * the given string
	       * the string returned by feminize()
	       * the string returned by makeplural()
	       * the string returned by makeplural(feminize())
	       We use the feminine forms if the monster can be generated as
	       either male or female and if feminize() changes the string.
	       Female-only monsters have their names entered in the table in
	       feminine form already. */
	    char *fem;

	    fem = NULL;
	    /* Can the monster be generated as either male or female? */
	    if ((mons[i].mflags2 & (M2_MALE | M2_FEMALE | M2_NEUTER)) == 0
	    &&  (mons[i].geno & G_UNIQ) == 0) {
		/* It can.  Get the feminine form of the name */
		fem = feminize(mons[i].mname, TRUE);
		/* This eliminates a call to makeplural() when there is
		   no distinct feminine name */
		if (strcmpi2a(mons[i].mname, fem) == 0)
		    fem = NULL;
	    }

	    if (strcmpi2a(mons[i].mname, str) == 0
	    ||  strcmpi2a(makeplural(mons[i].mname), str) == 0)
		return i;	/* Matches the basic form or its plural */
	    if (fem
	    &&  (strcmpi2a(fem, str) == 0
	      || strcmpi2a(makeplural(fem), str) == 0))
		return i;	/* Matches the feminine form or its plural */
	}

	return title_to_mon(str, (int *)0, (int *)0);
}

#endif /* OVL1 */
#ifdef OVL2

/* returns 3 values (0=male, 1=female, 2=none) */
int
gender(mtmp)
register struct monst *mtmp;
{
	if (is_neuter(mtmp->data)) return 2;
	return mtmp->female;
}

/* Like gender(), but lower animals and such are still "it". */
/* This is the one we want to use when printing messages. */
int
pronoun_gender(mtmp)
register struct monst *mtmp;
{
	if (is_neuter(mtmp->data) || !canspotmon(mtmp)) return 2;
	return (humanoid(mtmp->data) || (mtmp->data->geno & G_UNIQ) ||
		type_is_pname(mtmp->data)) ? (int)mtmp->female : 2;
}

#endif /* OVL2 */
#ifdef OVLB

/* used for nearby monsters when you go to another level */
boolean
levl_follower(mtmp)
struct monst *mtmp;
{
	/* monsters with the Amulet--even pets--won't follow across levels */
	if (mon_has_amulet(mtmp)) return FALSE;

	/* some monsters will follow even while intending to flee from you */
	if (mtmp->mtame || mtmp->iswiz || is_fshk(mtmp)) return TRUE;

	/* stalking types follow, but won't when fleeing unless you hold
	   the Amulet */
	return (boolean)((mtmp->data->mflags2 & M2_STALK) &&
				(!mtmp->mflee || u.uhave.amulet));
}

static const short grownups[][2] = {
	{PM_CHICKATRICE, PM_COCKATRICE},
	{PM_LITTLE_DOG, PM_DOG}, {PM_DOG, PM_LARGE_DOG},
	{PM_HELL_HOUND_PUP, PM_HELL_HOUND},
	{PM_WINTER_WOLF_CUB, PM_WINTER_WOLF},
	{PM_KITTEN, PM_HOUSECAT}, {PM_HOUSECAT, PM_LARGE_CAT},
	{PM_PONY, PM_HORSE}, {PM_HORSE, PM_WARHORSE},
	{PM_KOBOLD, PM_LARGE_KOBOLD}, {PM_LARGE_KOBOLD, PM_KOBOLD_LORD},
	{PM_GNOME, PM_GNOME_LORD}, {PM_GNOME_LORD, PM_GNOME_KING},
	{PM_DWARF, PM_DWARF_LORD}, {PM_DWARF_LORD, PM_DWARF_KING},
	{PM_MIND_FLAYER, PM_MASTER_MIND_FLAYER},
	{PM_ORC, PM_ORC_CAPTAIN}, {PM_HILL_ORC, PM_ORC_CAPTAIN},
	{PM_MORDOR_ORC, PM_ORC_CAPTAIN}, {PM_URUK_HAI, PM_ORC_CAPTAIN},
	{PM_SEWER_RAT, PM_GIANT_RAT},
	{PM_CAVE_SPIDER, PM_GIANT_SPIDER},
	{PM_OGRE, PM_OGRE_LORD}, {PM_OGRE_LORD, PM_OGRE_KING},
	{PM_ELF, PM_ELF_LORD}, {PM_WOODLAND_ELF, PM_ELF_LORD},
	{PM_GREEN_ELF, PM_ELF_LORD}, {PM_GREY_ELF, PM_ELF_LORD},
	{PM_ELF_LORD, PM_ELVENKING},
	{PM_LICH, PM_DEMILICH}, {PM_DEMILICH, PM_MASTER_LICH},
	{PM_MASTER_LICH, PM_ARCH_LICH},
	{PM_VAMPIRE, PM_VAMPIRE_LORD}, {PM_BAT, PM_GIANT_BAT},
	{PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
	{PM_BABY_SILVER_DRAGON, PM_SILVER_DRAGON},
#if 0	/* DEFERRED */
	{PM_BABY_SHIMMERING_DRAGON, PM_SHIMMERING_DRAGON},
#endif
	{PM_BABY_RED_DRAGON, PM_RED_DRAGON},
	{PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON},
	{PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON},
	{PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON},
	{PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON},
	{PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON},
	{PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON},
	{PM_RED_NAGA_HATCHLING, PM_RED_NAGA},
	{PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA},
	{PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA},
	{PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA},
	{PM_SMALL_MIMIC, PM_LARGE_MIMIC}, {PM_LARGE_MIMIC, PM_GIANT_MIMIC},
	{PM_BABY_LONG_WORM, PM_LONG_WORM},
	{PM_BABY_PURPLE_WORM, PM_PURPLE_WORM},
	{PM_BABY_CROCODILE, PM_CROCODILE},
	{PM_SOLDIER, PM_SERGEANT},
	{PM_SERGEANT, PM_LIEUTENANT},
	{PM_LIEUTENANT, PM_CAPTAIN},
	{PM_WATCHMAN, PM_WATCH_CAPTAIN},
	{PM_ALIGNED_PRIEST, PM_HIGH_PRIEST},
	{PM_STUDENT, PM_ARCHEOLOGIST},
	{PM_ATTENDANT, PM_HEALER},
	{PM_PAGE, PM_KNIGHT},
	{PM_ACOLYTE, PM_PRIEST},
	{PM_APPRENTICE, PM_WIZARD},
	{PM_MANES,PM_LEMURE},
#ifdef KOPS
	{PM_KEYSTONE_KOP, PM_KOP_SERGEANT},
	{PM_KOP_SERGEANT, PM_KOP_LIEUTENANT},
	{PM_KOP_LIEUTENANT, PM_KOP_KAPTAIN},
#endif
	{NON_PM,NON_PM}
};

int
little_to_big(montype)
int montype;
{
#ifndef AIXPS2_BUG
	register int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][0]) return grownups[i][1];
	return montype;
#else
/* AIX PS/2 C-compiler 1.1.1 optimizer does not like the above for loop,
 * and causes segmentation faults at runtime.  (The problem does not
 * occur if -O is not used.)
 * lehtonen@cs.Helsinki.FI (Tapio Lehtonen) 28031990
 */
	int i;
	int monvalue;

	monvalue = montype;
	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][0]) monvalue = grownups[i][1];

	return monvalue;
#endif
}

int
big_to_little(montype)
int montype;
{
	register int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][1]) return grownups[i][0];
	return montype;
}

/*
 * Return the permonst ptr for the race of the monster.
 * Returns correct pointer for non-polymorphed and polymorphed
 * player.  It does not return a pointer to player role character.
 */
const struct permonst *
raceptr(mtmp)
struct monst *mtmp;
{
    if (mtmp == &youmonst && !Upolyd) return(&mons[urace.malenum]);
    else return(mtmp->data);
}

struct loco_verb
{
    char *infinitive;
    char *gerund;
    char *present;
    /* others to be added as needed */
};
static const char *FDECL(subst_verb, (const struct loco_verb *verb, const char *def));

/* Verbs for locomotion() and stagger() */
/* LENGUA: original English verb given in comments */

static const struct loco_verb flotar      = { "flotar",      "flotando",      "flota"       }; /* float */
static const struct loco_verb volar       = { "volar",       "volando",       "vuela"       }; /* fly */
static const struct loco_verb resbalarse  = { "resbalarse",  "resbalándose",  "se resbala"  }; /* slither */
static const struct loco_verb rezumarse   = { "rezumarse",   "rezumándose",   "se rezuma"   }; /* ooze */
static const struct loco_verb menear      = { "menear",      "meneando",      "menea"       }; /* wiggle */
static const struct loco_verb arrastrarse = { "arrastrarse", "arrastrándose", "se arrastra" }; /* crawl */
static const struct loco_verb tambolearse = { "tambolearse", "tamboleándose", "se tambolea" }; /* wobble */
static const struct loco_verb palpitarse  = { "palpitarse",  "palpitándose",  "se palpita"  }; /* flutter */
static const struct loco_verb bambolearse = { "bambolearse", "bamboleándose", "se bambolea" }; /* stagger */
static const struct loco_verb vacilar     = { "vacilar",     "vacilando",     "vacila"      }; /* falter */
static const struct loco_verb tremolarse  = { "tremolarse",  "tremolándose",  "se tremola"  }; /* tremble */
static const struct loco_verb pulsar      = { "pulsar",      "pulsando",      "pulsa"       }; /* pulsate */

const char *
locomotion(ptr, def)
const struct permonst *ptr;
const char *def;
{
	const struct loco_verb *verb;

	/* Choose the verb to be returned */
	verb = (
		is_floater(ptr) ? &flotar :
		is_flyer(ptr)   ? &volar :
		slithy(ptr)     ? &resbalarse :
		amorphous(ptr)  ? &rezumarse :
		!ptr->mmove	? &menear :
		nolimbs(ptr)    ? &arrastrarse :
		(const struct loco_verb *)0  /* return def unchanged */
	       );
	return subst_verb(verb, def);
}

const char *
stagger(ptr, def)
const struct permonst *ptr;
const char *def;
{
	const struct loco_verb *verb;

	/* Choose the verb to be returned */
	verb = (
		is_floater(ptr) ? &tambolearse :
		(is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? &palpitarse :
		(is_flyer(ptr) && ptr->msize > MZ_SMALL)  ? &bambolearse :
		slithy(ptr)     ? &vacilar :
		amorphous(ptr)  ? &tremolarse :
		!ptr->mmove	? &pulsar :
		nolimbs(ptr)    ? &vacilar :
		(const struct loco_verb *)0  /* return def unchanged */
	       );
	return subst_verb(verb, def);
}

/* Verb conjugation for locomotion() and stagger() */

static const char *
subst_verb(verb, def)
const struct loco_verb *verb;
const char *def;
{
	int len;
	char *str = nextobuf();

	if (!verb) return def;	/* No substitute verb */

	/*
	 * We assume that def is of the 2nd or 3rd person singular forms,
         * the 3rd person plural, an infinitive, or a gerund.
	 *
	 * We want to return a verb in the same conjugation.
	 */
	len = strlen(def);
	if (len>=1 && def[len-1]=='r')     /* Infinitive */
		/* FIXME: we could check for -rse to detect reflexive
		   infinitives, but that risks collision with a present form
		   that happens to end in -rse */
		Strcpy(str, verb->infinitive);
	else if (len>=3 && strcmp(def+len-3, "ndo")==0)     /* Gerund */
		Strcpy(str, verb->gerund);
	else {   /* Present */
		/* The present verb is given as third person singular, but
		   def may also be second person singular or third person
		   plural. */
		Strcpy(str, verb->present);
		if (len>=1 && (def[len-1]=='n' || def[len-1]=='s')) {
			const char *conj = def+len-1;
			Strcat(str, conj);
			if (conj[0]=='s' && strncmp(str, "se ", 3)==0)
				/* Reflexive verb in 2ps; change se to te */
				str[0] = 't';
		}
	}

	/* Preserve capitalization */
	if (isupper2(def[0]))
		str[0] = toupper2(str[0]);

	return str;
}

/* return a phrase describing the effect of fire attack on a type of monster */
char *
on_fire(mptr, mattk)
struct permonst *mptr;
struct attack *mattk;
{
    char *buf = nextobuf();
    const char *what;
    /* Second versus third person stuff */
    boolean tu = (mptr == youmonst.data);
    const char *verb = tu? "estás" : "está";
    const char *pron = tu? "te" : "se";

    switch (monsndx(mptr)) {
    case PM_FLAMING_SPHERE:
    case PM_FIRE_VORTEX:
    case PM_FIRE_ELEMENTAL:
    case PM_SALAMANDER:
	Sprintf(buf, "ya %s ardiendo", verb);
	break;
    case PM_WATER_ELEMENTAL:
    case PM_FOG_CLOUD:
    case PM_STEAM_VORTEX:
	Sprintf(buf, "%s hirviendo", verb);
	break;
    case PM_ICE_VORTEX:
    case PM_GLASS_GOLEM:
	Sprintf(buf, "%s fundiendo", verb);
	break;
    case PM_STONE_GOLEM:
    case PM_CLAY_GOLEM:
    case PM_GOLD_GOLEM:
    case PM_AIR_ELEMENTAL:
    case PM_EARTH_ELEMENTAL:
    case PM_DUST_VORTEX:
    case PM_ENERGY_VORTEX:
	Sprintf(buf, "%s poniéndo%s caliente", verb, pron);
	break;
    default:
	Sprintf(buf, "%s %s%s",
		verb,
		what = (mattk->aatyp == AT_HUGS) ? "asando" : "quemándo",
		pron);
	break;
    }
    return buf;
}

#endif /* OVLB */

/*mondata.c*/
