/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "list.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>

void new_word(mfcc_frame *frames, unsigned int n, char *name)
{
	FILE *f = NULL;
	unsigned int i = 0, j = 0;
	chdir("words");
	f = fopen(name, "w");
	fprintf(f, "%u\n", n);
	//In der Datei weden einfach alle Merkmale von Leerzeichen getrennt ausgegeben
	for (i = 0; i < n; i++)
		for (j = 0; j < N_MFCC; j++)
			fprintf(f, "%lf ", frames[i].features[j]);
	fclose(f);
	chdir("..");
}

int get_list(word *head)
{
	DIR *d = opendir("words");
	FILE *f = NULL;
	struct dirent *dentry = readdir(d);
	unsigned int i = 0, j = 0;
	unsigned int count = 0;

	//Holen der Woerter
	chdir("words");
	while (dentry != NULL)
	{
		//Ignoriere die Standardeintraege
		if (!memcmp(dentry->d_name, ".", 1))
		{
			dentry = readdir(d);
			continue;
		}

		//Eintrag mit Daten fuellen
		f = fopen(dentry->d_name, "r");
		head->name = malloc(strlen(dentry->d_name) + 1);
		memcpy(head->name, dentry->d_name, strlen(dentry->d_name) + 1);
		fscanf(f, "%u", &(head->n));
		head->frames = malloc(sizeof(mfcc_frame) * head->n);
		//Auslesen
		for (i = 0; i < head->n; i++)
			for (j = 0; j < N_MFCC; j++)
				fscanf(f, "%lf", &(head->frames[i].features[j]));
		fclose(f);

		dentry = readdir(d);
		count++;

		//Erzeuge neuen Eintrag, wenn benoetigt und ignoriere die Standardeintraege
		head->next = NULL;
		if (dentry != NULL && memcmp(dentry->d_name, ".", 2) && memcmp(dentry->d_name, "..", 3))
		{
			head->next = malloc(sizeof(word));
			head = head->next;
		}
	}
	chdir("..");

	return count;
}
