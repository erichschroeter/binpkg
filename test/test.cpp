
#include <cstring>
#include <iostream>
#include <catch2/catch_test_macros.hpp>

#include <binpkg.h>

using namespace BinPkg;

template < typename char_type >
struct memstreambuf :
	public std::basic_streambuf< char_type, std::char_traits< char_type > >
{
	memstreambuf( char_type * buf, std::streamsize bufLength )
	{
		this->setp( buf, buf + bufLength );
		this->setg( buf, buf, buf + bufLength );
	}

	std::streampos seekoff(
		std::ios::off_type off,
		std::ios_base::seekdir dir,
		std::ios_base::openmode which = std::ios_base::in ) override
	{
		int offset = static_cast< int >( off ); // Avoid loss of data warning

		if ( which == std::ios_base::in )
		{
			this->gbump( offset );
		}
		else
		{
			this->pbump( offset );
		}
		return which == std::ios_base::out ?
		       this->pptr() - this->eback() :
		       this->gptr() - this->eback();
	}
};

template < typename char_type >
struct memstream :
	public std::basic_iostream< char_type, std::char_traits< char_type > >
{
	memstreambuf< char_type > m_buf;

	memstream( char_type * buf, std::streamsize bufLength )
		:
		m_buf( buf, bufLength ),
		std::basic_iostream< char_type >( &m_buf )
	{
	}
};

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

template< typename Range >
struct EqualsRangeMatcher :
	Catch::Matchers::MatcherGenericBase
{
	EqualsRangeMatcher( Range const & range ) :
		range{ range }
	{
	}

	template< typename OtherRange >
	bool match( OtherRange const & other ) const
	{
		using std::begin; using std::end;

		return std::equal( begin( range ), end( range ), begin( other ), end( other ));
	}

	std::string describe() const override
	{
		return "Equals: " + Catch::rangeToString( range );
	}

private:
	Range const & range;
};

template< typename Range >
auto EqualsRange( const Range & range )->EqualsRangeMatcher< Range >
{
	return EqualsRangeMatcher< Range >{range};
}

void CreateAsciiPatternCString( char * buf, size_t len )
{
	constexpr char MIN_ASCII = 0x20; // ' '
	constexpr char MAX_ASCII = 0x7E; // '~'

	for ( int i = 0, ascii = MIN_ASCII; i < len; ++i, ++ascii )
	{
		if ( ascii >= MAX_ASCII )
		{
			ascii = MIN_ASCII;
		}
		buf[i] = ascii;
	}
}

TEST_CASE( "Item IsEmpty returns true when all values are zero" )
{
	Item item( "", 0, 0 );
	REQUIRE( item.IsEmpty() );
}

TEST_CASE( "Item Size includes null terminator" )
{
	Item   item( "", 0, 0 );
	size_t expected_size = sizeof( uint32_t ) + sizeof( uint32_t ) + 1;
	REQUIRE( item.Size() == expected_size );
}

TEST_CASE( "Header ItemCount returns 1 when 1 Item exists" )
{
	Header hdr;
	hdr.Add( Item( "", 0, 0 ) );
	REQUIRE( hdr.ItemCount() == 1 );
}

TEST_CASE( "Header CalcSize returns size of version and empty item" )
{
	Header hdr;
	size_t expected_size = Item::EMPTY_ITEM_SIZE + sizeof( hdr.Version());
	REQUIRE( hdr.CalcSize() == expected_size );
}

TEST_CASE( "Header CalcSize with one item includes null terminator" )
{
	Header hdr;
	hdr.Add( Item( "zero", 0, 0 ) );
	size_t expected_size = Item::EMPTY_ITEM_SIZE
	                       + sizeof( hdr.Version())
	                       + sizeof( Item::ItemInternal::Offset )
	                       + sizeof( Item::ItemInternal::Length )
	                       + sizeof( "zero" );
	REQUIRE( hdr.CalcSize() == expected_size );
}

