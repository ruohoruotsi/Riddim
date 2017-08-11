//  BeatRoot Version 0.2
//  An interactive beat tracking and visualisation program
//  File: tree.h
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

class event;
class eventList;

typedef event* DATA_TYPE;

class AVLtree {
		AVLtree* left;
		AVLtree* right;
		int height, size;
	protected:
		DATA_TYPE data;

	private:
		AVLtree* resize(bool change);
		AVLtree* rotateRight();
		AVLtree* rotateLeft();
		AVLtree* rotateRightLeft();
		AVLtree* rotateLeftRight();
		void init();
	protected:
		AVLtree* add(AVLtree* val);
		AVLtree() { init(); }
		void init(DATA_TYPE value);
		AVLtree* getLeft() const { return left; }
		AVLtree* getRight() const { return right; }
	public:
		AVLtree(DATA_TYPE value) { init(value); };
		virtual AVLtree* add(DATA_TYPE value) = 0;
		int getHeight() const { return height; }
		int getSize() const { return size; }
		void print(int indent = 0) const;
		virtual void printValue() const;
		virtual double getKey() const = 0;
};

class eventTree : public AVLtree {
	public:
		eventTree* add(DATA_TYPE value) { return dynamic_cast<eventTree*>(
										  AVLtree::add(new eventTree(value))); }
		eventTree(DATA_TYPE value) { init(value); }
		double getKey() const { return data->onset; }
		void printValue() const { data->print(); }
		eventList* getContents() const;
		void addContents(eventList* l) const;
};
