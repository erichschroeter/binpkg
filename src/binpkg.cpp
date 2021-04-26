#include <cstring>

#include "binpkg.h"

using namespace BinPkg;

#pragma region Pkg

/// \param stream The stream to write to.
Pkg::Pkg( std::iostream & stream )
	:
	m_stream( stream )
{
}

/// \brief Reads the stream, placing data in \p buf, until a null-terminator is found.
/// \param buf The buffer to read data into.
/// \param buf_length The max number of bytes available in the buffer.
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

/// \return The mutable reference of the Header member.
Header & Pkg::HeaderMut() &
{
	return m_header;
}

/// \brief Reads the stream until an empty Item is found.
Header Pkg::ParseHeader()
{
	Header hdr;
	bool   empty_item_found = false;

	do
	{
		Item item;
		m_stream.read( (char*)&item.OffsetMut(), sizeof( item.Offset()) );
		m_stream.read( (char*)&item.LengthMut(), sizeof( item.Length()) );
		ReadCString( item.NameMut(), Item::MAX_NAME_LENGTH );

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

/// \brief Adds a new item to the header and maps \p stream to it for writing at a later time.
/// \param name The name of the item.
/// \param length The number of bytes the item consists of.
/// \param stream The stream to read data for item from.
void Pkg::Add( std::string name, uint32_t length, std::iostream & stream )
{
	int index = static_cast< int >( m_header.ItemCount() );
	m_header.Add( Item{ name.c_str(), 0, length } );
	m_items_map[index] = &stream;
}

const Item * Pkg::Get( int index ) const
{
	return m_header.Get( index );
}

void Pkg::Write( const Header & hdr )
{
	auto version = hdr.Version();
	m_stream.write( (char*)&version, sizeof( version ) );

	for ( const auto & item : hdr.Items() )
	{
		uint32_t    offset = item.Offset();
		uint32_t    length = item.Length();
		std::string name = item.NameCopy();
		m_stream.write( (char*)&offset, sizeof( offset ) );
		m_stream.write( (char*)&length, sizeof( length ) );
		m_stream.write( name.c_str(), name.size() + 1 ); // +1 for null terminator
	}
}

void Pkg::Write( const Item & item, const char * data, size_t data_length )
{
	m_stream.seekp( item.Offset(), m_stream.beg );
	// std::cout << "writing " << data_length << " bytes at offset " << item.Offset() << std::endl;
	// for (int i = 0; i < data_length; ++i)
	// {
	//     std::cout << data[i];
	// }
	// std::cout << std::endl;
	m_stream.write( data, data_length );
}

/// \brief Writes the header and item data.
void Pkg::Write()
{
	Write( m_header );

	int    index = 0;
	size_t bytes_read_total = 0;

	for ( const auto & item : m_header.Items() )
	{
		std::iostream * stream = m_items_map[ index ];
		char          buffer[4096] = {0};

		while ( bytes_read_total < item.Size() )
		{
			stream->read( buffer, sizeof( buffer ) );
			size_t bytes_read = stream->gcount();
			bytes_read_total += bytes_read;

			if ( bytes_read > 0 )
			{
				// std::cout << "read " << bytes_read << " from " << item.NameCopy() << std::endl;
				Write( item, buffer, bytes_read );
			}
			else
			{
				// std::cerr << "0 bytes read from " << item.NameCopy() << std::endl;
				// std::cerr << "moving on to next item" << std::endl;
				break;
			}
		}
		index++;
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

/// \return The number of items, excluding the terminating empty item.
size_t Header::ItemCount() const
{
	return m_items.size();
}

/// \brief Iterates over the items adding up their size.
size_t Header::CalcSize() const
{
	size_t length = Item::EMPTY_ITEM_SIZE + sizeof( m_version );

	for ( auto & item : Items() )
	{
		length += item.Size();
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

/// Iterates over the list of items and updates their offset fields.
void Header::UpdateOffsets()
{
	size_t offset = CalcSize();
	Item   * previous_item = nullptr;

	for ( auto & item : ItemsMut() )
	{
		if ( previous_item != nullptr )
		{
			offset += previous_item->Length();
		}

		item.SetOffset( static_cast< uint32_t >( offset ) );
		previous_item = &item;
	}
}

void Header::Add( Item item )
{
	m_items.push_back( item );
	UpdateOffsets();
}

const Item * Header::Get( int index ) const
{
	return &m_items.at( index );
}

#pragma endregion Header

#pragma region item

Item::Item( const char * name, uint32_t offset, uint32_t length )
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

/// \return The mutable reference to the offset member.
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

/// \return The mutable reference to the length member.
uint32_t & Item::LengthMut()
{
	return m_item.Length;
}

const char * Item::Name() const
{
	return m_name;
}

/// \return The mutable pointer to name member.
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
	return sizeof( m_item.Offset ) + sizeof( m_item.Length ) + std::strlen( m_name ) + 1; // +1 for null terminator
}

bool Item::IsEmpty()
{
	return ( m_item.Offset == 0 && m_item.Length == 0 && m_name[0] == '\0' );
}

#pragma endregion item
