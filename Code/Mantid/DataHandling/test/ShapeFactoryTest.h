#ifndef SHAPEFACTORYTEST_H_
#define SHAPEFACTORYTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidDataHandling/ShapeFactory.h"
#include "MantidDataHandling/LoadEmptyInstrument.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/Instrument.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidKernel/Exception.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/Workspace.h"
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/SpectraDetectorMap.h"
#include "MantidGeometry/Component.h"
#include <vector>

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"

using Poco::XML::DOMParser;
using Poco::XML::Document;
using Poco::XML::Element;


using namespace Mantid::API;
using namespace Mantid::Kernel;
using namespace Mantid::Geometry;
using namespace Mantid::DataHandling;
using namespace Mantid::DataObjects;

class ShapeFactoryTest : public CxxTest::TestSuite
{
public:


	void testCuboid()
	{
		std::string xmlShape = "<cuboid id=\"shape\"> ";
		xmlShape +=	"<left-front-bottom-point x=\"0.005\" y=\"-0.1\" z=\"0.0\" /> " ;
  	xmlShape +=	"<left-front-top-point x=\"0.005\" y=\"-0.1\" z=\"0.0001\" />  " ;
  	xmlShape +=	"<left-back-bottom-point x=\"-0.005\" y=\"-0.1\" z=\"0.0\" />  " ;
  	xmlShape +=	"<right-front-bottom-point x=\"0.005\" y=\"0.1\" z=\"0.0\" />  " ;
  	xmlShape +=	"</cuboid> ";
		xmlShape +=	"<algebra val=\"shape\" /> ";  
		
		boost::shared_ptr<Object> shape_sptr = getObject(xmlShape);

    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.0,0.00001)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.0,0.001)) );
    TS_ASSERT( shape_sptr->isValid(V3D(-0.004,0.0,0.00001)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(-0.006,0.0,0.00001)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.09, 0.00001)) );
	}
	
	void testSphere()
	{
		//algebra line is essential
		std::string xmlShape = "<sphere id=\"shape\"> ";
		xmlShape +=	"<centre x=\"4.1\"  y=\"2.1\" z=\"8.1\" /> " ;
  	xmlShape +=	"<radius val=\"3.2\" /> " ;
  	xmlShape +=	"</sphere>";
		xmlShape +=	"<algebra val=\"shape\" /> ";  

		boost::shared_ptr<Object> shape_sptr = getObject(xmlShape);

    TS_ASSERT( shape_sptr->isValid(V3D(4.1,2.1,8.1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(47.1,2.1,8.1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(5.1,2.1,8.1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(-0.006,0.0,0.00001)) );
    TS_ASSERT( shape_sptr->isValid(V3D(4.1,2.1,9.1)) );
	}

	void testCylinderHit()
	{
		//algebra line is essential
		std::string xmlShape = "<cylinder id=\"shape\"> ";
		xmlShape +=	"<centre-of-bottom-base x=\"0.0\" y=\"0.0\" z=\"0.0\" /> " ; 
  	xmlShape +=	"<axis x=\"0.0\" y=\"0.0\" z=\"1\" /> " ;
  	xmlShape +=	"<radius val=\"0.1\" /> " ;
  	xmlShape +=	"<height val=\"3\" /> " ;
  	xmlShape +=	"</cylinder>";
		xmlShape +=	"<algebra val=\"shape\" /> ";  

		boost::shared_ptr<Object> shape_sptr = getObject(xmlShape);

    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.0,1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.0,10)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.05,1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.15,1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.01,0.01,1)) );
	}

	void testInfiniteCylinderHit()
	{
		//algebra line is essential
		std::string xmlShape = "<infinite-cylinder id=\"shape\"> ";
		xmlShape +=	"<centre x=\"0.0\" y=\"0.0\" z=\"0.0\" /> " ; 
  	xmlShape +=	"<axis x=\"0.0\" y=\"0.0\" z=\"1\" /> " ;
  	xmlShape +=	"<radius val=\"0.1\" /> " ;
  	xmlShape +=	"</infinite-cylinder>";
		xmlShape +=	"<algebra val=\"shape\" /> ";  

		boost::shared_ptr<Object> shape_sptr = getObject(xmlShape);

    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.0,1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.0,10)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.05,1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.15,1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.01,0.01,1)) );
	}

	void testCone()
	{
		//algebra line is essential
		std::string xmlShape = "<cone id=\"shape\"> ";
		xmlShape +=	"<tip-point x=\"0.0\" y=\"0.0\" z=\"0.0\" /> " ; 
  	xmlShape +=	"<axis x=\"0.0\" y=\"0.0\" z=\"1\" /> " ;
  	xmlShape +=	"<angle val=\"8.1\" /> " ;
  	xmlShape +=	"<height val=\"4\" /> " ;
  	xmlShape +=	"</cone>";
		xmlShape +=	"<algebra val=\"shape\" /> ";  

		boost::shared_ptr<Object> shape_sptr = getObject(xmlShape);

    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.0,1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.0,-1)) );
    TS_ASSERT( !shape_sptr->isValid(V3D(0.0,0.001,1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.0,0.001,-1)) );
    TS_ASSERT( shape_sptr->isValid(V3D(0.01,0.01,-1)) );
	}

	boost::shared_ptr<Object> getObject(std::string xmlShape)
  {
		std::string shapeXML = "<type name=\"userShape\"> " + xmlShape + " </type>";

	  // Set up the DOM parser and parse xml string
		DOMParser pParser;
		Document* pDoc;

  	pDoc = pParser.parseString(shapeXML);

		// Get pointer to root element
		Element* pRootElem = pDoc->documentElement();

		//convert into a Geometry object
		ShapeFactory sFactory;
		boost::shared_ptr<Object> shape_sptr = sFactory.createShape(pRootElem);
		pDoc->release();
    return shape_sptr;
  }



private:
  std::string inputFile;

};

#endif /*SHAPEFACTORYTEST_H_*/
