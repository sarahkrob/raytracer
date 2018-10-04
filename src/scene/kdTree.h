#pragma once
#include <vector>
#include <iostream>
#include <fstream>

#include "ray.h"
#include "bbox.h"
#include "../ui/TraceUI.h"

using namespace std;
extern TraceUI* traceUI;

// Note: you can put kd-tree here

template <class T>
class Node {
	public:
		Node<T>* left;
		Node<T>* right;
		double position;
		int axis;
		BoundingBox bbox;
		std::vector<T> objectList;

		Node() {};

		//leaf node
		Node(std::vector<T>& objlist, BoundingBox& box) {
			objectList = objlist;
			bbox = box;
			left = nullptr;
			right = nullptr;
		}

		//split node
		Node(double pos, int a, BoundingBox& box, Node<T>* l, Node<T>* r) {
			position = pos;
			bbox = box;
			axis = a;
			left = l;
			right = r;
		}

		bool findIntersection(ray &r, isect &i, double& tmin, double& tmax, bool& have_one) {
			//split node
			if (left != nullptr && right != nullptr) {
				//make sure it hits this node's box at all
				if (bbox.intersect(r, tmin, tmax)) {
					//check left and right
					bool righthit = right->bbox.intersect(r, tmin, tmax);
					bool lefthit = left->bbox.intersect(r, tmin, tmax);
					//only hit left
					if (lefthit && !righthit) {
						if (left->findIntersection(r, i, tmin, tmax, have_one)) {
							return true;
						}
					}
					//only hit right
					if (!lefthit && righthit) {
						if (right->findIntersection(r, i, tmin, tmax, have_one)) {
							return true;
						}
					}
					//both true
					else {
						if (left->findIntersection(r, i, tmin, tmax, have_one)) {
							return true;
						}
						if (right->findIntersection(r, i, tmin, tmax, have_one)) {
							return true;
						}
					}
				}
			}
			//leaf node
			if (left == nullptr || right == nullptr) {
				if (bbox.intersect(r, tmin, tmax)) {
					isect cur;
					for(int j = 0; j < objectList.size(); ++j) {
						bool intersectgood = objectList[j]->intersect(r, cur);
						if (intersectgood && (!have_one || cur.getT() < i.getT())) {
							i = cur;
							have_one = true;
						}
					}
				}
			}
			return false;
		}
};

class SplitPlane {
	public: 
		double position;
		int axis;
		BoundingBox rightBB;
		BoundingBox leftBB;
		int leftCount;
		int rightCount;

		SplitPlane() {};
		SplitPlane(int axis, double pos) : axis(axis), position(pos) {};
};

template <class T>
class kdTree {
	public:
		Node<T>* root;

		kdTree(std::vector<T>& objectList, BoundingBox& bbox, int leafSize) {
			root = buildTree(objectList, bbox, 0, leafSize);
		}

		Node<T>* buildTree(std::vector<T>& objectList, BoundingBox& bbox, int depth, int leafSize) {
			if (objectList.size() <= leafSize || ++depth == traceUI->getMaxDepth()) {
				return new Node<T>(objectList, bbox);
			}
			SplitPlane bestPlane = findBestSplitPlane(objectList, bbox, leafSize);
			std::vector<T> leftList;
			std::vector<T> rightList;

			//push back objects into lists
			for (int i = 0; i < objectList.size(); ++i) {
				if (objectList[i]->getBoundingBox().getMin()[bestPlane.axis] < bestPlane.position) {
					leftList.push_back(objectList[i]);
				}
				else if (bestPlane.position == objectList[i]->getBoundingBox().getMax()[bestPlane.axis] && 
					objectList[i]->getBoundingBox().getMin()[bestPlane.axis] == bestPlane.position && (objectList[i]->getNormal().x 
						< 0.0 && objectList[i]->getNormal().y < 0.0 && objectList[i]->getNormal().z < 0.0)) {
					leftList.push_back(objectList[i]);
				}
				if (objectList[i]->getBoundingBox().getMax()[bestPlane.axis] > bestPlane.position) {
					rightList.push_back(objectList[i]);
				}
				else if (bestPlane.position == objectList[i]->getBoundingBox().getMax()[bestPlane.axis] && 
					objectList[i]->getBoundingBox().getMin()[bestPlane.axis] == bestPlane.position && (objectList[i]->getNormal().x 
						>= 0.0 && objectList[i]->getNormal().y >= 0.0 && objectList[i]->getNormal().z >= 0.0)) {
					rightList.push_back(objectList[i]);
				}
			}

			//stop if all objects on one side
			if (leftList.empty() || rightList.empty()) {
				return new Node<T>(objectList, bbox);
			}

			//otherwise recurse
			else {
				return new Node<T>(bestPlane.position, bestPlane.axis, bbox, buildTree(leftList, bestPlane.leftBB, depth, leafSize), 
					buildTree(rightList, bestPlane.rightBB, depth, leafSize));
			}
		}
		
