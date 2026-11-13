/* All user-facing strings (French). Keep every UI string here so another language is one file. */
#pragma once

#define S_PLAY "JOUER"
#define S_RESUME "REPRENDRE"
#define S_PLAYERS_FMT "%d JOUEUR%s"   /* count, plural suffix */
#define S_LIFE_FMT "%d PV"
#define S_DOT " · "
#define S_START "START"
#define S_HOST "HÉBERGER"
#define S_JOIN "REJOINDRE"
#define S_RESUME_SUB_FMT "T%d | J%d" /* turn, players */
#define S_FORMAT "FORMAT"
#define S_PLAYERS "JOUEURS"
#define S_SOLO "SOLO"
#define S_NEXT "SUIVANT"
#define S_SEAT_FMT "J%d"
#define S_SEAT_PROFILE_FMT "J%d · %s"
#define S_POISON "POISON"
#define S_CMD "CMDT"
#define S_DEFEATED "VAINCU"
#define S_MENU_UNDO "ANNULER"
#define S_MENU_LOG "JOURNAL"
#define S_MENU_SHEET_FMT "FICHE J%d"
#define S_MENU_NEW "NOUVELLE PARTIE"
#define S_MENU_BACK "RETOUR"
#define S_NOTHING_TO_UNDO "RIEN À ANNULER"
#define S_LOG_EMPTY "AUCUNE ACTION"
#define S_LOG_LIFE_FMT "J%d  PV %+d"
#define S_LOG_POISON_FMT "J%d  POISON %+d"
#define S_LOG_CMD_FMT "J%d  CMDT J%d %+d"
#define S_SHEET_POISON_FMT "%d / %d"
#define S_WINS_FMT "J%d GAGNE"
#define S_DEFEAT "DÉFAITE"
#define S_GAME_OVER "PARTIE TERMINÉE"
#define S_TURN_FMT "TOUR %d"
#define S_SEP " \xE2\x80\xBA " /* U+203A, text separator "J1 › J3" */
#define S_TAP_BACK "TOUCHER POUR REVENIR"
/* markers panel: one entry per kind from SD_POISON to SD_DAYNIGHT, full name and 3-letter chip label */
#define S_MARKER_NAMES \
    {"POISON", "ÉNERGIE", "EXPÉRIENCE", "TRÉSOR", "NOURRITURE", "TEMPÊTE", "RADIATION", "ASCENSION", "MONARQUE", \
     "INITIATIVE", "JOUR / NUIT"}
#define S_MARKER_ABBR {"PSN", "ÉNG", "EXP", "TRÉ", "NOU", "TMP", "RAD", "ASC", "MON", "INI", "J/N"}
#define S_YES "OUI"
#define S_NO "NON"
#define S_NONE "—"
#define S_DAY "JOUR"
#define S_NIGHT "NUIT"
#define S_CMD2 "CMDT 2"
