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

    class Item
    {
    public:
        Item( uint32_t offset, uint32_t length, const char * name );

        uint32_t Offset() const;
        uint32_t Length() const;
        std::string Name();
        bool IsEmpty();

    protected:
        uint32_t m_offset;
        uint32_t m_length;
        std::string m_name;
    };

    class Header
    {
    public:
        size_t ItemCount() const;
        void Add( Item item );

    protected:
        std::vector< Item > m_items;
    };

    class Pkg
    {
    public:
        Pkg( std::iostream & stream );

        Header ParseHeader();
        int ReadCString( char * buf, size_t buf_length );

    protected:
        std::iostream & m_stream;
    };
}
