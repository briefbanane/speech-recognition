/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "wave.h"

/*
 * Beginnt mit der Aufnahme von Audio mithilfe des Geraetes device, wobei die Daten mit capture_stop geholt werden koennen.
 * Im Fehlerfall wird ein negativer Wert zurueck gegeben und eine Meldung auf stderr ausgegeben.
 * Nach jedem Aufruf von capture_start muss ein Aufruf von capture_stop erfolgen.
 * Die Aufnahme wird mit den Parametern aus ../wave/wave.h durchgefuehrt.
 * Ist device NULL, wird default ausgewaehlt.
 * <<<INPUT>>>
 * (const char *) device	Zeichenkette, die das Geraet beschreibt (siehe get_device_list)
 * <<<OUTPUT>>>
 * (int)			Im Fehlerfall ein negativer Wert, sonst 0
 */
int capture_start(const char *device);

/*
 * Stoppt die Aufnahme und liefert die Aufnahmedaten zurueck.
 * Vor jedem Aufruf von capture_stop muss ein Aufruf von capture_start erfolgen.
 * Der Speicher fuer den Puffer wird innerhalb der Funktion reserviert und muss spaeter freigegeben werden.
 * <<<INPUT>>>
 * (unsigned int *) size	Die Groesse des Puffers mit der Aufnahme
 * (void **) buffer		Die Adresse des Puffers mit den Aufnahmedaten
 */
void capture_stop(unsigned int *size, void **buffer);

/*
 * Spielt die Daten, die sich im Puffer befinden, mit dem Geraet device ab.
 * Im Fehlerfall wird ein negativer Wert zurueck gegeben und eine Meldung auf stderr ausgegeben.
 * Die Daten muessen mit den Parametern aus ../wave/wave.h zusammen passen.
 * Ist device NULL, wird default ausgewaehlt.
 * <<<INPUT>>>
 * (const char *) device	Zeichenkette, die das Geraet beschreibt (siehe get_device_list)
 * (const void *) buffer	Puffer, der die abzuspielenden Daten enthaelt
 * (const unsigned int ) size	Die Groesse des Puffers
 * <<<OUTPUT>>>
 * (int)			Im Fehlerfall ein negativer Wert, sonst 0
 */
int play(const char *device, const void *buffer, const unsigned int size);

/*
 * Gibt eine Liste mit Informationen von Geraeten als zweidimensionales Zeichenketten-Array zurueck:
 * Geraet1	Beschreibung1	Input
 * Geraet2	Beschreibung2	Output
 * Geraet3	Beschreibung3	Beides
 * ...
 * Der Speicherplatz fuer die Liste wird innerhalb der Funktion reserviert und muss spaeter freigegeben werden.
 * <<<INPUT>>>
 * (char ****) devices		Die Adresse der Liste von Gereaten als zweidimensionales Zeichenketten-Array
 * (unsigned int *) number	Zeiger auf die Variable, in der die Anzahl der Gereate gespeichert wird
 */
void get_device_list(char ****devices, unsigned int *number);

#endif
