/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __FFT_H__
#define __FFT_H__

/*
 * Eine Struktur, die eine komplexe Zahl darstellt.
 * (double) real	Der Realteil der komplexen Zahl
 * (double) imag	Dar Imaginaerteil der komplexen Zahl
 */
typedef
	struct
	{
		double real;
		double imag;
	}
comp;

/*
 * Berechnet die diskrete Fourier-Transformation des Parameters time, der einen komplexen Vektor der Laenge n darstellt.
 * <<<INPUT>>>
 * (comp) time		Komplexer Vektor, der transformiert werden soll (Darstellung der Funktion im Zeitraum)
 * (comp) freq		Komplexer Vektor, in den die Ergebnisse der Transformation (also die Darstellung der Funktion im Frequenzraum) abgespeichert werden
 * (unsigned int) n	Die Anzahl der Werte im Eingangs- bzw. Ausgangsvektor
 */
void fft(comp *time, comp *freq, unsigned int n);

#endif
