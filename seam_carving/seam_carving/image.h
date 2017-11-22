#pragma once

#include <cstdlib>
#include <cstring>
#include <cmath>

#include "utils.h"

namespace seam_carving {
	template <typename RealType> inline std::enable_if_t<
		std::is_floating_point<RealType>::value, RealType
	> cast_color_component(unsigned char v) {
		return static_cast<RealType>(v) / 255;
	}
	template <typename RealType> inline std::enable_if_t<
		std::is_floating_point<RealType>::value, unsigned char
	> cast_color_component(RealType v) {
		return static_cast<unsigned char>(std::round(v * 255));
	}
	template <typename From, typename To> inline To cast_color_component(From v) {
		return static_cast<To>(v);
	}

	template <typename Elem = unsigned char> struct color_rgb {
		using element_type = Elem;

		color_rgb() = default;
		color_rgb(Elem rr, Elem gg, Elem bb) : r(rr), g(gg), b(bb) {
		}

		template <typename To> color_rgb<To> cast() const {
			return color_rgb<To>(
				cast_color_component<Elem, To>(r),
				cast_color_component<Elem, To>(g),
				cast_color_component<Elem, To>(b)
				);
		}

		color_rgb &operator+=(const color_rgb &rhs) {
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			return *this;
		}
		friend color_rgb operator+(color_rgb lhs, color_rgb rhs) {
			return lhs += rhs;
		}

		color_rgb &operator-=(const color_rgb &rhs) {
			r -= rhs.r;
			g -= rhs.g;
			b -= rhs.b;
			return *this;
		}
		friend color_rgb operator-(color_rgb lhs, color_rgb rhs) {
			return lhs -= rhs;
		}

		template <typename T> color_rgb &operator*=(T rhs) {
			r = static_cast<Elem>(r * rhs);
			g = static_cast<Elem>(g * rhs);
			b = static_cast<Elem>(b * rhs);
			return *this;
		}
		template <typename T> friend color_rgb operator*(color_rgb lhs, T rhs) {
			return lhs *= rhs;
		}
		template <typename T> friend color_rgb operator*(T lhs, color_rgb rhs) {
			return rhs *= lhs;
		}

		template <typename T> color_rgb &operator/=(T rhs) {
			r = static_cast<Elem>(r / rhs);
			g = static_cast<Elem>(g / rhs);
			b = static_cast<Elem>(b / rhs);
			return *this;
		}
		template <typename T> friend color_rgb operator/(color_rgb lhs, T rhs) {
			return lhs /= rhs;
		}

		Elem r, g, b;
	};
	using color_rgb_u8 = color_rgb<unsigned char>;
	using color_rgb_f = color_rgb<float>;
	using color_rgb_d = color_rgb<double>;

	template <typename Elem = unsigned char> struct color_rgba {
		using element_type = Elem;

		color_rgba() = default;
		color_rgba(Elem rr, Elem gg, Elem bb, Elem aa) : r(rr), g(gg), b(bb), a(aa) {
		}

		template <typename To> color_rgba<To> cast() const {
			return color_rgba<To>(
				cast_color_component<Elem, To>(r),
				cast_color_component<Elem, To>(g),
				cast_color_component<Elem, To>(b),
				cast_color_component<Elem, To>(a)
				);
		}

		color_rgba &operator+=(const color_rgba &rhs) {
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			a += rhs.a;
			return *this;
		}
		friend color_rgba operator+(color_rgba lhs, color_rgba rhs) {
			return lhs += rhs;
		}

		color_rgba &operator-=(const color_rgba &rhs) {
			r -= rhs.r;
			g -= rhs.g;
			b -= rhs.b;
			a -= rhs.a;
			return *this;
		}
		friend color_rgba operator-(color_rgba lhs, color_rgba rhs) {
			return lhs -= rhs;
		}

		template <typename T> color_rgba &operator*=(T rhs) {
			r = static_cast<Elem>(r * rhs);
			g = static_cast<Elem>(g * rhs);
			b = static_cast<Elem>(b * rhs);
			a = static_cast<Elem>(a * rhs);
			return *this;
		}
		template <typename T> friend color_rgba operator*(color_rgba lhs, T rhs) {
			return lhs *= rhs;
		}
		template <typename T> friend color_rgba operator*(T lhs, color_rgba rhs) {
			return rhs *= lhs;
		}

		template <typename T> color_rgba &operator/=(T rhs) {
			r = static_cast<Elem>(r / rhs);
			g = static_cast<Elem>(g / rhs);
			b = static_cast<Elem>(b / rhs);
			a = static_cast<Elem>(a / rhs);
			return *this;
		}
		template <typename T> friend color_rgba operator/(color_rgba lhs, T rhs) {
			return lhs /= rhs;
		}

		Elem r, g, b, a;
	};
	using color_rgba_u8 = color_rgba<unsigned char>;
	using color_rgba_f = color_rgba<float>;
	using color_rgba_d = color_rgba<double>;

