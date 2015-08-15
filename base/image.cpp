#include "image.hpp"

#include <zlib.h>
#include <zconf.h>
#include <png.h>
#include <pngconf.h>
#include <pngstruct.h>
extern "C"{
	#include <jpeglib.h>
	#include <jconfig.h>
	#include <jmorecfg.h>
	#include <bmp.h>
}

namespace granite { namespace base {

namespace detail {
//- PNG
void PNGAPI read_data_fn(png_structp png_ptr, png_bytep outdata, png_size_t length) {
	stream *st = (stream*)png_ptr->io_ptr;
	st->read(outdata, length);
}

void PNGAPI write_data_fn(png_structp png_ptr, png_bytep outdata, png_size_t length) {
	stream *st = (stream*)png_ptr->io_ptr;
	st->write(outdata, length);
}

//- JPEG
// odczyt
void init_source(j_decompress_ptr cinfo) {}

boolean fill_input_buffer(j_decompress_ptr cinfo) {
	return 1; // zawsze ok
}

void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
	jpeg_source_mgr *jsm = cinfo->src;
	if (num_bytes > 0) {
		jsm->bytes_in_buffer -= num_bytes; // 'zmniejsza' ilosc danych pozostalych do odczytu
		jsm->next_input_byte += num_bytes; // i jednoczesnie zwieksza pozycje
	}
}

void term_source(j_decompress_ptr cinfo) {
	// konczy kompresje (jpeg_finish_decomress) - recznie usune pamiec
}

// jpeg - zapis
typedef struct {
	struct jpeg_destination_mgr jdm;
	stream *os;
	JOCTET buffer[4096];
} jpeg_dst_mgr;

// ustawia poczatkowe wartosci bufora i ustala wskazniki
void jpeg_init_destination(j_compress_ptr cinfo) {
	jpeg_dst_mgr *dest = (jpeg_dst_mgr*)cinfo->dest;
	dest->jdm.next_output_byte = dest->buffer;
	dest->jdm.free_in_buffer = 4096;
}

// resetuje bufor - zapisuje wyniki na bufor i jako wolne dane podaje te same co na poczatku
boolean jpeg_empty_output_buffer(j_compress_ptr cinfo) {
	jpeg_dst_mgr *dest = (jpeg_dst_mgr*)cinfo->dest;
	dest->os->write(dest->buffer, 4096);
	dest->jdm.next_output_byte = dest->buffer;
	dest->jdm.free_in_buffer = 4096;
	return TRUE;
}

// koniec zapisu jpega - zapisyjemy ile trzeba do pliku
void jpeg_term_destination(j_compress_ptr cinfo) {
	jpeg_dst_mgr *dest = (jpeg_dst_mgr*)cinfo->dest;
	uint32 saved = 4096 - dest->jdm.free_in_buffer;
	dest->os->write(dest->buffer, saved);
}

// ustawia wszystkie powyzsze funkcje do obslugi granitowego stream-a
void jpeg_file_dest(j_compress_ptr cinfo, stream *oStream) {
	if (cinfo->dest == NULL) // alokuje pamiec
		cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(jpeg_dst_mgr));

	jpeg_dst_mgr *dest = (jpeg_dst_mgr*)cinfo->dest;

	// tu ustawiam te powyzsze funkcje
	dest->jdm.init_destination = jpeg_init_destination;
	dest->jdm.empty_output_buffer = jpeg_empty_output_buffer;
	dest->jdm.term_destination = jpeg_term_destination;

	// i najwazniejsze - wskaznik do 'granitowego' stream-a
	dest->os = oStream;
}
}

bool toImage(const stream &s, image &i) {
	return false;
}

}}

// TODO: 3d image file format - research