TEST_CASE( "Header Add recursively updates offsets" )
{
	Item   item1( "first.bin", 0, 7 );
	Item   item2( "test.txt", 0, 9 );
	Header hdr;
	hdr.Add( item1 );
	hdr.Add( item2 );
	size_t expected_offset = Item::EMPTY_ITEM_SIZE
	                         + sizeof( hdr.Version())
	                         + item1.Size()
	                         + item2.Size();
	REQUIRE( hdr.Get( 0 )->Offset() == expected_offset );
	expected_offset += item1.Length();
	REQUIRE( hdr.Get( 1 )->Offset() == expected_offset );
}

TEST_CASE( "Pkg ReadCString returns zero when empty string" )
{
	char              data[] = {0};
	char              c_string[2] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	REQUIRE( pkg.ReadCString( c_string, sizeof( c_string ) ) == 0 );
}

TEST_CASE( "Pkg ReadCString returns one when single char" )
{
	char              data[] = {'1', 0};
	char              c_string[2] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	REQUIRE( pkg.ReadCString( c_string, sizeof( c_string ) ) == 1 );
}

TEST_CASE( "Pkg ReadCString when string length is under max length" )
{
	char              data[] = {'1', 0};
	char              c_string[2] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	pkg.ReadCString( c_string, sizeof( c_string ) );
	REQUIRE( std::string( c_string ) == "1" );
}

TEST_CASE( "Pkg ReadCString when string length is max length" )
{
	char data[Item::MAX_NAME_LENGTH + 1] = {0}; // +1 for null-termination
	char c_string[Item::MAX_NAME_LENGTH] = {0};
	CreateAsciiPatternCString( data, sizeof( data ) - 1 ); // -1 to keep space for null-termination
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	pkg.ReadCString( c_string, sizeof( c_string ) );
	REQUIRE( std::string( c_string, sizeof( c_string ) ) == std::string( data, sizeof( data ) - 1 ) );
}

TEST_CASE( "Pkg ReadCString when string length is over max length" )
{
	char data[Item::MAX_NAME_LENGTH + 1 + 1] = {0}; // +1 for null-termination
	char c_string[Item::MAX_NAME_LENGTH] = {0};
	CreateAsciiPatternCString( data, sizeof( data ) - 1 ); // -1 to keep space for null-termination
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	pkg.ReadCString( c_string, sizeof( c_string ) );
	REQUIRE( std::string( c_string, sizeof( c_string ) ) == std::string( data, sizeof( data ) - 1 - 1 ) );
}

TEST_CASE( "Pkg ParseHeader returns zero Item when zero exists" )
{
	char              data[16] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	REQUIRE( pkg.ParseHeader().ItemCount() == 0 );
}

TEST_CASE( "Pkg ParseHeader returns one Item when one exists" )
{
	char              data[] = {0, 0, 0, 0, 0, 0, 0, 0, '1', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '\0'};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	REQUIRE( pkg.ParseHeader().ItemCount() == 1 );
}

TEST_CASE( "Pkg ParseHeader returns two Item when two exists" )
{
	char              data[] = {0, 0, 0, 0, 0, 0, 0, 0, '1', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '2', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '\0'};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	REQUIRE( pkg.ParseHeader().ItemCount() == 2 );
}

TEST_CASE( "Pkg Write Header writes version" )
{
	char              data[64] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	Header            hdr( 2 );
	pkg.Write( hdr );
	uint32_t actual_offset = data[0];
	REQUIRE( actual_offset == 2 );
}

TEST_CASE( "Pkg Write Header writes item offset" )
{
	char              data[64] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	Header            hdr;
	hdr.Add( Item( "", 22, 0 ) );
	pkg.Write( hdr );
	uint32_t expected_offset = static_cast< uint32_t >( hdr.CalcSize() );
	uint32_t actual_offset = data[sizeof( hdr.Version())];
	REQUIRE( actual_offset == expected_offset );
}

