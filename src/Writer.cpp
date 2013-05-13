/******************************************************************************
* Copyright (c) 2013, Bradley J Chambers, brad.chambers@gmail.com
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <prc/Writer.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>

#include <hpdf.h>
#include <hpdf_u3d.h>
#include <hpdf_annotation.h>

#include <pdal/Dimension.hpp>
#include <pdal/Schema.hpp>
#include <pdal/PointBuffer.hpp>
#include <pdal/pdal_macros.hpp>
#include <pdal/StageFactory.hpp>

#include <prc/oPRCFile.hpp>

MAKE_WRITER_CREATOR(prcWriter, pdal::drivers::prc::Writer)
CREATE_WRITER_PLUGIN(prc, pdal::drivers::prc::Writer)

enum OUTPUT_FORMAT {
    OUTPUT_FORMAT_PDF,
    OUTPUT_FORMAT_PRC
};


namespace pdal
{
namespace drivers
{
namespace prc
{

Writer::Writer(Stage& prevStage, const Options& options)
  : pdal::Writer(prevStage, options)
  //, m_prcFile(getOptions().getValueOrThrow<std::string>("prc_filename"))
  , m_prcFile(options.getOption("prc_filename").getValue<std::string>())
  //, m_prcFilename()
  //, m_pdfFilename()
  , m_outputFormat(OUTPUT_FORMAT_PDF)
{
  std::cout << "writer\n";
  std::cout << options.getOption("prc_filename").getValue<std::string>() << std::endl;
  
  return;
}


Writer::~Writer()
{
  return;
}


void Writer::initialize()
{
  std::cout << "init\n";

  pdal::Writer::initialize();

  std::cout << "succes\n";

  //m_prcFilename = getOptions().getValueOrThrow<std::string>("prc_filename");
  //m_pdfFilename = getOptions().getValueOrThrow<std::string>("pdf_filename");

  //std::cout << m_prcFilename << "\n" << m_pdfFilename << std::endl;

  std::string output_format = getOptions().getValueOrDefault<std::string>("output_format", "pdf");

  if(boost::iequals(output_format, "pdf"))
    m_outputFormat = OUTPUT_FORMAT_PDF;
  else if (boost::iequals(output_format, "prc"))
    m_outputFormat = OUTPUT_FORMAT_PRC;
  else
  {
    std::ostringstream oss;
    oss << "Unrecognized output format " << output_format;
    throw prc_driver_error("Unrecognized output format");
  }

  //m_prcFile(m_filename);

  return;
}



Options Writer::getDefaultOptions()
{
  Options options;

  Option prc_filename("prc_filename", "", "Filename to write PRC file to");
  Option pdf_filename("pdf_filename", "", "Filename to write PDF file to");
  Option output_format("output_format", "", "PRC or PDF");

  options.add(prc_filename);
  options.add(pdf_filename);
  options.add(output_format);

  return options;
}


void Writer::writeBegin(boost::uint64_t /*targetNumPointsToWrite*/)
{
  PRCoptions grpopt;
  grpopt.no_break = true;
  grpopt.do_break = false;
  grpopt.tess = true;

  m_prcFile.begingroup("points",&grpopt);

  std::cout << "begin\n";

  return;
}


