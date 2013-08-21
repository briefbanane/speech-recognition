/*
 * Copyright 2012, 2013 Fabian Ebner
 * The program is licensed under the GPL version 3 or (at your opinion) any later version, see the file COPYING for details.
 */

#ifndef __COMPARE_H__
#define __COMPARE_H__

#include "frame.h"

/*
 * Vergleicht die beiden uebergebenen Vektoren von MFCC-Auspraegungen und liefert einen Wert zurueck,
 * der die Korrelation angibt. Je kleiner dieser ist, desto aehnlicher sind die beiden Vektoren.
 * <<<INPUT>>>
 * (mfcc_frame) mfcc_frames1	Der erste Vektor mit den MFCC-Merkmale
 * (unsigned int) n1		Die Laenge des zweiten Vektors
 * (mfcc_frame) mfcc_frames2	Der zweite Vektor mit den MFCC-Merkmale
 * (unsigned int) n2		Die Laenge des zweiten Vektors
 * <<<OUTPUT>>>
 * (double)			Wert, der die Korrelation der beiden Vektoren angibt
 */
double compare(mfcc_frame *mfcc_frames1, unsigned int n1, mfcc_frame *mfcc_frames2, unsigned int n2);

#endif
