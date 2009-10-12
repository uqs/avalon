#ifndef GRID_TYPES_H
#define GRID_TYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * \global
 * This file describes different object that can be stored in a VoxSpace
 * Such an objcet must have an operator=, and functions save,load,print
 * **/

/**
 * \class GenericCell
 * A generic Cell to contains anything
 * **/
template <class C>
class GenericCell {
	public:
		C value;
		GenericCell() {}
		GenericCell(const C & v) : value(v) {}
		const GenericCell & operator=(const C & v) {value = v;return *this;}
		const GenericCell & operator=(const GenericCell & v) {value = v.value;return *this;}
		bool load(FILE * fp) {return fread(&value,sizeof(C),1,fp)==1;}
		bool save(FILE * fp) const {return fwrite(&value,sizeof(C),1,fp)==1;}
		void print(FILE * fp) const {fprintf(fp,"%p ",&value);}
};

/**
 * \class IntegerCell
 * A Cell that contains an integer value
 * **/
template <class C>
class IntegerCell : public GenericCell<C> 
{
	public :
		IntegerCell() : GenericCell<C>() {}
		IntegerCell(const C & v) : GenericCell<C>(v) {}
		void print(FILE * fp) const {fprintf(fp,"%d ",GenericCell<C>::value);}
};

/**
 * \class FloatingCell
 * A Cell that contains floating point value
 * **/
template <class C>
class FloatingCell : public GenericCell<C> 
{
	public:
		FloatingCell() : GenericCell<C>() {}
		FloatingCell(const C & v) : GenericCell<C>(v) {}
		void print(FILE * fp) const {fprintf(fp,"%le ",GenericCell<C>::value);}
};



#endif // GRID_TYPES_H
