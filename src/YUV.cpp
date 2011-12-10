#include <YUV.hpp>

namespace YUV 
{
	YUVStream::YUVStream(string _filename, YUVImage* _yuvImage) 
		: filename_(_filename),
		yuvImage_(_yuvImage)
	{
	}

	YUVInStream::YUVInStream(string _filename, YUVImage* _yuvImage, int _repeat) : YUVStream(_filename,_yuvImage)
	{
		repeat_=_repeat;
		readBytes = 0;
		bytesToRead = _repeat*_yuvImage->width()*_yuvImage->height()*3/2;

		is_.open(filename().c_str(),ios::binary| ios::in);
	}

	bool YUVInStream::read()
	{
			read(yuvImage_);
			return is_.good();
	}

	inline void YUVInStream::read(u8* buf, size_t size)
	{
		is_.read((char*)buf,size);
		readBytes += size;
		if (readBytes == bytesToRead && repeat_)
		{
			is_.seekg(0,ios::beg);
			readBytes = 0;
		}
	}

	inline void YUVInStream::read(YUVImage* pYi)
	{
		read(pYi->y(),pYi->size());
		read(pYi->u(),pYi->size()/4);
		read(pYi->v(),pYi->size()/4);
	}
	

	bool YUVOutStream::write() 
	{
		write(yuvImage_);
		return os_.good();
	}

	inline void YUVOutStream::write(u8* buf, size_t size)
	{
		os_.write((char*)buf,size);
	}

	inline void YUVOutStream::write(YUVImage* pYi)
	{
		write(pYi->y(),pYi->size());
		write(pYi->u(),pYi->size()/4);
		write(pYi->v(),pYi->size()/4);
	}


	YUVOutStream::YUVOutStream(string _filename, YUVImage* _yuvImage) : YUVStream(_filename,_yuvImage)
	{
		os_.open(filename().c_str(), ios::binary | ios::out);	
	}

	void YUVOutStream::reinit()
	{
		os_.close();
		os_.open(filename().c_str(), ios::binary | ios::out);	
	}


	YUVImage::YUVImage(int _width, int _height)
		: width_(_width),
		height_(_height)
	{
		size_=width_*height_;
		y_ = (u8*)malloc(size_*sizeof(u8));
		u_ = (u8*)malloc(size_*sizeof(u8)/4);
		v_ = (u8*)malloc(size_*sizeof(u8)/4);
	}
	
	YUVImage::~YUVImage()
	{
		free(y_); free(u_); free(v_);
	}
}