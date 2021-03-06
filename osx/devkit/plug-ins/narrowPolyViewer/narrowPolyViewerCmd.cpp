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

//	File Name: narrowPolyViewerCmd.cpp
//
//	Description:
//		A command used for testing multiple camera drawing into a single view.
//		See narrowPolyViewerCmd.h for a description.
//


#include "narrowPolyViewerCmd.h"

#include <maya/MItDag.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgParser.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MFnCamera.h>

#include <maya/MIOStream.h>

narrowPolyViewerCmd::narrowPolyViewerCmd()
: 	MPxModelEditorCommand()
//
//	Description:
//		Class constructor.
//
{
}

narrowPolyViewerCmd::~narrowPolyViewerCmd()
//
//	Description:
//		Class destructor.
//
{
}

void* narrowPolyViewerCmd::creator()
//
//	Description:
//		Create the command.
//
{
    return new narrowPolyViewerCmd();
}

MStatus narrowPolyViewerCmd::appendSyntax()
//
//	Description:
//		Add syntax to the command. All of the parent syntax is added
//		before this call is made.
//
{
	MStatus ReturnStatus;

	MSyntax theSyntax = syntax(&ReturnStatus);
	if (MS::kSuccess != ReturnStatus) {
		MGlobal::displayError("Could not get the parent's syntax");
		return ReturnStatus;
	}

	theSyntax.addFlag(kInitFlag,
					  kInitFlagLong);

	theSyntax.addFlag(kResultsFlag,
					  kResultsFlagLong);

	theSyntax.addFlag(kClearFlag,
					  kClearFlagLong);

	theSyntax.addFlag(kToleranceFlag,
		              kToleranceFlagLong,
					  MSyntax::kDouble);

	return ReturnStatus;
}

MStatus narrowPolyViewerCmd::doEditFlags()
//
//	Description:
//		Handle edits for flags added by this class.
//		If the flag is unknown, return MS::kSuccess and the parent class
//		will attempt to process the flag. Returning MS::kUnknownParameter
//		will cause the parent class to process the flag.
//
{
	MStatus ReturnStatus = MS::kSuccess;

	MPx3dModelView *user3dModelView = modelView();
	if (NULL == user3dModelView) {
		MGlobal::displayError("NULL == user3dModelView!");
		return MS::kFailure;
	}

	if (user3dModelView->viewType() != "narrowPolyViewer") {
		MGlobal::displayError("This view is not a narrowPolyViewer");
		return MS::kFailure;
	}

	//	This is now safe to do, since the above test passed.
	//
	narrowPolyViewer *mpView = (narrowPolyViewer *)user3dModelView;

	MArgParser argData = parser();
	if (argData.isFlagSet(kInitFlag)) {
		return initTests(*mpView);
	} else if (argData.isFlagSet(kResultsFlag)) {
		return testResults(*mpView);
	} else if (argData.isFlagSet(kClearFlag)) {
		return clearResults(*mpView);
	} else if (argData.isFlagSet(kToleranceFlag)) {
		double tol;
		argData.getFlagArgument(kToleranceFlag, 0, tol);
		mpView->tol = tol;
		mpView->refresh(true, true);
	} else {
		return MS::kUnknownParameter;
	}

	return ReturnStatus;
}

MStatus narrowPolyViewerCmd::doQueryFlags()
//
//	Description:
//		Don't do anything for the query.
//
{
	return ParentClass::doQueryFlags();
}

MStatus narrowPolyViewerCmd::clearResults(narrowPolyViewer &view)
//
//	Description:
//
{	
	view.removeAllCameras();
	fCameraList.clear();
	return MS::kSuccess;
}

MStatus narrowPolyViewerCmd::initTests(narrowPolyViewer &view)
//
//	Description:
//
{
	MGlobal::displayInfo("narrowPolyViewerCmd::initTests");

	clearResults(view);

	//	Add every camera into the scene. Don't change the main camera,
	//	it is OK that it gets reused.
	//
	MDagPath cameraPath;
	MStatus status = MS::kSuccess;
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kCamera);
	for (; !dagIterator.isDone(); dagIterator.next()) {
		if (!dagIterator.getPath(cameraPath)) {
			continue;
		}

		MFnCamera camera(cameraPath, &status);
		if (MS::kSuccess != status) {
			continue;
		}

		MGlobal::displayInfo(camera.fullPathName());
		fCameraList.append(cameraPath);
	}

	if (MS::kSuccess !=
		view.setCameraList(fCameraList)) {
		MGlobal::displayError("Could not set the list of cameras");
		return MS::kFailure;
	} else {
		view.refresh();
	}

	return MS::kSuccess;
}

MStatus narrowPolyViewerCmd::testResults(narrowPolyViewer &view)
{
	cout << "fCameraList.length() = " << fCameraList.length() << endl;
	cout << "fCameraList = " << fCameraList << endl;

	cout << "view.fTestCameraList.length() = " << view.fTestCameraList.length() << endl;
	cout << "view.fTestCameraList = " << view.fTestCameraList << endl;

	return MS::kSuccess;
}
