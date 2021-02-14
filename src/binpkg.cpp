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

Header Pkg::ParseHeader()
{
    Header hdr;
    bool empty_item_found = false;

    do
    {
        Item item;
        m_stream.read( (char*)&item.m_offset, sizeof(item.m_offset) );
        m_stream.read( (char*)&item.m_length, sizeof(item.m_length) );
        ReadCString( item.m_name, sizeof(item.m_name) );
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

#pragma endregion Pkg

#pragma region Header

size_t Header::ItemCount() const
{
    return m_items.size();
}

void Header::Add( Item item )
{
    m_items.push_back( item );
}

#pragma endregion Header

#pragma region item

Item::Item( uint32_t offset, uint32_t length, const char * name )
    :
    m_offset( offset ),
    m_length( length ),
    m_name{ 0 }
{
    std::strncpy( m_name, name, sizeof( m_name ) );
}

uint32_t Item::Offset() const
{
    return m_offset;
}

uint32_t Item::Length() const
{
    return m_length;
}

std::string Item::Name()
{
    return m_name;
}

bool Item::IsEmpty()
{
    return ( m_offset == 0 && m_length == 0 && m_name[0] == '\0' );
}

#pragma endregion item
