/*	SCCS Id: @(#)sounds.c	3.4	2002/05/06	*/
/*	Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"
#ifdef USER_SOUNDS
# ifdef USER_SOUNDS_REGEX
#include <regex.h>
# endif
#endif

#ifdef OVLB

static int FDECL(domonnoise,(struct monst *));
static int NDECL(dochat);

#endif /* OVLB */

#ifdef OVL0

static int FDECL(mon_in_room, (struct monst *,int));

/* this easily could be a macro, but it might overtax dumb compilers */
static int
mon_in_room(mon, rmtyp)
struct monst *mon;
int rmtyp;
{
    int rno = levl[mon->mx][mon->my].roomno;

    return rooms[rno - ROOMOFFSET].rtype == rmtyp;
}

void
dosounds()
{
    register struct mkroom *sroom;
    register int hallu, vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif
    struct monst *mtmp;

    if (!flags.soundok || u.uswallow || Underwater) return;

    hallu = Hallucination ? 1 : 0;

    if (level.flags.nfountains && !rn2(400)) {
	static const char * const fountain_msg[4] = {
		"agua corriente.",
		"agua cayendo sobre monedas.",
		"el chapotear de una náyade.",
		"una fuente de soda!",
	};
	Oyes(fountain_msg[rn2(3)+hallu]);
    }
#ifdef SINK
    if (level.flags.nsinks && !rn2(300)) {
	static const char * const sink_msg[3] = {
		"un goteo despacio.",
		"un ruido borbotante.",
		"platos siendo lavados!",
	};
	Oyes(sink_msg[rn2(2)+hallu]);
    }
#endif
    if (level.flags.has_court && !rn2(200)) {
	static const char * const throne_msg[4] = {
		"los sonidos de conversación fina.",
		"un cetro martillado por juicio.",
		"Alguien grita \"¡Cortadle la cabeza!\"",
		"los gatos de la Reina Beruthiel!",
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping ||
			is_lord(mtmp->data) || is_prince(mtmp->data)) &&
		!is_animal(mtmp->data) &&
		mon_in_room(mtmp, COURT)) {
		/* finding one is enough, at least for now */
		int which = rn2(3)+hallu;

		if (which != 2) Oyes(throne_msg[which]);
		else		pline(throne_msg[2]);
		return;
	    }
	}
    }
    if (level.flags.has_swamp && !rn2(200)) {
	static const char * const swamp_msg[3] = {
		"¡Oyes mosquitos!",
		"¡Hueles gas de ciénaga!",	/* so it's a smell...*/
		"¡Oyes a Donald Duck!",
	};
	pline(swamp_msg[rn2(2)+hallu]);
	return;
    }
    if (level.flags.has_vault && !rn2(200)) {
	if (!(sroom = search_special(VAULT))) {
	    /* strange ... */
	    level.flags.has_vault = 0;
	    return;
	}
	if(gd_sound())
	    switch (rn2(2)+hallu) {
		case 1: {
		    boolean gold_in_vault = FALSE;

		    for (vx = sroom->lx;vx <= sroom->hx; vx++)
			for (vy = sroom->ly; vy <= sroom->hy; vy++)
			    if (g_at(vx, vy))
				gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
		    /* Bug in aztec assembler here. Workaround below */
		    xx = ROOM_INDEX(sroom) + ROOMOFFSET;
		    xx = (xx != vault_occupied(u.urooms));
		    if(xx)
#else
		    if (vault_occupied(u.urooms) !=
			 (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
		    {
			/* LENGUA: "quarterback" should become some reference
			   to soccer */
			if (gold_in_vault)
			    Oyes(!hallu ? "a alguien contando dinero." :
				"al quarterback llamando la jugada.");
			else
			    Oyes("a alguien buscando.");
			break;
		    }
		    /* fall into... (yes, even for hallucination) */
		}
		case 0:
		    Oyes("los pasos de un guardia patrullando.");
		    break;
		case 2:
		    Oyes("a Ebenezer Scrooge!");
		    break;
	    }
	return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->data->mlet == S_ANT && is_flyer(mtmp->data)) &&
		mon_in_room(mtmp, BEEHIVE)) {
		switch (rn2(2)+hallu) {
		    case 0:
			Oyes("un zumbido bajo.");
			break;
		    case 1:
			Oyes("un zumbido enojado.");
			break;
		    case 2:
			/* LENGUA:  "a bee in one's bonnet" doesn't really
			   translate well */
			Oyes("abejas en tu %ssombrero!",
			    uarmh ? "" : "(inexistente) ");
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_morgue && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_undead(mtmp->data) &&
		mon_in_room(mtmp, MORGUE)) {
		switch (rn2(2)+hallu) {
		    case 0:
			pline("Te das cuenta de repente que todo es innaturalmente silente.");
			break;
		    case 1:
			pline("%s detrás de tu %s se levanta.",
				The(body_part(HAIR)), body_part(NECK));
			break;
		    case 2:
			pline("%s de tu %s parece levantarse.",
				The(body_part(HAIR)), body_part(HEAD));
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_barracks && !rn2(200)) {
	static const char * const barracks_msg[4] = {
		"espadas siendo afiladas.",
		"ronquidos fuertes.",
		"dados siendo tirados.",
		"a General MacArthur!",
	};
	int count = 0;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_mercenary(mtmp->data) &&
#if 0		/* don't bother excluding these */
		!strstri(mtmp->data->mname, "vigilia") &&
		!strstri(mtmp->data->mname, "guarda") &&
#endif
		mon_in_room(mtmp, BARRACKS) &&
		/* sleeping implies not-yet-disturbed (usually) */
		(mtmp->msleeping || ++count > 5)) {
		Oyes(barracks_msg[rn2(3)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_zoo && !rn2(200)) {
	static const char * const zoo_msg[3] = {
		"un sonido reminisciente de un elefante pisando un cacahuate.",
		"un sonido reminisciente de una foca ladrante.",
		"a Doctor Doolittle!",
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping || is_animal(mtmp->data)) &&
		    mon_in_room(mtmp, ZOO)) {
		Oyes(zoo_msg[rn2(2)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_shop && !rn2(200)) {
	if (!(sroom = search_special(ANY_SHOP))) {
	    /* strange... */
	    level.flags.has_shop = 0;
	    return;
	}
	if (tended_shop(sroom) &&
		!index(u.ushops, ROOM_INDEX(sroom) + ROOMOFFSET)) {
	    static const char * const shop_msg[3] = {
		    "a alguien maldiciendo a los ladrones.",
		    "el campaneo de una caja registradora.",
		    "a Neiman y Marcus arguyendo!",
	    };
	    Oyes(shop_msg[rn2(2)+hallu]);
	}
	return;
    }
    if (Is_oracle_level(&u.uz) && !rn2(400)) {
	/* make sure the Oracle is still here */
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if (!DEADMONSTER(mtmp) && mtmp->data == &mons[PM_ORACLE])
		break;
	/* and don't produce silly effects when she's clearly visible */
	if (mtmp && (hallu || !canseemon(mtmp))) {
	    static const char * const ora_msg[5] = {
		    "un viento extraño.",	/* Jupiter at Dodona */
		    "desvarios convulsivos.",	/* Apollo at Delphi */
		    "serpientes roncantes.",	/* AEsculapius at Epidaurus */
		    /* LENGUA:  This joke doesn't really work in Spanish
		       because "How much wood...." doesn't translate */
		    "a alguien diciendo \"¡No más marmotas!\"",
		    "un fuerte ZOT!"		/* both rec.humor.oracle */
	    };
	    Oyes(ora_msg[rn2(3)+hallu*2]);
	}
	return;
    }
}

#endif /* OVL0 */
#ifdef OVLB

/* LENGUA: review translations of onomatopoeia */
/* These are verbs in the third person singular */
static const char * const h_sounds[] = {
    "bipa", "boinga", "canta", "eructa", "cruje", "tose", "traquetea",
    "ulula", "reventa", "tintinea", "sorbe", "choca", "ipa"
};

/* Returned words are verbs in the third person singular */
const char *
growl_sound(mtmp)
register struct monst *mtmp;
{
	const char *ret;

	switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_HISS:
	    ret = "sisea";
	    break;
	case MS_BARK:
	case MS_GROWL:
	    ret = "gruñe";
	    break;
	case MS_ROAR:
	    ret = "ruge";
	    break;
	case MS_BUZZ:
	    ret = "zumba";
	    break;
	case MS_SQEEK:
	    ret = "chilla";
	    break;
	case MS_SQAWK:
	    ret = "grazna";
	    break;
	case MS_NEIGH:
	    ret = "relincha";
	    break;
	case MS_WAIL:
	    ret = "lamenta";
	    break;
	case MS_SILENT:
		/* LENGUA: "commotion" in English, which is a noun; but this
		   shouldn't be reachable anyway */
		ret = "comociona";
		break;
	default:
		ret = "grita";
	}
	return ret;
}

/* the sounds of a seriously abused pet, including player attacking it */
void
growl(mtmp)
register struct monst *mtmp;
{
    register const char *growl_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
	growl_verb = growl_sound(mtmp);
    if (growl_verb) {
	pline("¡%s %s!", Monnam(mtmp), growl_verb);
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 18);
    }
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
register struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	yelp_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	    yelp_verb = "aulla";
	    break;
	case MS_BARK:
	case MS_GROWL:
	    yelp_verb = "ladra";
	    break;
	case MS_ROAR:
	    yelp_verb = "gruñe";
	    break;
	case MS_SQEEK:
	    yelp_verb = "chilla";
	    break;
	case MS_SQAWK:
	    yelp_verb = "chirria";
	    break;
	case MS_WAIL:
	    yelp_verb = "lamenta";
	    break;
    }
    if (yelp_verb) {
	pline("¡%s %s!", Monnam(mtmp), vtense((char *)0, yelp_verb));
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 12);
    }
}

/* the sounds of distressed pets */
void
whimper(mtmp)
register struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	whimper_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_GROWL:
	    whimper_verb = "gimotea";
	    break;
	case MS_BARK:
	    whimper_verb = "lloriquea";
	    break;
	case MS_SQEEK:
	    whimper_verb = "chilla";
	    break;
    }
    if (whimper_verb) {
	pline("%s %s.", Monnam(mtmp), vtense((char *)0, whimper_verb));
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 6);
    }
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove ||
	    !(carnivorous(mtmp->data) || herbivorous(mtmp->data)))
	return;

    /* presumably nearness and soundok checks have already been made */
    if (!is_silent(mtmp->data) && mtmp->data->msound <= MS_ANIMAL)
	(void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID) {
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
	verbalize("Tengo hambre.");
    }
}