TEST_CASE( "Pkg Write Header writes item length" )
{
	char              data[64] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	Header            hdr;
	hdr.Add( Item( "", 0, 5 ) );
	pkg.Write( hdr );
	uint32_t actual_length = data[sizeof( hdr.Version()) + sizeof( uint32_t )];
	REQUIRE( actual_length == 5 );
}

TEST_CASE( "Pkg Write Header writes item name" )
{
	char              data[64] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	Header            hdr;
	hdr.Add( Item( "hello world", 0, 0 ) );
	pkg.Write( hdr );
	char actual_name[12] = {0};
	std::strncpy( actual_name, &data[sizeof( hdr.Version()) + sizeof( uint32_t ) + sizeof( uint32_t )], sizeof( actual_name ) );
	REQUIRE( std::string( actual_name ) == "hello world" );
}

TEST_CASE( "Pkg Write write item data at offset" )
{
	char              data[64] = {0};
	memstream< char > stream( data, sizeof( data ) );
	Pkg               pkg( stream );
	std::string       item_name = "";
	uint32_t          data_offset = ( sizeof( uint32_t ) * 2 )
	                                + ( sizeof( uint32_t ) * 2 )
	                                + 1 // for empty Item name
	                                + static_cast< uint32_t >( item_name.size() );
	std::array< char, 8 > file_data = {0,1,2,3,4,5,6,7};
	Item item( item_name.c_str(), data_offset, static_cast< uint32_t >( file_data.size() ) );
	pkg.Write( item, file_data.data(), file_data.size() );
	std::array< char, file_data.size() > actual_data = {0};
	std::memcpy( actual_data.data(), &data[data_offset], file_data.size() );
	REQUIRE_THAT( actual_data, EqualsRange( file_data ) );
}

TEST_CASE( "Pkg Add recursively updates offsets" )
{
	std::array< char, 8 >  file_data1 = {0,1,2,3,4,5,6,7};
	std::array< char, 12 > file_data2 = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'};
	char              stream_buf[64] = {0};
	memstream< char > stream( stream_buf, sizeof( stream_buf ) );
	memstream< char > stream1( file_data1.data(), file_data1.size() );
	memstream< char > stream2( file_data2.data(), file_data2.size() );
	Pkg               pkg( stream );
	std::string       item_name1 = "first.bin";
	std::string       item_name2 = "test.txt";
	pkg.Add( item_name1, static_cast< uint32_t >( file_data1.size() ), stream1 );
	pkg.Add( item_name2, static_cast< uint32_t >( file_data2.size() ), stream2 );
	size_t expected_offset_for_item1 = pkg.HeaderMut().CalcSize();
	REQUIRE( pkg.Get( 0 )->Offset() == expected_offset_for_item1 );
	size_t expected_offset_for_item2 = expected_offset_for_item1 + file_data1.size();
	REQUIRE( pkg.Get( 1 )->Offset() == expected_offset_for_item2 );
}

// TEST_CASE( "Pkg write example with two files" )
// {
//     std::array<char, 8> file_data1 = {0,1,2,3,4,5,6,7};
//     std::array<char, 12> file_data2 = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'};
//     char stream_buf[64] = {0};
//     memstream<char> stream( stream_buf, sizeof(stream_buf) );
//     memstream<char> stream1( file_data1.data(), file_data1.size() );
//     memstream<char> stream2( file_data2.data(), file_data2.size() );
//     Pkg pkg( stream );
//     std::string item_name1 = "first.bin";
//     std::string item_name2 = "test.txt";
//     pkg.Add( item_name1, static_cast< uint32_t >( file_data1.size() ), stream1 );
//     pkg.Add( item_name2, static_cast< uint32_t >( file_data2.size() ), stream2 );
//     pkg.Write();
//     uint32_t file_data1_offset = static_cast< uint32_t >( pkg.HeaderMut().CalcSize() );
//     std::array<char, file_data1.size()> actual_data1 = {0};
//     std::memcpy( actual_data1.data(), &stream_buf[file_data1_offset], file_data1.size() );
//     REQUIRE_THAT( actual_data1, EqualsRange( file_data1 ) );
// }
