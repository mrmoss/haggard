/**
OpenCV bulls-eye detector library: uses a gradient-voting scheme.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#include "bullseye.h"
#include <algorithm> /* for std::sort */


typedef unsigned short accum_t;
inline accum_t fetchAccum(const cv::Mat &accum,int x,int y) {
	return ((const accum_t *)accum.data)[y*accum.cols+x];
}

/* Increment pixels along this line. */
static void accumulateLine(cv::Mat &accum,
	cv::Point S,cv::Point E)
{
	accum_t *accumDat=(accum_t *)accum.data;
	cv::Rect r(2,2,accum.cols-4,accum.rows-4);
	if (!cv::clipLine(r,S,E)) return;
	
	float rounding=0.49999; // compensates for rounding down
	
	int dx=E.x-S.x;
	int dy=E.y-S.y;
	if (abs(dx)>abs(dy)) 
	{ /* X-major line */
		if (E.x<S.x) std::swap(S,E);
		float m=(E.y-S.y)/float(E.x-S.x);
		float b=S.y-m*S.x+rounding;
		for (int x=S.x;x<=E.x;x++)
		{
			float y=m*x+b;
			//if (y<0 || y>=accum.rows) abort();
			accumDat[((int)y)*accum.cols+x]+=1.0;
		}
	}
	else  /* dx<=dy */
	{ /* Y-major line */
		if (E.y==S.y) return; // start and end are equal
		
		if (E.y<S.y) std::swap(S,E);
		float m=(E.x-S.x)/float(E.y-S.y);
		float b=S.x-m*S.y+rounding;
		for (int y=S.y;y<=E.y;y++)
		{
			float x=m*y+b;
			//if (x<0 || x>=accum.cols) abort();
			accumDat[y*accum.cols+(int)x]+=1.0;
		}
	}
}


/**
  Find a list of bullseyes in this grayscale (single channel) source image.
*/
bullseyeList findBullseyes(const cv::Mat &grayImage, // source grayscale image, use cv::cvtColor(colorImg,grayImg,CV_BGR2GRAY);
	double minimumGradientMagnitude, // gradient steepness required to draw line (low values slower but can detect weaker)
	double minimumVotesPerEye, // minimum vote count to be an eye (low values more sensitive but false positives)
	double gradientVotePixels, // number of pixels to extend gradient (low values faster but only support small eyes)
	int minimumEyeDistance // minimum distance between distinct bullseyes
	)
{
	bullseyeList bulls;
	
	// Accumulator for gradient power.  
	//  CV_8U doesn't have enough bits for typical vote counts.
	cv::Mat accum=cv::Mat::zeros(grayImage.rows,grayImage.cols,CV_16U);
	
/* Convert steep gradients to lines */
	// Gradient estimate (with filtering)
	int ksize=3; // pixel count for gradient filter+blur
	// 22fps, 98% of CPU with float gradients:
	const int grad_typecode=CV_32F;
	typedef float grad_t;
	
	/* 
	// 23fps, 98% of CPU with unsigned short gradients: 
	const int grad_typecode=CV_16S;
	typedef signed short grad_t;
	*/
	
	cv::Mat gradX,gradY;
	cv::Sobel(grayImage,gradX,grad_typecode, 1,0, ksize);
	cv::Sobel(grayImage,gradY,grad_typecode, 0,1, ksize);
	
	grad_t *gradXF=(grad_t *)gradX.data;
	grad_t *gradYF=(grad_t *)gradY.data;
	float minDiffSq=minimumGradientMagnitude*minimumGradientMagnitude;
	for (int y=0;y<grayImage.rows;y++)
	for (int x=0;x<grayImage.cols;x++)
	{
		int i=y*grayImage.cols+x;
		float dx=gradXF[i], dy=gradYF[i];
		float magSq=dx*dx+dy*dy; // squared magnitude of gradient vector
		if (magSq>minDiffSq)  
		{
			float mag=sqrt(magSq); // now a length
			float s=gradientVotePixels/mag; // scale factor from gradient to line length
			accumulateLine(accum,
				cv::Point(x+dx*s,y+dy*s),
				cv::Point(x-dx*s,y-dy*s));
			
			/* // cv::line doesn't support alpha blending (WHY NOT?!)
			cv::line(annot,
				cv::Point(x+dx*s,y+dy*s),
				cv::Point(x-dx*s,y-dy*s),
				cv::Scalar(255,0,0,10),0.1,CV_AA);
			*/
		}
	}
	
// Circle areas where there's a high gradient *and* a local maximum.
	int de=minimumEyeDistance; // must be maximum among neighborhood of this many pixels (==min distance between eyes)
	for (int y=de;y<accum.rows-de;y++)
	for (int x=de;x<accum.cols-de;x++)
	{
		int cur=fetchAccum(accum,x,y);
		if (cur>=minimumVotesPerEye) 
		{ /* it's big--but is there a bigger one nearby? */
			bool biggest=true;
			for (int dy=-de;dy<de && biggest;dy++)
			for (int dx=-de;dx<de;dx++)
			{
				float her=fetchAccum(accum,x+dx,y+dy);
				/* To break ties, I'm putting a slight tilt along both axes. */
				her+=dx*(1.0/1057)+dy*(1.0/8197);
				
				if (cur<her) {
					biggest=false;
					break;
				}
			}
			
			if (biggest) 
			{ /* This is a bullseye! */
				double cx=x, cy=y; // sub-pixel center

				float C=fetchAccum(accum,x,y); // 5 point stencil here
				float L=fetchAccum(accum,x-1,y), R=fetchAccum(accum,x+1,y);
				float B=fetchAccum(accum,x,y-1), T=fetchAccum(accum,x,y+1);

				cx+=-(R-L)/(2.0*(R+L-2.0*C)); // parabolic peak-polishing, -b/2a
				cy+=-(T-B)/(2.0*(T+B-2.0*C)); 
				
				bullseyeInfo eye;
				eye.x=cx; eye.y=cy;
				eye.votes=cur;
				bulls.eyes.push_back(eye);
			}
		}
	}
	
	// Sort by ascending size
	std::sort(bulls.eyes.begin(),bulls.eyes.end());
	return bulls;
}


