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

///////////////////////////////////////////////////////////////////////////////
//
// quadricShape.cpp
//
// Description:
//    Registers a new type of shape with maya called "quadricShape".
//    This shape will display spheres, cylinders, disks, and partial disks
//    using the OpenGL gluQuadric functions.
//
//    There are no output attributes for this shape.
//    The following input attributes define the type of shape to draw.
//
//       shapeType  : 0=cylinder, 1=disk, 2=partialDisk, 3=sphere
//       radius1	: cylinder base radius, disk inner radius, sphere radius
//       radius2	: cylinder top radius, disk outer radius
//       height		: cylinder height
//       startAngle	: partial disk start angle
//       sweepAngle	: partial disk sweep angle
//       slices		: cylinder, disk, sphere slices
//       loops		: disk loops
//       stacks		: cylinder, sphere stacks
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MIOStream.h>
#include <maya/MPxSurfaceShape.h>
#include <maya/MPxSurfaceShapeUI.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPoint.h>
#include <maya/MPlug.h>
#include <maya/MDrawData.h>
#include <maya/MDrawRequest.h>
#include <maya/MSelectionMask.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MMaterial.h>
#if defined(OSMac_MachO_)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

/////////////////////////////////////////////////////////////////////

#define MCHECKERROR(STAT,MSG)       \
    if ( MS::kSuccess != STAT ) {   \
        cerr << MSG << endl;        \
        return MS::kFailure;        \
    }

