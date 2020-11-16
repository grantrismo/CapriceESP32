// 15 August 1997, Rick Huebner:  Small changes made to adapt for MathLib

/* @(#)s_rint.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#include "Math_math.h"
#include "types.h"
#include "sections.h"

#include <stdlib.h>
#include <math.h>


tDouble math_rint(tDouble x)
/***********************************************************************
 *
 *	rint
 *
 ***********************************************************************/
	{
		/*
	  static const float TWO52 = 4503599627370496.0;
	  if (abs(x) < TWO52)
	    {
	      if (x > 0.0)
	        {
	          x += TWO52;
	          x -= TWO52;
	        }
	      else if (x < 0.0)
	        {
	          x = TWO52 - x;
	          x = -(x - TWO52);
	        }
	    }
			*/
	  return rint(x);

	}