void Writer::writeEnd(boost::uint64_t /*actualNumPointsWritten*/)
{
  std::cout << "end\n";

  m_prcFile.endgroup();
  m_prcFile.finish();

  std::cout << "format is " << m_outputFormat << std::endl;

  if(m_outputFormat == OUTPUT_FORMAT_PDF)
  {
    std::cout << "detected PDF\n";
 //   std::string prc_name (m_filename);
//    std::string pdf_name ("/Users/bchambers/dev/test.pdf");

    //std::cout << m_prcFilename << std::endl;
    //std::cout << m_pdfFilename << std::endl;

    const double width = 256.0f;
    const double height = 256.0f;
    const double depth = std::sqrt(width*height);

    std::cout << width << " " << height << " " << depth << std::endl;

    const HPDF_Rect rect = { 0, 0, width, height };
    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Annotation annot;
    HPDF_U3D u3d;
    HPDF_Dict view;

    pdf = HPDF_New( NULL, NULL );
    pdf->pdf_version = HPDF_VER_17;

    std::cout << "pdf version " << pdf->pdf_version << std::endl;

    page = HPDF_AddPage( pdf );
    HPDF_Page_SetWidth( page, width );
    HPDF_Page_SetHeight( page, height );

    std::string prcFilename = getOptions().getValueOrThrow<std::string>("prc_filename");
    std::cout << prcFilename;
    u3d = HPDF_LoadU3DFromFile( pdf, prcFilename.c_str() );
    std::cout << " loaded\n";

    view = HPDF_Create3DView( u3d->mmgr, "DefaultView" );

    std::cout << "default view\n";

    HPDF_3DView_SetCamera( view, 0, 0, 0, 0, 0, 1, /*depth*/ 10, 0 );
    //HPDF_3DView_SetOrthogonalProjection( view, 1 );
    HPDF_3DView_SetPerspectiveProjection( view, 30.0 );
    HPDF_3DView_SetBackgroundColor( view, 0, 0, 0 );
    HPDF_3DView_SetLighting( view, "Headlamp" );

    std::cout << "view created\n";

    HPDF_U3D_Add3DView( u3d, view );
    HPDF_U3D_SetDefault3DView( u3d, "DefaultView" );

    annot = HPDF_Page_Create3DAnnot( page, rect, u3d );

    HPDF_Dict action = (HPDF_Dict) HPDF_Dict_GetItem( annot, "3DA", HPDF_OCLASS_DICT );
    HPDF_Dict_AddBoolean( action, "TB", HPDF_TRUE );

    std::cout << "annotated\n";

    std::string pdfFilename = getOptions().getValueOrThrow<std::string>("pdf_filename");
    std::cout << pdfFilename;
    HPDF_SaveToFile( pdf, pdfFilename.c_str() );
    std::cout << " saved\n";

    HPDF_Free( pdf );
  }

  return;
}

boost::uint32_t Writer::writeBuffer(const PointBuffer& data)
{
  std::cout << "write\n";

  boost::uint32_t numPoints = 0;

  pdal::Schema const& schema = data.getSchema();

  pdal::Dimension const& dimX = schema.getDimension("X");
  pdal::Dimension const& dimY = schema.getDimension("Y");
  pdal::Dimension const& dimZ = schema.getDimension("Z");

  double **points;
  points = (double**) malloc(data.getNumPoints()*sizeof(double*));
  for (boost::uint32_t i = 0; i < data.getNumPoints(); ++i)
  {
    points[i] = (double*) malloc(3*sizeof(double));
  }

  double xd(0.0);
  double yd(0.0);
  double zd(0.0);

  for(boost::uint32_t i = 0; i < data.getNumPoints(); ++i)
  {
    boost::int32_t x = data.getField<boost::int32_t>(dimX, i);
    boost::int32_t y = data.getField<boost::int32_t>(dimY, i);
    boost::int32_t z = data.getField<boost::int32_t>(dimZ, i);

    xd = dimX.applyScaling<boost::int32_t>(x) - 1210000;
    yd = dimY.applyScaling<boost::int32_t>(y) - 320000;
    zd = dimZ.applyScaling<boost::int32_t>(z) - 4000;

    if(i % 10000) printf("%f %f %f\n", xd, yd, zd);

    points[i][0] = xd;
    points[i][1] = yd;
    points[i][2] = zd;

    numPoints++;
  }

  m_prcFile.addPoints(numPoints, const_cast<const double**>(points), RGBAColour(1.0,1.0,0.0,1.0),1.0);

  for (boost::uint32_t i = 0; i < data.getNumPoints(); ++i)
  {
    free(points[i]);
  }
  free(points);

  return numPoints;
}


}
}
} // namespaces
