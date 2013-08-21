/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "fft.h"

#include <math.h>
#include <stdlib.h>

void fft(comp *time, comp *freq, unsigned int n)
{
	double pi = 4 * atan(1.0);
	unsigned int i = 0, j = 0, k = 0;
	unsigned int bits = 0;

	if (n == 1)
	{
		freq[0] = time[0];
		return;
	}

	//Zaehle die benoetigten Bits zur binaeren Darstellung von n
	for (i = n; i > 1; i /= 2)
		bits++;

	//Veraendere die Indizierung fuer das Aufteilen in die Haelften
	for (i = 0; i < n; i++)
	{
		int reversed = 0;
		for (j = 0; j < bits; j++)
			reversed |= (((i >> j) % 2) << (bits - j - 1));
		freq[reversed] = time[i];
	}

	//Fuer alle Laengen, die die Haelften haben
	for (i = bits; i > 0; i--)
	{
		//Laenge der Haelften
		unsigned int length_groups = (n >> (i - 1));
		//Anzahl der Untergruppen
		unsigned int num_groups = n / length_groups;
		//Fuer alle Untergruppen
		for (j = 0; j < num_groups; j++)
		{
			//Fasse zwei Untergruppen zusammen
			for (k = 0; k < length_groups / 2; k++)
			{
				//Zahl in der ersten Haelfte
				unsigned int off1 = length_groups * j + k;
				//Zahl in der zweiten Haelfte
				unsigned int off2 = off1 + length_groups / 2;
				comp off1_val, off2_val;
				double cosine = cos((-2 * pi * k) / length_groups);
				double sine = sin((-2 * pi * k) / length_groups);
				//Multipliziere die komplexen Zahlen
				off1_val.real = freq[off1].real + (freq[off2].real * cosine - freq[off2].imag * sine);
				off1_val.imag = freq[off1].imag + (freq[off2].imag * cosine + freq[off2].real * sine);
				off2_val.real = freq[off1].real - (freq[off2].real * cosine - freq[off2].imag * sine);
				off2_val.imag = freq[off1].imag - (freq[off2].imag * cosine + freq[off2].real * sine);
				freq[off1] = off1_val;
				freq[off2] = off2_val;
			}
		}
	}
	//Normalisiere das Ergebnis
	for (i = 0; i < n; i++)
	{
		freq[i].real /= n;
		freq[i].imag /= n;
		freq[i].imag *= -1;
	}
}
