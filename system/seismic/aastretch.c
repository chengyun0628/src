/* Anti-aliasing interpolation for Kirchhoff-type operators. */
/*
  Copyright (C) 2004 University of Texas at Austin
  
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

#include "aastretch.h"

#include <rsf.h>
/*^*/

static int nt, nd, **x;
static float t0,dt, **w, *a, *tmp, *tmp2;
static bool *m;

static void doubint(int n, float *tmp);

void aastretch_init (bool box /* if box instead of triangle */,
		     int n1   /* trace length */, 
		     float o1 /* trace origin */, 
		     float d1 /* trace sampling */, 
		     int n2   /* number of data samples */)
/*< initialization >*/
{
    nt = n1; 
    t0 = o1; 
    dt = d1; 
    nd = n2;

    x = sf_intalloc2(3,nd);
    m = sf_boolalloc(nd);
    w = sf_floatalloc2(3,nd);
    a = sf_floatalloc(nd);

    tmp = sf_floatalloc(n1*3);
    tmp2 = sf_floatalloc(n1);
}

void aastretch_define (const float *coord /* data coordinates [nd] */, 
		       const float *delt  /* antialiasing length [nd] */, 
		       const float *amp   /* amplitude [nd] */)
/*< Set up interpolation >*/
{
    int id, ix[3], j;
    float rx[3];

    for (id = 0; id < nd; id++) {
	m[id] = false;

	rx[0] = coord[id] - delt[id] - dt;
	rx[1] = coord[id];          
	rx[2] = coord[id] + delt[id] + dt;
	for (j=0; j < 3; j++) {
	    rx[j] = (rx[j] - t0)/dt; 
	    ix[j] = rx[j]; 
	    if ((ix[j] < 0) || (ix[j] > nt - 2)) {
		m[id] = true;
		break;
	    }
	    w[id][j] = rx[j] - ix[j];	    
	}
	if (m[id]) continue;

	x[id][0] = ix[0];
	x[id][1] = ix[1] + nt;
	x[id][2] = ix[2] + 2*nt;

	a[id] = dt*dt/(delt[id]*delt[id] + dt*dt);
	if (NULL != amp) a[id] *= amp[id];
    }
}

void aastretch_lop (bool adj    /* adjoint flag */,
		    bool add    /* addition flag */,
		    int n1, int n2, /* sizes */
		    float *ord  /* data [nd] */, 
		    float *modl /* model [nt] */)
/*< apply interpolation >*/
{
    int id, i1, i2, j, it;
    float w1, w2, aa;

    if (n1 != nd || n2 != nt) sf_error("%s: wrong sizes",__FILE__);

    sf_adjnull(adj,add,nd,nt,ord,modl);

    if (adj) {
	for (it=0; it < nt; it++) {
	    tmp2[it] = modl[it];
	}
	
	doubint (nt, tmp2);
	
	for (it=0; it < nt; it++) {
	    tmp[it+nt]   = 2.*tmp2[it];
	    tmp[it]      =   -tmp2[it];
	    tmp[it+2*nt] =   -tmp2[it];
	}
    } else {
	for (it=0; it < 3*nt; it++) {
	    tmp[it]=0.;
	}
    }

    for (id = 0; id < nd; id++) {
	if (m[id]) continue;
	
	aa = a[id];
	for (j=0; j < 3; j++) {
	    i1 = x[id][j]; 
	    i2 = i1 + 1;
	    
	    w2 = w[id][j]; 
	    w1 = 1. - w2;
	
	    w2 *= aa;
	    w1 *= aa;

	    if (adj) {
		ord[id] += w2 * tmp[i2] + w1 * tmp[i1];
	    } else {		
		tmp[i1] += w1 * ord[id];
		tmp[i2] += w2 * ord[id];
	    } 
	}
    }
  
    if (!adj) {
	for (it=0; it < nt; it++) {
	    tmp2[it] = 2.* tmp[it+nt] - tmp[it] - tmp[it+2*nt];
	}
	
	doubint (nt, tmp2);

	for (it=0; it < nt; it++) {
	    modl[it] += tmp2[it];
	}
    } 
}

static void doubint(int n, float *tmp)
/* double integration */
{
    float t;
    int it;

    
    t = 0.;
    for (it = 0; it < n; it++) {
	t += tmp[it];
	tmp[it] = t;
    }
    t = 0.;
    for (it = n-1; it >=0; it--) {
	t += tmp[it];
	tmp[it] = t;
    }
}

void aastretch_close (void)
/*< free allocated storage >*/
{
    free (x[0]);
    free (x);
    free (m);
    free (w[0]);
    free (w);
    free (a);
    free (tmp);
    free (tmp2);
}

/* 	$Id$ */
