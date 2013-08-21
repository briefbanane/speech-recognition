/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "split.h"
#include "fft.h"
#include "wave.h"
#include "audio.h"

#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define LEN_PART_MIN 10
#define LEN_MIN 20

//Stellt eine Liste fuer Sprachsignale dar.
struct voice_signal_list
{
	voice_signal vs;
	struct voice_signal_list *next;
};

int split(int16_t *buffer, unsigned int n, voice_signal **signals)
{
	unsigned int i = 0, j = 0, k = 0, start_pos = 0, len_sig = 0, len_nothing = 0, count = 0;
	char started = 0;
	double means[n / N], sds[n / N];
	double sdmean = 0;
	void **free_list = NULL;
	struct voice_signal_list *head = malloc(sizeof(struct voice_signal_list));
	struct voice_signal_list *now = head;

	head->next = NULL;

	//Berechne die Standardabweichungen der einzelnen Rahmen und die mittlere Standardabweichung eines Rahmens
	for (i = 0; i < n / N; i++)
	{
		means[i] = 0;
		for (j = 0; j < N; j++)
			means[i] += ((int16_t *)buffer)[i * N + j];
		means[i] /= N;
		sds[i] = 0;
		for (j = 0; j < N; j++)
			sds[i] += pow(((int16_t *)buffer)[i * N +j] - means[i], 2);
		sds[i] = sqrt(sds[i]) / (N - 1);
		sdmean += sds[i];
	}
	sdmean /= (n / N);

	//Vergleiche die Standardabweichungen der Rahmen mit der mittleren Standardabweichung, um herauszufinden wo die Signale im Strom vorkommen
	for (i = 0; i < n / N; i++)
	{
		//Signal vorhanden
		if (sdmean < sds[i])
		{
			//Initialisiere Signalstart
			if (!started)
			{
				started = 1;
				start_pos = i;
				len_nothing = 0;
			}
			len_sig++;
		}
		//Signal zu schwach
		else
			//Minimale Signallaenge eines Teilsignals erreicht (Signal - kurze Stille - Signal) soll als ein einziges Signal interpretiert werden
			if (len_sig > LEN_PART_MIN)
			{
				len_nothing = 0;
				len_sig = 0;
			}
			else
			{
				//Laenge der Stille
				len_nothing++;

				//Minimale Laenge erreicht?
				//Am Ende wird noch ein Spielraum von zwei Mal minimaler Laenge fuer das Ausklingen beruecksichtigt
				//Auch wenn der Strom zu Ende ist kann das Signal nicht verworfen werden, solange die minimale Laenge erreicht wurde
				if (started && i - LEN_MIN > start_pos && (len_nothing > 2 * LEN_MIN || i == n / N - 1))
				{
					//Neuen Eintrag anlegen und fuellen
					now->next = malloc(sizeof(struct voice_signal_list));
					now = now->next;
					now->next = NULL;
					now->vs.number = (i - start_pos) * N;
					now->vs.buffer = malloc(sizeof(int16_t) * now->vs.number);

					for (j = start_pos; j < i; j++)
					{
						for (k = 0; k < N; k++)
							now->vs.buffer[(j - start_pos) * N + k] = ((int16_t *)buffer)[j * N + k];
					}

					count++;
					started = 0;
				}
			}
	}

	//Fuelle den Parameter signals mit den erkannten Sprachsignalen und gib den Speicher frei
	*signals = malloc(sizeof(voice_signal) * count);
	free_list = malloc(sizeof(void *) * count);
	now = head;
	for (i = 0; i < count; i++)
	{
		now = now->next;
		free_list[i] = now;
		(*signals)[i] = now->vs;
	}
	for (i = 0; i < count; i++)
		free(free_list[i]);
	free(free_list);
	free(head);

	return count;
}
