/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "wave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//Derzeit geoeffnete Datei
static FILE *f = NULL;
//Inhalt der derzeit geoffneten Datei
static void *f_buf = NULL;
//Grundgeruest jedes WAVE-Headers
static char header[45] = "RIFF1234WAVEfmt 12341212123412341212data1234";

/*
 * Fuellt den Vergleichs-Header mit gueltigen Werten, die in wave.h spezifiert werden und der richtigen Datei- und Blockgroesse, die vom Parameter abgeleitet wird.
 * <<<INPUT>>>
 * (const unsigned int) size		Groesse der Datei
 */
void prepare_header(const unsigned int size);

int open_wave(const char *path)
{
	struct stat st;

	//Oeffnen
	f = fopen(path, "r+");
	if (f == NULL)
		return -1;
	if (stat(path, &st) != 0 || st.st_size <= 44)
	{
		fclose(f);
		return -1;
	}

	//Lesen
	f_buf = malloc(st.st_size);
	fread(f_buf, st.st_size, 1, f);

	//Header vorbereiten
	prepare_header(st.st_size);

	//Ueberpruefe, ob die wave-Datei die Bedingungen erfuellt.
	if (memcmp(header, f_buf, 4) != 0)
	{
		free(f_buf);
		fclose(f);
		return -2;
	}
	if (memcmp(header + 4, (char *)f_buf + 4, 4) != 0)
		fprintf(stderr, "WARNUNG: Groesse im wave-Header nicht gleich der Groesse auf dem Dateisystem\n");
	if (memcmp(header + 8, (char *)f_buf + 8, 32) != 0)
	{
		free(f_buf);
		fclose(f);
		return -2;
	}
	if (memcmp(header + 40, (char *)f_buf + 40, 4) != 0)
		fprintf(stderr, "WARNUNG: Groesse im wave-Header nicht gleich der Groesse auf dem Dateisystem oder mehrere Datenbloecke\n");
	return 0;
}

unsigned int read_pcm(void **buffer)
{
	unsigned int data_size = *((int *)f_buf + 10);
	*buffer = malloc(data_size);
	memcpy(*buffer, (char *)f_buf + 44, data_size);
	return data_size;
}

int write_pcm(const void *buffer, const unsigned int size, const char *path)
{
	FILE *file = NULL;

	//Dateideskriptor zuweisen
	if (path != NULL)
		file = fopen(path, "w");
	else
	{
		rewind(f);
		file = f;
	}
	if (file == NULL)
		return -1;

	//Header vorbereiten
	prepare_header(size + 44);

	//Schreiben
	fwrite(header, 44, 1, file);
	fwrite(buffer, size, 1, file);

	//Eventuell schliessen
	if (path != NULL)
		fclose(file);
	return 0;
}

void close_wave(void)
{
	free(f_buf);
	fclose(f);
}

void prepare_header(const unsigned int size)
{
	int values[9] = {size - 8, 16, WAVE_FORMAT, WAVE_CHANNELS, WAVE_SAMPLE_RATE, WAVE_BYTES_PER_SECOND, WAVE_FRAME_SIZE, WAVE_BITS_PER_SAMPLE, size - 44};

	memcpy(header + 4, values, 4);
	memcpy(header + 16, values + 1, 4);
	memcpy(header + 20, values + 2, 2);
	memcpy(header + 22, values + 3, 2);
	memcpy(header + 24, values + 4, 4);
	memcpy(header + 28, values + 5, 4);
	memcpy(header + 32, values + 6, 2);
	memcpy(header + 34, values + 7, 2);
	memcpy(header + 40, values + 8, 4);
}
