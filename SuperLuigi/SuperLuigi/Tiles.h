#include <allegro5/allegro.h>
#include <stdio.h>
//slide 12 dialeksh 6
typedef unsigned short Dim;
struct Rect { int x, y, w, h; };
struct Point { int x, y; };
enum BitDepth { bits8 = 1, bits16, bits24, bits32 };


//slide 12 dialeskh 8
#define TILE_WIDTH 16
#define TILE_HEIGHT 16
#define ROW_MASK 0x0F
#define COL_MASK 0xF0
#define COL_SHIFT 4

typedef unsigned char byte;
byte MakeIndex(byte row, byte col)
{
	return (col << COL_SHIFT) | row;
}
byte GetCol(byte index)
{
	return index >> COL_SHIFT;
}
byte GetRow(byte index)
{
	return index & ROW_MASK;
}

Dim TileX(byte index)
{
	return GetCol(index) * TILE_WIDTH;
}
Dim TileY(byte index)
{
	return GetRow(index) * TILE_HEIGHT;
}

#define MUL_TILE_WIDTH(i) ((i)<<4)
#define MUL_TILE_HEIGHT(i) ((i)<<4)
#define DIV_TILE_WIDTH(i) ((i)>>4)
#define DIV_TILE_HEIGHT(i) ((i)>>4)
#define MOD_TILE_WIDTH(i) ((i)&15)
#define MOD_TILE_HEIGHT(i) ((i)&15)

Dim TileX2(byte index)
{
	return MUL_TILE_WIDTH(GetCol(index));
}
Dim TileY2(byte index)
{
	return MUL_TILE_HEIGHT(GetRow(index));
}


//slide 12 dialeksh 8

typedef unsigned short Index; // [MSB X][LSB Y]
#define TILEX_MASK 0xFF00
#define TILEX_SHIFT 8
#define TILEY_MASK 0x00FF

Dim TileX3(Index index) { return index >> TILEX_SHIFT; }
Dim TileY3(Index index) { return index & TILEY_MASK; }


Index MakeIndex2(byte row, byte col)
{
	return (MUL_TILE_WIDTH(col) << TILEX_SHIFT) | MUL_TILE_HEIGHT(row);
}
//changes target bitmap
void PutTile(ALLEGRO_BITMAP* dest, Dim x, Dim y, ALLEGRO_BITMAP* tiles, Index tile) {
	al_set_target_bitmap(dest);
	al_draw_bitmap_region(
		tiles,			//bitmap
		TileX3(tile),	//sx
		TileY3(tile),	//sy
		TILE_WIDTH,		//sw
		TILE_HEIGHT,	//sh
		x,				//dx
		y,				//dy
		0				//flags
	);
}


//slide 16 dialeksh 8
#define MAX_WIDTH 1024
#define MAX_HEIGHT 256

typedef Index TileMap[MAX_WIDTH][MAX_HEIGHT];
static TileMap map; // example of a global static map

void SetTile(TileMap* m, Dim col, Dim row, Index index)
{
	(*m)[row][col] = index;
}

Index GetTile(const TileMap* m, Dim col, Dim row)
{
	return (*m)[row][col];
}

//void WriteBinMap(const TileMap* m, FILE* fp) {
//	fwrite((m, sizeof(TileMap), 1, fp);
//}

void ReadBinMap(TileMap* m, FILE* fp) {
	/*binary formatted read, like descent parsing*/
}

void WriteTextMap(const TileMap*, FILE* fp) {
	/*custom write in text format*/
}

bool ReadTextMap(TileMap* m, FILE* fp) {
	/*parsing*/
	return true;
}