static int
domonnoise(mtmp)
register struct monst *mtmp;
{
    register const char *pline_msg = 0,	/* Monnam(mtmp) will be prepended */
			*verbl_msg = 0;	/* verbalize() */
    struct permonst *ptr = mtmp->data;
    char verbuf[BUFSZ];

    /* presumably nearness and sleep checks have already been made */
    if (!flags.soundok) return(0);
    if (is_silent(ptr)) return(0);

    /* Make sure its your role's quest quardian; adjust if not */
    if (ptr->msound == MS_GUARDIAN && ptr != &mons[urole.guardnum]) {
    	int mndx = monsndx(ptr);
    	ptr = &mons[genus(mndx,1)];
    }

    /* be sure to do this before talking; the monster might teleport away, in
     * which case we want to check its pre-teleport position
     */
    if (!canspotmon(mtmp))
	map_invisible(mtmp->mx, mtmp->my);

    switch (ptr->msound) {
	case MS_ORACLE:
	    return doconsult(mtmp);
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
	case MS_LEADER:
	case MS_NEMESIS:
	case MS_GUARDIAN:
	    quest_chat(mtmp);
	    break;
	case MS_SELL: /* pitch, pay, total */
	    shk_chat(mtmp);
	    break;
	case MS_VAMPIRE:
	    {
	    /* vampire messages are varied by tameness, peacefulness, and time of night */
		boolean isnight = night();
		boolean kindred =    (Upolyd && (u.umonnum == PM_VAMPIRE ||
				       u.umonnum == PM_VAMPIRE_LORD));
		boolean nightchild = (Upolyd && (u.umonnum == PM_WOLF ||
				       u.umonnum == PM_WINTER_WOLF ||
	    			       u.umonnum == PM_WINTER_WOLF_CUB));
		const char *racenoun = (flags.female && urace.individual.f) ?
					urace.individual.f : (urace.individual.m) ?
					urace.individual.m : urace.noun;

		/* It's often inconvenient to work in the the player's gender;
		   so we do this later by fixups */
		if (mtmp->mtame) {
			if (kindred) {
				Strcpy(verbuf,
				    isnight ? "¡Buenas noches a usted, dueñ@!  ¿Por qué no nos descansamos?"
					    : "¡Buenos días a usted, dueñ@!");
				verbl_msg = verbuf;
		    	} else {
			    /* LENGUA: "cansado" assumes vampires are male,
			       which is true in this version of Nethack */
		    	    Sprintf(verbuf,"%s%s",
				nightchild ? "Hij@ de la noche, " : "",
				midnight() ?
					"¡no puedo resistir más esta apetencia!" :
				isnight ?
					"¡l& pido, ayúdeme de satisfacer esta apetencia creciente!" :
					"¡me encuentro poniendo un poco cansado.");
				if (verbuf[0] == '¡')
					verbuf[1] = toupper2(verbuf[1]);
				verbl_msg = verbuf;
			}
		} else if (mtmp->mpeaceful) {
			if (kindred && isnight) {
				Strcpy(verbuf, "¡Buena comida, herman@!");
				verbl_msg = verbuf;
 			} else if (nightchild && isnight) {
				Sprintf(verbuf,
				    "¡Qué bueno oírle a usted, hij@ de la noche!");
				verbl_msg = verbuf;
	    		} else
		    		verbl_msg = "Sólo bebo... pociones.";
    	        } else {
			int vampindex;
	    		static const char * const vampmsg[] = {
			       /* These first two (0 and 1) are specially handled below */
				/* LENGUA:  the substitution of V for W in the
				   English doesn't translate */
	    			"¡Quiero chupar tu %s!",
	    			"¡Vendré para %s sin pesadumbre!",
		    	       /* other famous vampire quotes can follow here if desired */
	    		};
			if (kindred)
			    verbl_msg = "¡Esto es mi campo de caza que te atreves rondar!";
			else if (youmonst.data == &mons[PM_SILVER_DRAGON] ||
				 youmonst.data == &mons[PM_BABY_SILVER_DRAGON]) {
			    /* Silver dragons are silver in color, not made of silver */
			    Sprintf(verbuf, "¡%s! ¡Tu lustre de plata no me espanta!",
					youmonst.data == &mons[PM_SILVER_DRAGON] ?
					"Tont@" : "Tont@ joven");
			    verbl_msg = verbuf; 
			} else {
			    vampindex = rn2(SIZE(vampmsg));
			    if (vampindex == 0) {
				Sprintf(verbuf, vampmsg[vampindex], body_part(BLOOD));
	    			verbl_msg = verbuf;
			    } else if (vampindex == 1) {
				Sprintf(verbuf, vampmsg[vampindex],
					Upolyd ? an(mons[u.umonnum].mname) : an(racenoun));
	    			verbl_msg = verbuf;
		    	    } else
			    	verbl_msg = vampmsg[vampindex];
			}
	        }
	    }
	    break;
	case MS_WERE:
	    if (flags.moonphase == FULL_MOON && (night() ^ !rn2(13))) {
		pline("¡%s se tira atrás la cabeza y emite %s horrible!",
		      Monnam(mtmp),
		      ptr == &mons[PM_HUMAN_WERERAT] ? "una chilla" : "un aullido");
		wake_nearto(mtmp->mx, mtmp->my, 11*11);
	    } else
		pline_msg =
		     "cuchichea inaudiblemente.  Todo lo que puedes comprender es \"luna\".";
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
		pline_msg = "aulla.";
	    } else if (mtmp->mpeaceful) {
		if (mtmp->mtame &&
			(mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			 moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
		    pline_msg = "lloriquea.";
		else if (mtmp->mtame && EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "yepa.";
		else {
		    if (mtmp->data != &mons[PM_DINGO])	/* dingos do not actually bark */
			    pline_msg = "ladra.";
		}
	    } else {
		pline_msg = "gimotea.";
	    }
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			mtmp->mtame < 5)
		    pline_msg = "aulla.";
		else if (moves > EDOG(mtmp)->hungrytime)
		    pline_msg = "maulla.";
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "ronronea.";
		else
		    pline_msg = "maya.";
		break;
	    } /* else FALLTHRU */
	case MS_GROWL:
	    pline_msg = mtmp->mpeaceful ? "gruñe." : "gimotea!";
	    break;
	case MS_ROAR:
	    pline_msg = mtmp->mpeaceful ? "gruñe." : "gruñe!";
	    break;
	case MS_SQEEK:
	    pline_msg = "chilla.";
	    break;
	case MS_SQAWK:
	    if (ptr == &mons[PM_RAVEN] && !mtmp->mpeaceful)
	    	verbl_msg = "¡Nunca más!";
	    else
	    	pline_msg = "grazna.";
	    break;
	case MS_HISS:
	    if (!mtmp->mpeaceful)
		pline_msg = "sisea!";
	    else return 0;	/* no sound */
	    break;
	case MS_BUZZ:
	    pline_msg = mtmp->mpeaceful ? "zumba." : "zumba enojadamente.";
	    break;
	case MS_GRUNT:
	    pline_msg = "gruñe.";
	    break;
	case MS_NEIGH:
	    if (mtmp->mtame < 5)
		pline_msg = "relincha.";
	    else if (moves > EDOG(mtmp)->hungrytime)
		pline_msg = "relincha un poco.";
	    else
		pline_msg = "relincha un poquito.";
	    break;
	case MS_WAIL:
	    pline_msg = "lamenta tristemente.";
	    break;
	case MS_GURGLE:
	    pline_msg = "gorjea.";
	    break;
	case MS_BURBLE:
	    pline_msg = "borbotante.";
	    break;
	case MS_SHRIEK:
	    pline_msg = "grita.";
	    aggravate();
	    break;
	case MS_IMITATE:
	    pline_msg = "te imita.";
	    break;
	case MS_BONES:
	    pline("%s rattles ruidosamente.", Monnam(mtmp));
	    pline("Te bloqueas por un momento.");
	    nomul(-2);
	    break;
	case MS_LAUGH:
	    {
                /* LENGUA:
		static const char * const laugh_msg[4] = {
		    "giggles.", "chuckles.", "snickers.", "laughs.",
		};
		pline_msg = laugh_msg[rn2(4)];
                */
		static const char * const laugh_msg[3] = {
		    "chacotea.", "se ríe.", "se carcajea.",
		};
		pline_msg = laugh_msg[rn2(3)];
	    }
	    break;
	case MS_MUMBLE:
	    pline_msg = "murmura incomprensiblemente.";
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) {
		verbl_msg = "Lo siento, no tengo más deseos.";
	    } else if (mtmp->mpeaceful) {
		if (ptr == &mons[PM_WATER_DEMON])
		    pline_msg = "borbota.";
		else
		    verbl_msg = "¡Estoy libre!";
	    } else verbl_msg = "¡Esto te enseñará no molestarme!";
	    break;
	case MS_BOAST:	/* giants */
	    if (!mtmp->mpeaceful) {
		switch (rn2(4)) {
		case 0: pline("%s se jacta sobre su colección de joyas.",
			      Monnam(mtmp));
			break;
		case 1: pline_msg = "se queja sobre un dieta de carnero.";
			break;
	       default: pline_msg = "grita \"Fi Fai Fo Fu!\" y se carcajea.";
			wake_nearto(mtmp->mx, mtmp->my, 7*7);
			break;
		}
		break;
	    }
	    /* else FALLTHRU */
	case MS_HUMANOID:
	    if (!mtmp->mpeaceful) {
		if (In_endgame(&u.uz) && is_mplayer(ptr)) {
		    mplayer_talk(mtmp);
		    break;
		} else return 0;	/* no sound */
	    }
	    /* Generic peaceful humanoid behaviour. */
	    if (mtmp->mflee)
		pline_msg = "no quiere hablar contigo.";
	    else if (mtmp->mhp < mtmp->mhpmax/4)
		pline_msg = "geme.";
	    else if (mtmp->mconf || mtmp->mstun)
		verbl_msg = !rn2(3) ? "¿Huh?" : rn2(2) ? "¿Qué?" : "¿Eh?";
	    else if (!mtmp->mcansee)
		verbl_msg = "¡No puedo ver!";
	    else if (mtmp->mtrapped) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);

		if (t) t->tseen = 1;
		verbl_msg = mon_gender(mtmp)? "¡Estoy entrampada!" : "¡Estoy entrampado!";
	    } else if (mtmp->mhp < mtmp->mhpmax/2)
		pline_msg = "pide una poción de curación.";
	    else if (mtmp->mtame && !mtmp->isminion &&
						moves > EDOG(mtmp)->hungrytime)
		verbl_msg = "Tengo hambre.";
	    /* Specific monsters' interests */
	    else if (is_elf(ptr))
		pline_msg = "maldice a los orcos.";
	    else if (is_dwarf(ptr))
		pline_msg = "habla sobre minería.";
	    else if (likes_magic(ptr))
		pline_msg = "habla sobre magia.";
	    else if (ptr->mlet == S_CENTAUR)
		pline_msg = "discute la caza.";
	    else switch (monsndx(ptr)) {
		case PM_HOBBIT:
		    pline_msg = (mtmp->mhpmax - mtmp->mhp >= 10) ?
				"se queja sobre condiciones desagradables en el calabozo."
				: "te pregunta sobre el Anillo Único.";
		    break;
		case PM_ARCHEOLOGIST:
    pline_msg = "describe un artículo reciente en la revista \"Spelunker Today\".";
		    break;
#ifdef TOURIST
		case PM_TOURIST:
		    verbl_msg = "Aloha.";
		    break;
#endif
		default:
		    pline_msg = "discute la exploración del calabozo.";
		    break;
	    }
	    break;
	case MS_SEDUCE:
