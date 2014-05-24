#include "bullcolor.h"

bullcolor::bullcolor(const bullseyeInfo &bi,cv::Mat &rgbImage)
		:bullseyeInfo(bi)
{
	pixel=cyberalaska::vec3(bi.x,bi.y,0.0);
	radius=bi.votes/10.0; // very rough estimate of total radius (FIXME: this is crap)

	// Extract a color box around the center point
	double cr=radius*0.5; // color detection box radius (pixels)
	cv::Rect colorbox(bi.x-cr,bi.y-cr,2*cr,2*cr);
	colorbox &= cv::Rect(0,0,rgbImage.cols,rgbImage.rows); // clip to image dimensions
	cv::Mat colorimg=rgbImage(colorbox);

	// Extract the three channels
	enum {nchan=3}; // number of color channels to convert (B,G,R)
	cv::Mat channels[nchan];
	for (int c=0;c<nchan;c++) {
		channels[c]=cv::Mat(colorimg.rows,colorimg.cols,CV_8U);
	}
	int fromTo[2*nchan]={0,0,1,1,2,2};
	cv::mixChannels(&colorimg,1,channels,nchan,fromTo,nchan);

	// Calculate centers of mass
	cv::Moments moments[nchan]; // per-channel moments
	cv::Point_<float> com[nchan]; // center of mass
	for (int c=0;c<nchan;c++) {
		moments[c]=cv::moments(channels[c]);
		com[c]=cv::Point_<float>(moments[c].m00/moments[c].m10,moments[c].m00/moments[c].m01);
		color[c]=moments[c].m00/(colorbox.width*colorbox.height);
	}
	color[3]=0.0; // alpha?

	// Estimate orientation from channel center of mass differences
	cv::Point_<float> dirv=com[1]-com[2]; // points from green toward red
	dir=cyberalaska::vec3(dirv.x,dirv.y,0.0);
	float dm=length(dir); // magnitude
	if (dm>0.001) {
		dir*=1.0/dm; // make it have unit length
		angle=atan2_deg(dir);
		confidence=dm*50.0; // <- sets arbitrary scale factor on confidence
	}
	else
	{ // direction vector is too short to be correct
		dir=cyberalaska::vec3(0.0); // make it have zero length
		angle=-999.0;
		confidence=0.0;
	}
}
