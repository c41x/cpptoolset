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
}

namespace granite { namespace base {

namespace detail {
//- PNG
// odczyt
struct pngr {
	const_stream s;
	png_size_t pos;
};

void PNGAPI read_data_fn_const(png_structp png_ptr, png_bytep outdata, png_size_t length) {
	pngr *st = (pngr*)png_ptr->io_ptr;
	memcpy(outdata, (uint8*)st->s.data() + st->pos, length);
	st->pos += length;
}

// zapis
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

//- BMP
// why using pack? search for "mms-bitfields"
#pragma pack(push, 1)
struct bmp_header {
// file header
uint16 id;
uint32 size;
uint32 reserved;
uint32 data_offset;
// info header
uint32 header_size;
uint32 width;
uint32 height;
uint16 planes;
uint16 bpp;
uint32 compression;
uint32 image_size;
uint32 ppmx;
uint32 ppmy;
uint32 colors;
uint32 important_colors;
};

//- TGA
struct tga_header {
uint8 id;
uint8 color_map_type;
uint8 img_type;
uint8 first_entry_index[2];
uint16 color_map_length;
uint8 color_map_entry_size;
uint8 origx[2];
uint8 origy[2];
uint16 width;
uint16 height;
uint8 pxdepth;
uint8 img_descriptor;
};

struct tga_footer {
uint32 ext_offset;
uint32 developer_offset;
int8 sig[18];
};
#pragma pack(pop)

//- common
void rbSwapFlip(int channels, int width, int height, bool alphaFirst, image &i, bool flip = true) {
	// swap channels
	#ifdef GE_ENABLE_SSSE3
	if (channels == 3) {
		__m128i mask1 = SSE_SET8(2, 1, 0,
								 5, 4, 3,
								 8, 7, 6,
								 11, 10, 9,
								 14, 13, 12, 15);
		__m128i mask2 = SSE_SET8(0, 1,
								 4, 3, 2,
								 7, 6, 5,
								 10, 9, 8,
								 13, 12, 11,
								 14, 15);
		__m128i mask3 = SSE_SET8(0,
								 3, 2, 1,
								 6, 5, 4,
								 9, 8, 7,
								 12, 11, 10,
								 15, 14, 13);

		for (size_t v = 0; v < i.data.size(); v += 16 * 3) {
			__m128i data = _mm_load_si128((__m128i*)(i.data.data() + v));
			_mm_store_si128((__m128i*)(i.data.data() + v), _mm_shuffle_epi8(data, mask1));
			std::swap(i.data.data()[v + 15], i.data.data()[v + 17]);

			data = _mm_load_si128((__m128i*)(i.data.data() + v + 16));
			_mm_store_si128((__m128i*)(i.data.data() + v + 16), _mm_shuffle_epi8(data, mask2));
			std::swap(i.data.data()[v + 16 + 14], i.data.data()[v + 16 + 16]);

			data = _mm_load_si128((__m128i*)(i.data.data() + v + 16 + 16));
			_mm_store_si128((__m128i*)(i.data.data() + v + 16 + 16), _mm_shuffle_epi8(data, mask3));
		}
	}
	else if (channels == 4) {
		__m128i mask;
		if (alphaFirst)
			mask = SSE_SET8(3, 2, 1, 0,
							7, 6, 5, 3,
							11, 10, 9, 8,
							15, 14, 13, 12);
		else
			mask = SSE_SET8(2, 1, 0, 3,
							6, 5, 4, 7,
							10, 9, 8, 11,
							14, 13, 12, 15);

		for (size_t v = 0; v < i.data.size(); v += 16) {
			__m128i data = _mm_load_si128((__m128i*)(i.data.data() + v));
			_mm_store_si128((__m128i*)(i.data.data() + v), _mm_shuffle_epi8(data, mask));
		}
	}
	#else
	if (channels == 3 || (channels == 4 && !alphaFirst)) {
		for (size_t v = 0; v < i.data.size(); v += channels)
			std::swap(i.data.data()[v], i.data.data()[v + 2]);
	}
	else if (channels == 4 && alphaFirst) {
		for (size_t v = 0; v < i.data.size(); v += 4)
			std::swap(i.data.data()[v + 1], i.data.data()[v + 3]);
	}
	#endif

	// flip image
	if (flip) {
		uint8 *d = i.data.data();
		uint8 *b = new uint8[width * channels];
		const size_t lineSize = width * channels;
		for (int l = height; l > height / 2; --l) {
			uint8 *s = &i.data.data()[(l - 1) * lineSize];
			memcpy(b, s, lineSize);
			memcpy(s, d, lineSize);
			memcpy(d, b, lineSize);
			d += lineSize;
		}
		delete []b;
	}
}
} // > namespace detail

//- decoding
bool toImage(const_stream s, image &i) {
	if (s.size() > 2 && s.data()[0] == 0xff && s.data()[1] == 0xd8 && s.data()[2] == 0xff) {
		jpeg_source_mgr jsm;
		jsm.bytes_in_buffer = s.size();
		jsm.next_input_byte = (JOCTET*)s.data();
		jsm.init_source = detail::init_source;
		jsm.fill_input_buffer = detail::fill_input_buffer;
		jsm.skip_input_data = detail::skip_input_data;
		jsm.resync_to_restart = jpeg_resync_to_restart;
		jsm.term_source = detail::term_source;

		// czyta i zapisuje informacje (startuje dekompresje)
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		cinfo.src = &jsm;
		if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK) {
			cinfo.out_color_space = JCS_RGB;
			cinfo.out_color_components = 3;
			cinfo.do_fancy_upsampling = FALSE;
			jpeg_start_decompress(&cinfo);
			int _width = cinfo.image_width;
			int _height = cinfo.image_height;
			int _row = _width * 3;

			// alokuje i czyta dane z obrazu
			i.data.resize(3 * _width * _height);
			uint8 *pd = i.data.data();
			uint8 **rp = new uint8*[_height];
			for(int r = 0; r < _height; ++r, pd += _row)
				rp[r] = pd;
			uint32 rc = 0;
			while(cinfo.output_scanline < cinfo.output_height)
				rc += jpeg_read_scanlines(&cinfo, &rp[rc], cinfo.output_height - rc);
			delete []rp;

			// store info
			i.channels = 3;
			i.width = _width;
			i.height = _height;

			// konczy dekompresje
			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
		}
		else {
			logError("could not decode jpeg file");
			return false;
		}
	}
	else if (s.size() > 7 && 0 == png_sig_cmp(s.data(), 0, 8)) {
		// inicjalizuje struktury
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			logError("could not decode png (1)");
			return false;
		}
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			logError("could not decode png (2)");
			return false;
		}