#ifdef SEDUCE
	    if (ptr->mlet != S_NYMPH &&
		could_seduce(mtmp, &youmonst, (struct attack *)0) == 1) {
			(void) doseduce(mtmp);
			break;
	    }
	    switch ((poly_gender() != (int) mtmp->female) ? rn2(3) : 0)
#else
	    switch ((poly_gender() == 0) ? rn2(3) : 0)
#endif
	    {
		case 2:
			verbl_msg = "Hola, marinero.";
			break;
		case 1:
                        /* LENGUA:  English, "comes on to you" */
			pline_msg = "coquetea contigo.";
			break;
		default:
			pline_msg = "te halaga.";
	    }
	    break;
#ifdef KOPS
	case MS_ARREST:
	    if (mtmp->mpeaceful)
		verbalize("Sólo los hechos, señor%s.",
		      flags.female ? "a" : "");
	    else {
		static const char * const arrest_msg[3] = {
		    "Cualquiera que digas puede usarse contra ti.",
		    "¡Estás bajo aresto!",
		    "¡Párate en el nobre del Ley!",
		};
		verbl_msg = arrest_msg[rn2(3)];
	    }
	    break;
#endif
	case MS_BRIBE:
	    if (mtmp->mpeaceful && !mtmp->mtame) {
		(void) demon_talk(mtmp);
		break;
	    }
	    /* fall through */
	case MS_CUSS:
	    if (!mtmp->mpeaceful)
		cuss(mtmp);
	    break;
	case MS_SPELL:
	    /* deliberately vague, since it's not actually casting any spell */
	    pline_msg = "parece murmurar un hechizo.";
	    break;
	case MS_NURSE:
	    if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
		verbl_msg = "¡Aparte usted esta arma antes de que dañe a alguien!";
	    else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
		verbl_msg = Role_if(PM_HEALER) ?
			  "Médic@, no puedo ayudarl& sin que usted coopere." :
			  "Desvístase, por favor, para que pueda examinarl&.";
