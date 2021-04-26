# binpkg
A small, simple library for packaging multiple files within a single file.

# Package layout

The package layout is fairly simple where a header at the beginning of the file describes the contents after the header itself.
This allows pretty much anything to be packaged within the file.

The header starts with a 32-bit version for the file format version.
This allows for future iterations that may break backwards compatibility.
Following the version are a variable amount of item information.
This simple design allows for a variable number of items to be stored within the file.

The header must be ended with an "empty" `struct Item` (i.e. zero for all fields).
This allows for a trivial implementation using a loop that can exit once it detects this zero special case.

```c
struct Item
{
	/// Number of bytes within the file where the item starts.
	uint32_t offset;
	/// Number of bytes of the file.
	uint32_t length;
	/// User friendly name that can be presented to end user. Must be null-terminated.
	const char * name;
}
```

The decision to use `const char * name` was to allow for usage of whatever the filename happens to be for a particular file, as opposed to requiring a truncation if it was decided to instead use a `char` array of a hard-coded size.
This adds minor complexity to the header since the offsets may potentially change if the `name` changes in length.
This can be mitigated with a utility program that manages this functionality and is trivial to implement.

One constraint with this design is that filenames only support ASCII characters.

Example header with 3 items within the package.

```c
{
	0x00000000
}
{
	0x00000047,           // 4
	0x00000004,           // 4
	"something.tar.gz\0"  // 17
},
{
	0x0000004B,           // 4
	0x00000002,           // 4
	"hello_world.txt\0"   // 16
},
{
	0x0000004D,     // 4
	0x00000003,     // 4
	"test\0"        // 5
},
{
	0x00000000,     // 4
	0x00000000,     // 4
	"\0"            // 1
},
{ 0x00, 0x11, 0x22, 0x33 }, // something.tar.gz
{ 0x33, 0x22 },             // hello_world.txt
{ 0x22, 0x33, 0x11 }        // test
```

# Usage

Please see the `--help` documentation for usage.
However, for a quick example on generating a packaged file with `LICENSE`, `README.md`, and `CONTRIBUTING.md`:

```bash
binpkg.exe -o my_deliverable.binpkg LICENSE README.md CONTRIBUTING.md
```
