/*	SCCS Id: @(#)write.c	3.4	2001/11/29	*/
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL int FDECL(cost,(struct obj *));

/*
 * returns basecost of a scroll or a spellbook
 */
STATIC_OVL int
cost(otmp)
register struct obj *otmp;
{

	if (otmp->oclass == SPBOOK_CLASS)
		return(10 * objects[otmp->otyp].oc_level);

	switch (otmp->otyp) {
# ifdef MAIL
	case SCR_MAIL:
		return(2);
/*		break; */
# endif
	case SCR_LIGHT:
	case SCR_GOLD_DETECTION:
	case SCR_FOOD_DETECTION:
	case SCR_MAGIC_MAPPING:
	case SCR_AMNESIA:
	case SCR_FIRE:
	case SCR_EARTH:
		return(8);
/*		break; */
	case SCR_DESTROY_ARMOR:
	case SCR_CREATE_MONSTER:
	case SCR_PUNISHMENT:
		return(10);
/*		break; */
	case SCR_CONFUSE_MONSTER:
		return(12);
/*		break; */
	case SCR_IDENTIFY:
		return(14);
/*		break; */
	case SCR_ENCHANT_ARMOR:
	case SCR_REMOVE_CURSE:
	case SCR_ENCHANT_WEAPON:
	case SCR_CHARGING:
		return(16);
/*		break; */
	case SCR_SCARE_MONSTER:
	case SCR_STINKING_CLOUD:
	case SCR_TAMING:
	case SCR_TELEPORTATION:
		return(20);
/*		break; */
	case SCR_GENOCIDE:
		return(30);
/*		break; */
	case SCR_BLANK_PAPER:
	default:
		impossible("¡No puedes escribir un pergamino tan extraño!");
	}
	return(1000);
}

static NEARDATA const char write_on[] = { SCROLL_CLASS, SPBOOK_CLASS, 0 };