		// odczytuje dane z pliku
		detail::pngr rs { s, 0 };
		png_set_read_fn(png_ptr, &rs, detail::read_data_fn_const);
		png_set_sig_bytes(png_ptr, 0);
		png_read_info(png_ptr, info_ptr);
		int _width = png_get_image_width(png_ptr, info_ptr);
		int _height = png_get_image_height(png_ptr, info_ptr);
		int8 _depth = png_get_bit_depth(png_ptr, info_ptr);
		int _type = png_get_color_type(png_ptr, info_ptr);

		// przeksztalcamy dane zeby dopasowac do potrzeb
		if (_type == PNG_COLOR_TYPE_GRAY && _depth < 8)
			png_set_expand_gray_1_2_4_to_8(png_ptr); // jesli mniej niz 8 bitowy -> zamienia na 8 bitowy w skali szarosci
		if (_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr); // paleta na rgb
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr); // jesli sa informacje o kanale alpha -> dodajemy go do danych obrazu
		if (_depth == 16)
			png_set_strip_16(png_ptr); // png obsluguje 16 bitow na kanal, ale moja klasa nie
		//if ((_type & PNG_COLOR_MASK_ALPHA) && iIgnoreAlpha)
		//png_set_strip_alpha(png_ptr); // na zyczenie mozna usunac kanal alpha
		//if(iInvertAlpha)
		//	png_set_invert_alpha(png_ptr); // odwraca kanal alpha (wykonuje negacje ~)
		png_read_update_info(png_ptr, info_ptr); // zastosowuje zmiany

		// alokuje dane, dekompresja
		uint32 _row = png_get_rowbytes(png_ptr, info_ptr);
		int8 _bypp = png_get_channels(png_ptr, info_ptr);
		i.data.resize(_bypp * _width * _height);
		uint8 *pd = i.data.data();
		uint8 **rd = new uint8*[_height];
		for(int r = 0; r < _height; ++r, pd += _row)
			rd[r] = pd;
		png_read_image(png_ptr, rd);
		delete []rd;

		// store info
		i.channels = _bypp;
		i.width = _width;
		i.height = _height;

		// zwalnia dane
		png_read_end(png_ptr, NULL);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	}
	else if (s.size() > 1 && s.data()[0] == 0x42 && s.data()[1] == 0x4d) {
		if (s.size() < sizeof(detail::bmp_header)) {
			logError("could not read BMP - data corrupted (1)");
			return false;
		}

		detail::bmp_header *bmh = (detail::bmp_header*)(s.data());
		if (bmh->compression == 1 || bmh->compression == 2) {
			logError(strs("could not read BMP - compression not supported: ", int(bmh->compression)));
			return false;
		}

		size_t _size = (bmh->bpp / 8) * bmh->width * bmh->height;
		if (s.size() < _size + sizeof(detail::bmp_header)) {
			logError("could not read BMP - data corrupted (2)");
			return false;
		}

		// store info
		i.channels = bmh->bpp / 8;
		i.width = bmh->width;
		i.height = bmh->height;

		auto dataBegin = s.data() + sizeof(detail::bmp_header);
		i.data.assign(dataBegin, dataBegin + _size);
		detail::rbSwapFlip(bmh->bpp / 8, bmh->width, bmh->height, true, i);
	}
	else if (s.size() > 2 && (s.data()[1] == 0 ||  s.data()[1] == 1) && (s.data()[2] == 1 || s.data()[2] == 2 || s.data()[2] == 3)) {
		if (s.size() < sizeof(detail::tga_header)) {
			logError("could not read TGA - data corrupted (1)");
			return false;
		}

		detail::tga_header *h = (detail::tga_header*)(s.data());
		if (h->img_type != 2 && h->img_type != 3 && h->img_type != 1) {
			logError("could not read TGA - compression and color maps not supported");
			return false;
		}

		if (h->pxdepth != 8 && h->pxdepth != 24 && h->pxdepth != 32) {
			logError(strs("could not read TGA - pixel depth value not supported: ", (int)h->pxdepth));
			return false;
		}

		size_t _size = (h->pxdepth / 8) * h->width * h->height;
		if (s.size() < _size + sizeof(detail::tga_header)) {
			logError("could not read TGA - data corrupted (2)");
			return false;
		}

		// store info
		i.channels = h->pxdepth / 8;
		i.width = h->width;
		i.height = h->height;

		auto dataBegin = s.data() + sizeof(detail::tga_header);
		i.data.assign(dataBegin, dataBegin + _size);
		detail::rbSwapFlip(h->pxdepth / 8, h->width, h->height, false, i);
	}
	else {
		logError("could not find codec to decode image");
		return false;
	}
	return true;
}

