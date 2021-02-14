
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
};

template < typename char_type >
struct memstream :
    public std::basic_iostream< char_type, std::char_traits< char_type > >
{
    memstreambuf<char_type> m_buf;

    memstream( char_type * buf, std::streamsize bufLength )
        :
        m_buf( buf, bufLength ),
        std::basic_iostream<char_type>( &m_buf )
    {
    }
};

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
    Item item( 0, 0, "" );
    REQUIRE( item.IsEmpty() );
}

TEST_CASE( "Header ItemCount returns 1 when 1 Item exists" )
{
    Header hdr;
    hdr.Add( Item( 0, 0, "" ) );
    REQUIRE( hdr.ItemCount() == 1 );
}

TEST_CASE( "Pkg ReadCString returns zero when empty string" )
{
    char data[] = {0};
    char c_string[2] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    REQUIRE( pkg.ReadCString( c_string, sizeof(c_string) ) == 0 );
}

TEST_CASE( "Pkg ReadCString returns one when single char" )
{
    char data[] = {'1', 0};
    char c_string[2] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    REQUIRE( pkg.ReadCString( c_string, sizeof(c_string) ) == 1 );
}

TEST_CASE( "Pkg ReadCString when string length is under max length" )
{
    char data[] = {'1', 0};
    char c_string[2] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    pkg.ReadCString( c_string, sizeof(c_string) );
    REQUIRE( std::string( c_string ) == "1" );
}

TEST_CASE( "Pkg ReadCString when string length is max length" )
{
    char data[Item::MAX_NAME_LENGTH + 1] = {0}; // +1 for null-termination
    char c_string[Item::MAX_NAME_LENGTH] = {0};
    CreateAsciiPatternCString( data, sizeof( data ) - 1 ); // -1 to keep space for null-termination
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    pkg.ReadCString( c_string, sizeof(c_string) );
    REQUIRE( std::string( c_string, sizeof( c_string ) ) == std::string( data, sizeof( data ) - 1 ) );
}

TEST_CASE( "Pkg ReadCString when string length is over max length" )
{
    char data[Item::MAX_NAME_LENGTH + 1 + 1] = {0}; // +1 for null-termination
    char c_string[Item::MAX_NAME_LENGTH] = {0};
    CreateAsciiPatternCString( data, sizeof( data ) - 1 ); // -1 to keep space for null-termination
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    pkg.ReadCString( c_string, sizeof(c_string) );
    REQUIRE( std::string( c_string, sizeof( c_string ) ) == std::string( data, sizeof( data ) - 1 - 1 ) );
}

TEST_CASE( "Pkg ParseHeader returns zero Item when zero exists" )
{
    char data[16] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    REQUIRE( pkg.ParseHeader().ItemCount() == 0 );
}

TEST_CASE( "Pkg ParseHeader returns one Item when one exists" )
{
    char data[] = {0, 0, 0, 0, 0, 0, 0, 0, '1', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '\0'};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    REQUIRE( pkg.ParseHeader().ItemCount() == 1 );
}

TEST_CASE( "Pkg ParseHeader returns two Item when two exists" )
{
    char data[] = {0, 0, 0, 0, 0, 0, 0, 0, '1', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '2', '\0', 0, 0, 0, 0, 0, 0, 0, 0, '\0'};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    REQUIRE( pkg.ParseHeader().ItemCount() == 2 );
}
