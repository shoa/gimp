/*
 * "$Id$"
 *
 *   SGI image file format library definitions.
 *
 *   Copyright 1997-1998 Michael Sweet (mike@easysw.com)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *   for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Revision History:
 *
 *   $Log$
 *   Revision 1.8  1998/06/06 23:22:20  yosh
 *   * adding Lighting plugin
 *
 *   * updated despeckle, png, sgi, and sharpen
 *
 *   -Yosh
 *
 *   Revision 1.5  1998/05/17 16:01:33  mike
 *   Added <unistd.h> header file.
 *
 *   Revision 1.4  1998/04/23  17:40:49  mike
 *   Updated to support 16-bit <unsigned> image data.
 *
 *   Revision 1.3  1998/02/05  17:10:58  mike
 *   Added sgiOpenFile() function for opening an existing file pointer.
 *
 *   Revision 1.2  1997/06/18  00:55:28  mike
 *   Updated to hold length table when writing.
 *   Updated to hold current length when doing ARLE.
 *
 *   Revision 1.1  1997/06/15  03:37:19  mike
 *   Initial revision
 */

#ifndef _SGI_H_
#  define _SGI_H_

#  include <stdio.h>
#  include <stdlib.h>
#  include <unistd.h>
#  include <string.h>

#  ifdef __cplusplus
extern "C" {
#  endif


/*
 * Constants...
 */

#  define SGI_MAGIC	474	/* Magic number in image file */

#  define SGI_READ	0	/* Read from an SGI image file */
#  define SGI_WRITE	1	/* Write to an SGI image file */

#  define SGI_COMP_NONE	0	/* No compression */
#  define SGI_COMP_RLE	1	/* Run-length encoding */
#  define SGI_COMP_ARLE	2	/* Agressive run-length encoding */


/*
 * Image structure...
 */

typedef struct
{
  FILE			*file;		/* Image file */
  int			mode,		/* File open mode */
			bpp,		/* Bytes per pixel/channel */
			comp;		/* Compression */
  unsigned short	xsize,		/* Width in pixels */
			ysize,		/* Height in pixels */
			zsize;		/* Number of channels */
  long			firstrow,	/* File offset for first row */
			nextrow,	/* File offset for next row */
			**table,	/* Offset table for compression */
			**length;	/* Length table for compression */
  unsigned short	*arle_row;	/* Advanced RLE compression buffer */
  long			arle_offset,	/* Advanced RLE buffer offset */
			arle_length;	/* Advanced RLE buffer length */
} sgi_t;


/*
 * Prototypes...
 */

extern int	sgiClose(sgi_t *sgip);
extern int	sgiGetRow(sgi_t *sgip, unsigned short *row, int y, int z);
extern sgi_t	*sgiOpen(char *filename, int mode, int comp, int bpp,
		         int xsize, int ysize, int zsize);
extern sgi_t	*sgiOpenFile(FILE *file, int mode, int comp, int bpp,
		             int xsize, int ysize, int zsize);
extern int	sgiPutRow(sgi_t *sgip, unsigned short *row, int y, int z);

#  ifdef __cplusplus
}
#  endif
#endif /* !_SGI_H_ */

/*
 * End of "$Id$".
 */
