#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

namespace BinPkg
{
    struct ItemInternal
    {
        uint32_t Offset;
        uint32_t Length;
        char * Name;
    };

    struct Item
    {
    public:
        static constexpr size_t MAX_NAME_LENGTH = 1024;

        Item( uint32_t offset = 0, uint32_t length = 0, const char * name = "" );

        uint32_t Offset() const;
        uint32_t Length() const;
        std::string Name();
        bool IsEmpty();

        uint32_t m_offset;
        uint32_t m_length;
        // Array for storing C string with null-terminate accounted for.
        char m_name[MAX_NAME_LENGTH + 1];
    };

    class Header
    {
    public:
        size_t ItemCount() const;
        void Add( Item item );
        const std::vector< Item > & Items() const &;

    protected:
        std::vector< Item > m_items;
    };

    class Pkg
    {
    public:
        Pkg( std::iostream & stream );

        Header ParseHeader();
        int ReadCString( char * buf, size_t buf_length );
        void WriteHeader( const Header & hdr );
        void Write( const Item & item, const char * data, size_t data_length );

    protected:
        std::iostream & m_stream;
    };
}
