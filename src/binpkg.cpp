#include <cstring>

#include "binpkg.h"

using namespace BinPkg;

#pragma region Pkg

Pkg::Pkg( std::iostream & stream )
    :
    m_stream( stream )
{
}

int Pkg::ReadCString( char * buf, size_t buf_length )
{
    int length = 0;

    for ( int i = 0; i < buf_length; ++i )
    {
        m_stream.get( buf[i] );
        if ( buf[i] == '\0' )
        {
            break;
        }
        length++;
    }

    return length;
}

Header & Pkg::HeaderMut() &
{
    return m_header;
}

Header Pkg::ParseHeader()
{
    Header hdr;
    bool empty_item_found = false;

    do
    {
        Item item;
        m_stream.read( (char*)&item.OffsetMut(), sizeof(item.Offset()) );
        m_stream.read( (char*)&item.LengthMut(), sizeof(item.Length()) );
        // ReadCString( (char*)&item.NameMut(), Item::MAX_NAME_LENGTH );
        ReadCString( item.NameMut(), Item::MAX_NAME_LENGTH );
        // m_stream.read( (char*)&item.m_offset, sizeof(item.m_offset) );
        // m_stream.read( (char*)&item.m_length, sizeof(item.m_length) );
        // ReadCString( item.m_name, sizeof(item.m_name) );
        if ( item.IsEmpty() )
        {
            empty_item_found = true;
        }
        else
        {
            hdr.Add( item );
        }
    }
    while ( !empty_item_found );

    return hdr;
}

void Pkg::Add( Item item, std::iostream & stream )
{
    m_header.Add( item );
    m_items[item.Offset()] = &stream;
}

const Item * Pkg::Get( int index ) const
{
    return m_header.Get( index );
}

void Pkg::WriteHeader( const Header & hdr )
{
    auto version = hdr.Version();
    m_stream.write( (char*)&version, sizeof(version) );

    for ( const auto & item : hdr.Items() )
    {
        uint32_t offset = item.Offset();
        uint32_t length = item.Length();
        std::string name = item.NameCopy();
        m_stream.write( (char*)&offset, sizeof(offset) );
        m_stream.write( (char*)&length, sizeof(length) );
        m_stream.write( name.c_str(), name.size() + 1 ); // +1 for null terminator

        // m_stream.write( (char*)&item.OffsetMut(), sizeof(item.Offset()) );
        // m_stream.write( (char*)&item.LengthMut(), sizeof(item.Length()) );
        // m_stream.write( (char*)&item.NameMut(), std::strlen( item.NameMut() );

        // m_stream.write( (char*)&item.m_offset, sizeof( item.m_offset ) );
        // m_stream.write( (char*)&item.m_length, sizeof( item.m_length ) );
        // m_stream.write( item.m_name, std::strlen( item.m_name ) );
    }
}

void Pkg::Write( const Item & item, const char * data, size_t data_length )
{
    m_stream.seekp( item.Offset(), m_stream.beg );
    m_stream.write( data, data_length );
}

void Pkg::Write()
{
    WriteHeader( m_header );
    // for ( const auto & kp : m_items )
    for ( const auto & item : m_header.Items() )
    {
        std::iostream * stream = m_items[ item.Offset() ];
        char buffer[4096] = {0};
        stream->read( buffer, sizeof( buffer ) );
        size_t bytes_read = stream->gcount();
        if ( bytes_read > 0 )
        {
            Write( item, buffer, bytes_read );
        }
    }
}

#pragma endregion Pkg

#pragma region Header

Header::Header( int32_t version )
    :
    m_version( version )
{
}

int32_t Header::Version() const
{
    return m_version;
}

void Header::SetVersion( int32_t value )
{
    m_version = value;
}

size_t Header::ItemCount() const
{
    return m_items.size();
}

size_t Header::CalcSize() const
{
    size_t length = Item::EMPTY_ITEM_SIZE + sizeof(m_version);

    for ( auto & item : Items() )
    {
        length += sizeof(Item::ItemInternal::Offset);
        length += sizeof(Item::ItemInternal::Length);
        length += strlen( item.Name() ) + 1; // +1 for null-terminator
    }
    return length;
}

const std::vector< Item > & Header::Items() const &
{
    return m_items;
}

std::vector< Item > & Header::ItemsMut() &
{
    return m_items;
}

void Header::RecursivelyUpdateOffsets()
{
    size_t offset = CalcSize();
    Item * previous_item = nullptr;

    for ( auto & i : ItemsMut() )
    {
        if ( previous_item != nullptr )
        {
            offset += i.Length();
        }

        i.SetOffset( offset );
        previous_item = &i;
    }
}

void Header::Add( Item item )
{
    m_items.push_back( item );
    RecursivelyUpdateOffsets();
}

const Item * Header::Get( int index ) const
{
    return &m_items.at( index );
}

#pragma endregion Header

#pragma region item

Item::Item( uint32_t offset, uint32_t length, const char * name )
    :
    m_name{ 0 },
    m_item{ offset, length, m_name }
{
    std::strncpy( m_name, name, sizeof( m_name ) );
}

uint32_t Item::Offset() const
{
    return m_item.Offset;
}

uint32_t & Item::OffsetMut()
{
    return m_item.Offset;
}

void Item::SetOffset( uint32_t value )
{
    m_item.Offset = value;
}

uint32_t Item::Length() const
{
    return m_item.Length;
}

uint32_t & Item::LengthMut()
{
    return m_item.Length;
}

const char * Item::Name() const
{
    return m_name;
}

char * Item::NameMut()
{
    return m_name;
}

const std::string Item::NameCopy() const
{
    return std::string( m_name );
}

size_t Item::Size() const
{
    return sizeof(m_item.Offset) + sizeof(m_item.Length) + std::strlen( m_name ) + 1; // +1 for null terminator
}

bool Item::IsEmpty()
{
    return ( m_item.Offset == 0 && m_item.Length == 0 && m_name[0] == '\0' );
}

#pragma endregion item
