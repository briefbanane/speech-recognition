/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <alsa/asoundlib.h>


//Groesse eines Blocks beim Lesen
#define FRAMES_IN_BLOCK 0x1000
//Anzahl der Bytes pro Frame eines Blockes
#define BLOCKSIZE (FRAMES_IN_BLOCK * (WAVE_BITS_PER_SAMPLE / 8))

//Konstante, die das PCM-Format angibt, muss mit dem Format in ../wave/wave.h uebereinstimmen
#define PCM_FORMAT SND_PCM_FORMAT_S16_LE

/*
 * Eine einfach verkettete Liste, die beim Einlesen verwendet wird, da nicht im Voraus bekannt ist, wie viel gelesen wird.
 * (void *) buf			Puffer fuer Daten
 * (struct block *) next	Der naechste Block
 */
struct block
{
	void *buf;
	struct block *next;
};

//Zeiger auf den Kopf der Liste mit den Daten beim Lesen
struct block *head = NULL;
//Handle, der Zugriff auf die Hardware erlaubt
snd_pcm_t *handle = NULL;
/*
 * Hat drei verschiedene Status:
 *  0	Der Thread zum Lesen laeuft
 *  1	Thread soll beendet werden
 *  2	Thread laeft nicht
 */
int finish = 2;
//Anzahl der Bloecke, die gelesen wurden
unsigned int blocknumber = 0;

/*
 * Der Thread zum Einlesen von Audio
 * Es wird Blockweise gelesen, da nicht im Voraus bekannt ist, wie viel eingelesen wird.
 * Die Variablen head und blocknumber werden gesetzt und der Thread kann beendet werden, indem finish auf 1 gesetzt und gewartet wird, bis finish 2 ist.
 */
void capture(void);

