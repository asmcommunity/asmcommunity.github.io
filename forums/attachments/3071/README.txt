BHM File Format readme

The BHM file format was created by Scali and Samuel R�dal in 2003. At the time they were both involved in the demoscene group Bohemiq, which is where the BHM file format got its name from.
The goal was to create a file format which could store the various data required for a demo, most notably 3d geometry and animation. The file format was to be generic, flexible and hierarchical like XML, but compact and quick to process like a binary file.

The resulting file format consists of three parts:
First is a generic file header, describing the file format ("BHM"), the version number to indicate the particular revision of the file format used, and the size of the tree:

typedef struct {
	char description[4];
	unsigned int version;
	unsigned int treesize;
} FileInfo;

Secondly, the tree hierarchy which describes the different contents and their relation, much like how XML and its nested tag structure:

typedef struct {
	unsigned int numchildren;

	unsigned int chunkid;
	unsigned int type;
} TreeNodeInfo;


Lastly there's a collection of binary 'chunks', with each chunk containing an ID that relates it to its position in the tree:

typedef struct {
	unsigned int id;
	unsigned int type;

	unsigned int datasize;
	unsigned int totalsize;

	unsigned int dataoffs;
} ChunkInfo;

Since the tree nodes are all a fixed size, and the tree is stored as a flattened 'heap', the tree can be read from disk and parsed in a very simple and fast way.
The chunks contain the actual data, and can be of varying size. Chunks can be skipped easily when their data is not required or not understood by the reader, which should make it easier to remain backward and forward compatible to a certain extent, much like XML.

Since it is a binary format, you can store pretty much anything in a chunk. One major advantage of BHM over XML is in the handling of arrays. An entire array could easily be stored in a single chunk, and only require a single tree node. It can be read into memory in a single operation, and used by the application directly. There's no additional parsing involved. With BHM, one can store the data in a format that matches the way the application wants to use the data in-memory.

Since the BHM format was originally developed for a Bohemiq demo, one of its first applications was in an exporter for 3dsmax. This bhmexporter is included in this project. Parts of the bhmexporter code are based on examples from the 3dsmax SDK, ATi Radeon SDK and nVidia graphics SDK. You will require the 3dsmax SDK in order to build the plugin.