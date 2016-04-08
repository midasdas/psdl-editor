#define MAX_VERTICES 8000
#define MAX_FACES 8000

typedef struct
{
	float x, y, z;
}
VERTEX;

typedef struct {
	unsigned short a, b, c;
} FACE;

typedef struct {
	float u, v;
} UV;

class _3ds
{
public:

	unsigned char name[20];
	unsigned short n_vertices;
	std::vector<vertex> vertices;
	unsigned short n_faces;
	FACE faces[MAX_FACES];
	UV mapcoords[MAX_VERTICES];
};
