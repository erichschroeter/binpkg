
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
        if ( which == std::ios_base::in )
        {
            this->gbump( off );
        }
        else
        {
            this->pbump( off );
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
    memstreambuf<char_type> m_buf;

    memstream( char_type * buf, std::streamsize bufLength )
        :
        m_buf( buf, bufLength ),
        std::basic_iostream<char_type>( &m_buf )
    {
    }
};

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

template<typename Range>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase {
    EqualsRangeMatcher(Range const& range):
        range{ range }
    {}

    template<typename OtherRange>
    bool match(OtherRange const& other) const {
        using std::begin; using std::end;

        return std::equal(begin(range), end(range), begin(other), end(other));
    }

    std::string describe() const override {
        return "Equals: " + Catch::rangeToString(range);
    }

private:
    Range const& range;
};

template<typename Range>
auto EqualsRange(const Range& range) -> EqualsRangeMatcher<Range> {
    return EqualsRangeMatcher<Range>{range};
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

TEST_CASE( "Pkg WriteHeader writes item offset" )
{
    char data[64] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    Header hdr;
    hdr.Add( Item( 10, 0, "" ) );
    pkg.WriteHeader( hdr );
    Item actual;
    actual.m_offset = data[0];
    REQUIRE( actual.m_offset == 10 );
}

TEST_CASE( "Pkg WriteHeader writes item length" )
{
    char data[64] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    Header hdr;
    hdr.Add( Item( 0, 5, "" ) );
    pkg.WriteHeader( hdr );
    Item actual;
    actual.m_length = data[4];
    REQUIRE( actual.m_length == 5 );
}

TEST_CASE( "Pkg WriteHeader writes item name" )
{
    char data[64] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    Header hdr;
    hdr.Add( Item( 0, 0, "hello world" ) );
    pkg.WriteHeader( hdr );
    Item actual;
    std::strncpy( actual.m_name, &data[8], sizeof( actual.m_name ) );
    REQUIRE( std::string( actual.m_name ) == "hello world" );
}

TEST_CASE( "Pkg Write writes item data at offset" )
{
    char data[64] = {0};
    memstream<char> stream( data, sizeof(data) );
    Pkg pkg( stream );
    std::string item_name = "";
    size_t data_offset = ( sizeof(uint32_t) * 2 )
        + ( sizeof(uint32_t) * 2 )
        + 1 // for empty Item name
        + item_name.size();
    std::array<char, 8> file_data = {0,1,2,3,4,5,6,7};
    Item item( data_offset, file_data.size(), item_name.c_str() );
    pkg.Write( item, file_data.data(), file_data.size() );
    std::array<char, file_data.size()> actual_data = {0};
    std::memcpy( actual_data.data(), &data[data_offset], file_data.size() );
    REQUIRE_THAT( actual_data, EqualsRange( file_data ) );
}
