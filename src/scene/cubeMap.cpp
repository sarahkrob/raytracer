#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here
	glm::dvec3 d = r.getDirection();

	//need to find max
	double max = 0.0;
	//0 is x, 1 is y, 2 is z
	int dimension = 0;
	double x, y;
	int map = 0;
	for (int i = 0; i < 3; ++i) {
		if(fabs(d[i]) > fabs(max)) {
			max = d[i];
			dimension = i;
		}
	}

	//find the section we're in (make cleaner??)
	if (dimension == 0 && max > 0.0) {
		//positive x
		//y = y x = -z
		map = 0;
		x = ((-d[2]/fabs(max)) + 1) / 2.0;
		y = ((d[1]/fabs(max)) + 1) / 2.0;
	}	
	if (dimension == 0 && max < 0.0) {
		//negative x
		//y = y x = -z
		map = 1;
		x = ((d[2]/fabs(max)) + 1) / 2.0;
		y = ((d[1]/fabs(max)) + 1) / 2.0;
	}	
	if (dimension == 1 && max > 0.0) {
		//positive y
		//y = -z x = x
		map = 2;
		x = ((d[0]/fabs(max)) + 1) / 2.0;
		y = ((-d[2]/fabs(max)) + 1) / 2.0;
	}	
	if (dimension == 1 && max < 0.0) {
		//negative y
		//y = z x = x
		map = 3;
		x = ((d[0]/fabs(max)) + 1) / 2.0;
		y = ((d[2]/fabs(max)) + 1) / 2.0;
	}	
	if (dimension == 2 && max > 0.0) {
		//positive z
		//y = y x = x
		map = 4;
		x = ((d[0]/fabs(max)) + 1) / 2.0;
		y = ((d[1]/fabs(max)) + 1) / 2.0;
	}
	if (dimension == 2 && max < 0.0) {
		//positive z
		//y = y x = -x
		map = 5;
		x = ((-d[0]/fabs(max)) + 1) / 2.0;
		y = ((d[1]/fabs(max)) + 1) / 2.0;
	}	

	glm::dvec2 xy = glm::dvec2(x, y);
	return tMap[map]->getMappedValue(xy);
}

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get())
		tMap[n].reset(m);
}