bool fromImage(const image &i, stream &s, imageCodec codec) {
	if (codec == imageCodecJPEG) {
		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		detail::jpeg_file_dest(&cinfo, &s);
		cinfo.image_width = i.width;
		cinfo.image_height = i.height;
		cinfo.input_components = i.channels;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, 90, TRUE);
		jpeg_start_compress(&cinfo, TRUE);

		// ustawia zrodlo - i alokuje tablice na wiersze obrazu
		uint32 _row = i.width * i.channels;
		const uint8 *pd = i.data.data();
		const uint8 *rp[1];
		while (cinfo.next_scanline < cinfo.image_height) {
			rp[0] = pd;
			pd += _row;
			jpeg_write_scanlines(&cinfo, const_cast<uint8**>(rp), 1);
		}

		// konczy kompresowanie jpega
		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		return true;
	}
	else if (codec == imageCodecPNG) {
		// inicjalizuje struktury
		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr) {
			logError("could not write PNG - not initialized (1)");
			return false;
		}
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			logError("could not write PNG - not initialized (2)");
			png_destroy_write_struct(&png_ptr, NULL);
			return false;
		}

		// ustawia potrzebne informacje pliku
		png_set_write_fn(png_ptr, &s, detail::write_data_fn, NULL);
		int colorType;
		if (i.channels == 1)
			colorType = PNG_COLOR_TYPE_GRAY;
		else if (i.channels == 3)
			colorType = PNG_COLOR_TYPE_RGB;
		else if (i.channels == 4)
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
		else {
			logError("could not write PNG - channel layout not supported");
			return false;
		}
		png_set_IHDR(png_ptr, info_ptr, i.width, i.height, 8, colorType,
					 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		// alokuje dane
		uint32 _row = i.width * i.channels;
		const uint8 *pd = i.data.data();
		const uint8 **rd = new const uint8*[i.height];
		for (int r = 0; r < i.height; ++r, pd += _row)
			rd[r] = pd;
		png_set_rows(png_ptr, info_ptr, const_cast<uint8**>(rd));
		png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

		// koniec zapisu
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete []rd;
		return true;
	}
	else if (codec == imageCodecBMP) {
		detail::bmp_header bh;
		bh.bpp = i.channels * 8;
		bh.colors = 0;
		bh.compression = 0;
		bh.data_offset = sizeof(detail::bmp_header);
		bh.header_size = 0x28;
		bh.height = i.height;
		bh.id = 0x4d42;
		bh.image_size = i.width * i.height * i.channels;
		bh.important_colors = 0;
		bh.planes = 1;
		bh.ppmx = 0;
		bh.ppmy = 0;
		bh.reserved = 0;
		bh.size = bh.header_size + bh.image_size;
		bh.width = i.width;
		s.write(&bh, sizeof(detail::bmp_header));
		image imgCopy = i;
		detail::rbSwapFlip(imgCopy.channels, imgCopy.width, imgCopy.height, true, imgCopy);
		s.write(imgCopy.data.data(), bh.image_size);
		return true;
	}
	else if (codec == imageCodecTGA) {
		detail::tga_footer tf;
		detail::tga_header th;
		th.color_map_entry_size = 0;
		th.color_map_length = 0;
		th.color_map_type = 0;
		th.first_entry_index[0] = 0;
		th.first_entry_index[1] = 0;
		th.height = i.height;
		th.id = 0;
		th.img_descriptor = (1 << 5); //?
		th.img_type = 2;
		th.origx[0] = 0;
		th.origx[1] = 0;
		th.origy[0] = 0;
		th.origy[1] = 0;
		th.pxdepth = i.channels * 8;
		th.width = i.width;
		tf.developer_offset = 0;
		tf.ext_offset = 0;
		std::memcpy(tf.sig, "TRUEVISION-XFILE.", 18);
		image imgCopy = i;
		detail::rbSwapFlip(imgCopy.channels, imgCopy.width, imgCopy.height, false, imgCopy, false);
		s.write(&th, sizeof(th));
		s.write(imgCopy.data.data(), imgCopy.width * imgCopy.height * imgCopy.channels);
		s.write(&tf, sizeof(tf));
		return true;
	}
	return false;
}

}}

// TODO: 1 channel images support
// TODO: compressed TGA/BMP
// TODO: fromImage - settings (jpeg quality)
// TODO: universal data transformations for image 2d/3d channels (vvvv inspired?)
