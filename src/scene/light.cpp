#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	glm::dvec3 d = getDirection(p);
	ray newray = ray(p, d, glm::dvec3(1, 1, 1), ray::SHADOW);
	isect i;
	if(scene->intersect(newray, i)) {
		//modify based on transparency
		glm::dvec3 transparent = i.getMaterial().kt(i);
		return transparent;
	}
	return glm::dvec3(1,1,1);
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
	double distance = glm::distance(P, position);
	double atten = min(1.0, 1.0/(constantTerm + (linearTerm * distance) + (quadraticTerm * distance * distance)));
	return atten;
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	glm::dvec3 d = glm::normalize(position - p);
	ray newray = ray(p, d, glm::dvec3(1, 1, 1), ray::SHADOW);
	isect i;
	if(scene->intersect(newray, i)) {
		glm::dvec3 q = newray.at(i.getT());
		double qdist = glm::distance(p, q);
		double lightdist = glm::distance(p, position);
		glm::dvec3 transparent = i.getMaterial().kt(i);
		double distance = glm::distance(p, q);
		double atten = min(1.0, 1.0/(constantTerm + (linearTerm * distance) + (quadraticTerm * distance * distance)));
		//q is before light source
		//need to use both transparency and distance of point light
		if (qdist < lightdist) {
			return transparent * atten;
		}
	}
	return glm::dvec3(1,1,1);
}

#define VERBOSE 0

