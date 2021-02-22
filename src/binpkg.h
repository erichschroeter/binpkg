#pragma once

#include <cstddef>
#include <iostream>
#include <vector>
#include <map>

namespace BinPkg
{
    class Item
    {
    public:
        struct ItemInternal
        {
            uint32_t Offset;
            uint32_t Length;
            char * Name;
        };

        /// The byte count for an empty item. Includes null terminator.
        static constexpr size_t EMPTY_ITEM_SIZE = sizeof(ItemInternal::Offset) + sizeof(ItemInternal::Length) + 1;
        static constexpr size_t MAX_NAME_LENGTH = 1024;

        Item( uint32_t offset = 0, uint32_t length = 0, const char * name = "" );

        uint32_t Offset() const;
        uint32_t & OffsetMut();
        void SetOffset( uint32_t value );
        uint32_t Length() const;
        uint32_t & LengthMut();
        const char * Name() const;
        char * NameMut();
        const std::string NameCopy() const;
        // const char * NameMut();
        bool IsEmpty();
        size_t Size() const;

    protected:
        // Array for storing C string with null-terminate accounted for.
        char m_name[MAX_NAME_LENGTH + 1];
        ItemInternal m_item;
    };

    class Header
    {
    public:
        Header( int32_t version = 0 );
        int32_t Version() const;
        void SetVersion( int32_t value );
        size_t ItemCount() const;
        size_t CalcSize() const;
        void Add( Item item );
        const Item * Get( int index ) const;
        const std::vector< Item > & Items() const &;
        std::vector< Item > & ItemsMut() &;

    protected:
        void UpdateOffsets();
        std::vector< Item > m_items;
        int32_t m_version;
    };

    class Pkg
    {
    public:
        Pkg( std::iostream & stream );

        Header ParseHeader();
        Header & HeaderMut() &;
        void Add( Item item, std::iostream & stream );
        const Item * Get( int index ) const;
        int ReadCString( char * buf, size_t buf_length );
        void WriteHeader( const Header & hdr );
        void Write( const Item & item, const char * data, size_t data_length );
        void Write();

    protected:
        /// Map of offset to item data.
        std::map< uint32_t, std::iostream * > m_items;
        Header m_header;
        std::iostream & m_stream;
    };
}
