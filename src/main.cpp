#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sys/stat.h>

#include <binpkg.h>

using namespace BinPkg;

int VERBOSITY = 0;
#define DEBUG( x ) do { if ( VERBOSITY > 0 ) { std::cout << x } } while( 0 )

std::string Version()
{
	return "1.0";
}

std::string Help()
{
	return R"END(A small, simple library for packaging multiple files within a single file.

USAGE:
  binpkg --version
  binpkg -h
  binpkg [-V] -o OUTFILE FILES...

OPTIONS:
  --version                         Print the version info
  -h, --help                        Print this menu
  -V                                Verbose output.
  -o, --output                      The output file
)END";
}

char * cmdGetOption( char ** begin, char ** end, const std::string & option )
{
	char ** itr = std::find( begin, end, option );

	if ( itr != end && ++itr != end )
	{
		return *itr;
	}
	return nullptr;
}

bool cmdOptionExists( char ** begin, char ** end, const std::string & option )
{
	return std::find( begin, end, option ) != end;
}

std::vector< std::string > splitpath( const std::string & str, const std::set< char > delimiters )
{
	std::vector< std::string > result;
	char const                 * pch = str.c_str();
	char const                 * start = pch;

	for (; *pch; ++pch )
	{
		if ( delimiters.find( *pch ) != delimiters.end())
		{
			if ( start != pch )
			{
				std::string str( start, pch );
				result.push_back( str );
			}
			else
			{
				result.push_back( "" );
			}
			start = pch + 1;
		}
	}
	result.push_back( start );

	return result;
}

struct FileInfo
{
	std::string path;
	std::fstream file_stream;
};

std::vector< FileInfo > ParseFiles( char ** begin, char ** end )
{
	std::vector< FileInfo > files;
	for ( char ** arg = begin; arg != end; arg++ )
	{
		std::string   path( *arg );
		std::ifstream file( path, std::ifstream::binary );

		if ( file.good() )
		{
			files.push_back( FileInfo{ path, std::fstream( path, std::ios::in | std::ios::binary ) } );
		}
	}
	return files;
}

int main( int argc, char * argv[] )
{
	if ( cmdOptionExists( argv, argv + argc, "-h" ) || cmdOptionExists( argv, argv + argc, "--help" ) )
	{
		std::cout << Help() << std::endl;
		return 0;
	}

	if ( cmdOptionExists( argv, argv + argc, "-v" ) || cmdOptionExists( argv, argv + argc, "--version" ) )
	{
		std::cout << Version() << std::endl;
		return 0;
	}

	if ( cmdOptionExists( argv, argv + argc, "-V" ) )
	{
		VERBOSITY = 1;
	}

	char * output_path = cmdGetOption( argv, argv + argc, "-o" );

	if ( output_path == nullptr )
	{
		output_path = cmdGetOption( argv, argv + argc, "--output" );
	}

	if ( output_path != nullptr )
	{
		assert( argc >= 3 );
		// +3 to skip the program itself, -o, and -o arg
		std::vector< FileInfo > files = ParseFiles( argv + 3, argv + argc );
		std::fstream            os( output_path, std::fstream::out | std::fstream::binary );
		Pkg              pkg( os );
		uint32_t         offset = 0;
		std::set< char > delims{'/', '\\'};

		for ( auto & file : files )
		{
			struct stat                statinfo;
			int                        rc = stat( file.path.c_str(), &statinfo );
			std::vector< std::string > path_tokens = splitpath( file.path, delims );
			DEBUG( file.path << ": " << ( rc == 0 ? statinfo.st_size : -1 ) << std::endl; );
			pkg.Add( path_tokens.back(), statinfo.st_size, file.file_stream );
		}
		pkg.Write();
	}

	return 0;
}
