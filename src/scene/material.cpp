#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.
	// }
	glm::dvec3 color;
	glm::dvec3 p = r.at(i.getT());
	//first two phong terms
	color = ke(i) + ka(i) * scene->ambient();
	//check all lights
	for ( const auto& pLight : scene->getAllLights() )
	{
		//direction of light
		glm::dvec3 dir = pLight->getDirection(p);
		//dot N & L
		glm::dvec3 norm = glm::normalize(i.getN());

		double LdotN;
		//account for internal reflection and refraction
		if (r.type() == ray::REFRACTION || (norm.x < 0.0 || norm.y < 0.0 || norm.z < 0.0)) {
			LdotN = abs(glm::dot(dir, norm));
		}
		else {
			LdotN = max(glm::dot(dir, norm), 0.0);
		}
		//now need reflection
		glm::dvec3 ref = glm::reflect(dir, norm);
		glm::dvec3 viewdir = glm::normalize(r.getDirection());

		double RdotV = max(glm::dot(ref, viewdir), 0.0);
		//specular term
		double specterm = glm::pow(RdotV, shininess(i));
		glm::dvec3 attenuation = pLight->distanceAttenuation(p) * pLight->shadowAttenuation(r, p);
		color += pLight->getColor() * attenuation * ((kd(i) * LdotN) + (ks(i) * specterm));
	}
	return color;
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.


	//make right size
	double x = coord[0] * getWidth();
	double y = coord[1] * getHeight();

	//get the corners
	int xdown = (int)x;
	int xup = xdown + 1;
	int ydown = (int)y;
	int yup = ydown + 1;
	double x2 = (double)xup;
	double x1 = (double)xdown;
	double y2 = (double)yup;
	double y1 = (double)ydown;

	//do bilinear interpolation
	glm::dvec3 r1 = ((x2 - x)/(x2 - x1)) * getPixelAt(xdown, ydown) + ((x - x1)/(x2 - x1)) * getPixelAt(xup, ydown);
	glm::dvec3 r2 = ((x2 - x)/(x2 - x1)) * getPixelAt(xdown, yup) + ((x - x1)/(x2 - x1)) * getPixelAt(xup, yup);
	glm::dvec3 p = ((y2 - y)/(y2 - y1)) * r1 + ((y - y1)/(y2 - y1)) * r2;

	return p;
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.

	if (data.empty())
		return glm::dvec3(1.0, 1.0, 1.0);

	if (x >= width)
		x = width - 1;
	if (y >= height)
		y = height - 1;

	int pos = (y * width + x) * 3;
	return glm::dvec3(double(data[pos]) / 255.0,
	                  double(data[pos + 1]) / 255.0,
	                  double(data[pos + 2]) / 255.0);
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
