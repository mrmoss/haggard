/*
Orion's Standard Library
Orion Sky Lawlor, 2/14/2000
NAME:		io.h

Header file for image i/o implementation
routines.
*/
#ifndef __OSL_RASTER_FORMAT_H
#define __OSL_RASTER_FORMAT_H

#include "osl/graphics.h"
#include "osl/io.h"
#include "osl/format.h"

using osl::io::InputStream;
using osl::io::OutputStream;

namespace osl {
	namespace graphics2d {
		/**
		  Abstract base class of all readers of a raster image format.
		  To define a new image format, inherit from this class.
		*/
		class AbstractRasterFormat : public osl::io::Format {
		public:
			//Don't forget about osl::io::Format methods
			
			// Read and write image to stream.  Default calls read/writeNoThrow.
			virtual void read(Raster *dest,InputStream &is) const;
			virtual void write(const Raster *src,OutputStream &os) const;
			
			//Alternate read/write routines: 
			//  These prefer to return a string rather than throwing an exception.
			//  They are never called from outside; so you should override
			//  *either* read/write *or* readNoThrow/writeNoThrow.
			virtual const char *readNoThrow(Raster *dest,InputStream &is) const;
			virtual const char *writeNoThrow(const Raster *src,OutputStream &os) const;
		};
		
		/**
		  Add this format to our internal table of formats
		  used to match against filenames and by RasterFormat.
		*/
		void addFormat(AbstractRasterFormat *fmt);
	}; 
};

#endif //__OSL_RASTER_IO_H
