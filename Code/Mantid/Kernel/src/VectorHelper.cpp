#include <stdexcept>
#include <cmath>

#include "MantidKernel/VectorHelper.h"
#include <algorithm>

namespace Mantid{
	namespace Kernel{

void rebin(const std::vector<double>& xold, const std::vector<double>& yold, const std::vector<double>& eold,
      const std::vector<double>& xnew, std::vector<double>& ynew, std::vector<double>& enew, bool distribution)
{
	  int size_xold=xold.size();
	  int size_xnew=xnew.size();
	  if (size_xold!=static_cast<int>(yold.size()+1) || size_xold!=static_cast<int>(eold.size()+1))
		  throw std::runtime_error("rebin: x,y, and error vectors should be of same size");

	  ynew.clear();
	  enew.clear();
	  ynew.resize(size_xnew); // Make sure y and e vectors are of correct sizes
	  enew.resize(size_xnew);

      int iold = 0,inew = 0;
      double xo_low, xo_high, xn_low, xn_high, delta(0.0), width;

      while((inew < size_xnew) && (iold < size_xold))
      {
        xo_low = xold[iold];
        xo_high = xold[iold+1];
        xn_low = xnew[inew];
        xn_high = xnew[inew+1];
        if ( xn_high <= xo_low )
          inew++;		/* old and new bins do not overlap */
        else if ( xo_high <= xn_low )
          iold++;		/* old and new bins do not overlap */
        else
        {
          //        delta is the overlap of the bins on the x axis
          //delta = std::min(xo_high, xn_high) - std::max(xo_low, xn_low);
          delta = xo_high<xn_high?xo_high:xn_high;
          delta -= xo_low>xn_low?xo_low:xn_low;
          width = xo_high - xo_low;
          if ( (delta <= 0.0) || (width <= 0.0) )
          {
            throw std::runtime_error("rebin: no bin overlap detected");
          }
          /*
          *        yoldp contains counts/unit time, ynew contains counts
          *	       enew contains counts**2
          *        ynew has been filled with zeros on creation
          */
          if(distribution)
          {
            // yold/eold data is distribution
            ynew[inew] += yold[iold]*delta;
            // this error is calculated in the same way as opengenie
            enew[inew] += eold[iold]*eold[iold]*delta*width;
          }
          else
          {
            // yold/eold data is not distribution
            // do implicit division of yold by width in summing.... avoiding the need for temporary yold array
            // this method is ~7% faster and uses less memory
            ynew[inew] += yold[iold]*delta/width; //yold=yold/width
            // eold=eold/width, so divide by width**2 compared with distribution calculation
            enew[inew] += eold[iold]*eold[iold]*delta/width;
          }
          if ( xn_high > xo_high )
          {
            iold++;
          }
          else
          {
            inew++;
          }
        }
      }

      if(distribution)
      {
        /*
        * convert back to counts/unit time
        */
        for(int i=0; i<size_xnew; ++i)
        {
          {
            width = xnew[i+1]-xnew[i];
            if (width != 0.0)
            {
              ynew[i] /= width;
              enew[i] = sqrt(enew[i]) / width;
            }
            else
            {
              throw std::invalid_argument("rebin: Invalid output X array, contains consecutive X values");
            }
          }
        }
      }
      else
      {
        //non distribution , just square root final error value
        for(int i=0; i<size_xnew;++i)
          enew[i]=sqrt(enew[i]);
      }
      return; //without problems
    }
/// New method to rebin Histogram data, should be faster than previous one
///
 void rebinHistogram(const std::vector<double>& xold, const std::vector<double>& yold, const std::vector<double>& eold,
    const std::vector<double>& xnew, std::vector<double>& ynew, std::vector<double>& enew,bool addition)
{
  int size_xold=xold.size();
  int size_xnew=xnew.size();
  if (size_xold!=static_cast<int>(yold.size()+1) || size_xold!=static_cast<int>(eold.size()+1))
  	  throw std::runtime_error("rebin: x,y, and error vectors should be of same size");


  if (!addition)
  {
  	ynew.clear();
  	enew.clear();
  	ynew.resize(size_xnew-1); // Make sure y and e vectors are of correct sizes
  	enew.resize(size_xnew-1);
  }
  // First find the first Xpoint that is bigger than xnew[0]
  std::vector<double>::const_iterator it=std::find_if(xold.begin(),xold.end(),std::bind2nd(std::greater<double>(),xnew[0]));
  if (it==xold.end())
  	throw std::runtime_error("No overlap, max of Xold < min of Xnew");
  int iold=std::distance(xold.begin(),it); // Where we are now
  int inew=0;
  double frac, fracE;
  double width;
  if (iold==0)
  {
  	frac=0;
  	fracE=0;
  }
  else
  {
  	width=(xold[iold]-xold[iold-1]);
  	frac=yold[iold-1]/width;
  	fracE=std::pow(eold[iold-1],2)/width;
  }
  for(;;) //Start the loop here
  {
  	if ((iold+1)==size_xold || (inew+1)==size_xnew) // Both will be incremented here
  		break;
  	while(xnew[++inew]<xold[iold])
  	{
  		if (iold!=0) // If iold==0, then no counts to add
  		{
  			width=(xnew[inew]-xnew[inew-1]);
  			ynew[inew-1]+=frac*width; //Increment this
  			enew[inew-1]+=fracE*width;
  		}
  		if ((inew+1)==size_xnew)
  			break;
  	}
  	if (iold!=0) //Put the remaining counts here
  	{
  		width=(xold[iold]-xnew[inew-1]);
  		ynew[inew-1]+=frac*width;
  		enew[inew-1]+=fracE*width;
  	}

  	while(xold[++iold]<=xnew[inew])
  	{
  		ynew[inew-1]+=yold[iold-1];
  		enew[inew-1]+=std::pow(eold[iold-1],2);
  		if ((iold+1)==size_xold)
  			break;
  	}
  	if (iold!=0)
		{
  		width=(xold[iold+1]-xold[iold]);
			frac=yold[iold]/width; //Dealing with the new xold
			fracE=std::pow(eold[iold],2)/width;
		}
  	width=(xnew[inew]-xold[iold-1]);
  	ynew[inew-1]+=frac*width;
  	enew[inew-1]+=fracE*width;
  }

  if (!addition) //If this used to add at the same time then not necessary.
  {
		//Now take the root-square of the errors
		typedef double (*pf)(double);
		pf uf=std::sqrt;
		std::transform(enew.begin(),enew.end(),enew.begin(),uf);
	}
		return;
}



} // End namespace Kernel
} // End namespace Mantid
