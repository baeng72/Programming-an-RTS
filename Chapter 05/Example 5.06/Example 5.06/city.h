#ifndef city_h
#define city_h

#include <vector>
#include "intpoint.h"
#include "object.h"
#include "camera.h"

#define TILE_SIZE 13.99f

class CITY
{
	friend class APPLICATION;
public:
	CITY();
	void Init(INTPOINT _size);
	void Render(CAMERA* cam);
	D3DXVECTOR3 GetCenter();

private:

	std::vector<OBJECT> m_objects;
	INTPOINT m_size;
};


#endif