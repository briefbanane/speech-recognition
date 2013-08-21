/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __FRAME_H__
#define __FRAME_H__

#include <sys/types.h>

//Die Anzahl der Werte in jedem Rahmen
#define N 512

//Die Anzahl der Mel-Filter
#define N_FILTER 26

//Die Anzahl der MFCC-Merkmale
#define N_MFCC N_FILTER 

/*
 * Struktur, die einen Rahmen darstellt.
 * Die Werte im Rahmen sind die Intensitaeten der Darstellung der Funktion im Frequenzbereich.
 * (double) magnitudes	Die Intensitaeten der Fourier-transformierten Werte.
 */
typedef
	struct
	{
		double magnitudes[N];
	} frame;

/*
 * Struktur, die die MFCC-Merkmale eines Rahmens aufnimmt.
 * (double) features	Die errechneten MFCC-Merkmale
 */
typedef
	struct
	{
		double features[N_MFCC];
	} mfcc_frame;

/*
 * Erzeugt die Rahmen aus den Rohdateien.
 * Der Speicher fuer die Rahmen wird innerhalb der Funktion reserviert und sollte wieder frei gegeben werden.
 * <<<INPUT>>>
 * (int16_t) buffer	Puffer mit den PCM-Rohdateien, deren Rahmen erzeugt werden sollen
 * (unsigned int) n	Die Anzahl der Werte im Puffer
 * (frame **) frames	Adresse eines frame-Zeigers, damit Speicher reserviert und mit den Rahmen gefuellt werden kann
 * <<<OUTPUT>>>
 * (unsigned int)	Die Anzahl der erzeugten Rahmen, also die Laenge des frames-Vektors
 */
unsigned int make_frames_hamming(int16_t *buffer, unsigned int n, frame **frames);

/*
 * Errechnet die MFCC-Merkmale jedes uebergebenen Rahmens des Eingangs-Vektors frames mit Laenge n.
 * Der Ausgabe-Vektor mfcc_frames besitzt die selbe Laenge, der Speicher muss vorher reserviert werden.
 * <<<INPUT>>>
 * (frame *) frames		Die Rahmen, deren MFCC-Merkmale berechnet werden sollen
 * (unsigned int) n		Die Anzahl der Rahmen im Eingans-Vektor
 * (mfcc_frame *) mfcc_frames	Die errechneten MFCC-Merkmale
 */
void mfcc_features(frame *frames, unsigned int n, mfcc_frame *mfcc_frames);

#endif
