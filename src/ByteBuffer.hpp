#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#define DEFAULT_SIZE 4096

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <iostream>

typedef unsigned char byte;

class ByteBuffer
{
private:
	unsigned int rpos, wpos;
	std::vector<byte> buf;

	template <typename T>
	T read(void)
	{
		T data = read<T>(rpos);
		rpos += sizeof(T);
		return (data);
	}

	template <typename T>
	T read(unsigned int index) const
	{
		if (index + sizeof(T) <= buf.size())
			return *((T*)&buf[index]);
		return (0);
	}

	template <typename T>
	void append(T data)
	{
		unsigned int s = sizeof(data);

		if (size() < (wpos + s))
			buf.resize(wpos + s);
		memcpy(&buf[wpos], reinterpret_cast<byte*>(&data), s);

		wpos += s;
	}

	template <typename T>
	void insert(T data, unsigned int index)
	{
		if ((index + sizeof(data)) > size())
			return;
		
		memcpy(&buf[index], reinterpret_cast<T*>(&data), sizeof(data));
		wpos = index + sizeof(data);
	}

public:
	ByteBuffer(unsigned int size = DEFAULT_SIZE);
	ByteBuffer(byte *arr, unsigned int size);
	virtual ~ByteBuffer();

	unsigned int bytesRemaining();
	void clear();
	ByteBuffer *clone();
	bool equals(ByteBuffer *other);
	void resize(unsigned int newSize);
	unsigned int size();

	template <typename T>
	int find(T key, unsigned int start = 0)
	{
		int ret = -1;
		unsigned int len = buf.size();
		for (unsigned int i = start; i < len; i++)
		{
			T data = read<T>(i);
			if ((key != 0) && (data == 0))
				break;

			if (data == key)
			{
				ret = i;
				break;
			}
		}
		return (ret);
	}

	void replace(byte key, byte rep, unsigned int start = 0, bool firstOccuranceOnly = false);
	void erase(int startPos, int endPos);

	// Read
	byte peek();
	byte get();
	byte get(unsigned int index);
	void getBytes(byte *buf, unsigned int len);
	char getChar();
	char getChar(unsigned int index);
	double getDouble();
	double getDouble(unsigned int index);
	float getFloat();
	float getFloat(unsigned int index);
	int getInt();
	int getInt(unsigned int index);
	long getLong();
	long getLong(unsigned int index);
	short getShort();
	short getShort(unsigned int index);

	// Write
	void put(ByteBuffer *src);
	void put(byte b);
	void put(byte b, unsigned int index);
	void putBytes(byte *b, unsigned int len);
	void putBytes(byte *b, unsigned int len, unsigned int index);
	void putChar(char value);
	void putChar(char value, unsigned int index);
	void putString(byte *b, unsigned int len);
	void putDouble(double value);
	void putDouble(double value, unsigned int index);
	void putFloat(float value);
	void putFloat(float value, unsigned int index);
	void putInt(int value);
	void putInt(int value, unsigned int index);
	void putLong(long value);
	void putLong(long value, unsigned int index);
	void putShort(short value);
	void putShort(short value, unsigned int index);

	void setReadPos(unsigned int r)
	{
		rpos = r;
	}

	int getReadPos()
	{
		return (rpos);
	}

	void setWritePos(unsigned int w)
	{
		wpos = w;
	}

	int getWritePos()
	{
		return (wpos);
	}

	void printData()
	{
		if (size() == 0)
			return ;

		int printLen = (int)size();
		if (printLen > 1000)
		{
			std::cout << "-------- read Request --------" << std::endl;
			for (int i = 0; i < 100; i++)
				std::cout << buf[i];
			std::cout << "\n...." << std::endl;
			for (int i = printLen - 100; i < printLen; i++)
				std::cout << buf[i];
			std::cout << "\n------------------------------" << std::endl;
		}
		else
		{
			std::cout << "-------- read Request --------" << std::endl;
			for (int i = 0; i < (int)size(); i++)
			{
				if (buf[i] == 13 || buf[i] == 10)
				{
					std::cout << "/" << (int)buf[i];
					if (buf[i] == 10)
						std::cout << "\n";
				}
				else
					std::cout << buf[i];
			}
			std::cout << "\n------------------------------" << std::endl;
		}
	}
};

#endif