	template <typename Color> using image = dynamic_array2<Color>;
	using image_rgba_u8 = image<color_rgba_u8>;
	using image_rgba_f = image<color_rgba_f>;

	template <typename To, typename From> void image_cast(const image<From> &img, image<To> &result) {
		if (result.width() != img.width() || result.height() != img.height()) {
			result = image<To>(img.width(), img.height());
		}
		for (size_t y = 0; y < img.height(); ++y) {
			const image<From>::element_type *src = img.at_y(y);
			image<To>::element_type *dst = result.at_y(y);
			for (size_t x = 0; x < img.width(); ++x, ++src, ++dst) {
				*dst = src->cast<To::element_type>();
			}
		}
	}
	template <typename To, typename From> image<To> image_cast(const image<From> &img) {
		image<To> result(img.width(), img.height());
		image_cast<To, From>(img, result);
		return result;
	}

	struct image_loader {
	public:
		image_loader() {
			HRESULT hr = CoCreateInstance(
				CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(&_factory)
			);
			assert(hr == S_OK);
		}
		~image_loader() {
			_factory->Release();
		}
		image_rgba_u8 load_image(LPCWSTR filename) {
			IWICBitmapDecoder *decoder = nullptr;
			SC_COM_CHECK(_factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder));
			IWICBitmapFrameDecode *frame = nullptr;
			IWICBitmapSource *convertedframe = nullptr;
			SC_COM_CHECK(decoder->GetFrame(0, &frame));
			SC_COM_CHECK(WICConvertBitmapSource(GUID_WICPixelFormat32bppRGBA, frame, &convertedframe));
			frame->Release();
			UINT w, h;
			SC_COM_CHECK(convertedframe->GetSize(&w, &h));
			image_rgba_u8 result(static_cast<size_t>(w), static_cast<size_t>(h));
			SC_COM_CHECK(convertedframe->CopyPixels(
				nullptr, static_cast<UINT>(4 * w), static_cast<UINT>(4 * w * h),
				reinterpret_cast<BYTE*>(result.data())
			));
			convertedframe->Release();
			decoder->Release();
			return result;
		}
	protected:
		IWICImagingFactory *_factory = nullptr;
		com_usage _uses_com;
	};

#define SC_DEVICE_COLOR_ARGB(A, R, G, B)      \
	(								          \
		(static_cast<DWORD>(A) << 24) |       \
		(static_cast<DWORD>(R) << 16) |       \
		(static_cast<DWORD>(G) << 8) |        \
		static_cast<DWORD>(B)		          \
	)								          \

#define SC_DEVICE_COLOR_GETB(X) static_cast<unsigned char>((X) & 0xFF)
#define SC_DEVICE_COLOR_GETG(X) static_cast<unsigned char>(((X) >> 8) & 0xFF)
#define SC_DEVICE_COLOR_GETR(X) static_cast<unsigned char>(((X) >> 16) & 0xFF)
#define SC_DEVICE_COLOR_GETA(X) static_cast<unsigned char>(((X) >> 24) & 0xFF)

