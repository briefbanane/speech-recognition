/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "frame.h"
#include "fft.h"
#include "wave.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//Ueberschneidung der Rahmen
#define OVERLAP (N / 8)

//Asymmetrische Dreiecksfilter
//Liste von Mittelfrequenzen; Bandbreite von links benachbarter bis rechts benachbarter Mittelfrequenz
static int mel_filters[N_FILTER] = {150, 200, 250, 300, 350, 400, 450,	//Linear
				490, 560, 640, 730, 840, 960, 1100,	//500-1000Hz Logarithmisch
				1210, 1340, 1480, 1640, 1810, 2000,	//1000-2000Hz Logarithmisch
				2250, 2520, 2840, 3190, 3580, 4020};	//2000-4000Hz Logarithmisch

unsigned int make_frames_hamming(int16_t *buffer, unsigned int n, frame **frames)
{
	//Die Rahmen ueberlappen sich zum Teil
	unsigned int frame_number = (n / (N - OVERLAP)) - 1;
	comp *result = malloc(sizeof(comp) * frame_number * N);
	comp *data = malloc(sizeof(comp) * frame_number * N);
	unsigned int pos = 0, i = 0, j = 0;
	double pi = 4 * atan(1.0);

	*frames = malloc(sizeof(frame) * frame_number);
	//Erzeuge die Rahmen
	for (i = 0; i < frame_number; i++)
	{
		pos = (i + 1) * (N - 64);
		for (j = 0; j < N; j++)
		{
			//Filtere die Werte, sodass sie am Rand duerfen weniger Gewicht haben.
			//Die diskrete Fourier-Transformation setzt eine periodische Funktion voraus, somit wuerden die Werte am Rand des Rahmens diese verfaelschen.
			data[i * N + j].real = buffer[pos + j] * (0.54 + 0.46 * cos(2 * pi * (j - N / 2) / N));
			data[i * N + j].imag = 0;
		}
	}
	//Transformiere die einzelnen Rahmen
	for (i = 0; i < frame_number; i++)
		fft(data + i * N, result + i * N, N);
	//Berechne die Intensitaeten und ignoriere die Phasenverschiebung
	for (i = 0; i < frame_number; i++)
		for (j = 0; j < N; j++)
			(*frames)[i].magnitudes[j] = sqrt(pow(result[i * N + j].real, 2) + pow(result[i * N + j].imag, 2));
	//Normalisiere Intensitaeten
	for (i = 0; i < frame_number; i++)
	{
		double mean = 0;
		for (j = 0; j < N; j++)
			mean += (*frames)[i].magnitudes[j];
		mean /= N;
		for (j = 0; j < N; j++)
			(*frames)[i].magnitudes[j] /= mean;
	}
	free(data);
	free(result);
	return frame_number;
}

void mfcc_features(frame *frames, unsigned int n, mfcc_frame *mfcc_frames)
{

	unsigned int i = 0, j = 0, k = 0;
	double filterOutput[N_FILTER];
	double filterSpectrum[N_FILTER][N] = {{0}};
	double c0 = sqrt(1.0 / N_FILTER);
	double cn = sqrt(2.0 / N_FILTER);
	double pi = 4 * atan(1.0);

	//Erzeuge fuer jeden Filter sein Spektrum.
	for (i = 0; i < N_FILTER; i++)
	{
		double maxFreq = 0, minFreq = 0, centerFreq = 0;

		//Der erste Mel-Filter besitzt unten keinen Nachbarfilter, also wird er symmetrisch gemacht.
		if (i == 0)
			minFreq = mel_filters[0] - (mel_filters[1] - mel_filters[0]);
		else
			minFreq = mel_filters[i - 1];
		centerFreq = mel_filters[i];

		//Der letzte Mel-Filter besitzt oben keinen Nachbarfilter, also wird er symmetrisch gemacht.
		if (i == N_FILTER - 1)
			maxFreq = mel_filters[N_FILTER - 1] + (mel_filters[N_FILTER - 1] - mel_filters[N_FILTER - 2]);
		else
			maxFreq = mel_filters[i + 1];

		//Berechne den Koeffizienten des Filters fuer jede Frequenz.
		for (j = 0; j < N; j++)
		{
			double freq = 1.0 * j * WAVE_SAMPLE_RATE / N;
			//Ist die Frequenz innerhalb des Filterbereiches?
			if (freq > minFreq && freq < maxFreq)
				//Bei der aufsteigenden oder absteigenden Flanke?
				if (freq < centerFreq)
					filterSpectrum[i][j] = 1.0 * (freq - minFreq) / (centerFreq - minFreq);
				else
					filterSpectrum[i][j] = 1 - 1.0 * (freq - centerFreq) / (maxFreq - centerFreq);
			else
				filterSpectrum[i][j] = 0;
		}
	}

	//Berechne die MFCC-Merkmale fuer jeden Rahmen.
	for (i = 0; i < n; i++)
	{
		//Setze jeden Filter ein und errechne die Summe.
		for (j = 0; j < N_FILTER; j++)
		{
			filterOutput[j] = 0;
			for (k = 0; k < N; k++)
				filterOutput[j] += frames[i].magnitudes[k] * filterSpectrum[j][k];
		}
		//Berechne die MFCC-Merkmale mithilfe der Summen und der diskreten Kosinus-Transformation.
		for (j = 0; j < N_MFCC; j++)
		{
			mfcc_frames[i].features[j] = 0;

			for (k = 0; k < N_FILTER; k++)
				mfcc_frames[i].features[j] += log(fabs(filterOutput[k]) + 1e-10) * cos((pi * (2 * k + 1) * j) / (2 * N_FILTER));

			if (j)
				mfcc_frames[i].features[j] *= cn;
			else
				mfcc_frames[i].features[j] *= c0;
		}
	}
}
