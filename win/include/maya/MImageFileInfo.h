#ifndef _MImageFileInfo
#define _MImageFileInfo
//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc., and/or its licensors.  All
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files contain unpublished
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its
// licensors,  which is protected by U.S. and Canadian federal copyright law
// and by international treaties.
//
// The Data may not be disclosed or distributed to third parties or be
// copied or duplicated, in whole or in part, without the prior written
// consent of Autodesk.
//
// The copyright notices in the Software and this entire statement,
// including the above license grant, this restriction and the following
// disclaimer, must be included in all copies of the Software, in whole
// or in part, and all derivative works of the Software, unless such copies
// or derivative works are solely in the form of machine-executable object
// code generated by a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
// OR ARISING FROM A COURSE OF DEALING, USAGE, OR TRADE PRACTICE. IN NO
// EVENT WILL AUTODESK AND/OR ITS LICENSORS BE LIABLE FOR ANY LOST
// REVENUES, DATA, OR PROFITS, OR SPECIAL, DIRECT, INDIRECT, OR
// CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS LICENSORS HAS
// BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.
// ==========================================================================
//+
//
// CLASS:    MImageFileInfo
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MImage.h>
#include <maya/MTypes.h>

// ****************************************************************************
// CLASS DECLARATION (MImageFileInfo)

//! \ingroup OpenMaya
//! \brief Image manipulation 
/*!

  This class provides methods for reading file images stored on disk.

  MImageFileInfo is a utility class used to describe the characteristics
  of an image file, such as dimensions, channel count, and pixel format.
  This class is used in MPxImageFile.
*/
class OPENMAYA_EXPORT MImageFileInfo
{
public:
	//! Hardware texture types.
	enum MHwTextureType {
		kHwTextureUnknown,	//!< \nop
		kHwTexture1D,		//!< \nop
		kHwTexture2D,		//!< \nop
		kHwTexture3D,		//!< \nop
		kHwTextureRectangle,	//!< \nop
		kHwTextureCubeMap	//!< \nop
	};

	//! Image types.
	enum MImageType {
		kImageTypeUnknown,	//!< \nop
		kImageTypeColor,	//!< \nop
		kImageTypeNormal,	//!< \nop
		kImageTypeBump		//!< \nop
	};

					MImageFileInfo();
					void width( unsigned int value);
					void height( unsigned int value);
					void channels( unsigned int value);
					void numberOfImages( unsigned int value);
					void pixelType( MImage::MPixelType value);
					void imageType( MImageType value);
					void hardwareType( MHwTextureType value);
					void hasAlpha( bool value);
					void hasMipMaps( bool value);
					unsigned int width() const;
					unsigned int height() const;
					unsigned int channels() const;
					unsigned int numberOfImages() const;
					MImage::MPixelType pixelType() const;
					MImageType imageType() const;
					MHwTextureType hardwareType() const;
					bool hasAlpha() const;
					bool hasMipMaps() const;

protected:
	// No protected members

private:

	unsigned int	imageWidth;
	unsigned int	imageHeight;
	unsigned int	imageChannels;
	unsigned int	imageCount;
	MImage::MPixelType imagePixelType;
	MImageType		imageImageType;
	MHwTextureType	imageHardwareType;
	bool			imageHasAlpha;
	bool			imageHasMipMaps;
};

#endif /* __cplusplus */
#endif /* _MImageFileInfo */
