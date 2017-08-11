//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: tree.cpp
// 
//  Copyright (C) 2001  Simon Dixon <simon@oefai.at>
// 
//  This file is part of BeatRoot.
//
//  BeatRoot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  BeatRoot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with BeatRoot; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include "event.h"
#include "includes.h"
#include "tree.h"
#include "util.h"


// AVL tree implementation of a binary search tree
// O(log(n)) performance for insertion, deletion, retrieval

void AVLtree :: init() {
	left = NULL;
	right = NULL;
	height = 0;
	size = 0;
} // init()

void AVLtree :: init(DATA_TYPE value) {
	init();
	data = value;
	height = 1;
	size = 1;
} // init(DATA_TYPE)

int heightOf(const AVLtree* t) {
	if (t == NULL)
		return 0;
	else
		return t->getHeight();
}

int sizeOf(const AVLtree* t) {
	if (t == NULL)
		return 0;
	else
		return t->getSize();
}

AVLtree* AVLtree :: rotateRight() {
	AVLtree* new_top = left;
	left = new_top->right;
	new_top->right = this;
	assertWarning(this == resize(false), "Error 1 in AVLtree");
	assertWarning(new_top == new_top->resize(false),"Error 2 in AVLtree");
	return new_top;
} // rotateRight()

AVLtree* AVLtree :: rotateLeft() {
	AVLtree* new_top = right;
	right = new_top->left;
	new_top->left = this;
	assertWarning(this == resize(false), "Error 3 in AVLtree");
	assertWarning(new_top == new_top->resize(false),"Error 4 in AVLtree");
	return new_top;
} // rotateLeft()

AVLtree* AVLtree :: rotateRightLeft() {
	right = right->rotateRight();
	return rotateLeft();
} // rotateRightLeft()

AVLtree* AVLtree :: rotateLeftRight() {
	left = left->rotateLeft();
	return rotateRight();
} // rotateLeftRight()


AVLtree* AVLtree :: add(AVLtree* val) {
	if (val->getKey() < getKey()) {
		if (left != NULL)
			left = left->add(val);
		else
			left = val;
	} else {
		if (right != NULL)
			right = right->add(val);
		else
			right = val;
	}
	return resize(true);
} // add()

// assumes subtrees are correctly sized
AVLtree* AVLtree :: resize(bool change) {
	int h1 = heightOf(left);
	int h2 = heightOf(right);
	if (change && (h1 - h2 > 1)) {
		if (heightOf(left->left) > heightOf(left->right))
			return rotateRight();
		else
			return rotateLeftRight();
	} else if (change && (h2 - h1 > 1)) {
		if (heightOf(right->right) > heightOf(right->left))
			return rotateLeft();
		else
			return rotateRightLeft();
	} else {
		height = (h1 > h2 ? h1 : h2) + 1;
		size = sizeOf(left) + sizeOf(right) + 1;
		return this;
	}
} // resize()

void AVLtree :: print(int indent) const {
	if (indent == 0)
		cout << "AVLtree(" << heightOf(left) << "," << heightOf(right) << ")\n";
	if (left != NULL)
		left->print(indent+1);
	for (int i=0; i < indent; i++)
		cout << "    ";
	printValue();
	if (right != NULL)
		right->print(indent+1);
} // print()

void AVLtree :: printValue() const {
	cout.setf(ios::fixed, ios::scientific);
	cout << setprecision(3) << getKey() << endl;
} // printValue()


// Implementation of CLASS EVENTTREE

eventList* eventTree :: getContents() const {
	eventList* list = new eventList();
	addContents(list);
    return list;
} // getContents()

void eventTree :: addContents(eventList* list) const {
	if (getLeft() != NULL)
		((eventTree*)getLeft())->addContents(list);
	list->add(data);
	if (getRight() != NULL)
		((eventTree*)getRight())->addContents(list);
} // addContents()