#ifdef TOURIST
	    else if (uarmu)
		verbl_msg = "Quítese su camisa, por favor.";
#endif
	    else verbl_msg = "Relaje, esto no l& dolará no hay de qué.";
	    break;
	case MS_GUARD:
#ifndef GOLDOBJ
	    if (u.ugold)
#else
	    if (money_cnt(invent))
#endif
		verbl_msg = "Por favor, deje caer ese oro y sígame.";
	    else
		verbl_msg = "Sígame, por favor.";
	    break;
	case MS_SOLDIER:
	    {
		static const char * const soldier_foe_msg[3] = {
		    "¡Resistancia es inútil!",
		    "¡Eres carne para perro!",
		    "¡Réndete!",
		},		  * const soldier_pax_msg[3] = {
		    "¡Qué paga asquerosa recibimos aquí!",
		    "¡Las comidas no son aptas para orcos!",
		    "¡Me duelen los pies, he estado sobre ellos todo el día!",
		};
		verbl_msg = mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
					    : soldier_foe_msg[rn2(3)];
	    }
	    break;
	case MS_RIDER:
	    if (ptr == &mons[PM_DEATH] && !rn2(10))
		pline_msg = "está ocupado leyendo una copia de Sandman #8.";
	    else verbl_msg = "¿Qúien piensas que tú eres, Guerra?";
	    break;
    }

    if (pline_msg) {
	/* Punctuation fixup */
	const char *punct = "";
	int len = strlen(pline_msg);
	if (len!=0) {
	    if (pline_msg[len-1]=='?')
		punct = "¿";
	    else if (pline_msg[len-1]=='!')
		punct = "¡";
	}
	pline("%s%s %s", punct, Monnam(mtmp), pline_msg);
    } else if (verbl_msg) {
	if (strchr(verbl_msg, '@') || strchr(verbl_msg, '&')) {
	    /* Gender fixup */
	    if (verbl_msg != verbuf) {
		int i;
		Strcpy(verbuf, verbl_msg);
		verbl_msg = verbuf;
		for (i=0; verbuf[i]!=0; i++) {
		    if (verbuf[i]=='@')
			verbuf[i] = flags.female? 'a' : 'o';
		    else if (verbuf[i]=='&')
			verbuf[i] = flags.female? 'a' : 'e';
		}
	    }
	}
	verbalize(verbl_msg);
    }
    return(1);
}


