#include "ByteBuffer.hpp"

ByteBuffer::ByteBuffer(unsigned int size)
{
	buf.resize(size);
	clear();
}

ByteBuffer::ByteBuffer(byte *arr, unsigned int size)
{
	if (arr == NULL)
	{
		buf.reserve(size);
		clear();
	}
	else
	{
		buf.reserve(size);
		clear();
		putBytes(arr, size);
	}
}

ByteBuffer::~ByteBuffer() { }

unsigned int ByteBuffer::bytesRemaining()
{
	return (size() - rpos);
}

void ByteBuffer::clear()
{
	rpos = 0;
	wpos = 0;
	
	buf.clear();
}

ByteBuffer *ByteBuffer::clone()
{
	ByteBuffer *ret = new ByteBuffer(buf.size());

	for (unsigned int i = 0; i < buf.size(); i++)
		ret->put(i, static_cast<byte>(get(i)));
	
	ret->setReadPos(0);
	ret->setWritePos(0);

	return (ret);
}

bool ByteBuffer::equals(ByteBuffer *other)
{
	if (size() != other->size())
		return (false);
	
	unsigned int len = size();
	for (unsigned int i = 0; i < len; i++)
	{
		if (static_cast<byte>(get(i)) != static_cast<byte>(other->get(i)))
			return (false);
	}
	return (true);
}

void ByteBuffer::resize(unsigned int newSize)
{
	buf.resize(newSize);
	rpos = 0;
	wpos = 0;
}

unsigned int ByteBuffer::size()
{
	return (buf.size());
}

void ByteBuffer::replace(byte key, byte rep, unsigned int start, bool firstOccuranceOnly)
{
	unsigned int len = buf.size();
	for (unsigned int i = start; i < len; i++)
	{
		byte data = read<byte>(i);

		if ((key != 0) && (data == 0))
			break;
		
		if (data == key)
		{
			buf[i] = rep;
			if (firstOccuranceOnly)
				return;
		}
	}
}

void ByteBuffer::erase(int startPos, int endPos)
{
	if (startPos == 0 && endPos == static_cast<int>(size()))
		clear();
	else
	{
		std::vector<byte>::iterator iter = this->buf.begin();
		this->rpos -= (endPos - startPos);
		this->wpos -= (endPos - startPos);
		buf.erase(iter + startPos, iter + endPos);
	}
}

byte ByteBuffer::peek()
{
	return (read<byte>(rpos));
}

byte ByteBuffer::get()
{
	return (read<byte>());
}

byte ByteBuffer::get(unsigned int index)
{
	return (read<byte>(index));
}

void ByteBuffer::getBytes(byte *buf, unsigned int len)
{
	for (unsigned int i = 0; i < len; i++)
		buf[i] = read<byte>();
}

int ByteBuffer::getDataToString(std::string& tempBody, size_t len)
{
	if (len > size())
	{
		tempBody.append(buf.begin(), buf.end());
		rpos = size();
		return (size());
	}
	else
	{
		tempBody.append(buf.begin(), buf.begin() + len);
		rpos += len;
		return (len);
	}
}

char ByteBuffer::getChar()
{
	return (read<char>());
}

char ByteBuffer::getChar(unsigned int index)
{
	return (read<char>(index));
}

double ByteBuffer::getDouble()
{
	return (read<double>());
}

double ByteBuffer::getDouble(unsigned int index)
{
	return (read<double>(index));
}

float ByteBuffer::getFloat()
{
	return (read<float>());
}

float ByteBuffer::getFloat(unsigned int index)
{
	return (read<float>(index));
}

int ByteBuffer::getInt()
{
	return (read<int>());
}

int ByteBuffer::getInt(unsigned int index)
{
	return (read<int>(index));
}

long ByteBuffer::getLong()
{
	return (read<long>());
}

long ByteBuffer::getLong(unsigned int index)
{
	return (read<long>(index));
}

short ByteBuffer::getShort()
{
	return (read<short>());
}

short ByteBuffer::getShort(unsigned int index)
{
	return (read<short>(index));
}



void ByteBuffer::put(ByteBuffer *src)
{
	int len = src->size();
	for (int i = 0; i < len; i++)
		append<byte>(src->get(i));
}

void ByteBuffer::put(byte b)
{
	append<byte>(b);
}

void ByteBuffer::put(byte b, unsigned int index)
{
	insert<byte>(b, index);
}

void ByteBuffer::putString(byte *b, unsigned int len)
{
	if (size() < (wpos + len))
		buf.resize(wpos + len);
	memcpy(&buf[wpos], reinterpret_cast<byte*>(b), len);

	wpos += len;
}

void ByteBuffer::putBytes(byte *b, unsigned int len)
{
	for (unsigned int i = 0; i < len; i++)
		append<byte>(b[i]);
}

void ByteBuffer::putBytes(byte *b, unsigned int len, unsigned int index)
{
	wpos = index;

	for (unsigned int i = 0; i < len; i++)
		append<byte>(b[i]);
}

void ByteBuffer::putChar(char value)
{
	append<char>(value);
}

void ByteBuffer::putChar(char value, unsigned int index)
{
	insert<char>(value, index);
}

void ByteBuffer::putDouble(double value)
{
	append<double>(value);
}

void ByteBuffer::putDouble(double value, unsigned int index)
{
	insert<double>(value, index);
}
void ByteBuffer::putFloat(float value)
{
	append<float>(value);
}

void ByteBuffer::putFloat(float value, unsigned int index)
{
	insert<float>(value, index);
}

void ByteBuffer::putInt(int value)
{
	append<int>(value);
}

void ByteBuffer::putInt(int value, unsigned int index)
{
	insert<int>(value, index);
}

void ByteBuffer::putLong(long value)
{
	append<long>(value);
}

void ByteBuffer::putLong(long value, unsigned int index)
{
	insert<long>(value, index);
}

void ByteBuffer::putShort(short value)
{
	append<short>(value);
}

void ByteBuffer::putShort(short value, unsigned int index)
{
	insert<short>(value, index);
}