		SplitPlane findBestSplitPlane(std::vector<T>& objectList, BoundingBox& bbox, int& leafSize) {
			std::vector<SplitPlane> candidates;
			SplitPlane p1, p2;
			double minSAM = INT_MAX;
			SplitPlane bestPlane;
			//for each axis
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < objectList.size(); j += leafSize) {
					p1.position = objectList[j]->getBoundingBox().getMin()[i];
					p1.axis = i;
					p2.position = objectList[j]->getBoundingBox().getMax()[i];
					p2.axis = i;
					candidates.push_back(p1);
					candidates.push_back(p2);
				}
			}
			for (int p = 0; p < candidates.size(); ++p) {
				//first recalculate bounding boxes for each split plane
				candidates[p].leftBB = calculateLeftBBox(bbox, candidates[p].axis, candidates[p].position);
				candidates[p].rightBB = calculateRightBBox(bbox, candidates[p].axis, candidates[p].position);
				//then check if new bounding boxes intersect objects to count
				candidates[p].leftCount = countLeftObjects(objectList, candidates[p].leftBB);
				candidates[p].rightCount = countRightObjects(objectList, candidates[p].rightBB);
			}
			for (int plane = 0; plane < candidates.size(); ++plane) {
				double SAM = (candidates[plane].leftCount * candidates[plane].leftBB.area() + 
					candidates[plane].rightCount * candidates[plane].rightBB.area());
				if (SAM < minSAM) {
					minSAM = SAM;
					bestPlane = candidates[plane];
				}
			}
			return bestPlane;
		}

		int countLeftObjects(std::vector<T>& objectList, BoundingBox& bbox) {
			int count = 0;
			for (int i = 0; i < objectList.size(); ++i) {
				if (objectList[i]->getBoundingBox().intersects(bbox))
					count += 1;
			}
			return count;
		}

		int countRightObjects(std::vector<T>& objectList, BoundingBox& bbox) {
			int count = 0;
			for (int i = 0; i < objectList.size(); ++i) {
				if (objectList[i]->getBoundingBox().intersects(bbox))
					count += 1;
			}
			return count;
		}

		BoundingBox calculateLeftBBox(BoundingBox& bbox, int axis, double position) {
			glm::dvec3 max;
			//x
			if (axis == 0) {
				max = glm::dvec3(position, bbox.getMax().y, bbox.getMax().z);
			}
			//y
			if (axis == 1) {
				max = glm::dvec3(bbox.getMax().x, position, bbox.getMax().z);
			}
			//z
			if (axis == 2) {
				max = glm::dvec3(bbox.getMax().x, bbox.getMax().y, position);
			}
			return BoundingBox(bbox.getMin(), max);
		}

		BoundingBox calculateRightBBox(BoundingBox& bbox, int axis, double position) {
			glm::dvec3 min;
			//x
			if (axis == 0) {
				min = glm::dvec3(position, bbox.getMin().y, bbox.getMin().z);
			}
			//y
			if (axis == 1) {
				min = glm::dvec3(bbox.getMin().x, position, bbox.getMin().z);
				
			}
			//z
			if (axis == 2) {
				min = glm::dvec3(bbox.getMin().x, bbox.getMin().y, position);
			}
			return BoundingBox(min, bbox.getMax());
		}
};