int
dotalk()
{
    int result;
    boolean save_soundok = flags.soundok;
    flags.soundok = 1;	/* always allow sounds while chatting */
    result = dochat();
    flags.soundok = save_soundok;
    return result;
}

static int
dochat()
{
    register struct monst *mtmp;
    register int tx,ty;
    struct obj *otmp;

    if (is_silent(youmonst.data)) {
	pline("Siendo %s, no puedes hablar.", an(youmonst.data->mname));
	return(0);
    }
    if (Strangled) {
	No_puedes("hablar.  ¡Te estás ahogando!");
	return(0);
    }
    if (u.uswallow) {
	pline("Nadie puede oírte afuera.");
	return(0);
    }
    if (Underwater) {
	pline("No se puede comprender tu habla bajo el agua.");
	return(0);
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *)0) {
	/* standing on something in a shop and chatting causes the shopkeeper
	   to describe the price(s).  This can inhibit other chatting inside
	   a shop, but that shouldn't matter much.  shop_object() returns an
	   object iff inside a shop and the shopkeeper is present and willing
	   (not angry) and able (not asleep) to speak and the position contains
	   any objects other than just gold.
	*/
	price_quote(otmp);
	return(1);
    }

    if (!getdir("¿Hablar con quién? (en qué dirección)")) {
	/* decided not to chat */
	return(0);
    }

