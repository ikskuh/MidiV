#pragma once

#include <vector>
#include <fstream>
#include <cstdint>
#include <utility>
#include <string>
#include <json.hpp>

namespace Utils
{
    using Byte = std::uint8_t;
    using ByteArray = std::vector<Byte>;

	bool HasError();
	void FlagError();
	void ResetError();

    template<typename T>
    class Optional
    {
        char blob[sizeof(T)];
        T * _object;

    public:
        Optional() : _object(nullptr) { }
        Optional(T const & value) :
            _object(nullptr)
        {
            _object = new (blob) T (value);
        }
        Optional(T && value)
        {
            _object = new (blob) T (value);
        }
        Optional(Optional const & other)
        {
            if(other._object != nullptr)
            {
                _object = new (blob) T (other.data());
            }
            else
            {
                _object = nullptr;
            }
        }
        Optional(Optional && other)
        {
            if(other._object != nullptr)
            {
                _object = new (blob) T (std::move(other.data()));
            }
            else
            {
                _object = nullptr;
            }
        }
        ~Optional()
        {
            if(_object != nullptr)
                _object->~T();
        }
    public:
        bool good() const { return (_object != nullptr); }

        T & data() { return *_object; }
        T const & data() const { return *_object; }

    public:
        operator bool () const { return (_object != nullptr); }

        T * operator ->() { return _object; }
        T const * operator ->() const { return _object; }

        T & operator *() { return *_object; }
        T const & operator *() const { return *_object; }
    public:
        Optional operator= (Optional const & source)
        {
            if(_object != nullptr)
                _object->~T();
            if(source)
                _object = new (blob) T (*source._object);
            else
                _object = nullptr;
            return *this;
        }
        Optional operator= (Optional && source)
        {
            if(_object != nullptr)
                _object->~T();
            if(source)
                _object = new (blob) T (std::move(*source._object));
            else
                _object = nullptr;
            return *this;
        }
    };


    template<typename T = unsigned char>
    Optional<std::vector<T>> LoadFile(std::string const & fileName)
    {
        static_assert(sizeof(T) == 1, "T must be either unsigned or signed byte!");
        std::ifstream file;
		file.open(fileName, std::ios::binary | std::ios::in);
        if(file.good())
        {
            file.seekg(0, file.end);
            size_t const len = file.tellg();
            file.seekg(0, file.beg);

            std::vector<T> raw(len);
            file.read(reinterpret_cast<char*>(raw.data()), raw.size());
			file.close();
            return raw;
        }
        else
        {
            return Optional<std::vector<T>>();
        }
    }


	template<typename T>
	static T get(nlohmann::json const & data, char const * name, T const & _default)
	{
		if(data.find(name) != data.end())
			return data[name].get<T>();
		else
			return _default;
	}

	static nlohmann::json get(nlohmann::json const & data, char const * name)
	{
		if(data.find(name) != data.end())
			return data[name];
		else
			return nlohmann::json { };
	}
}
