/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __WAVE_H__
#define __WAVE_H__

/*
 * Konstanten, die die wave-Dateien, mit denen gearbeitet wird, erfuellen muessen.
 */
#define WAVE_FORMAT 0x001 //PCM
#define WAVE_CHANNELS 1
#define WAVE_SAMPLE_RATE 44100
#define WAVE_BITS_PER_SAMPLE 16
#define WAVE_FRAME_SIZE ((WAVE_BITS_PER_SAMPLE + 7) >> 3 * WAVE_CHANNELS) //(bits/sample + 7) / 8 * channels
#define WAVE_BYTES_PER_SECOND (WAVE_SAMPLE_RATE * WAVE_FRAME_SIZE) //framesize * samplerate

/**
 * Versucht die mit filename spezifizierte wave-Datei zu oeffnen und liefert folgenden Wert zurueck:
 * (0) Erfolg
 * (-1) Datei kann nicht geoffnet werden
 * (-2) Ungueltiger wave-Header
 * Nach einem erfolgreichen open_wave-Aufruf koennen read_pcm und write_pcm aufgerufen werden.
 * War der Aufruf erfolgreich, darf diese Funktion erst wieder aufgerufen werden, wenn in der Zwischenzeit close_wave aufgerufen wurde.
 * <<<INPUT>>>
 * (const char *) path	Der Pfad zu der zu oeffnenden wave-Datei
 * <<<OUTPUT>>>
 * (int)		0, falls erfolgreich geoffnet wurde, sonst eine negative Zahl (siehe oben)
 */
int open_wave(const char *path);

/**
 * Liest aus der vorher geoffneten wave-Datei den ersten PCM-Datenblock aus (es sollte nur einen geben).
 * Der Zeiger der uebergeben wird sollte auf keinen Speicherbereich zeigen, die Funktion reserviert selbstaendig Speicher und liefert dessen Groesse zurueck.
 * <<<INPUT>>>
 * (void **) buffer	Zeiger auf den Puffer, in den die gelesenen PCM-Daten geschrieben werden
 * <<<OUTPUT>>>
 * (unsigned int)	Die Groesse des Puffers
 */
unsigned int read_pcm(void **buffer);

/**
 * Schreibt die sich in buffer befindlichen PCM-Daten der Laenge size in eine Datei.
 * Wird fuer path NULL uebergeben so muss zuvor open_wave aufgerufen werden, da in diesem Fall in die zuvor geoffnete Datei hinein geschrieben wird (schon vorhandene Daten werden geloescht!).
 * Wird path gesetzt, so wird diese Datei erstellt und der PCM-Block wird in die neu erzeugte Datei hinen geschrieben.
 * Wird nach write_pcm read_pcm aufgerufen so stehen die neuen Werte noch nicht zur Verfuegung, nur wenn die Datei geschlossen und neu geoeffnet wird.
 * Konnte die Datei nicht angelegt werden wird -1 zurueck gegeben, sonst 0
 * <<<INPUT>>>
 * (const void *) buffer	Der Puffer, der die Daten enthaelt
 * (const unsigned int) size	Die Groesse des Puffers
 * (const char *) path		NULL, wenn in bestehende Datei geschrieben werden soll, sonst der Pfad fuer die neue Datei
 * <<<OUTPUT>>>
 * (int)			Im Fehlerfall -1, sonst 0
 */
int write_pcm(const void *buffer, const unsigned int size, const char *path);

/**
 * Schliesst die zuvor mit open_wave geoffnete Datei wieder.
 * Diese Funktion muss aufgerufen werden bevor ein weiterer open_wave-Aufruf erfolgt.
 */
void close_wave(void);

#endif