int capture_start(const char *device)
{
	unsigned int freq = WAVE_SAMPLE_RATE;
	int freq_adjust_direction = 1;
	snd_pcm_hw_params_t *hw_params = NULL;
	pthread_t t;

	//Oeffnen des Handle zum Aufnehmen
	if (snd_pcm_open(&handle, !device ? "default" : device, SND_PCM_STREAM_CAPTURE, 0) < 0)
	{
		fprintf(stderr, "Konnte Geraet %s nicht oeffnen\n", !device ? "default" : device);
		return -1;
	}
	//Speicher fuer Hardware-Parameter reservieren
	if (snd_pcm_hw_params_malloc(&hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Reservieren von Speicher fuer die Hardware-Parameter\n");
		return -2;
	}
	//Standard Hardware-Parameter erfragen
	if (snd_pcm_hw_params_any(handle, hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Holen der Standard-Hardware-Parameter\n");
		return -3;
	}
	//Setzen der Hardware-Parameter innerhalb der Struktur
	if (snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen des Lese-\\Schreibe-Zugriffs\n");
		return -4;
	}
	if (snd_pcm_hw_params_set_format(handle, hw_params, PCM_FORMAT) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen des PCM-Formates\n");
		return -5;
	}
	if (snd_pcm_hw_params_set_rate_near(handle, hw_params, &freq, &freq_adjust_direction) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Frequenz\n");
		return -6;
	}
	if (snd_pcm_hw_params_set_channels(handle, hw_params, WAVE_CHANNELS) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Anzahl der Kanaele\n");
		return -7;
	}
	//Setzen der Hardware-Parameter beim Handle
	if (snd_pcm_hw_params(handle, hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Hardwareparameter\n");
		return -8;
	}
	//Speicher freigeben
	snd_pcm_hw_params_free(hw_params);
	//Hardware vorbereiten
	if (snd_pcm_prepare(handle) < 0)
	{
		fprintf(stderr, "Fehler beim Vorbereiten des Handles\n");
		return -9;
	}
	//Starten des Thread
	finish = 0;
	if (pthread_create(&t, NULL, (void *)capture, NULL) < 0)
	{
		fprintf(stderr, "Fehler beim Starten des Aufnahme-Threads\n");
		return -10;
	}
	return 0;
}

void capture_stop(unsigned int *size, void **buffer)
{
	unsigned int i = 0;
	struct block *now = NULL;
	//Hier werden die Adressen der verketteten Liste gespeichert, damit der Speicher freigegeben werden kann
	void **adresses = NULL;

	//Beenden des Thread
	finish = 1;
	while (finish != 2);
	//Handle schliessen
	snd_pcm_close(handle);
	//Speicher fuer Puffer und Adress-Liste reservieren und Groesse setzen
	*buffer = malloc(BLOCKSIZE * blocknumber);
	*size = BLOCKSIZE * blocknumber;
       	adresses = malloc(blocknumber * sizeof(void *));
	//Kopieren der Daten der verketteten Liste in den Puffer
	now = head;
	for (i = 0; i < blocknumber; i++)
	{
		now = now->next;
		//Merken der Adresse
		adresses[i] = now;
		memcpy(*buffer + (i * BLOCKSIZE), now->buf, BLOCKSIZE);
		free(now->buf);
	}
	//Speicher freigeben
	for (i = 0; i < blocknumber; i++)
		free(adresses[i]);
	free(head);
	free(adresses);
}

int play(const char *device, const void *buffer, const unsigned int size)
{
	int r = 0;
	unsigned int freq = WAVE_SAMPLE_RATE;
	unsigned int framenumber = size / (WAVE_BITS_PER_SAMPLE / 8);
	snd_pcm_hw_params_t *hw_params = NULL;

	//Oeffnen des Handle zum Abspielen
	if (snd_pcm_open(&handle, !device ? "default" : device, SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		fprintf(stderr, "Konnte Geraet %s nicht oeffnen\n", !device ? "default" : device);
		return -1;
	}
	//Speicher fuer Hardware-Parameter reservieren
	if (snd_pcm_hw_params_malloc(&hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Reservieren von Speicher fuer die Hardwareparameter\n");
		return -2;
	}
	//Standard Hardware-Parameter erfragen
	if (snd_pcm_hw_params_any(handle, hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Holen der Standard-Hardwareparameter\n");
		return -3;
	}
	//Setzen der Hardware-Parameter innerhalb der Struktur
	if (snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen des Lese-\\Schreibe-Zugriffs\n");
		return -4;
	}
	if (snd_pcm_hw_params_set_format(handle, hw_params, PCM_FORMAT) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen des PCM-Formates\n");
		return -5;
	}
	if (snd_pcm_hw_params_set_rate_near(handle, hw_params, &freq, 0) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Frequenz\n");
		return -6;
	}
	if (snd_pcm_hw_params_set_channels(handle, hw_params, WAVE_CHANNELS) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Anzahl der Kanaele\n");
		return -7;
	}
	//Setzen der Hardware-Parameter beim Handle
	if (snd_pcm_hw_params(handle, hw_params) < 0)
	{
		fprintf(stderr, "Fehler beim Setzen der Hardwareparameter\n");
		return -8;
	}
	//Speicher freigeben
	snd_pcm_hw_params_free(hw_params);
	//Setzen der Hardware-Parameter beim Handle
	if (snd_pcm_prepare(handle) < 0)
	{
		fprintf(stderr, "Fehler beim Vorbereiten des Handles\n");
		return -9;
	}
	//Abspielen der Daten im Puffer
	do
	{
		r = snd_pcm_writei(handle, buffer + (size - framenumber * (WAVE_BITS_PER_SAMPLE / 8)), framenumber);
		if (r < 0)
			fprintf(stderr, "WARNUNG: Fehler beim Schreiben von Audio\n");
		else
			framenumber -= r;
	} while (framenumber > 0);
	//Handle schliessen
	snd_pcm_close(handle);
	return 0;
}

void get_device_list(char ****devices, unsigned int *number)
{
	unsigned int i = 0;
	void **hints = NULL;
	//Hole die Liste der PCM-Geraete
	snd_device_name_hint(-1, "pcm", &hints);
	//Anzahl der Geraete ermitteln
	while (hints[i])
		i++;
	*number = i;
	//Speicher reservieren
	*devices = malloc(sizeof(char **) * i);
	//Eigenschaften der einzelnen Geraete ermitteln
	i = 0;
	while (hints[i])
	{
		char *ioid = NULL;
		(*devices)[i] = malloc(sizeof(char *) * 3);
		//Name holen
		(*devices)[i][0] = snd_device_name_get_hint(hints[i], "NAME");
		//Beschreibung holen
		(*devices)[i][1] = snd_device_name_get_hint(hints[i], "DESC");
		//Typ holen
		ioid = snd_device_name_get_hint(hints[i], "IOID");
		//Input oder Output
		if (ioid)
			(*devices)[i][2] = ioid;
		//Beides
		else
		{
			(*devices)[i][2] = malloc(sizeof(char) * 7);
			strncpy((*devices)[i][2], "Beides\0", 7);
		}
		i++;
	}
	//Speicher freigeben
	snd_device_name_free_hint(hints);
}

void capture(void)
{
	struct block *now = NULL;
	int r;

	//Initialisieren
	blocknumber = 0;
	head = malloc(sizeof(struct block));
	head->buf = NULL;
	head->next = NULL;
	//Schleife in der gelesen wird
	now = head;
	while (!finish)
	{
		//Neuen Block erzeugen
		unsigned int framenumber = FRAMES_IN_BLOCK;
		blocknumber++;
		now->next = malloc(sizeof(struct block));
		now = now->next;
		now->buf = malloc(BLOCKSIZE);
		now->next = NULL;
		//Lesen eines Blockes von der Hardware
		do
		{
			r = snd_pcm_readi(handle, now->buf + (FRAMES_IN_BLOCK - framenumber) * (WAVE_BITS_PER_SAMPLE / 8), framenumber);
			if (r < 0)
				fprintf(stderr, "WARNUNG: Fehler beim Lesen von Audio\n");
			else
				framenumber -= r;
		} while (framenumber > 0);
	}
	//Thread ist terminiert
	finish = 2;
}
