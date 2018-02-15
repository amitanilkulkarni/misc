/*
 * 	polygon.cpp
 * 	    Created on: Feb 24, 2015
 *      Author: amit
 */

#include<iostream>
#include<list>
#include<fstream>
#include<cstdlib>
#include<string>
#include<sstream>

using namespace std;


/* Done:
 * 		- Classes for edge, polygon, plane
 * 		- Functions for establishing polygon validity:
 * 			# trans intersection
 * 			# cis intersection
 * 			# cumsum check
 *
 * To Do List:
 *
 * 		- function to check the validity of a given polygon,
 * 		- function to check whether the polygon is simple,
 * 		- functions to check the intersection of given cis/trans edges,
 * 		- main algorithm
 */

class Edge {
public:
	double com, mn, mx, index;
	bool intersectIsPossible(double);
	bool isAdjacent(double, double);
};

bool Edge::intersectIsPossible( double cm ) {
	if ( (mx > cm) && (mn < cm) )
		return true;
	else
		return false;
}

bool Edge::isAdjacent ( double ind, double size ) {
	if ( (ind == index+1) || (ind == index-1) || ((int)ind % (int)size == index+1) || ((int)ind % (int)size == index-1) )
		return true;
	else
		return false;
}

class Polygon {
public:
	double index;
	list <Edge> HorzList, VertList;
	bool addEdge( list <Edge> targetList, double len, double comprev, double mxprev, double edgeIndex );
	bool isSimple();
};

bool Polygon::addEdge( list <Edge> targetList, double len, double comprev, double mxprev, double edgeIndex ) {
	// We assume that no cis-edges come one after another (ie., two horizontals in a row)
	Edge newEdge;

	newEdge.mn = comprev;
	newEdge.mx = comprev + len;
	newEdge.com = mxprev;
	newEdge.index = edgeIndex;

	targetList.push_back(newEdge);

	return false;
}

class Box {
public:
	double xmin, xmax, ymin, ymax;
	bool isInside(Box);
	bool isIntersect (Box);
};

bool Box::isIntersect( Box box ) {
	if ( (box.xmax <= xmin) || (box.ymin >= ymax) || (box.xmin >= xmax) || (box.ymax <= ymin) )
		 return false;
	else
		return true;
}

Box intersection(Box box1, Box box2) {
	Box intersect;
	double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
	intersect.xmin = xmin;
	intersect.xmax = xmax;
	intersect.ymin = ymin;
	intersect.ymax = ymax;

	if (box1.isIntersect(box2)) {

		// Setting xmin of the intersection
		if ( box1.xmin <= box2.xmin && box1.xmax >= box2.xmin )
			xmin = box2.xmin;
		else if ( box2.xmin <= box1.xmin && box2.xmax >= box1.xmin )
			xmin = box1.xmin;

		// Setting ymin of the intersection
		if ( box1.ymin <= box2.ymin && box1.ymax >= box2.ymin )
			ymin = box2.ymin;
		else if ( box2.ymin <= box1.ymin && box2.ymax >= box1.ymin )
			ymin = box1.ymin;

		// Setting ymax of the intersection
		if ( box1.ymax <= box2.ymax && box1.ymax >= box2.ymin )
			ymax = box1.ymax;
		else if ( box2.ymax <= box1.ymax && box2.ymax >= box1.ymin )
			ymax = box2.ymax;

		// Setting xmax of the intersection
		if ( box1.xmax <= box2.xmax && box1.xmax >= box2.xmin )
			xmax = box1.xmax;
		else if ( box2.xmax <= box1.xmax && box2.xmax >= box1.xmin )
			xmax = box2.xmax;

		// Populating the return box
		intersect.xmin = xmin;
		intersect.xmax = xmax;
		intersect.ymin = ymin;
		intersect.ymax = ymax;
	}
	return intersect;
}



int main(int argc, char* argv[]) {

	ifstream testcase;
	string line, token;
	stringstream str;

	if ( argc != 2 ) {
		cout<<"usage: "<< argv[0] <<" <filename>\n";
		return 1;
	}
	else {

		testcase.open(argv[1]);
		if ( !testcase.is_open() )
	      cout<<"Could not open file\n";

	    else {
	    	while (getline(testcase, line, '\n')) {

	    		str << line;

	    		while (getline(str, token)) {
	    			cout << token << " ";
	    		}

	    		str.clear();

			}
			return 0;
	    }
	}
}
