/* Tapering (2d and 3d) */
/*
  Copyright (C) 2007 Colorado School of Mines
   
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
   
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
   
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <math.h>
#include <rsf.h>

#include "wexutl.h"
/*^*/

#include "wextap.h"

/*------------------------------------------------------------*/
wextap3d wextap_init(int n1_,
		     int n2_,
		     int   nt1_, 
		     int   nt2_, 
		     bool  b1_,
		     bool  b2_)
/*< 3-D initialize >*/
{
    int it;
    float gain;

    wextap3d tap;
    tap = (wextap3d) sf_alloc(1,sizeof(*tap));

    tap->n1 =n1_;
    tap->n2 =n2_;

    tap->nt1=nt1_;
    tap->nt2=nt2_;

    tap->b1 =b1_;
    tap->b2 =b2_;

    if (tap->nt1 > 0) {
	tap->tap1 = sf_floatalloc(tap->nt1);
	for (it=0; it < tap->nt1; it++) {
	    gain = sinf(0.5*SF_PI*it/tap->nt1);
	    tap->tap1[it]=(1+gain)/2.;
	}
    }
    if (tap->nt2 > 0) {
	tap->tap2 = sf_floatalloc(tap->nt2);
	for (it=0; it < tap->nt2; it++) {
	    gain = sinf(0.5*SF_PI*it/tap->nt2);
	    tap->tap2[it]=(1+gain)/2.;
	}
    }

    return tap;
}

/*------------------------------------------------------------*/
void wextap_close(wextap3d tap)
/*< 2-D free allocated storage >*/
{
    if (tap->nt1 > 0) free(tap->tap1);
    if (tap->nt2 > 0) free(tap->tap2);
}

/*------------------------------------------------------------*/
void wextap(sf_complex **tt  /* [n2][n1] tapered array (in and out) */,
	    wextap3d tap)
/*< 2-D taper >*/
{
    int it,i2,i1;
    float gain;

    for (it=0; it < tap->nt2; it++) {
	gain = tap->tap2[it];
	for (i1=0; i1 < tap->n1; i1++) {
#ifdef SF_HAS_COMPLEX_H
	    if (tap->b2) tt[        it  ][i1] *= gain;
	    ;            tt[tap->n2-it-1][i1] *= gain;
#else
	    if (tap->b2) tt[        it  ][i1] = sf_crmul(tt[        it  ][i1],gain);
	    ;            tt[tap->n2-it-1][i1] = sf_crmul(tt[tap->n2-it-1][i1],gain);
#endif
	}
    }

    for (it=0; it < tap->nt1; it++) {
	gain = tap->tap1[it];
	for (i2=0; i2 < tap->n2; i2++) {
#ifdef SF_HAS_COMPLEX_H
	    if (tap->b1) tt[i2][        it  ] *= gain;
	    ;            tt[i2][tap->n1-it-1] *= gain;
#else
	    if (tap->b1) tt[i2][        it  ] = sf_crmul(tt[i2][        it  ],gain);
	    ;            tt[i2][tap->n1-it-1] = sf_crmul(tt[i2][tap->n1-it-1],gain);
#endif
	}
    }
}
