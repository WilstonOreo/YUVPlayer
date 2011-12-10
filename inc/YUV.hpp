#pragma once
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

namespace YUV {

	typedef unsigned char u8;

	class YUVImage
	{
	public:
		YUVImage(int _width, int _height);
		~YUVImage();

		int width() { return width_; } 
		int height() { return height_; } 
		size_t size() { return size_; }

		inline u8 y(int _x, int _y) { return y_[_y*width_+_x]; }
		inline u8 u(int _x, int _y) { return u_[_y/2*width_/2+_x/2]; } 
		inline u8 v(int _x, int _y) { return v_[_y/2*width_/2+_x/2]; }

		inline u8* y() { return y_; }
		inline u8* u() { return u_; }
		inline u8* v() { return v_; }

	private:
		int width_, height_; 
		size_t size_;
		u8 *y_,*u_,*v_;
	};


	class YUVStream
	{
	public:
		YUVStream() {};
		YUVStream(string _filename, YUVImage* _yuvImage);

		string& filename() { return filename_; }

		virtual void close() = 0;
	private:
		string filename_;
	protected:
		YUVImage* yuvImage_;
	};

	class YUVInStream : YUVStream
	{
	public:
		YUVInStream(string _filename, YUVImage* _yuvImage, int _repeat = 0);
		~YUVInStream() { is_.close(); }

		bool read();

		YUVInStream& operator>> (YUVImage& yi)
		{
			if (!is_.good()) return *this;
			read(&yi);
			return *this;
		}

		void close() { is_.close(); }
		const bool good() { return is_.good(); }
	private:
		ifstream is_;
		bool repeat_;
		size_t readBytes,bytesToRead;

		void read(u8* buf, size_t size);
		void read(YUVImage* pYi);
	};

	class YUVOutStream: YUVStream
	{
	public:
		YUVOutStream(string _filename, YUVImage* _yuvImage);
		~YUVOutStream() { os_.close(); }

		bool write();
		void reinit();

		YUVOutStream& operator<< ( YUVImage& yi)
		{
			if (!yi.y() || !yi.u() || !yi.v()) return *this;
			write(yuvImage_);
			return *this;
		}

		void close() { os_.close(); }
	private:
		ofstream os_;

		void write(u8* buf, size_t size);
		void write(YUVImage* pYi);
	};




}

typedef unsigned char byte;