#define SC_DEVICE_COLOR_SETB(X, B) ((X) ^= ((X) & 0xFF) ^ static_cast<DWORD>(B))
#define SC_DEVICE_COLOR_SETG(X, G) ((X) ^= ((X) & 0xFF00) ^ (static_cast<DWORD>(G) << 8))
#define SC_DEVICE_COLOR_SETR(X, R) ((X) ^= ((X) & 0xFF0000) ^ (static_cast<DWORD>(R) << 16))
#define SC_DEVICE_COLOR_SETA(X, A) ((X) ^= ((X) & 0xFF000000) ^ (static_cast<DWORD>(A) << 24))

	struct device_color {
		device_color() = default;
		device_color(unsigned char a, unsigned char r, unsigned char g, unsigned char b) : argb(SC_DEVICE_COLOR_ARGB(a, r, g, b)) {
		}
		explicit device_color(const color_rgba_u8 &c) : device_color(c.a, c.r, c.g, c.b) {
		}
		explicit operator color_rgba_u8() const {
			return color_rgba_u8(
				SC_DEVICE_COLOR_GETR(argb),
				SC_DEVICE_COLOR_GETG(argb),
				SC_DEVICE_COLOR_GETB(argb),
				SC_DEVICE_COLOR_GETA(argb)
			);
		}

		void set(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
			argb = SC_DEVICE_COLOR_ARGB(a, r, g, b);
		}

		unsigned char get_a() const {
			return SC_DEVICE_COLOR_GETA(argb);
		}
		unsigned char get_r() const {
			return SC_DEVICE_COLOR_GETR(argb);
		}
		unsigned char get_g() const {
			return SC_DEVICE_COLOR_GETG(argb);
		}
		unsigned char get_b() const {
			return SC_DEVICE_COLOR_GETB(argb);
		}

		void set_a(unsigned char a) {
			SC_DEVICE_COLOR_SETA(argb, a);
		}
		void set_r(unsigned char r) {
			SC_DEVICE_COLOR_SETR(argb, r);
		}
		void set_g(unsigned char g) {
			SC_DEVICE_COLOR_SETG(argb, g);
		}
		void set_b(unsigned char b) {
			SC_DEVICE_COLOR_SETB(argb, b);
		}

		DWORD argb;
	};

	class sys_image {
	public:
		sys_image(HDC hdc, size_t w, size_t h) : _w(w), _h(h), _dc(CreateCompatibleDC(hdc)) {
			BITMAPINFO info;
			std::memset(&info, 0, sizeof(BITMAPINFO));
			info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			info.bmiHeader.biWidth = _w;
			info.bmiHeader.biHeight = _h;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 32;
			info.bmiHeader.biCompression = BI_RGB;
			_bmp = CreateDIBSection(_dc, &info, DIB_RGB_COLORS, reinterpret_cast<void**>(&_arr), nullptr, 0);
			_old = static_cast<HBITMAP>(SelectObject(_dc, _bmp));
		}
		sys_image(sys_image&&) = delete;
		sys_image(const sys_image&) = delete;
		sys_image &operator=(sys_image&&) = delete;
		sys_image &operator=(const sys_image&) = delete;
		~sys_image() {
			SelectObject(_dc, _old);
			DeleteObject(_bmp);
			DeleteDC(_dc);
		}

		size_t width() const {
			return _w;
		}
		size_t height() const {
			return _h;
		}
		device_color *data() const {
			return _arr;
		}
		device_color &at(size_t x, size_t y) {
			assert(x < _w && y < _h);
			return _arr[_w * y + x];
		}

		void copy_from_image(const image<color_rgba_u8> &img, size_t x = 0, size_t y = 0) {
			for (size_t dy = 0; dy < img.height(); ++dy) {
				device_color *cptr = _arr + ((y + dy) * _w + x);
				const color_rgba_u8 *orig = img.at_y(img.height() - dy - 1);
				for (size_t dx = 0; dx < img.width(); ++dx, ++cptr, ++orig) {
					*cptr = device_color(*orig);
				}
			}
		}

		void display(HDC hdc) const {
			display_region(hdc, 0, 0, 0, 0, _w, _h);
		}
		void display_region(HDC hdc, int sx, int sy, int dx, int dy, size_t w, size_t h) const {
			SC_WINAPI_CHECK(BitBlt(hdc, dx, dy, w, h, _dc, sx, sy, SRCCOPY));
		}
	protected:
		size_t _w, _h;
		HBITMAP _bmp, _old;
		HDC _dc;
		device_color *_arr = nullptr;
	};
}