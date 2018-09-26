#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for (auto m : materials)
		delete m;
	for (auto f : faces)
		delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3& v)
{
	vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material* m)
{
	materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3& n)
{
	normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
	int vcnt = vertices.size();

	if (a >= vcnt || b >= vcnt || c >= vcnt)
		return false;

	TrimeshFace* newFace = new TrimeshFace(
	        scene, new Material(*this->material), this, a, b, c);
	newFace->setTransform(this->transform);
	if (!newFace->degen)
		faces.push_back(newFace);
	else
		delete newFace;

	// Don't add faces to the scene's object list so we can cull by bounding
	// box
	return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char* Trimesh::doubleCheck()
{
	if (!materials.empty() && materials.size() != vertices.size())
		return "Bad Trimesh: Wrong number of materials.";
	if (!normals.empty() && normals.size() != vertices.size())
		return "Bad Trimesh: Wrong number of normals.";

	return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	bool have_one = false;
	for (auto face : faces) {
		isect cur;
		if (face->intersectLocal(r, cur)) {
			if (!have_one || (cur.getT() < i.getT())) {
				i = cur;
				have_one = true;
			}
		}
	}
	if (!have_one)
		i.setT(1000.0);
	return have_one;
}

bool TrimeshFace::intersect(ray& r, isect& i) const
{
	return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
	// YOUR CODE HERE
	//
	// FIXME: Add ray-trimesh intersection
	//std::cout << endl;
	//std::cout << "starting intersection" << endl;
	glm::dvec3 a = parent->vertices[ids[0]];
	glm::dvec3 b = parent->vertices[ids[1]];
	glm::dvec3 c = parent->vertices[ids[2]];
	//std::cout << "vertices are " << a.x << a.y << a.z << endl;

	double denom = glm::dot(normal, r.getDirection());

	//parallel
	if (abs(denom) < RAY_EPSILON) {
		return false;
	}

	//check plane collision
	//check what d is
	double t = -1.0 * ((glm::dot(normal, r.getPosition()) - dist) / denom);
	glm::dvec3 q = r.at(t);

	//intersected plane, not behind ray
	if (t <= RAY_EPSILON)
		return false;

	//time for inside outside test
	glm::dvec3 edge_ab = glm::cross((b-a), (q-a));
	glm::dvec3 edge_bc = glm::cross((c-b), (q-b));
	glm::dvec3 edge_ac = glm::cross((a-c), (q-c));
	double io1 = glm::dot(edge_ab, normal);
	double io2 = glm::dot(edge_bc, normal);
	double io3 = glm::dot(edge_ac, normal);

	//all inside triangle
	if ((io1 < 0) || (io2 < 0) || (io3 < 0))
		return false;

	//now need barycentric coordinates
	double area = glm::dot(normal, glm::cross((b - a), (c - a)));
	double areabc = glm::dot(normal, glm::cross((b - q), (c - q)));
	double areaca = glm::dot(normal, glm::cross((c - q), (a - q)));
	double alpha = areabc / area;
	double beta = areaca / area;
	double gamma = 1.0 - alpha - beta;

	i.setObject(this);
	i.setBary(alpha, beta, gamma);
	i.setT(t);
	i.setUVCoordinates(glm::dvec2(alpha, beta));

	//adjust normals based on barycentric coordinates
	if (parent->vertNorms) {
		glm::dvec3 na = parent->normals[ids[0]] * alpha;
		glm::dvec3 nb = parent->normals[ids[1]] * beta;
		glm::dvec3 nc = parent->normals[ids[2]] * gamma;
		glm::dvec3 newnorm = na + nb + nc;
		i.setN(glm::normalize(newnorm));
	}
	else {
		i.setN(normal);
	} 

	//adjust colors based on barycentric coordinates as well
	if (!parent->materials.empty())
    {
        Material ma(*parent->materials[ids[0]]);
        Material mb(*parent->materials[ids[1]]);
        Material mc(*parent->materials[ids[2]]);

        Material m;
        m += (alpha * ma);
        m += (beta * mb);
        m += (gamma * mc);

        i.setMaterial(m);
    }
    else {
		i.setMaterial(this->getMaterial());
    }
	return true;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
	int cnt = vertices.size();
	normals.resize(cnt);
	std::vector<int> numFaces(cnt, 0);

	for (auto face : faces) {
		glm::dvec3 faceNormal = face->getNormal();

		for (int i = 0; i < 3; ++i) {
			normals[(*face)[i]] += faceNormal;
			++numFaces[(*face)[i]];
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if (numFaces[i])
			normals[i] /= numFaces[i];
	}

	vertNorms = true;
}