#ifdef STEED
    if (u.usteed && u.dz > 0)
	return (domonnoise(u.usteed));
#endif
    if (u.dz) {
	pline("Nadie puede oírte %s.", u.dz < 0 ? "arriba" : "abajo");
	return(0);
    }

    if (u.dx == 0 && u.dy == 0) {
/*
 * Let's not include this.  It raises all sorts of questions: can you wear
 * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
 * etc...  --KAA
	if (u.umonnum == PM_ETTIN) {
	    pline("Descubres que tu otra cabeza hace conversación aburrida.");
	    return(1);
	}
*/
	pline("Hablar contigo mismo es un mal hábito por un%s mazmorrer%c.",
		poly_gender()==1? "a" : "",
		poly_gender()==1? 'a' : 'o');
	return(0);
    }

    tx = u.ux+u.dx; ty = u.uy+u.dy;
    mtmp = m_at(tx, ty);

    if (!mtmp || mtmp->mundetected ||
		mtmp->m_ap_type == M_AP_FURNITURE ||
		mtmp->m_ap_type == M_AP_OBJECT)
	return(0);

    /* sleeping monsters won't talk, except priests (who wake up) */
    if ((!mtmp->mcanmove || mtmp->msleeping) && !mtmp->ispriest) {
	/* If it is unseen, the player can't tell the difference between
	   not noticing him and just not existing, so skip the message. */
	if (canspotmon(mtmp))
	    pline("%s no parece noticiarte.", Monnam(mtmp));
	return(0);
    }

    /* if this monster is waiting for something, prod it into action */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (mtmp->mtame && mtmp->meating) {
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
	pline("%s está comiendo ruidosamente.", Monnam(mtmp));
	return (0);
    }

    return domonnoise(mtmp);
}

