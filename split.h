/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __SPLIT_H__
#define __SPLIT_H__

#include "frame.h"
#include <sys/types.h>

/*
 * Struktur, die ein Sprachsignal aufnimmt, indem es dessen PCM-Daten abspeichert
 * (int16_t *) buffer	Die PCM-Werte des Signals
 * (unsigned int) n	Die Anzahl der Werte
 */
typedef
	struct
	{
		int16_t *buffer;
		unsigned int number;
	} voice_signal;

/*
 * Sucht im Audiosignal, das in buffer enthalten ist, Woerter und Laute.
 * Werden solche erkannt, so wird der Bereich in eine voice_signal Struktur kopiert und diese der Liste hinzugefuegt.
 * Der Speicher der voice_signal-Liste wird innerhalb der Funktion reserviert und muss wieder frei gegeben werden.
 * <<<INPUT>>>
 * (int16_t *) buffer		Die PCM-Werte des gesamten Audiosignals
 * (unsigned int) n		Die Anzahl der Werte
 * (voice_signal **) signals	Adresse eines voice_signal-Zeigers, damit der Speicher reserviert und mit den PCM-Werte der erkannten Laute und Woerter gefuellt werden kann
 * <<<OUTPU>>>
 * (int)			Die Anzahl der erkannten Woerter, also die Laenge des Ergebnis-Vektors
 */
int split(int16_t *buffer, unsigned int n, voice_signal **signals);

#endif
