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
		/// An internal struct storing the basic info required for writing an item in a packaged file.
		/// The fields are not intended to be edited manually, which is left to the Item member functions.
		struct ItemInternal
		{
			/// The offset into the packaged file.
			uint32_t Offset;
			/// The number of bytes of the item.
			uint32_t Length;
			/// A user friendly name for the item.
			char * Name;
		};

		/// The byte count for an empty item. Includes null terminator.
		static constexpr std::size_t EMPTY_ITEM_SIZE = sizeof( ItemInternal::Offset ) + sizeof( ItemInternal::Length ) + 1;
		static constexpr std::size_t MAX_NAME_LENGTH = 1024;

		Item( const char * name = "", uint32_t offset = 0, uint32_t length = 0 );

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
		std::size_t Size() const;

	protected:
		// Array for storing C string (accounting for null-terminator).
		char m_name[MAX_NAME_LENGTH + 1];
		ItemInternal m_item;
	};

	class Header
	{
	public:
		Header( int32_t version = 0 );
		int32_t Version() const;
		void SetVersion( int32_t value );
		std::size_t ItemCount() const;
		std::size_t CalcSize() const;
		void Add( Item item );
		const Item * Get( int index ) const;
		const std::vector< Item > & Items() const &;
		std::vector< Item > & ItemsMut() &;

	protected:
		void UpdateOffsets();
		std::vector< Item > m_items;
		/// The version of the package file format.
		/// This exists mainly for future backwards compatibility.
		int32_t m_version;
	};

	class Pkg
	{
	public:
		Pkg( std::iostream & stream );

		Header ParseHeader();
		Header & HeaderMut() &;
		// void Add( Item item, std::iostream & stream );
		void Add( std::string name, uint32_t length, std::iostream & stream );
		const Item * Get( int index ) const;
		int ReadCString( char * buf, std::size_t buf_length );
		void Write( const Header & hdr );
		void Write( const Item & item, const char * data, std::size_t data_length );
		void Write();

	protected:
		/// Map of indexes of Items to their respective iostream.
		std::map< int, std::iostream * > m_items_map;
		Header m_header;
		/// The io stream for the package file.
		std::iostream & m_stream;
	};
}
