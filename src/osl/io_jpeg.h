/*
Expose internals of jpeglib JPEG-reading routines.
This is rather hideous-- almost everybody should use
Raster::read/write instead of calling this directly.

This is used by specialized libraries like an MJPEG
reader that needs direct control over things like Huffman
tables, etc.

Orion Sky Lawlor, olawlor@acm.org, 7/20/2002
*/

#ifndef __OSL_RASTER_IO_JPEG_H
#define __OSL_RASTER_IO_JPEG_H

#include "osl/io.h"
#include "osl/raster.h"

extern "C" {
#include <jpeglib.h> /*for JPEG structs*/
};

namespace osl { namespace rasterIO { namespace jpeg {


class readJpegTwiddler {
public:
	virtual void afterHeader(jpeg_decompress_struct *j) =0;
};
void readJpeg(osl::graphics2d::Raster *dest,readJpegTwiddler *tw,osl::io::InputStream &is);


class writeJpegTwiddler {
public:
	virtual int afterDefaults(jpeg_compress_struct *j) =0;
};
void writeJpeg(const osl::graphics2d::Raster *src,writeJpegTwiddler *tw,osl::io::OutputStream &is);


}; }; };

#endif