#define MAKE_NUMERIC_ATTR( NAME, SHORTNAME, TYPE, DEFAULT, KEYABLE ) \
	MStatus NAME##_stat;                                             \
	MFnNumericAttribute NAME##_fn;                                   \
	NAME = NAME##_fn.create( #NAME, SHORTNAME, TYPE, DEFAULT );      \
	MCHECKERROR(NAME##_stat, "numeric attr create error");		     \
	NAME##_fn.setHidden( !KEYABLE );								 \
	NAME##_fn.setKeyable( KEYABLE );								 \
	NAME##_fn.setInternal( true );									 \
	NAME##_stat = addAttribute( NAME );                              \
	MCHECKERROR(NAME##_stat, "addAttribute error");

#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue

/////////////////////////////////////////////////////////////////////
//
// Geometry class
//
class quadricGeom 
{
public:
	double radius1;
	double radius2;
	double height;
	double startAngle;
	double sweepAngle;
	short slices;
	short loops;
	short stacks;
    short shapeType;
};

/////////////////////////////////////////////////////////////////////
//
// Shape class - defines the non-UI part of a shape node
//
class quadricShape : public MPxSurfaceShape
{
public:
	quadricShape();
	virtual ~quadricShape(); 

	virtual void			postConstructor();
	virtual MStatus			compute( const MPlug&, MDataBlock& );
    virtual bool			getInternalValue( const MPlug&,
											  MDataHandle& );
    virtual bool			setInternalValue( const MPlug&,
											  const MDataHandle& );
					  
	virtual bool            isBounded() const;
	virtual MBoundingBox    boundingBox() const; 

	static  void *		creator();
	static  MStatus		initialize();
	quadricGeom*		geometry();

private:
	quadricGeom*		fGeometry;

	// Attributes
	//
    static  MObject     shapeType;
	static	MObject		radius1;
	static	MObject		radius2;
	static	MObject		height;
	static	MObject		startAngle;
	static	MObject		sweepAngle;
	static	MObject		slices;
	static	MObject		loops;
	static	MObject		stacks;
 
public:
	// Shape type id
	//
	static	MTypeId		id;
};

/////////////////////////////////////////////////////////////////////
//
// UI class	- defines the UI part of a shape node
//
class quadricShapeUI : public MPxSurfaceShapeUI
{
public:
	quadricShapeUI();
	virtual ~quadricShapeUI(); 

	virtual void	getDrawRequests( const MDrawInfo & info,
									 bool objectAndActiveOnly,
									 MDrawRequestQueue & requests );
	virtual void	draw( const MDrawRequest & request,
						  M3dView & view ) const;
	virtual bool	select( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	void			getDrawRequestsWireframe( MDrawRequest&,
											  const MDrawInfo& );
	void			getDrawRequestsShaded(	  MDrawRequest&,
											  const MDrawInfo&,
											  MDrawRequestQueue&,
											  MDrawData& data );

	static  void *  creator();

private:
	enum {
		kDrawCylinder,
		kDrawDisk,
		kDrawPartialDisk,
		kDrawSphere
	};

	// Draw Tokens
	//
	enum {
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
		kLastToken
	};
};

/////////////////////////////////////////////////////////////////////
// SHAPE NODE IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

MObject quadricShape::shapeType;
MObject quadricShape::radius1;
MObject quadricShape::radius2;
MObject quadricShape::height;
MObject quadricShape::startAngle;
MObject quadricShape::sweepAngle;
MObject quadricShape::slices;
MObject quadricShape::loops;
MObject quadricShape::stacks;
MTypeId quadricShape::id( 0x80111 );

quadricShape::quadricShape()
{
	fGeometry = new quadricGeom;
	fGeometry->radius1		= 1.0;
	fGeometry->radius2		= 1.0;
	fGeometry->height		= 2.0;
	fGeometry->startAngle	= 0.0;
	fGeometry->sweepAngle	= 90.0;
	fGeometry->slices		= 8;
	fGeometry->loops		= 6;
	fGeometry->stacks		= 4;
    fGeometry->shapeType	= 0;
}

quadricShape::~quadricShape()
{
	delete fGeometry;
}

/* override */
void quadricShape::postConstructor()
//
// Description
// 
//    When instances of this node are created internally, the MObject associated
//    with the instance is not created until after the constructor of this class
//    is called. This means that no member functions of MPxSurfaceShape can
//    be called in the constructor.
//    The postConstructor solves this problem. Maya will call this function
//    after the internal object has been created.
//    As a general rule do all of your initialization in the postConstructor.
//
{ 
	// This call allows the shape to have shading groups assigned
	//
	setRenderable( true );
}

/* override */
MStatus quadricShape::compute( const MPlug& /*plug*/, MDataBlock& /*datablock*/ )
//
// Since there are no output attributes this is not necessary but
// if we wanted to compute an output mesh for rendering it would
// be done here base on the inputs.
//
{ 
	return MS::kUnknownParameter;
}

/* override */
bool quadricShape::getInternalValue( const MPlug& plug,
									 MDataHandle& datahandle )
//
// Handle internal attributes.
// In order to impose limits on our attribute values we
// mark them internal and use the values in fGeometry intead.
//
{
	bool isOk = true;

	if ( plug == radius1 ) {
		datahandle.set( fGeometry->radius1 );
		isOk = true;
	}
	else if ( plug == radius2 ) {
		datahandle.set( fGeometry->radius2 );
		isOk = true;
	}
	else if ( plug == height ) {
		datahandle.set( fGeometry->height );
		isOk = true;
	}
	else if ( plug == startAngle ) {
		datahandle.set( fGeometry->startAngle );
		isOk = true;
	}
	else if ( plug == sweepAngle ) {
		datahandle.set( fGeometry->sweepAngle );
		isOk = true;
	}
	else if ( plug == slices ) {
		datahandle.set( fGeometry->slices );
		isOk = true;
	}
	else if ( plug == loops ) {
		datahandle.set( fGeometry->loops );
		isOk = true;
	}
	else if ( plug == stacks ) {
		datahandle.set( fGeometry->stacks );
		isOk = true;
	}
	else {
		isOk = MPxSurfaceShape::getInternalValue( plug, datahandle );
	}

	return isOk;
}
/* override */
bool quadricShape::setInternalValue( const MPlug& plug,
									 const MDataHandle& datahandle )
//
// Handle internal attributes.
// In order to impose limits on our attribute values we
// mark them internal and use the values in fGeometry intead.
//
{
	bool isOk = true;

	// In the case of a disk or partial disk the inner radius must
	// never exceed the outer radius and the minimum radius is 0
	//
	if ( plug == radius1 ) {
		double innerRadius = datahandle.asDouble();
		double outerRadius = fGeometry->radius2;

		if ( innerRadius > outerRadius ) {
			outerRadius = innerRadius;
		}

		if ( innerRadius < 0 ) {
			innerRadius = 0;
		}

		fGeometry->radius1 = innerRadius;
		fGeometry->radius2 = outerRadius;
		isOk = true;
	}
	else if ( plug == radius2 ) {
		double outerRadius = datahandle.asDouble();
		double innerRadius = fGeometry->radius1;

		if ( outerRadius <= 0 ) {
			outerRadius = 0.1;
		}

		if ( innerRadius > outerRadius ) {
			innerRadius = outerRadius;
		}

		if ( innerRadius < 0 ) {
			innerRadius = 0;
		}

		fGeometry->radius1 = innerRadius;
		fGeometry->radius2 = outerRadius;
		isOk = true;
	}
	else if ( plug == height ) {
		double val = datahandle.asDouble();
		if ( val <= 0 ) {
			val = 0.1;
		}
		fGeometry->height = val;
	}
	else if ( plug == startAngle ) {
		double val = datahandle.asDouble();
		fGeometry->startAngle = val;
	}
	else if ( plug == sweepAngle ) {
		double val = datahandle.asDouble();
		fGeometry->sweepAngle = val;
	}
	else if ( plug == slices ) {
		short val = datahandle.asShort();
		if ( val < 3 ) {
			val = 3;
		}
		fGeometry->slices = val;
	}
	else if ( plug == loops ) {
		short val = datahandle.asShort();
		if ( val < 3 ) {
			val = 3;
		}
		fGeometry->loops = val;
	}
	else if ( plug == stacks ) {
		short val = datahandle.asShort();
		if ( val < 2 ) {
			val = 2;
		}
		fGeometry->stacks = val;
	}
	else {
		isOk = MPxSurfaceShape::setInternalValue( plug, datahandle );
	}

	return isOk;
}

/* override */
bool quadricShape::isBounded() const { return true; }

/* override */
MBoundingBox quadricShape::boundingBox() const
//
// Returns the bounding box for the shape.
// In this case just use the radius and height attributes
// to determine the bounding box.
//
{
	MBoundingBox result;	
	quadricShape* nonConstThis = const_cast <quadricShape*> (this);
	quadricGeom* geom = nonConstThis->geometry();

	double r = geom->radius1;
	result.expand( MPoint(r,r,r) );	result.expand( MPoint(-r,-r,-r) );
	r = geom->radius2;
	result.expand( MPoint(r,r,r) );	result.expand( MPoint(-r,-r,-r) );
	r = geom->height;
	result.expand( MPoint(r,r,r) );	result.expand( MPoint(-r,-r,-r) );

    return result;
}

void* quadricShape::creator()
{
	return new quadricShape();
}

MStatus quadricShape::initialize()
{ 
	MStatus				stat;
    MFnNumericAttribute	numericAttr;
    MFnEnumAttribute	enumAttr;

	// QUADRIC type enumerated attribute
	//
	shapeType = enumAttr.create( "shapeType", "st", 0, &stat );
	MCHECKERROR( stat, "create shapeType attribute" );
		enumAttr.addField( "cylinder", 0 );
		enumAttr.addField( "disk", 1 );
		enumAttr.addField( "partialDisk", 2 );
		enumAttr.addField( "sphere", 3 );
	enumAttr.setHidden( false );
	enumAttr.setKeyable( true );
	stat = addAttribute( shapeType );
	MCHECKERROR( stat, "Error adding shapeType attribute." );
	
	// QUADRIC ATTRIBUTES
	//
	MAKE_NUMERIC_ATTR( radius1, "r1", MFnNumericData::kDouble, 1.0, true );
	MAKE_NUMERIC_ATTR( radius2, "r2", MFnNumericData::kDouble, 1.0, true );
	MAKE_NUMERIC_ATTR( height, "ht", MFnNumericData::kDouble, 2.0, true );
	MAKE_NUMERIC_ATTR( startAngle, "sta", MFnNumericData::kDouble, 0.0, true );
	MAKE_NUMERIC_ATTR( sweepAngle, "swa", MFnNumericData::kDouble, 90.0, true );
	MAKE_NUMERIC_ATTR( slices, "sl", MFnNumericData::kShort, 8, true );
	MAKE_NUMERIC_ATTR( loops, "lp", MFnNumericData::kShort, 6, true );
	MAKE_NUMERIC_ATTR( stacks, "sk", MFnNumericData::kShort, 4, true );

	return stat;
}

quadricGeom* quadricShape::geometry()
//
// This function gets the values of all the attributes and
// assigns them to the fGeometry. Calling MPlug::getValue
// will ensure that the values are up-to-date.
//
{
	MObject this_object = thisMObject();
	MPlug plug( this_object, radius1 );	plug.getValue( fGeometry->radius1 );
	plug.setAttribute( radius2 );		plug.getValue( fGeometry->radius2 );
	plug.setAttribute( height );		plug.getValue( fGeometry->height );
	plug.setAttribute( startAngle );	plug.getValue( fGeometry->startAngle );
	plug.setAttribute( sweepAngle );	plug.getValue( fGeometry->sweepAngle );
	plug.setAttribute( slices );		plug.getValue( fGeometry->slices );
	plug.setAttribute( loops );			plug.getValue( fGeometry->loops );
	plug.setAttribute( stacks );		plug.getValue( fGeometry->stacks );
	plug.setAttribute( shapeType );		plug.getValue( fGeometry->shapeType );

	return fGeometry;
}

/////////////////////////////////////////////////////////////////////
// UI IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

quadricShapeUI::quadricShapeUI() {}
quadricShapeUI::~quadricShapeUI() {}

void* quadricShapeUI::creator()
{
	return new quadricShapeUI();
}

/* override */
void quadricShapeUI::getDrawRequests( const MDrawInfo & info,
							 bool /*objectAndActiveOnly*/,
							 MDrawRequestQueue & queue )
{
	// The draw data is used to pass geometry through the 
	// draw queue. The data should hold all the information
	// needed to draw the shape.
	//
	MDrawData data;
	MDrawRequest request = info.getPrototype( *this );
	quadricShape* shapeNode = (quadricShape*)surfaceShape();
	quadricGeom* geom = shapeNode->geometry();
	getDrawData( geom, data );
	request.setDrawData( data );

	// Are we displaying meshes?
	if ( ! info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
		return;

	// Use display status to determine what color to draw the object
	//
	switch ( info.displayStyle() )
	{		 
		case M3dView::kWireFrame :
			getDrawRequestsWireframe( request, info );
			queue.add( request );
			break;
		
		case M3dView::kGouraudShaded :
			request.setToken( kDrawSmoothShaded );
			getDrawRequestsShaded( request, info, queue, data );
			queue.add( request );
			break;
		
		case M3dView::kFlatShaded :
			request.setToken( kDrawFlatShaded );
 			getDrawRequestsShaded( request, info, queue, data );
			queue.add( request );
			break;
		default:	
			break;
	}
}

/* override */
void quadricShapeUI::draw( const MDrawRequest & request, M3dView & view ) const
//
// From the given draw request, get the draw data and determine
// which quadric to draw and with what values.
//
{ 	
	MDrawData data = request.drawData();
	quadricGeom * geom = (quadricGeom*)data.geometry();
	int token = request.token();
	bool drawTexture = false;

	view.beginGL(); 

	if ( (token == kDrawSmoothShaded) || (token == kDrawFlatShaded) )
	{
#if		defined(SGI) || defined(MESA)
		glEnable( GL_POLYGON_OFFSET_EXT );
#else
		glEnable( GL_POLYGON_OFFSET_FILL );
#endif
		// Set up the material
		//
		MMaterial material = request.material();
		material.setMaterial( request.multiPath(), request.isTransparent() );

		// Enable texturing
		//
		drawTexture = material.materialIsTextured();
		if ( drawTexture ) glEnable(GL_TEXTURE_2D);

		// Apply the texture to the current view
		//
		if ( drawTexture ) {
			material.applyTexture( view, data );
		}
	}

	GLUquadricObj* qobj = gluNewQuadric();

	switch( token )
	{
		case kDrawWireframe :
		case kDrawWireframeOnShaded :
			gluQuadricDrawStyle( qobj, GLU_LINE );
			break;

		case kDrawSmoothShaded :
			gluQuadricNormals( qobj, GLU_SMOOTH );
			gluQuadricTexture( qobj, true );
			gluQuadricDrawStyle( qobj, GLU_FILL );
			break;

		case kDrawFlatShaded :
			gluQuadricNormals( qobj, GLU_FLAT );
			gluQuadricTexture( qobj, true );
			gluQuadricDrawStyle( qobj, GLU_FILL );
			break;
	}

	switch ( geom->shapeType )
	{
	case kDrawCylinder :
		gluCylinder( qobj, geom->radius1, geom->radius2, geom->height,
					 geom->slices, geom->stacks );
		break;
	case kDrawDisk :
		gluDisk( qobj, geom->radius1, geom->radius2, geom->slices, geom->loops );
		break;
	case kDrawPartialDisk :
		gluPartialDisk( qobj, geom->radius1, geom->radius2, geom->slices,
						geom->loops, geom->startAngle, geom->sweepAngle );
		break;
	case kDrawSphere : 
	default :
		gluSphere( qobj, geom->radius1, geom->slices, geom->stacks );
		break;
	}

	// Turn off texture mode
	//
	if ( drawTexture ) glDisable(GL_TEXTURE_2D);

	view.endGL(); 
}

/* override */
bool quadricShapeUI::select( MSelectInfo &selectInfo,
							 MSelectionList &selectionList,
							 MPointArray &worldSpaceSelectPts ) const
//
// Select function. Gets called when the bbox for the object is selected.
// This function just selects the object without doing any intersection tests.
//
{
	MSelectionMask priorityMask( MSelectionMask::kSelectObjectsMask );
	MSelectionList item;
	item.add( selectInfo.selectPath() );
	MPoint xformedPt;
	selectInfo.addSelection( item, xformedPt, selectionList,
							 worldSpaceSelectPts, priorityMask, false );
	return true;
}

void quadricShapeUI::getDrawRequestsWireframe( MDrawRequest& request,
											   const MDrawInfo& info )
{
	request.setToken( kDrawWireframe );

	M3dView::DisplayStatus displayStatus = info.displayStatus();
	M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
	M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;
	switch ( displayStatus )
	{
		case M3dView::kLead :
			request.setColor( LEAD_COLOR, activeColorTable );
			break;
		case M3dView::kActive :
			request.setColor( ACTIVE_COLOR, activeColorTable );
			break;
		case M3dView::kActiveAffected :
			request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
			break;
		case M3dView::kDormant :
			request.setColor( DORMANT_COLOR, dormantColorTable );
			break;
		case M3dView::kHilite :
			request.setColor( HILITE_COLOR, activeColorTable );
			break;
		default:	
			break;
	}
}

void quadricShapeUI::getDrawRequestsShaded( MDrawRequest& request,
											const MDrawInfo& info,
											MDrawRequestQueue& queue,
											MDrawData& data )
{
	// Need to get the material info
	//
	MDagPath path = info.multiPath();	// path to your dag object 
	M3dView view = info.view();; 		// view to draw to
	MMaterial material = MPxSurfaceShapeUI::material( path );
	M3dView::DisplayStatus displayStatus = info.displayStatus();

	// Evaluate the material and if necessary, the texture.
	//
	if ( ! material.evaluateMaterial( view, path ) ) {
		cerr << "Couldnt evaluate\n";
	}

	bool drawTexture = true;
	if ( drawTexture && material.materialIsTextured() ) {
		material.evaluateTexture( data );
	}

	request.setMaterial( material );

	bool materialTransparent = false;
	material.getHasTransparency( materialTransparent );
	if ( materialTransparent ) {
		request.setIsTransparent( true );
	}
	
	// create a draw request for wireframe on shaded if
	// necessary.
	//
	if ( (displayStatus == M3dView::kActive) ||
		 (displayStatus == M3dView::kLead) ||
		 (displayStatus == M3dView::kHilite) )
	{
		MDrawRequest wireRequest = info.getPrototype( *this );
		wireRequest.setDrawData( data );
		getDrawRequestsWireframe( wireRequest, info );
		wireRequest.setToken( kDrawWireframeOnShaded );
		wireRequest.setDisplayStyle( M3dView::kWireFrame );
		queue.add( wireRequest );
	}
}

/////////////////////////////////////////////////////////////////////

MStatus initializePlugin( MObject obj )
{ 
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
	return plugin.registerShape( "quadricShape", quadricShape::id,
								   &quadricShape::creator,
								   &quadricShape::initialize,
								   &quadricShapeUI::creator  );
}

MStatus uninitializePlugin( MObject obj)
{
	MFnPlugin plugin( obj );
	return plugin.deregisterNode( quadricShape::id );
}