#ifdef USER_SOUNDS

extern void FDECL(play_usersound, (const char*, int));

typedef struct audio_mapping_rec {
#ifdef USER_SOUNDS_REGEX
	struct re_pattern_buffer regex;
#else
	char *pattern;
#endif
	char *filename;
	int volume;
	struct audio_mapping_rec *next;
} audio_mapping;

static audio_mapping *soundmap = 0;

char* sounddir = ".";

/* adds a sound file mapping, returns 0 on failure, 1 on success */
int
add_sound_mapping(mapping)
const char *mapping;
{
	char text[256];
	char filename[256];
	char filespec[256];
	int volume;

	if (sscanf(mapping, "MESG \"%255[^\"]\"%*[\t ]\"%255[^\"]\" %d",
		   text, filename, &volume) == 3) {
	    const char *err;
	    audio_mapping *new_map;

	    if (strlen(sounddir) + strlen(filename) > 254) {
		raw_print("nombre de archivo de sonido es demasiado largo");
		return 0;
	    }
	    Sprintf(filespec, "%s/%s", sounddir, filename);

	    if (can_read_file(filespec)) {
		new_map = (audio_mapping *)alloc(sizeof(audio_mapping));
#ifdef USER_SOUNDS_REGEX
		new_map->regex.translate = 0;
		new_map->regex.fastmap = 0;
		new_map->regex.buffer = 0;
		new_map->regex.allocated = 0;
		new_map->regex.regs_allocated = REGS_FIXED;
#else
		new_map->pattern = (char *)alloc(strlen(text) + 1);
		Strcpy(new_map->pattern, text);
#endif
		new_map->filename = strdup(filespec);
		new_map->volume = volume;
		new_map->next = soundmap;

#ifdef USER_SOUNDS_REGEX
		err = re_compile_pattern(text, strlen(text), &new_map->regex);
#else
		err = 0;
#endif
		if (err) {
		    raw_print(err);
		    free(new_map->filename);
		    free(new_map);
		    return 0;
		} else {
		    soundmap = new_map;
		}
	    } else {
		Sprintf(text, "no puedo leer %.243s", filespec);
		raw_print(text);
		return 0;
	    }
	} else {
	    raw_print("error de sintaxis en SOUND");
	    return 0;
	}

	return 1;
}

void
play_sound_for_message(msg)
const char* msg;
{
	audio_mapping* cursor = soundmap;

	while (cursor) {
#ifdef USER_SOUNDS_REGEX
	    if (re_search(&cursor->regex, msg, strlen(msg), 0, 9999, 0) >= 0) {
#else
	    if (pmatch(cursor->pattern, msg)) {
#endif
		play_usersound(cursor->filename, cursor->volume);
	    }
	    cursor = cursor->next;
	}
}

#endif /* USER_SOUNDS */

#endif /* OVLB */

/*sounds.c*/