int
dowrite(pen)
register struct obj *pen;
{
	register struct obj *paper;
	char namebuf[BUFSZ], *nm;
	register struct obj *new_obj;
	int basecost, actualcost;
	int curseval;
	char qbuf[QBUFSZ];
	int first, last, i;
	boolean by_descr = FALSE;
	const char *typeword;

	if (nohands(youmonst.data)) {
	    pline("¡Necesitas manos para poder escribir!");
	    return 0;
	} else if (Glib) {
	    pline("%s de tus %s.",
		  Tobjnam(pen, "se resbala"), makeplural(body_part(FINGER)));
	    dropx(pen);
	    return 1;
	}

	/* get paper to write on */
	paper = getobj(write_on, "en", "escribir");
	if(!paper)
		return(0);
	typeword = (paper->oclass == SPBOOK_CLASS) ? "libro" : "pergamino";
	if(Blind && !paper->dknown) {
		pline("¡No sabes si ese %s sea en blanco o no!", typeword);
		return(1);
	}
	paper->dknown = 1;
	if(paper->otyp != SCR_BLANK_PAPER && paper->otyp != SPE_BLANK_PAPER) {
		pline("¡Ese %s no es en blanco!", typeword);
		exercise(A_WIS, FALSE);
		return(1);
	}

	/* what to write */
	Sprintf(qbuf, "¿Qué tipo de %s quieres escribir?", typeword);
	getlin(qbuf, namebuf);
	(void)mungspaces(namebuf);	/* remove any excess whitespace */
	if(namebuf[0] == '\033' || !namebuf[0])
		return(1);
	nm = namebuf;
	if (!strncmpi2(nm, "pergamino ", 10)) nm += 10;
	else if (!strncmpi2(nm, "libro ", 6)) nm += 6;
	if (!strncmpi2(nm, "de ", 3)) nm += 3;

	first = bases[(int)paper->oclass];
	last = bases[(int)paper->oclass + 1] - 1;
	for (i = first; i <= last; i++) {
		/* extra shufflable descr not representing a real object */
		if (!OBJ_NAME(objects[i])) continue;

		if (!strcmpi2a(OBJ_NAME(objects[i]), nm))
			goto found;
		if (!strcmpi2(OBJ_DESCR(objects[i]), nm)) {
			by_descr = TRUE;
			goto found;
		}
	}

	pline("¡No existe ese %s!", typeword);
	return 1;
found:

	if (i == SCR_BLANK_PAPER || i == SPE_BLANK_PAPER) {
		No_puedes("escribir eso!");
		pline("¡Es obsceno!");
		return 1;
	} else if (i == SPE_BOOK_OF_THE_DEAD) {
		char fem = poly_gender()==1? 'a' : 'o';
		pline("%s mer%c aventurer%c de calabozos podría escribir eso.",
			fem=='a'? "Ninguna" : "Ningún",
			fem, fem);
		return 1;
	} else if (by_descr && paper->oclass == SPBOOK_CLASS &&
		    !objects[i].oc_name_known) {
		/* can't write unknown spellbooks by description */
		pline("Desgraciadamente no tienes bastante información para escribir.");
		return 1;
	}

	/* KMH, conduct */
	u.uconduct.literate++;

	new_obj = mksobj(i, FALSE, FALSE);
	new_obj->bknown = (paper->bknown && pen->bknown);

	/* shk imposes a flat rate per use, not based on actual charges used */
	check_unpaid(pen);

	/* see if there's enough ink */
	basecost = cost(new_obj);
	if(pen->spe < basecost/2)  {
		pline("¡Tu marcador está demasiado seco para escribir eso!");
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}

	/* we're really going to write now, so calculate cost
	 */
	actualcost = rn1(basecost/2,basecost/2);
	curseval = bcsign(pen) + bcsign(paper);
	exercise(A_WIS, TRUE);
	/* dry out marker */
	if (pen->spe < actualcost) {
		pen->spe = 0;
		pline("¡Tu marcador se seca!");
		/* scrolls disappear, spellbooks don't */
		if (paper->oclass == SPBOOK_CLASS) {
			pline(
		       "El libro de hechizo se deja incompleto y tu escritura se desvanece.");
			update_inventory();	/* pen charges */
		} else {
			pline("¡El pergamino ya es inútil y desaparece!");
			useup(paper);
		}
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}
	pen->spe -= actualcost;

	/* can't write if we don't know it - unless we're lucky */
	if(!(objects[new_obj->otyp].oc_name_known) &&
	   !(objects[new_obj->otyp].oc_uname) &&
	   (rnl(Role_if(PM_WIZARD) ? 3 : 15))) {
		pline("¡%s escribir eso!", by_descr ? "Fallas de" : "No sabes");
		/* scrolls disappear, spellbooks don't */
		if (paper->oclass == SPBOOK_CLASS) {
			pline(
       "Escribes en tu letra mejor:  \"Mi Diario\", pero se desvanece rápidamente.");
			update_inventory();	/* pen charges */
		} else {
			if (by_descr) {
			    Strcpy(namebuf, OBJ_DESCR(objects[new_obj->otyp]));
			    wipeout_text(namebuf, (6+MAXULEV - u.ulevel)/6, 0);
			} else
			    Sprintf(namebuf, "¡%s estaba aquí!", plname);
			pline("Escribes \"%s\" y el pergamino desvanece.", namebuf);
			useup(paper);
		}
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}

	/* useup old scroll / spellbook */
	useup(paper);

	/* success */
	if (new_obj->oclass == SPBOOK_CLASS) {
		/* acknowledge the change in the object's description... */
		pline("El libro se deforma extrañamente, y se vuelve %s.",
		      OBJ_DESCR(objects[new_obj->otyp]));
	}
	new_obj->blessed = (curseval > 0);
	new_obj->cursed = (curseval < 0);
#ifdef MAIL
	if (new_obj->otyp == SCR_MAIL) new_obj->spe = 1;
#endif
	new_obj = hold_another_object(new_obj, "¡Oops!  ¡%s afuera de tu agarro!",
					       The(aobjnam(new_obj, "resbala")),
					       (const char *)0);
	return(1);
}

/*write.c*/
