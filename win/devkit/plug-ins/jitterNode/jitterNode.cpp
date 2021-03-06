//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

// Jitter
// 
// Description:
// 
// 	This is a dependency graph node which takes a
// 	floating point value as input, adds a random value,
// 	and passes the result on as output.  A time input
// 	is used to ensure the output result is repeatable
// 	when played back.  A scale input is used to scale 
// 	the value beyond 1.0.
// 
// Attributes ( < input, > output ):
// 
// 	< input: the input float value
// 	< scale: the scalar for the random value (0 - 1.0)
// 	< time: the frame number
// 
// 	> output: the randomized float value
// 
// Usage:
// 
// 	Use jitterNode.mel to insert this DG node between
// 	a float value connection in your DG.  See that MEL
//  script for usage information.

#include <string.h>
#include <stdio.h>
#include <maya/MIOStream.h>

#include <maya/MPxNode.h>

#include <maya/MString.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MVector.h>

#include <maya/MFnPlugin.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

static unsigned int xRand, yRand, zRand;     /* seed */

float randomd()
{
    float result;
    unsigned int a = xRand/177, b = xRand%177;
    unsigned int c = yRand/176, d = yRand%176;
    unsigned int e = zRand/178, f = zRand%178;

    xRand = (171 * b - 2 * a) % 30269;
    yRand = (172 * d - 35 * c) % 30307;
    zRand = (170 * f - 63 * e) % 30323;

    result = (float) xRand/30269.0f + yRand/30307.0f + zRand/30323.0f;
    return result - ((int) result);
}

void seedd(unsigned char nx, unsigned char ny, unsigned char nz)
{
    xRand = nx;
    yRand = ny;
    zRand = nz;
    randomd();
    randomd();
    randomd();
    randomd();
}

class jitter : public MPxNode
{
public:
	jitter();
	virtual ~jitter(); 

	virtual	MStatus		compute( const MPlug& plug, MDataBlock& data );

	static	void*		creator();
	static	MStatus		initialize();
 
	static	MObject		time;		// The time value.
	static	MObject		scale;		// Scale of jitter.

	static	MObject		input;		// The input value.
	static	MObject		output;		// The jittered-output value.

	static	MTypeId		id;
};

MTypeId     jitter::id( 0x80009 );
MObject		jitter::time;
MObject		jitter::scale;
MObject     jitter::input;
MObject     jitter::output; 

void* jitter::creator()
{
	return new jitter();
}

MStatus jitter::initialize()
{
	MFnNumericAttribute nAttr;
	MStatus				stat;

	time = nAttr.create( "time", "tm", MFnNumericData::kFloat, 0.0 );
 	nAttr.setStorable(true);

	scale = nAttr.create( "scale", "sc", MFnNumericData::kFloat, 1.0 );
	nAttr.setStorable(true);

	input = nAttr.create( "input", "in", MFnNumericData::kFloat, 0.0 );
	nAttr.setStorable(true);

	output = nAttr.create( "output", "out", MFnNumericData::kFloat, 0.0 );
	nAttr.setWritable(false);
	nAttr.setStorable(false);

	stat = addAttribute( time );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( scale );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( input );
		if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( output );
		if (!stat) { stat.perror("addAttribute"); return stat;}

    stat = attributeAffects( time, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects( scale, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects( input, output );
		if (!stat) { stat.perror("attributeAffects"); return stat;}

	return MS::kSuccess;
} 

jitter::jitter() {}

jitter::~jitter() {}

// Compute the offset and add it to input
// as the output from this node.
MStatus jitter::compute( const MPlug& plug, MDataBlock& data )
{
	MStatus returnStatus;
 
	if( plug == output )
	{
		MDataHandle timeData = data.inputValue( time, &returnStatus );
		MDataHandle scaleData = data.inputValue( scale, &returnStatus );
		MDataHandle inputData = data.inputValue( input, &returnStatus );

		if( returnStatus != MS::kSuccess )
			cerr << "ERROR getting data\n";
		else
		{
			float currentFrame = timeData.asFloat();
			float scaleFactor  = scaleData.asFloat();
			float inValue = inputData.asFloat();

			// This is where we jitter the value

			unsigned char seed = (unsigned char)currentFrame;
			seedd( seed, seed * 17, seed * 23 );

			float tmp = randomd();
			float result = ( tmp - 0.5f ) * scaleFactor + inValue;

			MDataHandle outHandle = data.outputValue( jitter::output );

			outHandle.set( result );

			data.setClean(plug);
		}
	}
	else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");

	status = plugin.registerNode( "jitter", jitter::id, 
						 jitter::creator, jitter::initialize  );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus	  status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( jitter::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
