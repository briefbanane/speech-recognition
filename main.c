/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#include "fft.h"
#include "split.h"
#include "frame.h"
#include "list.h"
#include "compare.h"
#include "wave.h"
#include "audio.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void words_capture(int how);

int main(void)
{
	char ans[50];
	do
	{
		printf("(n)ew words, (l)ist words, (d)ictate words, (e)xit: ");
		scanf("%s", (char *)ans);
		switch (ans[0])
		{
			case 'n':
				usleep(200000);
				words_capture(0);
				break;
			case 'l':
				system("ls words");
				break;
			case 'd':
				usleep(200000);
				words_capture(1);
				break;
		}
	}
	while (ans[0] != 'e');
	return 0;
}

void words_capture(int how)
{
	void *buf = NULL;
	unsigned int size = 0, n = 0, i = 0;
	char ans[500];
	voice_signal *signals = NULL;
	printf("Please speak now (end with return)\n");
	capture_start(NULL);
	getchar();
	getchar();
	capture_stop(&size, &buf);
	n = split(buf, size / 2, &signals);
	for (i = 0; i < n; i++)
	{
		frame *frames = NULL;
		mfcc_frame *mfcc_frames = NULL;
		int frame_n = make_frames_hamming(signals[i].buffer, signals[i].number, &frames);
		mfcc_frames = malloc(sizeof(mfcc_frame) * frame_n);
		mfcc_features(frames, frame_n, mfcc_frames);
		if (!how)
		{
			play(NULL, signals[i].buffer, signals[i].number * 2);
			printf("Enter identifier (x to skip): ");
			scanf("%s", (char *)ans);
			if (!(ans[0] == 'x' && ans[1] == '\0'))
			{
				new_word(mfcc_frames, frame_n, (char *)ans);
				chdir("waves");
				char *path = malloc(strlen(ans) + 5);
				char *ext = ".wav";
				memcpy(path, ans, strlen(ans));
				memcpy(path + strlen(ans), ext, 5);
				write_pcm(signals[i].buffer, signals[i].number * 2, path);
				free(path);
				chdir("..");
			}
		}
		else
		{
			word *words = malloc(sizeof(word));
			double best = 1e10;
			char *name = NULL;
			void **word_adresses;
			unsigned int n = 0, i = 0, count = 0;
			if ((n = get_list(words)))
			{
				word_adresses = malloc(n * sizeof(void *));
				while (words != NULL)
				{
					double now = compare(mfcc_frames, frame_n, words->frames, words->n);
					word_adresses[count++] = words;
					if (now < best)
					{
						best = now;
						name = words->name;
					}
					words = words->next;
				}
				for (i = 0; i < count; i++)
					free(word_adresses[i]);
				free(word_adresses);
			}
			if (best < 3.5)
				printf("%f %s", best, name);
		}
		free(mfcc_frames);
		free(frames);
	}
	if (how)
		printf("\n");
	free(buf);
}
