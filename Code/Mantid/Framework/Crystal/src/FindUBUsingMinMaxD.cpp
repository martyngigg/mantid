/*WIKI* 


Given a set of peaks, and given a range of possible a,b,c values, this algorithm will attempt to find a UB matrix, that fits the data.  The algorithm searches over a range of possible directions and unit cell lengths for directions and lengths that match plane normals and plane spacings in reciprocal space.  It then chooses three of these vectors with the shortest lengths that are linearly independent and that are separated by at least a minimum angle.  The minimum angle is calculated from the specified min and max d values.  A UB matrix is formed using these three vectors and the resulting UB matrix is optimized using a least squares method. If the specified peaks are accurate and belong to a single crystal, this method should produce some UB matrix that indexes the peaks. However, other software will usually be needed to adjust this UB to match a desired conventional cell.


*WIKI*/
#include "MantidCrystal/FindUBUsingMinMaxD.h"
#include "MantidKernel/System.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidDataObjects/Peak.h"
#include "MantidGeometry/Crystal/IndexingUtils.h"
#include "MantidGeometry/Crystal/OrientedLattice.h"
#include <cstdio>

namespace Mantid
{
namespace Crystal
{
  Kernel::Logger& FindUBUsingMinMaxD::g_log = 
                        Kernel::Logger::get("FindUBUsingMinMaxD");

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(FindUBUsingMinMaxD)

  using namespace Mantid::Kernel;
  using namespace Mantid::API;
  using namespace Mantid::DataObjects;
  using namespace Mantid::Geometry;

  //--------------------------------------------------------------------------
  /** Constructor
   */
  FindUBUsingMinMaxD::FindUBUsingMinMaxD()
  {
  }
    
  //--------------------------------------------------------------------------
  /** Destructor
   */
  FindUBUsingMinMaxD::~FindUBUsingMinMaxD()
  {
  }
  
  //--------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void FindUBUsingMinMaxD::initDocs()
  {
    std::string summary("Calculate the UB matrix from a peaks workspace, ");
    summary += "given estimates of the min and max real space unit cell ";
    summary += "edge lengths.";
    this->setWikiSummary( summary );

    std::string message("Calculate the UB matrix from a peaks workspace, ");
    message += "given min(a,b,c) and max(a,b,c).";
    this->setOptionalMessage( message );
  }

  //--------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void FindUBUsingMinMaxD::init()
  {
    this->declareProperty(new WorkspaceProperty<PeaksWorkspace>(
          "PeaksWorkspace","",Direction::InOut), "Input Peaks Workspace");

    BoundedValidator<double> *mustBePositive = new BoundedValidator<double>();
    mustBePositive->setLower(0.0);

    BoundedValidator<int> *atLeast3Int = new BoundedValidator<int>();
    atLeast3Int->setLower(3);

    // use negative values, force user to input all parameters
    this->declareProperty(new PropertyWithValue<double>( "MinD",-1.0,
          mustBePositive->clone(),Direction::Input),
          "Lower Bound on Lattice Parameters a, b, c");

    this->declareProperty(new PropertyWithValue<double>( "MaxD",-1.0,
          mustBePositive->clone(),Direction::Input),
          "Upper Bound on Lattice Parameters a, b, c");

    this->declareProperty(new PropertyWithValue<int>( "NumInitial", 20,
          atLeast3Int,Direction::Input), 
          "Number of Peaks to Use on First Pass(20)");

    this->declareProperty(new PropertyWithValue<double>( "tolerance",0.15,
          mustBePositive->clone(),Direction::Input),"Indexing Tolerance (0.15)");
  }

  //--------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void FindUBUsingMinMaxD::exec()
  {
    double min_d       = this->getProperty("MinD");
    double max_d       = this->getProperty("MaxD");
    int    num_initial = this->getProperty("NumInitial");
    double tolerance   = this->getProperty("tolerance");
                                          
    int    base_index         = -1;   // these "could" be properties if need be
    double degrees_per_step   =  1;

    PeaksWorkspace_sptr ws;
    ws = boost::dynamic_pointer_cast<PeaksWorkspace>(
         AnalysisDataService::Instance().retrieve(this->getProperty("PeaksWorkspace")) );

    if (!ws) throw std::runtime_error("Could not read the peaks workspace");

    std::vector<Peak> &peaks = ws->getPeaks();
    size_t n_peaks = ws->getNumberPeaks();

    std::vector<V3D>  q_vectors;
    q_vectors.reserve( n_peaks );
    for ( size_t i = 0; i < n_peaks; i++ )
      q_vectors.push_back( peaks[i].getQSampleFrame() );

    Matrix<double> UB(3,3,false);
    double error = IndexingUtils::Find_UB( UB, q_vectors, 
                                           min_d, max_d,
                                           tolerance, 
                                           base_index, 
                                           num_initial, 
                                           degrees_per_step );

    std::cout << "Error = " << error << std::endl;
    std::cout << "UB = " << UB << std::endl;

    if ( ! IndexingUtils::CheckUB( UB ) ) // UB not found correctly
    {
      g_log.notice( std::string(
         "Found Invalid UB...peaks used might not be linearly independent") );
      g_log.notice( std::string(
         "UB NOT SAVED.") );
    }
    else                                  // tell user how many would be indexed
    {                                     // and save the UB in the sample 
      char logInfo[200];
      int num_indexed = IndexingUtils::NumberIndexed(UB, q_vectors, tolerance);
      sprintf( logInfo, 
               std::string("New UB will index %1d Peaks out of %1d with tolerance %5.3f").c_str(),    
               num_indexed, n_peaks, tolerance);
      g_log.notice( std::string(logInfo) );

      OrientedLattice o_lattice;
      o_lattice.setUB( UB );
      double calc_a = o_lattice.a();
      double calc_b = o_lattice.b();
      double calc_c = o_lattice.c();
      double calc_alpha = o_lattice.alpha();
      double calc_beta  = o_lattice.beta();
      double calc_gamma = o_lattice.gamma();
                                       // Show the modified lattice parameters
      sprintf( logInfo, 
               std::string("Lattice Parameters: %8.3f %8.3f %8.3f %8.3f %8.3f %8.3f").c_str(),
               calc_a, calc_b, calc_c, calc_alpha, calc_beta, calc_gamma);
      g_log.notice( std::string(logInfo) );

      ws->mutableSample().setOrientedLattice( new OrientedLattice(o_lattice) );
    }
  }


} // namespace Mantid
} // namespace Crystal

