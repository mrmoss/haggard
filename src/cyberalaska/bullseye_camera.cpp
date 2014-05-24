/**
Grabs images from an OpenCV-supported camera, and passes them to the bullseye detector.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#include "../cyberalaska/bullseye_camera.h"
#include "../cyberalaska/time.h"
#include "opencv2/opencv.hpp" /* needed for cv::Mat */
//#include "opencv/highgui.h"


#include "../cyberalaska/porthread.h"


class bullseye_camera_backend : public bullseye_camera {
	cv::VideoCapture *cap;

	/// This lock protects results against multithreaded changes during reads.
	///   You MUST grab this lock before reading or writing results.
	porlock results_lock;
	std::vector<bullcolor> results;

	// Dr. Lawlor expects that storing these images out here will improve performance (measure?)
	cv::Mat frame; ///< Last captured video frame
	cv::Mat gray; ///< Grayscale version of last frame

public:
	bool good; ///< Keep running while this is true.
	bullseye_camera_backend(cv::VideoCapture *cap_)
		:cap(cap_)

	{
		good=true;
		while (!capture()) {
			std::cerr<<"Still waiting for first frame from camera...\n";
		}
	}

	/** Extract the latest copy of the detected bullseyes.
		All bullseyes above our internal vote threshold are reported.
		Bullseyes are in descending vote count, so [0] is the most likely.

	  There is no point in calling this function unless
	  	our timestamped::has_newer reports a new frame.

	  MULTITHREADED: this function will be called by other threads.
	*/
	virtual std::vector<bullcolor> extract()
	{
		porlock_scoped guard(&results_lock);
		return results;
	}

	/** Capture and analyze one frame.  Called repeatedly by our camera thread.
	  Returns false if the capture device is indisposed.
	*/
	bool capture() {
		if (!cap->read(frame)) return false;
		double capture_time=cyberalaska::time(); // timestamp early

	// Convert image to grayscale
		cv::cvtColor(frame,gray,CV_BGR2GRAY);

	// Find bullseyes
		bullseyeList bulls=findBullseyes(gray);

	// Find color around each bullseye
		std::vector<bullcolor> results;
		for (unsigned int i=0;i<bulls.eyes.size();i++) {
			bullseyeInfo &bi=bulls.eyes[i];
			bullcolor bc(bi,frame);
			results.push_back(bc);
		}

	// MULTITHREAD: publish results for other threads
		{
			porlock_scoped guard(&results_lock);
			this->results=results;
		}

	// Mark our new results
		update_timestamp(capture_time);

		return true;
	}

	/** Close the camera. */
	virtual ~bullseye_camera_backend()
	{
		delete cap;
		cap=NULL;
	}
};

bullseye_camera::~bullseye_camera() {
}

void bullseye_camera_thread(void *camptr) {
	bullseye_camera_backend *bc=((bullseye_camera_backend *)camptr);
	while (bc->good) {
		bc->capture();
	}
}

bullseye_camera *make_bullseye_camera(int cvCameraNumber)
{
	cv::VideoCapture *cap=new cv::VideoCapture(cvCameraNumber);

	int wid=640, ht=480;
	cap->set(CV_CAP_PROP_FRAME_WIDTH,wid); // <- this seems to do nothing...
	cap->set(CV_CAP_PROP_FRAME_HEIGHT,ht);

	if (!cap->isOpened()) {
		std::cerr<<"Error opening camera "<<cvCameraNumber<<"\n";
		return NULL;
	}

	bullseye_camera_backend *bc=new bullseye_camera_backend(cap);
	porthread_detach(porthread_create(bullseye_camera_thread,bc));
	return bc;
}

